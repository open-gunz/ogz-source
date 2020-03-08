#pragma once

#include <utility>
#include <type_traits>
#include <new>
#include <cassert>
#include <memory>
#include "TMP.h"

struct monostate {};

template <typename T>
struct unique_ptr_with_copy : std::unique_ptr<T>
{
	using std::unique_ptr<T>::unique_ptr;

	unique_ptr_with_copy(std::unique_ptr<T>&& src)
		: std::unique_ptr<T>(std::move(src)) {}

	unique_ptr_with_copy(const unique_ptr_with_copy& src)
	{
		*this = std::make_unique<T>(*src.get());
	}
};

template <typename T>
struct heap_wrapper : unique_ptr_with_copy<T>
{
	using unique_ptr_with_copy<T>::unique_ptr_with_copy;
};

namespace detail
{
using namespace tmp;

template <typename T>
struct identity
{
	using type = T;
};

#define MAX_PROPERTY(prop) \
template <typename T, typename... rest> \
struct max_##prop \
{ \
    using type = std::conditional_t<(prop(T) > prop(typename max_##prop<rest...>::type)), T, typename max_##prop<rest...>::type>; \
}; \
 \
template <typename T> \
struct max_##prop<T> \
{ \
	using type = T; \
}; \
 \
template <typename... Ts> \
using max_##prop##_t = typename max_##prop<Ts...>::type

MAX_PROPERTY(alignof);
MAX_PROPERTY(sizeof);

#undef MAX_PROPERTY

template <int index, typename T, typename U, typename... Ts>
struct get_index_impl
{
	static constexpr int value = std::conditional_t < std::is_same<T, U>::value,
		std::integral_constant<int, index>,
		get_index_impl < index + 1, T, Ts... >> ::value;
};

template <int index, typename T, typename U>
struct get_index_impl<index, T, U>
{
	static constexpr int value = index;
	static_assert(std::is_same<T, U>::value, "");
};

template <typename T, typename U, typename... Ts>
struct get_index
{
	static constexpr int value = get_index_impl<0, T, U, Ts...>::value;
};

template <typename T, typename... Ts>
struct is_part_of
{
	static constexpr auto value = tmp::any_of<
		std::is_same<
		std::remove_const_t<std::remove_reference_t<T>>, Ts>::value...>::value;
};

template <typename T>
struct substitute_heap_wrapper : identity<T> {};

template <typename T>
struct substitute_heap_wrapper<heap_wrapper<T>> : identity<T> {};

template <typename T>
using substitute_heap_wrapper_t = typename substitute_heap_wrapper<T>::type;

template <typename T, typename... Ts>
struct is_in_heap_wrapper
{
	static constexpr bool value = tmp::any_of<std::is_same<heap_wrapper<T>, Ts>::value...>::value;
};

template <typename T, typename...>
struct head { using type = T; };

template <typename... Ts>
using head_t = typename head<Ts...>::type;

}

template <size_t alignment, size_t size, typename... Ts>
struct variant_impl
{
	template <bool B = detail::is_part_of<monostate, Ts...>::value, typename = std::enable_if_t<B>>
	variant_impl() {
		this->construct(monostate{});
	}

	template <typename T, typename = std::enable_if_t<detail::is_part_of<T, detail::substitute_heap_wrapper_t<Ts>...>::value>>
	variant_impl(T&& obj) {
		this->construct(std::forward<T>(obj));
	}

	variant_impl(const variant_impl& src) {
		src.visit([&](auto&& val) { this->construct(val); });
	}
	variant_impl(variant_impl&& src) {
		src.visit([&](auto&& val) { this->construct(std::move(val)); });
	}

	variant_impl& operator =(const variant_impl& src) { return assign(src); }
	variant_impl& operator =(variant_impl&& src) { return assign(std::move(src)); }

	~variant_impl()
	{
		auto fn = [&](auto&& val) {
			using type = std::remove_reference_t<decltype(val)>;
			val.~type();
		};
		visit<decltype(fn)&, true>(fn);
	}

	template <typename fn_t, bool include_monostate = true>
	decltype(auto) visit(fn_t&& fn) { return visit_impl<fn_t, include_monostate, 0, Ts...>(std::forward<fn_t>(fn)); }
	template <typename fn_t, bool include_monostate = true>
	decltype(auto) visit(fn_t&& fn) const { return visit_impl<fn_t, include_monostate, 0, Ts...>(std::forward<fn_t>(fn)); }

	template <typename T, typename = std::enable_if_t<
		detail::any_of<(std::is_same<T, detail::substitute_heap_wrapper_t<Ts>>::value
			|| std::is_base_of<T, detail::substitute_heap_wrapper_t<Ts>>::value)...>::value>>
		auto& get_ref() { return get_ref_impl<T>(); }

	template <typename T, typename = std::enable_if_t<
		detail::any_of<(std::is_same<T, detail::substitute_heap_wrapper_t<Ts>>::value
			|| std::is_base_of<T, detail::substitute_heap_wrapper_t<Ts>>::value)...>::value>>
		auto& get_ref() const { return get_ref_impl<T>(); }

