#include "stdafx.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

rvector RCameraPosition, RCameraDirection{ 1, 0, 0 }, RCameraUp{ 0, 0, 1 };
rmatrix RView, RProjection, RViewProjection, RViewport, RViewProjectionViewport;
rfrustum RViewFrustum;
static float RFov_horiz, RFov_vert, RNearZ, RFarZ;
static D3DVIEWPORT9 g_d3dViewport;

static void UpdateViewFrustum()
{
	RViewFrustum = MakeViewFrustum(
		RView,
		RCameraPosition, RCameraDirection,
		RFov_horiz, RFov_vert,
		RNearZ, RFarZ);

	RViewProjection = RView * RProjection;
	RViewProjectionViewport = RViewProjection * RViewport;
}

void RSetCamera(const rvector &from, const rvector &at, const rvector &up)
{
	RCameraPosition = from;
	RCameraDirection = at - from;
	RCameraUp = up;

	RUpdateCamera();
}

void RUpdateCamera()
{
	RView = ViewMatrix(RCameraPosition, RCameraDirection, RCameraUp);
	RSetTransform(D3DTS_VIEW, RView);

	auto CheckNaN = [](auto& vec, const v3& def = { 1, 0, 0 })
	{
		if (isnan(vec))
			vec = def;
	};

	CheckNaN(RCameraPosition);
	CheckNaN(RCameraDirection);
	CheckNaN(RCameraUp, { 0, 0, 1 });

	UpdateViewFrustum();
}

void RSetProjection(float fFov, float fAspect, float fNearZ, float fFarZ)
{
	RFov_horiz = fFov;
	RFov_vert = ComputeVerticalFOV(RFov_horiz, fAspect);
	RNearZ = fNearZ; RFarZ = fFarZ;

	RProjection = PerspectiveProjectionMatrix(fAspect, RFov_vert, fNearZ, fFarZ);
	RSetTransform(D3DTS_PROJECTION, RProjection);

	UpdateViewFrustum();
}

void RSetProjection(float fFov, float fNearZ, float fFarZ) {
	RSetProjection(fFov, static_cast<float>(RGetScreenWidth()) / RGetScreenHeight(), fNearZ, fFarZ);
}

D3DVIEWPORT9* RGetViewport() { return &g_d3dViewport; }

void RSetViewport(int x1, int y1, int x2, int y2)
{
	D3DVIEWPORT9 *pViewport = RGetViewport();

	pViewport->X = x1; pViewport->Y = y1;
	pViewport->Width = x2 - x1; pViewport->Height = y2 - y1;
	pViewport->MinZ = 0;
	pViewport->MaxZ = 1;
	HRESULT hr = RGetDevice()->SetViewport(pViewport);
	_ASSERT(hr == D3D_OK);

	float RSwx = (float)(x2 - x1) / 2;
	float RSwy = (float)(y2 - y1) / 2;
	float RScx = (float)RSwx + x1;
	float RScy = (float)RSwy + y1;

	RViewport = IdentityMatrix();
	RViewport._11 = RSwx;
	RViewport._22 = -RSwy;
	RViewport._41 = RScx;
	RViewport._42 = RScy;
}

rvector RGetCameraPosition()
{
	return RCameraPosition;
}

void RResetTransform()
{
	RSetTransform(D3DTS_VIEW, RView);
	RSetTransform(D3DTS_PROJECTION, RProjection);
}
_NAMESPACE_REALSPACE2_END