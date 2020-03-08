#include "stdafx.h"
#include "RBspObjectDrawVulkan.h"
#include "RBspObject.h"
#include "RS2.h"

// Shader objects
#include "BasicRenderVS.h"
#include "BasicRenderFS.h"
#include "AlphaTestingFS.h"

RBspObjectDrawVulkan::~RBspObjectDrawVulkan()
{
	if (!Initialized)
		return;

	auto& Device = GetRS2Vulkan().Device;
	vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	VertexBuffer.destroy();
	IndexBuffer.destroy();
	uniformBufferVS.destroy();

	for (auto& Pipeline : Pipelines.Array)
		vkDestroyPipeline(Device, Pipeline, nullptr);
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	for (auto& Layout : DescriptorSetLayouts.Layouts)
		vkDestroyDescriptorSetLayout(Device, Layout, nullptr);
}

void RBspObjectDrawVulkan::CreateBuffers()
{
	auto ret = GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&VertexBuffer,
		bsp.OcVertices.size() * sizeof(bsp.OcVertices[0]),
		bsp.OcVertices.data());

	if (ret != VK_SUCCESS)
	{
		MLog("RBspObjectDrawVulkan::Init - Failed to create vertex buffer\n");
	}

	ret = GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&IndexBuffer,
		bsp.OcIndices.size() * sizeof(bsp.OcIndices[0]),
		bsp.OcIndices.data());

	if (ret != VK_SUCCESS)
	{
		MLog("RBspObjectDrawVulkan::Init - Failed to create index buffer\n");
	}
}

void RBspObjectDrawVulkan::RenderNode(VkCommandBuffer CmdBuffer, RSBspNode& Node, int Material)
{
	// Leaf node
	if (Node.nPolygon)
	{
		auto TriangleCount = Node.pDrawInfo[Material].nTriangleCount;
		if (TriangleCount)
		{
#ifdef _DEBUG
			g_nCall++;
			g_nPoly += TriangleCount;
#endif

			vkCmdDrawIndexed(CmdBuffer,
				TriangleCount * 3, 1,
				Node.pDrawInfo[Material].nIndicesOffset, 0, 0);
		}
		return;
	}

	// Branch node
	auto DrawNode = [&](auto Branch) {
		if (Node.*Branch)
			RenderNode(CmdBuffer, *(Node.*Branch), Material);
	};

	DrawNode(&RSBspNode::m_pPositive);
	DrawNode(&RSBspNode::m_pNegative);
}

template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags, bool SetTextures>
void RBspObjectDrawVulkan::Render(VkCommandBuffer CmdBuffer)
{
	for (size_t i = 0; i < bsp.Materials.size(); ++i)
	{
		auto& Material = bsp.Materials[i];

		if ((Material.dwFlags & Flags) != (ShouldHaveFlags ? Flags : 0))
			continue;

		if (Material.VkMaterial.DescriptorSet == VK_NULL_HANDLE)
			continue;

		std::array<VkDescriptorSet, 2> descriptorSets;
		// Set 0: Scene descriptor set containing global matrices
		descriptorSets[0] = DescriptorSet;
		// Set 1: Per-Material descriptor set containing bound images
		descriptorSets[1] = Material.VkMaterial.DescriptorSet;

		vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);

		if (SetAlphaTestFlags)
		{
			vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				Material.dwFlags & RM_FLAG_USEALPHATEST ? Pipelines.AlphaTesting : Pipelines.Transparent);
		}
		
		RenderNode(CmdBuffer, bsp.OcRoot[0], i);
	}
}

void RBspObjectDrawVulkan::CreateCommandBuffers()
{
	auto Width = RGetScreenWidth();
	auto Height = RGetScreenHeight();

	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0, 0, 0.5, 1 } };
	clearValues[1].depthStencil = { 1, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = GetRS2Vulkan().RenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = Width;
	renderPassBeginInfo.renderArea.extent.height = Height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (size_t i = 0; i < GetRS2Vulkan().DrawCmdBuffers.size(); ++i)
	{
		auto CmdBuffer = GetRS2Vulkan().DrawCmdBuffers[i];

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = GetRS2Vulkan().FrameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(CmdBuffer, &cmdBufInfo));

		vkCmdBeginRenderPass(CmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vkTools::initializers::viewport((float)Width, (float)Height, 0.0f, 1.0f);
		vkCmdSetViewport(CmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(Width, Height, 0, 0);
		vkCmdSetScissor(CmdBuffer, 0, 1, &scissor);

		// Bind the vertex and index buffers
		{
			constexpr u32 VERTEX_BUFFER_BIND_ID{};
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(CmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &VertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(CmdBuffer, IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
		}

		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Solid);
		Render<RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY, false, false>(CmdBuffer);

		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Transparent);
		Render<RM_FLAG_USEOPACITY, true, true>(CmdBuffer);

		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Additive);
		Render<RM_FLAG_ADDITIVE, true, false>(CmdBuffer);

		vkCmdEndRenderPass(CmdBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(CmdBuffer));
	}
}

