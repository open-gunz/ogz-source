#pragma once

#include <type_traits>

#define HAS_XXX(member) \
template <typename T, typename = int> \
struct has_##member : std::false_type { }; \
template <typename T> \
struct has_##member <T, decltype(void(&T::member), 0)> : std::true_type { };
