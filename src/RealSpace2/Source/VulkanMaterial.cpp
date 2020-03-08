#include "stdafx.h"
#include "VulkanMaterial.h"
#include "RS2.h"
#include "VulkanTextureLoader.h"

void DestroyVkMaterial(VulkanMaterial& Material)
{
	if (Material.Texture.view != VK_NULL_HANDLE)
		GetRS2Vulkan().TextureLoader->destroyTexture(Material.Texture);
}