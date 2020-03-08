#pragma once

#include "RTypes.h"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class RImpactPlanes : public std::list<rplane> {
public:
	bool Add(rplane &p);
};

enum RCOLLISIONMETHOD{
	RCW_SPHERE,
	RCW_CYLINDER
};

class RSolidBspNode
{
public:
	bool GetColPlanes_Cylinder(RImpactPlanes *pOutList, const rvector &origin, const rvector &to, float fRadius, float fHeight);
	bool GetColPlanes_Sphere(RImpactPlanes *pOutList, const rvector &origin, const rvector &to, float fRadius);
	rvector GetLastColPos() { return impact; }
	rplane GetLastColPlane() { return impactPlane; }

	static bool CheckWall(RSolidBspNode *pRootNode, const rvector &origin, rvector &targetpos,
		float fRadius, float fHeight = 0.f, RCOLLISIONMETHOD method = RCW_CYLINDER,
		int nDepth = 0, rplane *pimpactplane = NULL);
	static bool CheckWall2(RSolidBspNode *pRootNode, RImpactPlanes &impactPlanes, const rvector &origin,
		rvector &targetpos, float fRadius, float fHeight, RCOLLISIONMETHOD method);

	static bool				m_bTracePath;

	rplane			m_Plane;
	RSolidBspNode	*m_pPositive{}, *m_pNegative{};
	bool			m_bSolid;

#ifndef _PUBLISH
	int	nPolygon;
	rvector* pVertices{};
	std::unique_ptr<rvector[]> pNormals;

	rboundingbox m_bb;

	void DrawPolygon();
	void DrawPolygonWireframe();
	void DrawPolygonNormal();

	void DrawSolidPolygon();
	void DrawSolidPolygonWireframe();
	void DrawSolidPolygonNormal();

	void DrawPos(const rvector &pos);
	void DrawPlaneVertices(const rplane &plane);
	void ConstructBoundingBox();
#endif

private:
	bool GetColPlanes_Recurse(int nDepth = 0);

	static RCOLLISIONMETHOD m_ColMethod;
	static float	m_fColRadius;
	static float	m_fColHeight;
	static rvector	m_ColOrigin;
	static rvector	m_ColTo;
	static RImpactPlanes* m_pOutList;
	static float	fImpactDist;
	static rvector	impact;
	static rplane	impactPlane;
};

_NAMESPACE_REALSPACE2_END