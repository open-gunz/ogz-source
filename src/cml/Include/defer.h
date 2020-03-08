#pragma once

#include <type_traits>
#include <tuple>
#include "MUtil.h"
#include "apply.h"

template <typename T, typename... ArgsType>
struct defer : private std::tuple<std::decay_t<ArgsType>...>
{
	using Base = std::tuple<std::decay_t<ArgsType>...>;

	defer(T&& fn, ArgsType&&... Args) : fn(std::forward<T>(fn)), Base{ std::forward<ArgsType>(Args)... } {}
	defer(const defer& src) = delete;
	defer& operator=(const defer& src) = delete;
	~defer() { /*std::*/::apply(fn, *static_cast<Base*>(this)); }

private:
	std::decay_t<T> fn;
};

template <typename T, typename... ArgsType>
defer<T, ArgsType...> make_defer(T&& fn, ArgsType&&... Args) {
	// Copy-list-initialization avoids a move here.
	return{ std::forward<T>(fn), std::forward<ArgsType>(Args)... };
}

#define DEFER(...) const auto&& TOKENIZE(defer_instance, __COUNTER__){make_defer(__VA_ARGS__)}