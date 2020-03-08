#pragma once

#include "VariantPolymorphism.h"
#include "RBspObjectDrawD3D9.h"
#include "RBspObjectDrawVulkan.h"
#include "RNameSpace.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

class RBspObjectDraw
{
public:
	RBspObjectDraw(GraphicsAPI API, RBspObject& bsp) {
		if (API == GraphicsAPI::D3D9)
			var = RBspObjectDrawD3D9{ bsp };
		else if (API == GraphicsAPI::Vulkan)
			var = RBspObjectDrawVulkan{ bsp };
		else
			assert(false);
	}

	template <typename T>
	auto& Get() { return var.get_ref<T>(); }

	POLYVAR_METHOD(Init)
	POLYVAR_METHOD(Draw)
	POLYVAR_METHOD(SetLighting)
	POLYVAR_METHOD(OnInvalidate);
	POLYVAR_METHOD(OnRestore);

private:
	variant<monostate, RBspObjectDrawD3D9, RBspObjectDrawVulkan> var;
};

_NAMESPACE_REALSPACE2_END