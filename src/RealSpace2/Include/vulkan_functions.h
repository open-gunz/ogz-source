#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

#define MAKE_FUNCTION_POINTER(name) extern PFN_##name name;
#define VK_EXPORTED_FUNCTION(name) MAKE_FUNCTION_POINTER(name)
#define VK_GLOBAL_LEVEL_FUNCTION(name) MAKE_FUNCTION_POINTER(name)
#define VK_INSTANCE_LEVEL_FUNCTION(name) MAKE_FUNCTION_POINTER(name)
#define VK_DEVICE_LEVEL_FUNCTION(name) MAKE_FUNCTION_POINTER(name)
#define USE_SWAPCHAIN_EXTENSIONS
#include "vulkan_function_list.h"
