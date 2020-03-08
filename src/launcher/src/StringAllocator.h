#pragma once

#include <vector>
#include <memory>
#include "StringView.h"
#include "ArrayView.h"

inline ArrayView<char> AllocateString(size_t Size)
{
	using PtrType = std::unique_ptr<char[]>;
	thread_local std::vector<PtrType> Strings;
	auto Ptr = PtrType{new char[Size]};
	Strings.emplace_back(std::move(Ptr));
	return { Strings.back().get(), Size };
}

inline StringView AllocateString(const StringView& Src)
{
	const auto Size = Src.size() + 1;
	auto String = AllocateString(Size);
	memcpy(String.data(), Src.data(), Size);
	return { String.data(), Src.size() };
}