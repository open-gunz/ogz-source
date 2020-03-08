#pragma once

#include <string>
#include <memory>
#include <algorithm>
#include <climits>
#include <cstring>
#include "TMP.h"
#include "GlobalTypes.h"
#include "StringView.h"
#include "optional.h"
#include "function_view.h"

#ifdef _MSC_VER
#include <intrin.h>
#include <intsafe.h>
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif

template <typename T>
inline void SafeRelease(T& ptr)
{
	if (!ptr)
		return;
	ptr->Release();
	ptr = nullptr;
}

struct D3DDeleter {
	void operator()(struct IUnknown* ptr) const;
};

template <typename T>
using D3DPtr = std::unique_ptr<T, D3DDeleter>;

template <typename T>
inline void SafeRelease(D3DPtr<T>& ptr)
{
	ptr = nullptr;
}

using WIN_DWORD_PTR = std::conditional_t<sizeof(void*) == 4, unsigned long, unsigned long long>;

#define SAFE_RELEASE(p)      { SafeRelease(p); }

#define EXPAND_VECTOR(v) v[0], v[1], v[2]

#define SetBitSet(sets, item)		(sets |= (1 << item))
#define ClearBitSet(sets, item)		(sets &= ~(1 << item))
#define CheckBitSet(sets, item)		(sets & (1 << item))

#ifdef _MSC_VER
#define WARN_UNUSED_RESULT _Check_return_
#define STDCALL __stdcall
#else
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define STDCALL __attribute__((stdcall))
#endif

#define TOKENIZE_IMPL(a, b) a##b
#define TOKENIZE(a, b) TOKENIZE_IMPL(a, b)

inline constexpr uint32_t ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

inline constexpr uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ARGB(a, r, g, b);
}

inline constexpr uint32_t XRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ARGB(0xFF, r, g, b);
}

inline constexpr uint32_t XRGB(uint8_t value)
{
	return XRGB(value, value, value);
}

