#include "stdafx.h"
#include "MClipboard.h"
#include "defer.h"

#ifdef WIN32

#include "MWindows.h"

template <typename CharType> constexpr u32 Format();
template <> constexpr u32 Format<char>() { return CF_TEXT; }
template <> constexpr u32 Format<wchar_t>() { return CF_UNICODETEXT; };

template <typename CharType>
static bool GetImpl(void* WindowHandle, CharType* Output, size_t OutputSize)
{
	if (!IsClipboardFormatAvailable(Format<CharType>()))
		return{};

	auto hWnd = reinterpret_cast<HWND>(WindowHandle);
	if (!OpenClipboard(hWnd))
		return{};
	DEFER([&] { CloseClipboard(); });

	auto ClipboardData = GetClipboardData(Format<CharType>());
	if (!ClipboardData)
		return{};

	auto ptr = static_cast<const CharType*>(GlobalLock(ClipboardData));
	if (!ptr)
		return{};
	DEFER([&] { GlobalUnlock(ClipboardData); });

	strcpy_safe(Output, OutputSize, ptr);

	return true;
}

template <typename CharType>
static bool SetImpl(const BasicStringView<CharType>& Str)
{
	auto MemSize = (Str.size() + 1) * sizeof(CharType);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, MemSize);
	if (!hMem)
		return false;

	auto pMem = static_cast<CharType*>(GlobalLock(hMem));
	if (!pMem)
	{
		GlobalUnlock(pMem);
		GlobalFree(pMem);
		return false;
	}

	strcpy_safe(pMem, MemSize, Str);
	GlobalUnlock(hMem);

	auto Success = SetClipboardData(Format<CharType>(), hMem) != NULL;
	if (!Success)
		GlobalFree(hMem);

	return Success;
}

#else

template <typename CharType>
static bool GetImpl(void* WindowHandle, CharType* Output, size_t OutputSize)
{
	return false;
}

template <typename CharType>
static bool SetImpl(const BasicStringView<CharType>& Str)
{
	return false;
}

#endif

bool MClipboard::Get(void* WindowHandle, char* Output, size_t OutputSize) {
	return GetImpl(WindowHandle, Output, OutputSize); }
bool MClipboard::Get(void* WindowHandle, wchar_t* Output, size_t OutputSize) {
	return GetImpl(WindowHandle, Output, OutputSize); }
bool MClipboard::Set(void* WindowHandle, const StringView& Str) { return SetImpl(Str); }
bool MClipboard::Set(void* WindowHandle, const WStringView& Str) { return SetImpl(Str); }