void RBspObjectDrawVulkan::SetupVertexDescriptions()
{
	constexpr u32 VERTEX_BUFFER_BIND_ID{};
	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] =
		vkTools::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			sizeof(BSPVERTEX),
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	// Describes memory layout and shader positions
	vertices.attributeDescriptions.resize(3);
	// Location 0: Position
	vertices.attributeDescriptions[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(BSPVERTEX, x));
	// Location 1: Diffuse texture coordinates
	vertices.attributeDescriptions[1] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			offsetof(BSPVERTEX, tu1));
	// Location 2: Lightmap texture coordinates
	vertices.attributeDescriptions[2] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32_SFLOAT,
			offsetof(BSPVERTEX, tu2));

	vertices.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void RBspObjectDrawVulkan::CreatePipelines()
{
	// Base solid pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vkTools::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vkTools::initializers::pipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vkTools::initializers::pipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	VkDynamicState dynamicStateEnables[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(
			dynamicStateEnables,
			static_cast<uint32_t>(std::size(dynamicStateEnables)),
			0);

	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	shaderStages[0] = GetRS2Vulkan().loadShader(BasicRenderVS, std::size(BasicRenderVS), VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = GetRS2Vulkan().loadShader(BasicRenderFS, std::size(BasicRenderFS), VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo PipelineCreateInfos[5];
	for (auto& PipelineCreateInfo : PipelineCreateInfos)
	{
		PipelineCreateInfo = vkTools::initializers::pipelineCreateInfo(
			PipelineLayout,
			GetRS2Vulkan().RenderPass,
			0);

		PipelineCreateInfo.pVertexInputState = &vertices.inputState;
		PipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		PipelineCreateInfo.pRasterizationState = &rasterizationState;
		PipelineCreateInfo.pColorBlendState = &colorBlendState;
		PipelineCreateInfo.pMultisampleState = &multisampleState;
		PipelineCreateInfo.pViewportState = &viewportState;
		PipelineCreateInfo.pDepthStencilState = &depthStencilState;
		PipelineCreateInfo.pDynamicState = &dynamicState;
		PipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		PipelineCreateInfo.pStages = shaderStages.data();
	}

	// The first pipeline, Solid, is the base pipeline, the rest are derivatives.
	PipelineCreateInfos[0].flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	for (size_t i = 1; i < std::size(PipelineCreateInfos); ++i)
	{
		PipelineCreateInfos[i].basePipelineIndex = 0;
		PipelineCreateInfos[i].flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	}

	int Index = 1;
	// Transparent pipeline
	auto TransparentRasterizationState = rasterizationState;
	TransparentRasterizationState.cullMode = VK_CULL_MODE_NONE;
	PipelineCreateInfos[Index].pRasterizationState = &TransparentRasterizationState;
	auto TransparentColorBlendState = colorBlendState;
	auto TransparentBlendAttachmentState = blendAttachmentState;
	TransparentBlendAttachmentState.blendEnable = VK_TRUE;
	TransparentBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	TransparentBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	TransparentBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	TransparentColorBlendState.pAttachments = &TransparentBlendAttachmentState;
	PipelineCreateInfos[Index].pColorBlendState = &TransparentColorBlendState;
	++Index;

	// Alpha testing pipeline
	//PipelineCreateInfos[Index] = PipelineCreateInfos[Index - 1];
	auto AlphaTestingShaderStages = shaderStages;
	PipelineCreateInfos[Index].pStages = AlphaTestingShaderStages.data();
	AlphaTestingShaderStages[1] = GetRS2Vulkan().loadShader(AlphaTestingFS, std::size(AlphaTestingFS), VK_SHADER_STAGE_FRAGMENT_BIT);
	++Index;

	// Additive pipeline
	auto AdditiveRasterizationState = rasterizationState;
	AdditiveRasterizationState.cullMode = VK_CULL_MODE_NONE;
	PipelineCreateInfos[Index].pRasterizationState = &AdditiveRasterizationState;
	auto AdditiveColorBlendState = colorBlendState;
	auto AdditiveBlendAttachmentState = blendAttachmentState;
	AdditiveBlendAttachmentState.blendEnable = VK_TRUE;
	AdditiveBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	AdditiveBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	AdditiveBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	AdditiveColorBlendState.pAttachments = &AdditiveBlendAttachmentState;
	PipelineCreateInfos[Index].pColorBlendState = &AdditiveColorBlendState;
	++Index;

	// Wireframe pipeline
	auto WireframeColorBlendState = colorBlendState;
	auto WireframeBlendAttachmentState = blendAttachmentState;
	WireframeBlendAttachmentState.blendEnable = false;
	WireframeColorBlendState.pAttachments = &WireframeBlendAttachmentState;
	PipelineCreateInfos[Index].pColorBlendState = &WireframeColorBlendState;
	auto WireframeRasterizationState = rasterizationState;
	WireframeRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	WireframeRasterizationState.lineWidth = 1.0f;
	PipelineCreateInfos[Index].pRasterizationState = &WireframeRasterizationState;
	++Index;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(
		GetRS2Vulkan().Device, PipelineCache,
		std::size(PipelineCreateInfos), PipelineCreateInfos,
		nullptr, &Pipelines.Solid));
}