inline constexpr uint32_t RGBAF(float r, float g, float b, float a)
{
	return RGBA(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
}

inline constexpr uint32_t ARGBF(float r, float g, float b, float a)
{
	return RGBA(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
}

enum MDateType
{
	MDT_Y = 1,
	MDT_YM,
	MDT_YMD,
	MDT_YMDH,
	MDT_YMDHM,
};

std::string MGetStrLocalTime(MDateType = MDT_YMDHM);

namespace detail{
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, decltype(std::begin(std::declval<T>()), std::end(std::declval<T>()), void())> : std::true_type {};
}

template <typename T>
struct Range
{
	using value_type = std::decay_t<decltype(*std::declval<T>())>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = T;
	using const_iterator = const T;

	T first;
	T second;

	Range() = default;

	Range(const T& first, const T& second) : first{ first }, second{ second } {}

	template <typename U, typename = std::enable_if_t<detail::is_container<U>::value>>
	Range(U&& Container) : first{ Container.begin() }, second{ Container.end() } {}

	auto begin() { return first; }
	auto end() { return second; }
	auto begin() const { return first; }
	auto end() const { return second; }
};

template <typename T>
auto MakeRange(const T& begin, const T& end) {
	return Range<T>{ begin, end };
}

template <typename T>
auto MakeRange(T&& x) -> std::enable_if_t<detail::is_container<T>::value, Range<decltype(std::begin(x))>> {
	return Range<decltype(std::begin(x))>{ x };
}

template <typename T>
auto MakeRange(const std::pair<T, T>& x) {
	return Range<T>{ x.first, x.second };
}

template <template <typename...> class itT, typename T>
auto MakeAdapter(T& Container) {
	return MakeRange(
		itT<decltype(Container.begin())>{ Container.begin() },
		itT<decltype(Container.end())>{ Container.end() });
}

template <typename ItT>
class ValueIterator
{
public:
	ValueIterator(ItT i) : it(i) { }

	ValueIterator& operator++()
	{
		++it;
		return *this;
	}

	ValueIterator operator++(int)
	{
		auto temp(*this);
		++*this;
		return temp;
	}

	bool operator==(const ValueIterator& rhs) const { return it == rhs.it; }
	bool operator!=(const ValueIterator& rhs) const { return it != rhs.it; }
	auto& operator*() { return it->second; }
	auto& operator->() { return this->operator*(); }

private:
	ItT it;
};

// Returns an adapter whose iterators returns .second of the pair that
// iterators of the original container return, i.e. the values of a map.
template <typename T>
auto MakePairValueAdapter(T& Container) { return MakeAdapter<ValueIterator>(Container); }

#ifdef _MSC_VER
inline bool add_overflow(u16 a, u16 b, u16* c) { return UShortAdd(a, b, c) != S_OK; }
inline bool add_overflow(u32 a, u32 b, u32* c) { return UIntAdd(a, b, c) != S_OK; }
inline bool add_overflow(u64 a, u64 b, u64* c) { return ULongLongAdd(a, b, c) != S_OK; }
inline bool sub_overflow(u16 a, u16 b, u16* c) { return UShortSub(a, b, c) != S_OK; }
inline bool sub_overflow(u32 a, u32 b, u32* c) { return UIntSub(a, b, c) != S_OK; }
inline bool sub_overflow(u64 a, u64 b, u64* c) { return ULongLongSub(a, b, c) != S_OK; }
inline bool mul_overflow(u16 a, u16 b, u16* c) { return UShortMult(a, b, c) != S_OK; }
inline bool mul_overflow(u32 a, u32 b, u32* c) { return UIntMult(a, b, c) != S_OK; }
inline bool mul_overflow(u64 a, u64 b, u64* c) { return ULongLongMult(a, b, c) != S_OK; }
template <typename T>
bool add_overflow(T a, T b, T* c) { return add_overflow(a, b, c); }
template <typename T>
bool sub_overflow(T a, T b, T* c) { return sub_overflow(a, b, c); }
template <typename T>
bool mul_overflow(T a, T b, T* c) { return mul_overflow(a, b, c); }
#else
template <typename T>
bool add_overflow(T a, T b, T* c) { return __builtin_add_overflow(a, b, c); }
template <typename T>
bool sub_overflow(T a, T b, T* c) { return __builtin_sub_overflow(a, b, c); }
template <typename T>
bool mul_overflow(T a, T b, T* c) { return __builtin_mul_overflow(a, b, c); }
#endif

namespace detail
{
inline bool isdecdigit(char c) { return c >= '0' && c <= '9'; }
inline char tolower(char c) { return char(u8(u8(c) | 0x20)); }

// Returns the positive value of the digit, or -1 on error.
template <int Radix>
int GetDigit(char c)
{
	if (isdecdigit(c))
	{
		const auto Digit = c - '0';
		if (Digit > Radix - 1)
			return -1;
		return Digit;
	}

	if (Radix <= 10)
		return -1;

	c = toupper(c);

	constexpr auto LastValidDigitInBase = 'A' + (Radix - 10);
	if (c < 'A' || c > LastValidDigitInBase)
		return -1;

	return c - 'A' + 10;
}

template <typename T>
struct CounterType
{
	using type = std::conditional_t<sizeof(T) * CHAR_BIT < 32, u32, std::make_unsigned_t<T>>;
};

template <> struct CounterType<bool> { using type = u32; };
}

template <typename T>
auto Reverse(T&& Container)
{
	return MakeRange(std::rbegin(Container), std::rend(Container));
}

template <typename T = int, int Radix = 0, bool Wrap = false>
optional<T> StringToInt(StringView Str)
{
	static_assert(Radix >= 0 && Radix != 1 && Radix <= 36, "Invalid radix");

	Str = trim(Str);

	if (Str.empty())
		return nullopt;

	bool IsNegative = Str[0] == '-';
	if (IsNegative)
	{
		if (!Wrap && std::is_unsigned<T>::value)
			return nullopt;
		Str = Str.substr(1);
	}

	using U = typename detail::CounterType<T>::type;

	auto MAbsVal = [&]() -> optional<U> {
		if ((Radix == 0 || Radix == 16) && starts_with(Str, "0x"))
			return StringToInt<U, 16>(Str.substr(2));
		if ((Radix == 0 || Radix == 8) && starts_with(Str, "0o"))
			return StringToInt<U, 8>(Str.substr(2));
		if (Radix == 0)
			return StringToInt<U, 10>(Str);

		U Accumulator = 0;
		U Coefficient = 1;

		bool MulWrapped = false;
		for (auto DigitChar : Reverse(Str))
		{
			auto Digit = detail::GetDigit<Radix>(DigitChar);
			if (Digit == -1)
				return nullopt;
			if (Wrap)
			{
				Accumulator += Digit * Coefficient;
				Coefficient *= Radix;
			}
			else
			{
				if (MulWrapped ||
					add_overflow<U>(Accumulator, Digit * Coefficient, &Accumulator))
					return nullopt;
				MulWrapped = mul_overflow<U>(Coefficient, Radix, &Coefficient);
			}
		}

		return Accumulator;
	}();

	if (!MAbsVal)
		return nullopt;
	auto AbsVal = *MAbsVal;

	U Max = U((std::numeric_limits<T>::max)()) + U(IsNegative);
	if (!Wrap && AbsVal > Max)
		return nullopt;

	if (IsNegative)
	{
		if (AbsVal == 0)
			return T(0);
		return T(T(-1) - T(AbsVal - 1));
	}

	return T(AbsVal);
}

// Comparison functions for mixed-sign values (e.g. -2 < 4u is false, but
// mixed_sign::lt(-2, 4u) is true.
namespace mixed_sign
{
template <typename A, typename B>
int cmp(A a, B b)
{
	// Disable signed-unsigned mismatch warning since we're explicitly checking that already.
#pragma warning(push)
#pragma warning(disable:4018)
	constexpr bool sa = std::is_signed<A>::value;
	constexpr bool sb = std::is_signed<B>::value;
	if (!(sa ^ sb))
	{
		if (a < b) return -1;
		if (a == b) return 0;
		if (a > b) return 1;
	}
	if (sa && a < 0)
		return -1;
	if (sb && b < 0)
		return 1;
	return cmp(static_cast<std::make_unsigned_t<A>>(a),
	           static_cast<std::make_unsigned_t<B>>(b));
#pragma warning(pop)
}
template <typename A, typename B> bool eq   (A a, B b) { return cmp(a, b) == 0; }
template <typename A, typename B> bool lt   (A a, B b) { return cmp(a, b) <  0; }
template <typename A, typename B> bool lt_eq(A a, B b) { return cmp(a, b) <= 0; }
template <typename A, typename B> bool gt   (A a, B b) { return cmp(a, b) >  0; }
template <typename A, typename B> bool gt_eq(A a, B b) { return cmp(a, b) >= 0; }
}

// WriteProxy
// A class that acts as a pointer-to-pointer wrapper for smart pointers for passing to functions
// that expect to "return" a pointer by writing to a pointer-to-pointer argument.
// Should only ever be instantiated by MakeWriteProxy as a temporary for a argument.
// The smart pointer will then have the returned value (if set) after the full expression it appears in.
//
// Example:
// void foo(int**); std::unique_ptr<int> ptr; foo(MakeWriteProxy(ptr));
template <typename T>
class WriteProxy
{
	using StoredType = tmp::get_template_argument_t<T, 0>;
public:
	~WriteProxy() { ptr = T{ temp }; }

	operator StoredType**() && { return &temp; }

private:
	WriteProxy(T& ptr) : ptr(ptr), temp(ptr.get()) {}
	WriteProxy(const WriteProxy&) = delete;
	WriteProxy& operator=(const WriteProxy&) = delete;

	template <typename... U>
	friend WriteProxy<std::unique_ptr<U...>> MakeWriteProxy(std::unique_ptr<U...>&);

	T& ptr;
	StoredType* temp{};
};

template <typename... T>
WriteProxy<std::unique_ptr<T...>> MakeWriteProxy(std::unique_ptr<T...>& ptr) {
	return{ ptr };
}

// Converts an rvalue to a mutable lvalue
template <typename T>
T& unmove(T&& x) { return x; }

inline u32 bsr(u32 Value)
{
#ifdef _WIN32
	unsigned long ret = 0;
	_BitScanReverse(&ret, Value);
	return ret;
#else
	if (!Value)
		return 0;
	return u32(31 - __builtin_clz(Value));
#endif
}

// Returns value rounded up towards the nearest power of two
inline u32 NextPowerOfTwo(u32 value)
{
	if (value == 0)
		return 2;
	auto rightmost_bit = bsr(value);
	auto rightmost_bit_value = 1u << rightmost_bit;
	if ((value ^ rightmost_bit_value) != 0)
		rightmost_bit_value <<= 1;
	return rightmost_bit_value;
}

template <typename ContainerType, typename ValueType>
void erase_remove(ContainerType&& Container, ValueType&& Value) {
	Container.erase(std::remove(Container.begin(), Container.end(), Value), Container.end());
}

template <typename ContainerType, typename PredicateType>
void erase_remove_if(ContainerType&& Container, PredicateType&& Predicate) {
	Container.erase(std::remove_if(Container.begin(), Container.end(),
		std::forward<PredicateType>(Predicate)),
		Container.end());
}

struct CFileCloser {
	void operator()(FILE* ptr) const {
		if (ptr)
			fclose(ptr);
	}
};

using CFilePtr = std::unique_ptr<FILE, CFileCloser>;

template <typename DerivedType, typename ValueType, typename CategoryType = std::random_access_iterator_tag>
struct IteratorBase
{
	bool operator!=(const IteratorBase& rhs) const { return !(Derived() == rhs.Derived()); }

	IteratorBase& operator++(int) {
		auto temp = *this;
		++Derived();
		return temp;
	}

	using difference_type = ptrdiff_t;
	using value_type = ValueType;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = CategoryType;

private:
	auto& Derived() { return static_cast<DerivedType&>(*this); }
	auto& Derived() const { return static_cast<const DerivedType&>(*this); }
};

// Returns a mod n, handling negative dividends correctly. (-1 % 5 == -1, mod(-1, 5) == 4.)
template <typename T1, typename T2>
auto mod(T1 a, T2 n)
{
	const auto div = a / n - int(a < 0);
	const auto off = div * n;
	return a - off;
}

inline void Split(StringView Str, StringView Delim, function_view<void(StringView)> Callback)
{
	size_t idx = 0;
	while (true)
	{
		auto idx2 = Str.find_first_of(Delim, idx);
		if (idx2 == Str.npos)
		{
			Callback(Str.substr(idx));
			return;
		}
		Callback(Str.substr(idx, idx2 - idx));
		idx = idx2 + 1;
		if (idx >= Str.size())
			return;
	}
}

template <typename ContainerType, typename... ArgsType>
constexpr decltype(auto) emplace_back(ContainerType& Container, ArgsType&&... Args)
{
	Container.emplace_back(std::forward<ArgsType>(Args)...);
	return Container.back();
}
