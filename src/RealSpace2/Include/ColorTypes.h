#pragma once

#include "GlobalTypes.h"

typedef struct _D3DCOLORVALUE D3DCOLORVALUE;

struct color_r32
{
	float r, g, b, a;

	color_r32() = default;
	explicit color_r32(u32 argb) :
		a{ ((argb & 0xFF000000) >> 24) * (1.0f / 255.0f) },
		r{ ((argb & 0xFF0000) >> 16) * (1.0f / 255.0f) },
		g{ ((argb & 0xFF00) >> 8) * (1.0f / 255.0f) },
		b{ (argb & 0xFF) * (1.0f / 255.0f) }
	{}
	color_r32(float r, float g, float b, float a) : r{ r }, g{ g }, b{ b }, a{ a } {}
	color_r32(const float(&arr)[4]) :
		a{ arr[0] },
		r{ arr[1] },
		g{ arr[2] },
		b{ arr[3] }
	{}

	explicit operator D3DCOLORVALUE() const;
	explicit operator u32() const;

	color_r32& operator +=(const color_r32& rhs) {
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;
		return *this;
	}

	color_r32& operator *=(const color_r32& rhs) {
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;
		a *= rhs.a;
		return *this;
	}

	color_r32& operator *=(float rhs) {
		r *= rhs;
		g *= rhs;
		b *= rhs;
		a *= rhs;
		return *this;
	}

	color_r32& operator /=(float rhs) {
		r /= rhs;
		g /= rhs;
		b /= rhs;
		a /= rhs;
		return *this;
	}
};

inline color_r32 operator +(const color_r32& a, const color_r32& b) {
	auto ret = a; return ret += b;
}

inline color_r32 operator *(const color_r32& a, const color_r32& b) {
	auto ret = a; return ret *= b;
}

inline color_r32 operator *(const color_r32& a, const float& b) {
	auto ret = a; return ret *= b;
}

inline color_r32 operator *(const float& a, const color_r32& b) {
	auto ret = b; return ret *= a;
}

inline color_r32 operator /(const color_r32& a, const float& b) {
	auto ret = a; return ret /= b;
}