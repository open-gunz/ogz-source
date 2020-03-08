#pragma once

#include <string>
#include "Renderer.h"
#include "variant.h"
#include "RS2D3D9.H"
#include "RS2Vulkan.h"
#include "VariantPolymorphism.h"

_NAMESPACE_REALSPACE2_BEGIN

class RS2
{
private:
	std::string ApplicationName;

	variant<RS2D3D9, RS2Vulkan> var;

public:
	RS2(D3D9Tag) : var{ RS2D3D9{} } {};
	template <typename... Ts>
	RS2(VulkanTag, Ts&&... Args) : var{ RS2Vulkan{std::forward<Ts>(Args)...} } {}
	RS2(const RS2&) = delete;

	void OnInvalidate();
	void OnRestore();

	bool UsingD3D9() const { return var.is_type<RS2D3D9>(); }
	bool UsingVulkan() const { return !UsingD3D9(); }

	POLYVAR_METHOD(Create)
	POLYVAR_METHOD(Draw)

	template <typename T>
	auto& Get() { return var.get_ref<T>(); }

	static RS2& Get();

	Renderer Render;
};

inline auto& GetRS2() { return RS2::Get(); }
inline auto& GetRS2Vulkan() { return GetRS2().Get<RS2Vulkan>(); }
inline Renderer& GetRenderer() { return GetRS2().Render; }

_NAMESPACE_REALSPACE2_END
