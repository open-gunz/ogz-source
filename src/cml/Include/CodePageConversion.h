#pragma once

constexpr size_t ConversionError = size_t(-1);

size_t CodePageConversion(char* Output, size_t OutputSize, const wchar_t* Input, int CodePage);
size_t CodePageConversion(wchar_t* Output, size_t OutputSize, const char* Input, int CodePage);

template <int MultiByteCodePage, size_t OutputSize>
size_t CodePageConversion(char(&Output)[OutputSize], const wchar_t* Input)
{
	return CodePageConversion(Output, OutputSize, Input, MultiByteCodePage);
}

template <int MultiByteCodePage, size_t OutputSize>
size_t CodePageConversion(wchar_t(&Output)[OutputSize], const char* Input)
{
	return CodePageConversion(Output, OutputSize, Input, MultiByteCodePage);
}

template <int DestCodePage, int SourceCodePage, size_t OutputSize>
size_t CodePageConversion(char(&Output)[OutputSize], const char* Input)
{
	// TODO: Replace with a direct conversion.
	wchar_t Buffer[OutputSize];
	if (CodePageConversion<SourceCodePage>(Buffer, Input) == ConversionError)
		return ConversionError;
	return CodePageConversion<DestCodePage>(Output, Buffer);
}