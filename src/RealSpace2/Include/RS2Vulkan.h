#pragma once

#include <vector>
#include "vulkanswapchain.h"
#include "vulkandebug.h"
#include "vulkantools.h"
#include "vulkandevice.h"
#include "VulkanTextureLoader.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

extern MZFileSystem* g_pFileSystem;

class RS2Vulkan
{
public:
	~RS2Vulkan();

	bool Create(HWND hwnd, HINSTANCE inst);

	VkResult CreateInstance(bool enableValidation)
	{
		this->enableValidation = enableValidation;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "RealSpace2";
		appInfo.pEngineName = "RealSpace2";
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = NULL;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		if (enabledExtensions.size() > 0)
		{
			if (enableValidation)
			{
				enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}
			instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
			instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
		}
		if (enableValidation)
		{
			instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
			instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
		}
		return vkCreateInstance(&instanceCreateInfo, nullptr, &Instance);
	}

	void CreateCommandPool()
	{
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = SwapChain.queueNodeIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(Device, &cmdPoolInfo, nullptr, &CmdPool));
	}

	void CreateSetupCommandBuffer()
	{
		if (SetupCmdBuffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(Device, CmdPool, 1, &SetupCmdBuffer);
			SetupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
		}

		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vkTools::initializers::commandBufferAllocateInfo(
				CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

		VK_CHECK_RESULT(vkAllocateCommandBuffers(Device, &cmdBufAllocateInfo, &SetupCmdBuffer));

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CHECK_RESULT(vkBeginCommandBuffer(SetupCmdBuffer, &cmdBufInfo));
	}

	void CreateSwapChain()
	{
		SwapChain.create(&Width, &Height, false);
	}

	void CreateCommandBuffers()
	{
		// Create one command buffer for each swap chain image and reuse for rendering
		DrawCmdBuffers.resize(SwapChain.imageCount);

		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vkTools::initializers::commandBufferAllocateInfo(
				CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				static_cast<uint32_t>(DrawCmdBuffers.size()));

		VK_CHECK_RESULT(vkAllocateCommandBuffers(Device, &cmdBufAllocateInfo, DrawCmdBuffers.data()));
	}

	uint32_t Width{ 1920 }, Height{ 1080 };

	void SetupDepthStencil()
	{
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = DepthFormat;
		image.extent = { Width, Height, 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.pNext = NULL;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = DepthFormat;
		depthStencilView.flags = 0;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;

		VkMemoryRequirements memReqs;

		VK_CHECK_RESULT(vkCreateImage(Device, &image, nullptr, &DepthStencil.image));
		vkGetImageMemoryRequirements(Device, DepthStencil.image, &memReqs);
		mem_alloc.allocationSize = memReqs.size;
		mem_alloc.memoryTypeIndex = VulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(Device, &mem_alloc, nullptr, &DepthStencil.mem));
		VK_CHECK_RESULT(vkBindImageMemory(Device, DepthStencil.image, DepthStencil.mem, 0));

		depthStencilView.image = DepthStencil.image;
		VK_CHECK_RESULT(vkCreateImageView(Device, &depthStencilView, nullptr, &DepthStencil.view));
	}

	struct
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} DepthStencil;

	VkPhysicalDeviceFeatures EnabledFeatures{};

	void loadInstanceLevelFunctions()
	{
#define VK_INSTANCE_LEVEL_FUNCTION(name) name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(Instance, #name)); \
if (!name) MLog("Error loading " #name "!\n");
#include "vulkan_function_list.h"
	}

	void loadDeviceLevelFunctions()
	{
#define VK_DEVICE_LEVEL_FUNCTION(name) name = reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(Device, #name)); \
if (!name) MLog("Error loading " #name "!\n");
#include "vulkan_function_list.h"
	}

	void InitVulkan(bool enableValidation)
	{
		VkResult err;

		// Vulkan instance
		err = CreateInstance(enableValidation);
		if (err)
		{
			vkTools::exitFatal("Could not create Vulkan instance:\n" + vkTools::errorString(err), "Fatal error");
		}

		loadInstanceLevelFunctions();

		// If requested, we enable the default validation layers for debugging
		if (enableValidation)
		{
			// The report flags determine what type of messages for the layers will be displayed
			// For validating (debugging) an appplication the error and warning bits should suffice
			VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT; // | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			// Additional flags include performance info, loader and layer debug messages, etc.
			vkDebug::setupDebugging(Instance, debugReportFlags, VK_NULL_HANDLE);
		}

		// Physical device
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Instance, &gpuCount, nullptr));
		assert(gpuCount > 0);
		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		err = vkEnumeratePhysicalDevices(Instance, &gpuCount, physicalDevices.data());
		if (err)
		{
			vkTools::exitFatal("Could not enumerate phyiscal devices : \n" + vkTools::errorString(err), "Fatal error");
		}

		// Note:
		// This example will always use the first physical device reported,
		// change the vector index if you have multiple Vulkan devices installed
		// and want to use another one
		PhysicalDevice = physicalDevices[0];

		// Vulkan device creation
		// This is handled by a separate class that gets a logical device representation
		// and encapsulates functions related to a device
		VulkanDevice = new vk::VulkanDevice(PhysicalDevice);
		EnabledFeatures.fillModeNonSolid = true;
		VK_CHECK_RESULT(VulkanDevice->createLogicalDevice(EnabledFeatures));
		Device = VulkanDevice->logicalDevice;
		loadDeviceLevelFunctions();
		VulkanDevice->commandPool = VulkanDevice->createCommandPool(VulkanDevice->queueFamilyIndices.graphics);

		// todo: remove
		// Store properties (including limits) and features of the phyiscal device
		// So examples can check against them and see if a feature is actually supported
		vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &DeviceFeatures);
		// Gather physical device memory properties
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &DeviceMemoryProperties);

		// Get a graphics queue from the device
		vkGetDeviceQueue(Device, VulkanDevice->queueFamilyIndices.graphics, 0, &Queue);

		// Find a suitable depth format
		VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(PhysicalDevice, &DepthFormat);
		assert(validDepthFormat);

		SwapChain.connect(Instance, PhysicalDevice, Device);

		// Create synchronization objects
		VkSemaphoreCreateInfo semaphoreCreateInfo = vkTools::initializers::semaphoreCreateInfo();
		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queu
		VK_CHECK_RESULT(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been sumbitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands for the text overlay have been sumbitted and executed
		// Will be inserted after the render complete semaphore if the text overlay is enabled
		VK_CHECK_RESULT(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &semaphores.textOverlayComplete));

		// Set up submit info structure
		// Semaphores will stay the same during application lifetime
		// Command buffer submission info is set by each example
		SubmitInfo = vkTools::initializers::submitInfo();
		SubmitInfo.pWaitDstStageMask = &SubmitPipelineStages;
		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = &semaphores.presentComplete;
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = &semaphores.renderComplete;
	}

	void SetupRenderPass()
	{
		// This example will use a single render pass with one subpass

		// Descriptors for the attachments used by this renderpass
		std::array<VkAttachmentDescription, 2> attachments = {};

		// Color attachment
		attachments[0].format = Colorformat;
		// We don't use multi sampling in this example
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		// Clear this attachment at the start of the render pass
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// Keep it's contents after the render pass is finished (for displaying it)
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// We don't use stencil, so don't care for load
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		// Same for store
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// Layout at render pass start. Initial doesn't matter, so we use undefined
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Layout to which the attachment is transitioned when the render pass is finished
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// As we want to present the color buffer to the swapchain, we transition to PRESENT_KHR

		// Depth attachment
		attachments[1].format = DepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		// Clear depth at start of first subpass
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// We don't need depth after render pass has finished (DONT_CARE may result in better performance)
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// No stencil
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		// No stencil
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// Layout at render pass start. Initial doesn't matter, so we use undefined
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Transition to depth/stencil attachment
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Setup attachment references
		VkAttachmentReference colorReference = {};
		// Attachment 0 is color
		colorReference.attachment = 0;
		// Attachment layout used as color during the subpass
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		// Attachment 1 is color
		depthReference.attachment = 1;
		// Attachment used as depth/stemcil used during the subpass
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Setup a single subpass reference
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		// Subpass uses one color attachment
		subpassDescription.colorAttachmentCount = 1;
		// Reference to the color attachment in slot 0
		subpassDescription.pColorAttachments = &colorReference;
		// Reference to the depth attachment in slot 1
		subpassDescription.pDepthStencilAttachment = &depthReference;
		// Input attachments can be used to sample from contents of a previous subpass
		subpassDescription.inputAttachmentCount = 0;
		// (Input attachments not used by this example)
		subpassDescription.pInputAttachments = nullptr;									
		// Preserved attachments can be used to loop (and preserve) attachments through subpasses
		subpassDescription.preserveAttachmentCount = 0;
		// (Preserve attachments not used by this example)
		subpassDescription.pPreserveAttachments = nullptr;
		// Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling
		subpassDescription.pResolveAttachments = nullptr;

		// Setup subpass dependencies
		// These will add the implicit ttachment layout transitionss specified by the attachment descriptions
		// The actual usage layout is preserved through the layout specified in the attachment reference		
		// Each subpass dependency will introduce a memory and execution dependency between the source and dest subpass described by
		// srcStageMask, dstStageMask, srcAccessMask, dstAccessMask (and dependencyFlags is set)
		// Note: VK_SUBPASS_EXTERNAL is a special constant that refers to all commands executed outside of the actual renderpass)
		std::array<VkSubpassDependency, 2> dependencies;

		// First dependency at the start of the renderpass
		// Does the transition from final to initial layout 

		// Producer of the dependency
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		// Consumer is our single subpass that will wait for the execution depdendency
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Second dependency at the end the renderpass
		// Does the transition from the initial to the final layout

		// Producer of the dependency is our single subpass
		dependencies[1].srcSubpass = 0;
		// Consumer are all commands outside of the renderpass
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		// Number of attachments used by this render pass
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		// Descriptions of the attachments used by the render pass
		renderPassInfo.pAttachments = attachments.data();
		// We only use one subpass in this example
		renderPassInfo.subpassCount = 1;
		// Description of that subpass
		renderPassInfo.pSubpasses = &subpassDescription;
		// Number of subpass dependencies
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		// Subpass dependencies used by the render pass
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass));
	}
	
	void CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(Device, &pipelineCacheCreateInfo, nullptr, &PipelineCache));
	}

	void SetupFrameBuffer()
	{
		// Create a frame buffer for every image in the swapchain
		FrameBuffers.resize(SwapChain.imageCount);
		for (size_t i = 0; i < FrameBuffers.size(); i++)
		{
			std::array<VkImageView, 2> attachments;
			// Color attachment is the view of the swapchain image
			attachments[0] = SwapChain.buffers[i].view;
			// Depth/Stencil attachment is the same for all frame buffers
			attachments[1] = DepthStencil.view;

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// All frame buffers use the same renderpass setup
			frameBufferCreateInfo.renderPass = RenderPass;
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = Width;
			frameBufferCreateInfo.height = Height;
			frameBufferCreateInfo.layers = 1;
			// Create the framebuffer
			VK_CHECK_RESULT(vkCreateFramebuffer(Device, &frameBufferCreateInfo, nullptr, &FrameBuffers[i]));
		}
	}
	
	void FlushSetupCommandBuffer()
	{
		if (SetupCmdBuffer == VK_NULL_HANDLE)
			return;

		VK_CHECK_RESULT(vkEndCommandBuffer(SetupCmdBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &SetupCmdBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK_RESULT(vkQueueWaitIdle(Queue));

		vkFreeCommandBuffers(Device, CmdPool, 1, &SetupCmdBuffer);
		SetupCmdBuffer = VK_NULL_HANDLE;
	}

	// Create the Vulkan synchronization primitives used in this example
	void prepareSynchronizationPrimitives()
	{
		// Semaphores (Used for correct command ordering)
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		// Semaphore used to ensures that image presentation is complete before starting to submit again
		VK_CHECK_RESULT(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &PresentCompleteSemaphore));

		// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr, &RenderCompleteSemaphore));

		// Fences (Used to check draw command buffer completion)
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Create in signaled state so we don't wait on first render of each command buffer
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		WaitFences.resize(DrawCmdBuffers.size());
		for (auto& fence : WaitFences)
		{
			VK_CHECK_RESULT(vkCreateFence(Device, &fenceCreateInfo, nullptr, &fence));
		}
	}

	uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
	{
		// Iterate over all memory types available for the device used in this example
		for (uint32_t i = 0; i < DeviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((DeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}
			typeBits >>= 1;
		}

		throw "Could not find a suitable memory type!";
	}
	
	VkCommandBuffer getCommandBuffer(bool begin)
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = CmdPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(Device, &cmdBufAllocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
		}

		return cmdBuffer;
	}

	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage)
	{
		VkPipelineShaderStageCreateInfo shaderStage = {};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;
#if defined(__ANDROID__)
		shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
		shaderStage.module = vkTools::loadShader(fileName.c_str(), Device, stage);
#endif
		shaderStage.pName = "main"; // todo : make param
		assert(shaderStage.module != NULL);
		ShaderModules.push_back(shaderStage.module);
		return shaderStage;
	}

	VkPipelineShaderStageCreateInfo loadShader(const unsigned char* Data, size_t Size, VkShaderStageFlagBits stage)
	{
		VkPipelineShaderStageCreateInfo shaderStage = {};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;

		// Load the module
		{
			VkShaderModuleCreateInfo moduleCreateInfo;
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.pNext = NULL;
			moduleCreateInfo.codeSize = Size;
			moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(Data);
			moduleCreateInfo.flags = 0;

			VK_CHECK_RESULT(vkCreateShaderModule(Device, &moduleCreateInfo, NULL, &shaderStage.module));
		}

		shaderStage.pName = "main";
		assert(shaderStage.module != NULL);
		ShaderModules.push_back(shaderStage.module);
		return shaderStage;
	}

	vkTools::VulkanTexture LoadTexture(const char* Filename)
	{
		vkTools::VulkanTexture ret;
		MZFile File;
		char ddsFilename[256];
		sprintf_safe(ddsFilename, "%s.dds", Filename);
		if (!File.Open(ddsFilename, g_pFileSystem))
			if (!File.Open(Filename, g_pFileSystem))
				return{};
		auto buf = File.Release();
		TextureLoader->loadTexture(buf.get(), File.GetLength(), VK_FORMAT_BC1_RGB_UNORM_BLOCK, &ret);
		return ret;
	}

	void BaseInit()
	{
		if (VulkanDevice->enableDebugMarkers)
		{
			vkDebug::DebugMarker::setup(Device);
		}
		CreateCommandPool();
		CreateSetupCommandBuffer();
		CreateSwapChain();
		CreateCommandBuffers();
		SetupDepthStencil();
		SetupRenderPass();
		CreatePipelineCache();
		SetupFrameBuffer();
		FlushSetupCommandBuffer();
		// Recreate setup command buffer for derived class
		CreateSetupCommandBuffer();
		// Create a simple texture loader class
		TextureLoader = new vkTools::VulkanTextureLoader(VulkanDevice, Queue, CmdPool);
	}

	void Init();

	void prepareFrame()
	{
		VK_CHECK_RESULT(SwapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer));
	}

	void submitFrame()
	{
		VK_CHECK_RESULT(SwapChain.queuePresent(Queue, currentBuffer, semaphores.renderComplete));

		VK_CHECK_RESULT(vkQueueWaitIdle(Queue));
	}

	void Draw();

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vkTools::initializers::commandBufferAllocateInfo(
				CmdPool,
				level,
				1);

		VK_CHECK_RESULT(vkAllocateCommandBuffers(Device, &cmdBufAllocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
		}

		return cmdBuffer;
	}

	// Create an image memory barrier for changing the layout of
	// an image and put it into an active command buffer
	void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange)
	{
		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier = vkTools::initializers::imageMemoryBarrier();;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Only sets masks for layouts used in this example
		// For a more complete version that can be used with other layouts see vkTools::setImageLayout

		// Source layouts (old)
		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Only valid as initial layout, memory contents are not preserved
			// Can be accessed directly, no source dependency required
			imageMemoryBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Old layout is transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		}

		// Target layouts (new)
		switch (newImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Transfer source (copy, blit)
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Transfer destination (copy, blit)
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Shader read (sampler, input attachment)
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}

		// Put barrier on top of pipeline
		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStageFlags,
			destStageFlags,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}
	
	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK_RESULT(vkQueueWaitIdle(queue));

		if (free)
		{
			vkFreeCommandBuffers(Device, CmdPool, 1, &commandBuffer);
		}
	}

	bool enableValidation{};

	// Vulkan instance, stores all per-application states
	VkInstance Instance;
	// Physical device (GPU) that Vulkan will ise
	VkPhysicalDevice PhysicalDevice;
	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties DeviceProperties;
	// Stores phyiscal device features (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures DeviceFeatures;
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties DeviceMemoryProperties;
	/** @brief Logical device, application's view of the physical device (GPU) */
	// todo: getter? should always point to VulkanDevice->device
	VkDevice Device;
	/** @brief Encapsulated physical and logical vulkan device */
	vk::VulkanDevice *VulkanDevice;
	// Handle to the device graphics queue that command buffers are submitted to
	VkQueue Queue;
	// Color buffer format
	VkFormat Colorformat = VK_FORMAT_B8G8R8A8_UNORM;
	// Depth buffer format
	// Depth format is selected during Vulkan initialization
	VkFormat DepthFormat;
	// Command buffer pool
	VkCommandPool CmdPool;
	// Command buffer used for setup
	VkCommandBuffer SetupCmdBuffer = VK_NULL_HANDLE;
	/** @brief Pipeline stages used to wait at for graphics queue submissions */
	VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo SubmitInfo;
	// Command buffers used for rendering
	std::vector<VkCommandBuffer> DrawCmdBuffers;
	// Global render pass for frame buffer writes
	VkRenderPass RenderPass;
	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer> FrameBuffers;
	// Active frame buffer index
	uint32_t currentBuffer = 0;
	// Descriptor set pool
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> ShaderModules;
	// Pipeline cache object
	VkPipelineCache PipelineCache;
	// Wraps the swap chain to present images (framebuffers) to the windowing system
	VulkanSwapChain SwapChain;
	// Synchronization semaphores
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
		// Text overlay submission and execution
		VkSemaphore textOverlayComplete;
	} semaphores;
	// Simple texture loader
	vkTools::VulkanTextureLoader *TextureLoader = nullptr;

	// Synchronization primitives
	// Synchronization is an important concept of Vulkan that OpenGL mostly hid away. Getting this right is crucial to using Vulkan.

	// Semaphores
	// Used to coordinate operations within the graphics queue and ensure correct command ordering
	VkSemaphore PresentCompleteSemaphore;
	VkSemaphore RenderCompleteSemaphore;

	// Fences
	// Used to check the completion of queue operations (e.g. command buffer execution)
	std::vector<VkFence> WaitFences;

private:
	// HMODULE for the Vulkan dll
	HMODULE hVulkan{};
};

_NAMESPACE_REALSPACE2_END