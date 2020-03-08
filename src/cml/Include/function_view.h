#pragma once

#include <type_traits>
#include <utility>

template <typename>
struct function_view;

template <typename Ret, typename... Args>
struct function_view<Ret(Args...)>
{
	function_view() = default;
	function_view(nullptr_t) noexcept {}

	template <typename T, typename = std::enable_if_t<
		!std::is_same<std::decay_t<T>, function_view>::value>>
	function_view(T&& x) noexcept
	{
		fn = [](void* data, Args... args) -> Ret {
			auto& callable = *reinterpret_cast<std::add_pointer_t<T>>(data);
			return callable(std::forward<Args>(args)...);
		};
		data = (void*)std::addressof(x);
	}

	function_view(Ret(*fn)(void*, Args...), void* data) : fn(fn), data(data) {}

	Ret operator()(Args... args) const
	{
		return fn(data, std::forward<Args>(args)...);
	}

	explicit operator bool() const { return fn; }

private:
	Ret(*fn)(void*, Args...){};
	void* data{};
};