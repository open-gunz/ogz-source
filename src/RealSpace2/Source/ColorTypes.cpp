#include "stdafx.h"
#include "ColorTypes.h"
#include <D2DBaseTypes.h>

color_r32::operator D3DCOLORVALUE() const
{
	return{ r, g, b, a };
}

color_r32::operator u32() const
{
	return ARGB(uint8_t(a * 255), uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255));
}