void RBspObjectDrawVulkan::PrepareUniformBuffers()
{
	VK_CHECK_RESULT(GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBufferVS,
		sizeof(uboVS),
		&uboVS));

	UpdateUniformBuffers();
}

void RBspObjectDrawVulkan::UpdateUniformBuffers()
{
	glm::vec3 cameraPos{ EXPAND_VECTOR(RCameraPosition) };

	uboVS.projection = glm::perspective(glm::radians(60.0f),
		(float)RGetScreenWidth() / (float)RGetScreenHeight(), 5.f, 10000.0f);

	uboVS.model = glm::lookAt(
		cameraPos,
		cameraPos + glm::vec3{ EXPAND_VECTOR(RCameraDirection) },
		glm::vec3{ EXPAND_VECTOR(RCameraUp) }),

	uboVS.viewPos = glm::vec4(cameraPos, 0.0f);

	VK_CHECK_RESULT(uniformBufferVS.map());
	memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
	uniformBufferVS.unmap();
}

void RBspObjectDrawVulkan::SetupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		// Only need one uniform buffer
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		// Need a texture descriptor for each material in the map
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, bsp.Materials.size())
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			bsp.Materials.size() + 1);

	VK_CHECK_RESULT(vkCreateDescriptorPool(GetRS2Vulkan().Device, &descriptorPoolInfo, nullptr, &DescriptorPool));
}

void RBspObjectDrawVulkan::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{ 1 };

	// Binding 0: Vertex shader uniform buffer
	setLayoutBindings[0] = vkTools::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT,
		0);

	auto descriptorLayout = vkTools::initializers::descriptorSetLayoutCreateInfo(
		setLayoutBindings.data(),
		static_cast<uint32_t>(setLayoutBindings.size()));

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(GetRS2Vulkan().Device, &descriptorLayout, nullptr,
		&DescriptorSetLayouts.Scene));

	setLayoutBindings[0] = vkTools::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0);

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(GetRS2Vulkan().Device, &descriptorLayout, nullptr,
		&DescriptorSetLayouts.Material));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vkTools::initializers::pipelineLayoutCreateInfo(
			DescriptorSetLayouts.Layouts,
			2);

	VK_CHECK_RESULT(vkCreatePipelineLayout(GetRS2Vulkan().Device, &pPipelineLayoutCreateInfo, nullptr, &PipelineLayout));
}

void RBspObjectDrawVulkan::SetupDescriptorSet()
{
	auto allocInfo = vkTools::initializers::descriptorSetAllocateInfo(
			DescriptorPool,
			&DescriptorSetLayouts.Scene,
			1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(GetRS2Vulkan().Device, &allocInfo, &DescriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0: Vertex shader uniform buffer
		vkTools::initializers::writeDescriptorSet(
			DescriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformBufferVS.descriptor)
	};

	vkUpdateDescriptorSets(GetRS2Vulkan().Device,
		static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(),
		0, nullptr);

	// Material index 0 always stores a white diffuse color and no texture,
	// so start from 1 and don't attempt to load its texture.
	for (size_t i = 1; i < bsp.Materials.size(); ++i)
	{
		auto& Material = bsp.Materials[i];

		if (Material.VkMaterial.Texture.view == VK_NULL_HANDLE)
			continue;

		allocInfo = vkTools::initializers::descriptorSetAllocateInfo(
				DescriptorPool,
				&DescriptorSetLayouts.Material,
				1);

		VK_CHECK_RESULT(vkAllocateDescriptorSets(GetRS2Vulkan().Device, &allocInfo, &Material.VkMaterial.DescriptorSet));

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0: Pixel shader texture
			vkTools::initializers::writeDescriptorSet(
				Material.VkMaterial.DescriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				0,
				&Material.VkMaterial.Texture.descriptor)
		};

		vkUpdateDescriptorSets(GetRS2Vulkan().Device, 
			static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(),
			0, nullptr);
	}
}

void RBspObjectDrawVulkan::Init()
{
	CreateBuffers();
	SetupVertexDescriptions();
	PrepareUniformBuffers();
	SetupDescriptorSetLayout();
	CreatePipelines();
	SetupDescriptorPool();
	SetupDescriptorSet();
	CreateCommandBuffers();

	Initialized = true;
}

void RBspObjectDrawVulkan::Draw()
{
}