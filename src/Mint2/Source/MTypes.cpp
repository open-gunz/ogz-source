#include "stdafx.h"
#include "MTypes.h"
#include "MWidget.h"
#include "Mint.h"
#include <algorithm>

void MPOINT::Scale(float sx, float sy)
{
	x = int(x * sx);
	y = int(y * sy);
}

void MPOINT::ScaleRes()
{
	x = x * MGetWorkspaceWidth() / 640;
	y = y * MGetWorkspaceHeight() / 480;
}

void MPOINT::TranslateRes()
{
	int nDiffX = x - 320;
	int nDiffY = y - 240;

	x = nDiffX + MGetWorkspaceWidth()/2;
	y = nDiffY + MGetWorkspaceHeight()/2;
}

void MRECT::ScalePos(float sx, float sy)
{
	x = int(x * sx);
	y = int(y * sy);
}

void MRECT::ScaleArea(float sx, float sy)
{
	x = int(x * sx);
	y = int(y * sy);
	w = int(w * sx);
	h = int(h * sy);
}

void MRECT::ScalePosRes()
{
	float x = MGetWorkspaceWidth() / 640.f;
	float y = MGetWorkspaceHeight() / 480.f;
	ScalePos(x, y);
}

void MRECT::ScaleAreaRes()
{
	float x = MGetWorkspaceWidth() / 640.f;
	float y = MGetWorkspaceHeight() / 480.f;
	ScaleArea(x, y);
}

void MRECT::TranslateRes()
{
	int nDiffX = x - 320;
	int nDiffY = y - 240;

	x = nDiffX + MGetWorkspaceWidth() / 2;
	y = nDiffY + MGetWorkspaceHeight() / 2;
}

void MRECT::EnLarge(int s)
{
	x -= s;
	y -= s;
	w += s * 2;
	h += s * 2;
}

void MRECT::Offset(int sx, int sy)
{
	x += sx;
	y += sy;
}

bool MRECT::Intersect(MRECT* pIntersect, const MRECT& r)
{
	_ASSERT(pIntersect != NULL);

	if (x > r.x) {
		pIntersect->x = x;
		pIntersect->w = std::min(r.x + r.w, x + w) - x;
	}
	else {
		pIntersect->x = r.x;
		pIntersect->w = std::min(r.x + r.w, x + w) - r.x;
	}
	if (y > r.y) {
		pIntersect->y = y;
		pIntersect->h = std::min(r.y + r.h, y + h) - y;
	}
	else {
		pIntersect->y = r.y;
		pIntersect->h = std::min(r.y + r.h, y + h) - r.y;
	}

	if (pIntersect->w < 0) return false;
	if (pIntersect->h < 0) return false;

	return true;
}
