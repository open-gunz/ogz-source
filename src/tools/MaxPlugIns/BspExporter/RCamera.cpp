#include "stdafx.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

// 카메라 관련
rvector RCameraPosition,RCameraDirection,RCameraUp;
rmatrix RView,RProjection,RViewProjection,RViewport,RViewProjectionViewport;
static rplane RViewFrustum[6];
static float RFov_horiz,RFov_vert,RNearZ,RFarZ;
D3DVIEWPORT9			g_d3dViewport;

void ComputeViewFrustum(rplane *plane,float x,float y,float z)
{
	/*
	rmatrix RView;
	RGetDevice()->GetTransform(D3DTS_VIEW,&RView);
*/
	plane->a=RView._11*x+RView._12*y+RView._13*z;
	plane->b=RView._21*x+RView._22*y+RView._23*z;
	plane->c=RView._31*x+RView._32*y+RView._33*z;
	plane->d=-plane->a*RCameraPosition.x
				-plane->b*RCameraPosition.y
				-plane->c*RCameraPosition.z;
}

void ComputeZPlane(rplane *plane,float z,int sign)
{
	static rvector normal,t;
	t=RCameraPosition+z*RCameraDirection;
	normal=float(sign)*RCameraDirection;
	D3DXVec3Normalize(&normal,&normal);
	plane->a=normal.x;plane->b=normal.y;plane->c=normal.z;
	plane->d=-plane->a*t.x-plane->b*t.y-plane->c*t.z;
}

void UpdateViewFrustrum()
{
	float fovh2=RFov_horiz/2.0f,fovv2=RFov_vert/2.0f;
	float ch=cosf(fovh2),sh=sinf(fovh2);
	float cv=cosf(fovv2),sv=sinf(fovv2);

	ComputeViewFrustum(RViewFrustum+0, -ch,  0, sh);
	ComputeViewFrustum(RViewFrustum+1,  ch,  0, sh);
	ComputeViewFrustum(RViewFrustum+2,  0,  cv, sv);
	ComputeViewFrustum(RViewFrustum+3,  0, -cv, sv);
	ComputeZPlane(RViewFrustum+4,RNearZ,1);
	ComputeZPlane(RViewFrustum+5,RFarZ,-1);

	RViewProjection=RView * RProjection;
	RViewProjectionViewport=RViewProjection * RViewport;

}

void RSetCamera(const rvector &from, const rvector &at, const rvector &up)
{
	RCameraPosition=from;
	RCameraDirection=at-from;
	RCameraUp=up;
	
	RUpdateCamera();
}

void RUpdateCamera()
{
	rvector at=RCameraPosition+RCameraDirection;
	D3DXMatrixLookAtLH( &RView, &RCameraPosition, &at, &RCameraUp );
	RGetDevice()->SetTransform( D3DTS_VIEW, &RView );

	UpdateViewFrustrum();
}

void RSetProjection(float fFov,float fAspect,float fNearZ,float fFarZ)
{
//	FLOAT fAspect = (FLOAT)RGetScreenWidth() / (FLOAT)RGetScreenHeight();

	RFov_horiz=fFov;
	RFov_vert=atanf(tanf(RFov_horiz/2.0f)/fAspect)*2.0f;
	RNearZ=fNearZ;RFarZ=fFarZ;

	D3DXMatrixPerspectiveFovLH( &RProjection, RFov_vert, fAspect, fNearZ, fFarZ );
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &RProjection );

	UpdateViewFrustrum();
}

void RSetProjection(float fFov,float fNearZ,float fFarZ)
{
	FLOAT fAspect = (FLOAT)RGetScreenWidth() / (FLOAT)RGetScreenHeight();
	
	RFov_horiz=fFov;
	RFov_vert=atanf(tanf(RFov_horiz/2.0f)/fAspect)*2.0f;
	RNearZ=fNearZ;RFarZ=fFarZ;

	D3DXMatrixPerspectiveFovLH( &RProjection, RFov_vert, fAspect, fNearZ, fFarZ );
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &RProjection );

	UpdateViewFrustrum();
}

D3DVIEWPORT9		*RGetViewport() { return &g_d3dViewport;}

void RSetViewport(int x1,int y1,int x2,int y2)
{
	D3DVIEWPORT9 *pViewport=RGetViewport();

	pViewport->X=x1;pViewport->Y=y1;
	pViewport->Width=x2-x1;pViewport->Height=y2-y1;
	pViewport->MinZ=0;
	pViewport->MaxZ=1;
	RGetDevice()->SetViewport(pViewport);

	float RSwx=(float)(x2-x1)/2;
	float RSwy=(float)(y2-y1)/2;
	float RScx=(float)RSwx+x1;
	float RScy=(float)RSwy+y1;

	D3DXMatrixIdentity(&RViewport);
	RViewport._11=RSwx;
	RViewport._22=-RSwy;
	RViewport._41=RScx;
	RViewport._42=RScy;
}

rvector RGetCameraPosition()
{
	return RCameraPosition;
}

rplane *RGetViewFrustum()
{
	return RViewFrustum;
}

void RResetTransform()
{
    RGetDevice()->SetTransform( D3DTS_VIEW, &RView );
	RGetDevice()->SetTransform( D3DTS_PROJECTION, &RProjection );
}
_NAMESPACE_REALSPACE2_END