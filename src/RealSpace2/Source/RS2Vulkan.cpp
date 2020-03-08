#include "stdafx.h"
#include "RS2Vulkan.h"
#include "RBspObject.h"

#define VK_USE_VALIDATION_LAYERS 0

static void LogError(const char* msg)
{
	MessageBox(0, msg, "iGunZ", 0);
	MLog(msg);
}

static bool CreateConsole(const char* Title)
{
	if (!AllocConsole()) return false;
	AttachConsole(GetCurrentProcessId());
	FILE *stream{};
	if (freopen_s(&stream, "CONOUT$", "w+", stdout) != 0) return false;
	SetConsoleTitle(TEXT(Title));
	return true;
}

static bool LoadGlobalVulkanFunctions(HMODULE hVulkan)
{
	auto LogFailedFunctionLoad = [&](const char* name) {
		char msg[256];
		sprintf_safe(msg, "Failed to load function %s from vulkan-1.dll.", name);
		LogError(msg);
	};

#define LOAD_VULKAN_FUNCTION(name) name = reinterpret_cast<PFN_##name>(GetProcAddress(hVulkan, #name)); \
	if (!name) { \
		LogFailedFunctionLoad(#name); \
		return false; }
#define VK_EXPORTED_FUNCTION(name) LOAD_VULKAN_FUNCTION(name)
#define VK_GLOBAL_LEVEL_FUNCTION(name) LOAD_VULKAN_FUNCTION(name)
#include "vulkan_function_list.h"

	return true;
}

RS2Vulkan::~RS2Vulkan()
{
	// TODO: Destroy stuff here
	FreeLibrary(hVulkan);
}

bool RS2Vulkan::Create(HWND hwnd, HINSTANCE inst)
{
#if VK_USE_VALIDATION_LAYERS
	if (!CreateConsole("Vulkan validation layer output"))
		MLog("Failed to create console\n");
#endif

	hVulkan = LoadLibrary("vulkan-1.dll");
	if (!hVulkan)
	{
		LogError("Failed to load vulkan-1.dll. Make sure your driver supports Vulkan.");
		return false;
	}

	if (!LoadGlobalVulkanFunctions(hVulkan))
		return false;

	InitVulkan(VK_USE_VALIDATION_LAYERS);
	SwapChain.initSurface(inst, hwnd);
	Init();

	return true;
}

static RBspObject* bsp;

void RS2Vulkan::Init()
{
	bsp = new RBspObject;
	BaseInit();
	bsp->Open("Maps/Mansion/Mansion.rs");
}

void RS2Vulkan::Draw()
{
	bsp->UpdateUBO();

	prepareFrame();

	// Command buffer to be submitted to the queue
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &DrawCmdBuffers[currentBuffer];

	// Submit to queue
	VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));

	submitFrame();
}