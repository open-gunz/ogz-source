#pragma once

#include "vulkan_functions.h"
#include "VulkanTextureLoader.h"

struct VulkanMaterial
{
	vkTools::VulkanTexture Texture;
	VkDescriptorSet DescriptorSet;
};

void DestroyVkMaterial(VulkanMaterial& Material);