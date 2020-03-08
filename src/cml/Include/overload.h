#pragma once

#include <type_traits>

template <typename F, typename ...G>
struct overload_t
	: overload_t<F>::type
	, overload_t<G...>::type
{
	using type = overload_t;
	using overload_t<F>::type::operator();
	using overload_t<G...>::type::operator();

	template <typename F_, typename ...G_>
	constexpr explicit overload_t(F_&& f, G_&& ...g)
		: overload_t<F>::type(static_cast<F_&&>(f))
		, overload_t<G...>::type(static_cast<G_&&>(g)...)
	{ }
};

template <typename F>
struct overload_t<F> { using type = F; };

template <typename R, typename ...Args>
struct overload_t<R(*)(Args...)> {
	using type = overload_t;
	R(*fptr_)(Args...);

	explicit constexpr overload_t(R(*fp)(Args...))
		: fptr_(fp)
	{ }

	constexpr R operator()(Args ...args) const
	{
		return fptr_(static_cast<Args&&>(args)...);
	}
};

struct make_overload_t {
	template <typename ...F,
		typename Overload = typename overload_t<
		typename std::decay<F>::type...
		>::type
	>
		constexpr Overload operator()(F&& ...f) const {
		return Overload(static_cast<F&&>(f)...);
	}
};

constexpr make_overload_t overload{};