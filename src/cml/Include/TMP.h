#pragma once

#include <type_traits>

namespace tmp
{

template <bool is_correct, size_t wanted_index, size_t cur_index, typename T, typename... rest>
struct get_template_argument_impl;

template <size_t wanted_index, size_t cur_index, typename T, typename... rest>
struct get_template_argument_impl<true, wanted_index, cur_index, T, rest...>
{
	using type = T;
};

template <size_t wanted_index, size_t cur_index, typename T, typename... rest>
struct get_template_argument_impl<false, wanted_index, cur_index, T, rest...>
{
	using type = typename get_template_argument_impl<wanted_index == cur_index + 1,
		wanted_index, cur_index, T, rest...>::type;
};

template <typename T, size_t index>
struct get_template_argument;

template <template <typename...> class template_, typename... arguments, size_t index>
struct get_template_argument<template_<arguments...>, index>
{
	using type = typename get_template_argument_impl<index == 0, index, 0, arguments...>::type;
};

template <typename T, size_t index>
using get_template_argument_t = typename get_template_argument<T, index>::type;

template <bool b, bool... rest>
struct any_of
{
	static constexpr bool value = b || any_of<rest...>::value;
};

template <bool b>
struct any_of<b>
{
	static constexpr bool value = b;
};

template <bool b, bool... rest>
struct all_of
{
	static constexpr bool value = b && all_of<rest...>::value;
};

template <bool b>
struct all_of<b>
{
	static constexpr bool value = b;
};

}