#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <new>

constexpr struct in_place_t {} in_place{};
constexpr struct nullopt_t {} nullopt{};

template <typename T>
class optional
{
public:
	optional() = default;

	optional(nullopt_t) {}

	optional(const optional& src)
	{
		if (src.has_value())
			emplace(*src);
	}

	optional(optional&& src)
	{
		if (src.has_value())
			emplace(*std::move(src));
	}

	template <typename... ArgsType>
	explicit optional(in_place_t, ArgsType&&... Args) {
		emplace(std::forward<ArgsType>(Args)...);
	}

	// Need the int/long parameters so that the functions have different signatures, to enable SFINAE.
	template <typename U, typename = std::enable_if_t<std::is_convertible<U&&, T>::value>, int = 0>
	optional(U&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
		emplace(std::forward<U>(value));
	}

	template <typename U, typename = std::enable_if_t<!std::is_convertible<U&&, T>::value>, long = 0>
	explicit optional(U&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
		emplace(std::forward<U>(value));
	}

	optional& operator=(const optional& src)
	{
		this->~optional();
		new (this) optional{ src };
		return *this;
	}

	optional& operator=(optional&& src)
	{
		this->~optional();
		new (this) optional{ std::move(src) };
		return *this;
	}

	~optional()
	{
		if (has_value())
			reset();
	}

	template <typename... ArgsType>
	T& emplace(ArgsType&&... Args)
	{
		assert(!Constructed);
		new (buf) T{ std::forward<ArgsType>(Args)... };
		Constructed = true;
		return value();
	}

	void reset()
	{
		assert(Constructed);
		value().~T();
		Constructed = false;
	}

	T& operator*() & { assert(has_value()); return *reinterpret_cast<T*>(buf); }
	const T& operator*() const& { assert(has_value()); return *reinterpret_cast<const T*>(buf); }
	T&& operator*() && { return std::move(**this); }
	const T&& operator*() const&& { return std::move(**this); }

	T* operator->() { return &**this; }
	const T* operator->() const { return &**this; }

	T& value() & { return **this; }
	const T& value() const & { return **this; }
	T&& value() && { return *std::move(*this); }
	const T& value() const && { return *std::move(*this); }

	template <typename U>
	T value_or(U&& DefaultValue) const { return has_value() ? value() : std::forward<U>(DefaultValue); }

	bool has_value() const { return Constructed; }

	explicit operator bool() const { return has_value(); }

private:
	alignas(T) char buf[sizeof(T)];
	bool Constructed{};
};

template <typename T>
optional<std::decay_t<T>> make_optional(T&& x)
{
	return optional<std::decay_t<T>>(x);
};

template <typename T, typename... Args>
optional<T> make_optional(Args&&... args)
{
	return optional<T>(in_place, std::forward<Args>(args)...);
};