	auto get_type_index() const { return type_index; }

	template <typename T>
	static constexpr auto get_index_of_type() { return detail::get_index<T,
		detail::substitute_heap_wrapper_t<Ts>...>::value; }

	template <typename T>
	auto is_type() const { return get_type_index() == get_index_of_type<T>(); }

	template <typename T>
	T& emplace(T&& src)
	{
		assign(std::forward<T>(src));
		return get_ref<std::decay_t<T>>();
	}

private:
	template <typename T>
	std::enable_if_t<!detail::is_in_heap_wrapper<std::remove_const_t<std::remove_reference_t<T>>, Ts...>::value> construct(T&& src)
	{
		using real_t = std::remove_const_t<std::remove_reference_t<T>>;
		new (buffer) real_t(std::forward<T>(src));
		set_index<real_t>();
	}

	template <typename T>
	std::enable_if_t<detail::is_in_heap_wrapper<std::remove_const_t<std::remove_reference_t<T>>, Ts...>::value> construct(T&& src)
	{
		using real_t = std::remove_const_t<std::remove_reference_t<T>>;
		new (buffer) heap_wrapper<real_t>(std::make_unique<real_t>(std::forward<T>(src)));
		set_index<real_t>();
	}

	template <typename T>
	std::enable_if_t<!detail::is_in_heap_wrapper<T, Ts...>::value, T&> get_ref_impl()
	{
		return reinterpret_cast<T&>(buffer);
	}

	template <typename T>
	std::enable_if_t<detail::is_in_heap_wrapper<T, Ts...>::value, T&> get_ref_impl()
	{
		return *reinterpret_cast<heap_wrapper<T>&>(buffer).get();
	}

	template <typename T>
	std::enable_if_t<!detail::is_in_heap_wrapper<T, Ts...>::value, const T&> get_ref_impl() const
	{
		return reinterpret_cast<const T&>(buffer);
	}

	template <typename T>
	std::enable_if_t<detail::is_in_heap_wrapper<T, Ts...>::value, const T&> get_ref_impl() const
	{
		return *reinterpret_cast<const heap_wrapper<T>&>(buffer).get();
	}

	template <typename T>
	auto&& assign(T&& src)
	{
		this->~variant_impl();
		new (this) variant_impl(std::forward<T>(src));
		return *this;
	}

	template <typename T>
	void set_index() { type_index = get_index_of_type<T>(); }

	template <typename fn_t, typename T>
	struct dummy
	{
		template <typename buffer_t>
		static decltype(auto) call_visitor(fn_t&& fn, buffer_t* buffer)
		{
			return fn(reinterpret_cast<T&>(*buffer));
		}
	};
	template <typename fn_t, typename T>
	struct dummy<fn_t, heap_wrapper<T>>
	{
		template <typename buffer_t>
		static decltype(auto) call_visitor(fn_t&& fn, buffer_t* buffer)
		{
			return fn(*reinterpret_cast<heap_wrapper<T>&>(*buffer).get());
		}
	};
	template <typename fn_t, typename T>
	struct dummy<fn_t, const heap_wrapper<T>>
	{
		template <typename buffer_t>
		static decltype(auto) call_visitor(fn_t&& fn, buffer_t* buffer)
		{
			return fn(*reinterpret_cast<const heap_wrapper<T>&>(*buffer).get());
		}
	};

	template <typename fn_t, bool include_monostate, int cur_type_index>
	decltype(auto) visit_impl(fn_t&& fn)
	{
		assert(false);
		return dummy<fn_t, detail::head_t<Ts...>>::call_visitor(std::forward<fn_t>(fn), buffer);
	}
	template <typename fn_t, bool include_monostate, int cur_type_index, typename cur_type, typename... rest>
	decltype(auto) visit_impl(fn_t&& fn, std::enable_if_t<!std::is_same<cur_type, monostate>::value || include_monostate>* = nullptr)
	{
		if (cur_type_index == type_index)
			return dummy<fn_t, cur_type>::call_visitor(std::forward<fn_t>(fn), buffer);
		else
			return visit_impl<fn_t, include_monostate, cur_type_index + 1, rest...>(std::forward<fn_t>(fn));
	}
	template <typename fn_t, bool include_monostate, int cur_type_index, typename cur_type, typename... rest>
	decltype(auto) visit_impl(fn_t&& fn, std::enable_if_t<std::is_same<cur_type, monostate>::value && !include_monostate>* = nullptr)
	{
		return visit_impl<fn_t, include_monostate, cur_type_index + 1, rest...>(std::forward<fn_t>(fn));
	}

