#pragma once

#include "RNameSpace.h"
#include "vulkan_functions.h"
#include <vector>
#include "glm/glm.hpp"
#include "vulkanbuffer.hpp"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

class RBspObjectDrawVulkan
{
public:
	RBspObjectDrawVulkan(RBspObject& bsp) : bsp(bsp) {}
	~RBspObjectDrawVulkan();
	RBspObjectDrawVulkan(RBspObjectDrawVulkan&&) = default;
	
	void Init();
	void Draw();
	void UpdateUniformBuffers();

	void SetLighting(bool) {}

private:
	// Creates and fills the vertex and index buffers
	void CreateBuffers();
	void SetupVertexDescriptions();
	void PrepareUniformBuffers();
	void SetupDescriptorSetLayout();
	void CreatePipelines();
	void SetupDescriptorPool();
	void SetupDescriptorSet();
	template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags, bool SetTextures = true>
	void Render(VkCommandBuffer CmdBuffer);
	void RenderNode(VkCommandBuffer CmdBuffer, struct RSBspNode& Node, int Material);
	void CreateCommandBuffers();

	// Awkward
	bool Initialized{};

	RBspObject& bsp;

	vk::Buffer VertexBuffer;
	vk::Buffer IndexBuffer;

	union {
		struct {
			VkPipeline Solid;
			VkPipeline Transparent;
			VkPipeline AlphaTesting;
			VkPipeline Additive;
			VkPipeline Wireframe;
		};
		VkPipeline Array[5];
	} Pipelines;
	VkPipelineLayout PipelineLayout{};
	VkPipelineCache PipelineCache{};

	// The descriptor set stores the resources bound to the binding points in a shader
	// It connects the binding points of the different shaders with the buffers and images used for those bindings
	VkDescriptorSet DescriptorSet;

	// The descriptor set layout describes the shader binding layout (without actually referencing descriptor)
	// Like the pipeline layout it's pretty much a blueprint and can be used with different descriptor sets as long as their layout matches
	union
	{
		struct
		{
			VkDescriptorSetLayout Scene;
			VkDescriptorSetLayout Material;
		};
		VkDescriptorSetLayout Layouts[2];
	} DescriptorSetLayouts;

	VkDescriptorPool DescriptorPool{};

	vk::Buffer uniformBufferVS;
	
	struct {
		glm::mat4 projection;
		glm::mat4 model;
		glm::vec4 viewPos;
		float lodBias = 0.0f;
	} uboVS;
	
	struct {
		VkPipelineVertexInputStateCreateInfo inputState{};
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;
};

_NAMESPACE_REALSPACE2_END