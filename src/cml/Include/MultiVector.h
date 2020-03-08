#pragma once

#include <vector>
#include <type_traits>

template <typename Type, typename... Rest>
struct MultiVector
{
	std::vector<Type> vec;
	MultiVector <Rest...> next;

	template <typename FuncT>
	void apply(const FuncT& Func)
	{
		vec.erase(std::remove_if(vec.begin(), vec.end(), [&](auto& x) { return !Func(x); }), vec.end());
		next.apply(Func);
	}

	template <typename T>
	void pushImpl(T&& val, std::true_type)
	{
		vec.push_back(std::forward<T>(val));
	}

	template <typename T>
	void pushImpl(T&& val, std::false_type)
	{
		next.push(std::forward<T>(val));
	}

	template <typename T>
	void push(T&& val)
	{
		pushImpl(std::forward<T>(val), std::is_same<std::remove_reference_t<T>, Type>());
	}

	template <typename T, typename... ArgsT>
	void emplaceImpl(std::true_type, ArgsT&&... Args)
	{
		vec.emplace_back(std::forward<ArgsT>(Args)...);
	}

	template <typename T, typename... ArgsT>
	void emplaceImpl(std::false_type, ArgsT&&... Args)
	{
		next.template emplace<T>(std::forward<ArgsT>(Args)...);
	}

	template <typename T, typename... ArgsT>
	void emplace(ArgsT&&... Args)
	{
		emplaceImpl<T>(std::is_same<std::remove_reference_t<T>, Type>(), std::forward<ArgsT>(Args)...);
	}
};

template <typename Type>
struct MultiVector<Type>
{
	std::vector<Type> vec;

	template <typename FuncT>
	void apply(FuncT Func)
	{
		vec.erase(std::remove_if(vec.begin(), vec.end(), [&](auto& x) { return !Func(x); }), vec.end());
	}

	template <typename T>
	void pushImpl(T&& val, std::true_type)
	{
		vec.push_back(std::forward<T>(val));
	}

	template <typename T>
	void push(T&& val)
	{
		static_assert(std::is_same<std::remove_reference_t<T>, Type>::value, "Type is not in MultiVector");
		pushImpl(std::forward<T>(val), std::is_same<std::remove_reference_t<T>, Type>());
	}

	template <typename T, typename... ArgsT>
	void emplaceImpl(std::true_type, ArgsT&&... Args)
	{
		vec.emplace_back(std::forward<ArgsT>(Args)...);
	}

	template <typename T, typename... ArgsT>
	void emplace(ArgsT&&... Args)
	{
		static_assert(std::is_same<std::remove_reference_t<T>, Type>::value, "Type is not in MultiVector");
		emplaceImpl<T>(std::is_same<std::remove_reference_t<T>, Type>(), std::forward<ArgsT>(Args)...);
	}
};