	template <typename fn_t, bool include_monostate, int cur_type_index>
	decltype(auto) visit_impl(fn_t&& fn) const
	{
		assert(false);
		return dummy<fn_t, const detail::head_t<Ts...>>::call_visitor(std::forward<fn_t>(fn), buffer);
	}
	template <typename fn_t, bool include_monostate, int cur_type_index, typename cur_type, typename... rest>
	decltype(auto) visit_impl(fn_t&& fn, std::enable_if_t<!std::is_same<cur_type, monostate>::value || include_monostate>* = nullptr) const
	{
		if (cur_type_index == type_index)
			return dummy<fn_t, const cur_type>::call_visitor(std::forward<fn_t>(fn), buffer);
		else
			return visit_impl<fn_t, include_monostate, cur_type_index + 1, rest...>(std::forward<fn_t>(fn));
	}
	template <typename fn_t, bool include_monostate, int cur_type_index, typename cur_type, typename... rest>
	decltype(auto) visit_impl(fn_t&& fn, std::enable_if_t<std::is_same<cur_type, monostate>::value && !include_monostate>* = nullptr) const
	{
		return visit_impl<fn_t, include_monostate, cur_type_index + 1, rest...>(std::forward<fn_t>(fn));
	}

	int type_index;
	alignas(alignment) char buffer[size];
};

template <typename... Ts>
using variant = variant_impl<alignof(detail::max_alignof_t<Ts...>), sizeof(detail::max_sizeof_t<Ts...>), Ts...>;

namespace detail
{
	template <typename T, typename = void>
	struct is_complete : std::false_type {};

	template <typename T>
	struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type {};

	struct recursive_variant_tag {};

	template <typename replacement, typename T>
	struct substitute : identity<T> {};

	template <typename replacement, typename T>
	using substitute_t = typename substitute<replacement, T>::type;

	template <typename replacement, typename T>
	struct substitute<replacement, const T> : identity<const substitute_t<replacement, T>> {};

	template <typename replacement, typename T>
	struct substitute<replacement, T&> : identity<substitute_t<replacement, T>&> {};

	template <typename replacement, typename T>
	struct substitute<replacement, T*> : identity<substitute_t<replacement, T>*> {};

	template <typename replacement>
	struct substitute<replacement, recursive_variant_tag> : identity<replacement> {};

	template <typename replacement, template <typename...> class U>
	struct substitute<replacement, U<recursive_variant_tag>>
	{
		/*using type = std::conditional_t<
		is_complete<U<replacement>>::value,
		U<replacement>,
		recursive_variant_wrapper_tag<U<replacement>>>;*/
		using type = U<replacement>;
	};

	template <typename replacement, template <typename...> class U, typename... Ts>
	struct substitute<replacement, U<Ts...>> : identity<U<substitute_t<replacement, Ts>...>> {};

	template <typename replacement, typename ret, typename... args>
	struct substitute<replacement, ret(args...)> : identity<substitute_t<replacement, ret>(substitute_t<replacement, args>...)> {};

	template <typename replacement, typename T>
	struct substitute_dummy : substitute<replacement, T> {};

	template <typename replacement, typename T>
	using substitute_dummy_t = typename substitute_dummy<replacement, T>::type;

	template <typename replacement, template <typename...> class U>
	struct substitute_dummy<replacement, U<recursive_variant_tag>>
	{
		/*using type = std::conditional_t<
		is_complete<U<replacement>>::value,
		U<int>,
		recursive_variant_wrapper_tag<U<replacement>>>;*/
		using type = U<int>;
		/*static_assert(!is_complete<U<replacement>>::value || sizeof(type) >= sizeof(U<replacement>),
		"container is specialized with a larger size for recursive_variant than it has for int! not good!");*/
	};

	template <typename replacement, template <typename...> class U, typename... Ts>
	struct substitute_dummy<replacement, U<Ts...>> : identity<U<substitute_dummy_t<replacement, Ts>...>> {};
}

template <typename... Ts>
struct recursive_variant : variant_impl<
	alignof(detail::max_alignof_t<detail::substitute_dummy_t<recursive_variant<Ts...>, Ts>...>),
	sizeof(detail::max_sizeof_t<detail::substitute_dummy_t<recursive_variant<Ts...>, Ts>...>),
	detail::substitute_t<recursive_variant<Ts...>, Ts>...>
{
	using base = variant_impl<
		alignof(detail::max_alignof_t<detail::substitute_dummy_t<recursive_variant<Ts...>, Ts>...>),
		sizeof(detail::max_sizeof_t<detail::substitute_dummy_t<recursive_variant<Ts...>, Ts>...>),
		detail::substitute_t<recursive_variant<Ts...>, Ts>...>;
	using base::base;
	using base::operator=;
};

template <typename VisitorType, typename VariantType>
decltype(auto) visit(VisitorType&& visitor, VariantType&& variant)
{
	return std::forward<VariantType>(variant).visit(std::forward<VisitorType>(visitor));
}

template <typename T, typename Var>
bool holds_alternative(Var& v) noexcept
{
	return v.template is_type<T>();
}

template <typename T, typename Var>
decltype(auto) get(Var& v)
{
	return v.template get_ref<T>();
}
