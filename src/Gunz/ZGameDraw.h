#pragma once

#include "variant.h"
#include "ZGameDrawD3D9.h"
#include "ZGameDrawVulkan.h"
#include <type_traits>
#include "VariantPolymorphism.h"

class ZGameDraw
{
public:
	ZGameDraw(class ZGame& Game, D3D9Tag) : var{ ZGameDrawD3D9{ Game } } {}
	ZGameDraw(class ZGame& Game, VulkanTag) : var{ ZGameDrawVulkan{ Game } } {}

	POLYVAR_METHOD(Draw)
	POLYVAR_METHOD(OnInvalidate)
	POLYVAR_METHOD(OnRestore)

private:
	variant<ZGameDrawD3D9, ZGameDrawVulkan> var;
};