#include "stdafx.h"
#include "Portal.h"
#ifdef PORTAL
#include "ZCamera.h"
#include "ZCombatInterface.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZGlobal.h"
#include "RTypes.h"
#include "RMeshUtil.h"
#include "RGMain.h"
#include "ZGameInterface.h"
#include "ZGameInput.h"
#include "ZConfiguration.h"
#include "Config.h"
#include "ZPickInfo.h"
#include "LogMatrix.h"

std::unique_ptr<Portal> g_pPortal;

const rvector Portal::PortalExtents = rvector(100, 150, 0.1);

struct PortalInfo
{
	// The position of the portal. It is in the center of the portal surface.
	rvector Pos{};
	// The direction the portal is facing.
	rvector Normal{};
	// The up vector.
	rvector Up{};
	// d in the portal surface's general plane equation. The normal constitutes a, b and c.
	float d{};
	// World transform
	rmatrix World{};
	// Inverse world transform
	rmatrix InvWorld{};
	// Rotates from this portal's orientation to the other portal's orientation, except rotated
	// 180 degrees around the up vector. That is, things in front of this portal appear
	// behind the other portal after rotation.
	rmatrix Rot{};
	// Transforms from this portal's space to the other portal's space, except rotated as previously described.
	rmatrix Transform{};
	// The near and far planes of a frustum looking through the portal.
	// The other four planes of the frustum change based
	// on the camera position, but these are constant.
	rplane Near{};
	rplane Far{};
	// Corner points of the portal.
	rvector TopRight{};
	rvector TopLeft{};
	rvector BottomRight{};
	rvector BottomLeft{};
	// Axis-aligned bounding box.
	rboundingbox BoundingBox{};
	// Determines if this portal has been put down.
	// If not, the player has only fired one portal,
	// and no wormhole has been established yet.
	bool Set{};
	// The portal's index in the pair, either 0 or 1.
	int Index{};
	// The other portal in the pair.
	PortalInfo& Other;

	PortalInfo(int Index, PortalInfo& Other) : Index{ Index }, Other { Other } {}
};

struct PortalPair
{
public:
	PortalPair() : pi{ {0, pi[1]}, {1, pi[0]} } {}

	auto& operator [](size_t Index) { return pi[Index]; }
	auto& operator [](size_t Index) const { return pi[Index]; }

	bool IsValid() const { return pi[0].Set && pi[1].Set; }

	auto begin() { return std::begin(pi); }
	auto end() { return std::end(pi); }
	auto begin() const { return std::begin(pi); }
	auto end() const { return std::end(pi); }

private:
	PortalInfo pi[2];
};

// Data about a portal in the context of the viewpoint at a particular level of rendering recursion.
struct PortalContext
{
	bool IsVisible;
	// The frustum for the view FROM the other portal THROUGH this one
	rfrustum Frustum;
	PortalInfo& pi;
	PortalContext& Other;

	PortalContext(PortalInfo& pi, PortalContext& Other) : pi{ pi }, Other{ Other } {}
};

struct PortalContextPair
{
	PortalContextPair(PortalPair& pair) : p{ { pair[0], p[1] },{ pair[1], p[0] } } {}

	auto& operator[](size_t idx) { return p[idx]; }
	auto& operator[](size_t idx) const { return p[idx]; }

	auto begin() { return std::begin(p); }
	auto end() { return std::end(p); }
	auto begin() const { return std::begin(p); }
	auto end() const { return std::end(p); }

private:
	PortalContext p[2];
};

struct CameraContext
{
	rvector Pos;
	rvector Dir;
	rvector Up;
	rfrustum Frustum;
};

struct RecursionContext
{
	int Depth = 0;
	PortalInfo* ViewingPortal;
	ArrayView<PortalContextPair> Portals;
	CameraContext Cam;
	const RecursionContext* Parent{};
};

class ValidPortalIterator
{
public:
	ValidPortalIterator(const PortalMap& PortalList, const PortalMap::iterator& it)
		: PortalList{ PortalList }, it{ it } {}

	ValidPortalIterator& operator++(int)
	{
		auto prev_it = *this;
		++*this;
		return prev_it;
	}

	ValidPortalIterator& operator++()
	{
		if (index == 0)
		{
			index = 1;
			return *this;
		}

		index = 0;

		do
		{
			it++;
		} while (it != PortalList.end() && !it->second.IsValid());

		return *this;
	}

	bool operator!=(const ValidPortalIterator& rhs) const { return it != rhs.it; }
	PortalInfo& operator*() { return it->second[index]; }

private:
	PortalMap::iterator it;
	int index = 0;
	const PortalMap& PortalList;
};

static bool LineOBBIntersection(const rmatrix &matWorld, const rmatrix &matInvWorld,
	const rvector &extents, const rvector &L1, const rvector &L2, rvector &Hit)
{
	rvector mins, maxs, l1, l2;

	mins = -extents;
	maxs = extents;

	l1 = L1 * matInvWorld;
	l2 = L2 * matInvWorld;

	float t;

	bool bRet = IntersectLineSegmentAABB(l1, l2, { mins, maxs }, &t);

	if (!bRet)
		return false;

	Hit = l1 + (Normalized(l2 - l1) * t);
	Hit = Hit * matWorld;

	return true;
}

static rmatrix MakeWorldMatrix(const v3& pos, const v3& dir, const v3& up) {
	rmatrix mat;
	MakeWorldMatrix(&mat, pos, dir, up);
	return mat;
}

static rmatrix MakeOrientationMatrix(const v3& dir, const v3& up) {
	return MakeWorldMatrix(v3{ 0, 0, 0 }, dir, up);
}

static rmatrix GetDefaultProjectionMatrix(float Near = DEFAULT_NEAR_Z, float Far = DEFAULT_FAR_Z) {
	return PerspectiveProjectionMatrixViewport(
		RGetScreenWidth(), RGetScreenHeight(),
		GetFOV(),
		Near, Far);
}

static rmatrix MakeObliquelyClippingProjectionMatrix(const rmatrix &matView,
	const rvector &p, const rvector &normal)
{
	rplane plane = PlaneFromPointNormal(p, normal);

	// We need the inverse transpose when transforming planes
	rmatrix WorldToView = Transpose(Inverse(matView));

	v4 projClipPlane;
	v4 vectorplane{ plane.a, plane.b, plane.c, plane.d };

	// Transform clip plane into view space
	projClipPlane = Transform(vectorplane, WorldToView);

	auto matProjection = GetDefaultProjectionMatrix();

	if (projClipPlane.w > 0)
	{
		projClipPlane = Transform(-vectorplane, WorldToView);
	}

	v4 q{
		(sgn(projClipPlane.x) + matProjection(2, 0)) / matProjection(0, 0),
		(sgn(projClipPlane.y) + matProjection(2, 1)) / matProjection(1, 1),
		-1,
		(1 + matProjection(2, 2)) / matProjection(3, 2),
	};

	auto c = projClipPlane * (1.0F / DotProduct(projClipPlane, q));
	
	// Copy c into the second column of the projection matrix
	for (int i = 0; i < 4; ++i)
		matProjection(i, 2) = c[i];

	return matProjection;
}

Portal::Portal()
{
	static constexpr std::pair<int, int> coords[] = {
		{ 200, 20 },
		{ 287, 50 },
		{ 363, 159 },
		{ 384, 372 },
		{ 290, 555 },
		{ 200, 586 },
		{ 118, 561 },
		{ 19, 388 },
		{ 35, 163 },
		{ 109, 49 },
	};

	static constexpr int NumVertices = std::size(coords);

	WorldSpaceTexVertex RectangleVertices[] = {
		{{-100, -150, 0}, 0, 0 },
		{{-100, 150, 0}, 0, 1},
		{{100, 150, 0}, 1, 1},
		{{100, -150, 0}, 1, 0},
	};

	u16 RectangleIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	RectangleMesh.Create(RectangleVertices, RectangleIndices);

	WorldSpaceTexVertex EdgeVertices[NumVertices];
	for (int i = 0; i < NumVertices; i++)
	{
		EdgeVertices[i].pos.x = float(coords[i].first - 200) / 400 * 200;
		EdgeVertices[i].pos.y = float(coords[i].second - 300) / 600 * 300;
		EdgeVertices[i].pos.z = 0;
		EdgeVertices[i].tu = float(coords[i].first) / 400;
		EdgeVertices[i].tv = float(coords[i].second) / 600;
	}

	u16 EdgeIndices[(NumVertices - 2) * 3];
	for (int i = 0; i < NumVertices - 2; i++)
	{
		EdgeIndices[i * 3] = 0;
		EdgeIndices[i * 3 + 1] = i + 1;
		EdgeIndices[i * 3 + 2] = i + 2;
	}

	EdgeMesh.Create(EdgeVertices, EdgeIndices);

	for (int i = 0; i < 2; i++)
	{
		char path[128];
		sprintf_safe(path, "Interface/default/portal%d.png", i + 1);
		PortalEdgeTex[i] = RBaseTexturePtr{ RCreateBaseTexture(path, RTextureType::Map) };
	}

	LastLClick = false;
	LastRClick = false;

	MyPortalInfo = 0;

	DontDraw = false;
	DontDrawChar = false;

	ForceProjection = false;
	LookingThroughPortal = false;

	if (!RIsStencilBuffer())
		MLog("Portal::Portal() - Your graphics card doesn't support stencil buffers;"
			"no portals will be drawn.\n");
}

Portal::~Portal() = default;
void Portal::OnLostDevice() {}
void Portal::OnResetDevice() {}

void Portal::RenderEdge(const PortalInfo& portalinfo)
{
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	rmatrix World;
	MakeWorldMatrix(&World,
		portalinfo.Pos + portalinfo.Normal * 0.5,
		portalinfo.Normal, portalinfo.Up);

	RSetTransform(D3DTS_WORLD, World);

	RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED |
		D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

	// Use alpha testing to draw only the edge
	RGetDevice()->SetRenderState(D3DRS_ALPHAREF, 0x00000001);
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	RGetDevice()->SetTexture(0, PortalEdgeTex[portalinfo.Index].get()->GetTexture());

	RectangleMesh.Draw();

	RGetDevice()->SetTexture(0, NULL);

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

static void GetIntersectedScreenLine(rvector &pos, rvector &dir)
{
	MPOINT Crosshair = ZGetCombatInterface()->GetCrosshairPoint();

	RGetScreenLine(Crosshair.x, Crosshair.y, &pos, &dir);

	rvector mypos = ZGetGame()->m_pMyCharacter->m_Position + rvector(0, 0, 100);
	rplane myplane = PlaneFromPointNormal(mypos, dir);

	rvector checkpos, checkposto = pos + 100000.f*dir;
	IntersectLineSegmentPlane(&checkpos, myplane, pos, checkposto);
}

void Portal::OnShot()
{
	int n = -1;

	{
		bool LClick = MEvent::IsKeyDown(VK_LBUTTON);
		bool RClick = MEvent::IsKeyDown(VK_RBUTTON);

		if (LClick && !LastLClick)
			n = 0;
		else if (RClick && !LastRClick)
			n = 1;
		else
		{
			LastLClick = LClick;
			LastRClick = RClick;
			return;
		}

		LastLClick = LClick;
		LastRClick = RClick;
	}

	if (!ZGetApplication()->IsDeveloperMode() &&
		ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_TRAINING)
		return;

	MMatchItemDesc *pDesc = ZGetGame()->m_pMyCharacter->GetSelectItemDesc();

	if (!pDesc)
		return;

	if (pDesc->m_nID != 8500) // Change this to something saner later
		return;
	
	rvector Pos, Dir;
	GetIntersectedScreenLine(Pos, Dir);

	ZPICKINFO zpi;
	if (!ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, Pos, Dir, &zpi,
		RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET) || !zpi.bBspPicked)
		return;

	rvector normal{ EXPAND_VECTOR(zpi.bpi.pInfo->plane) };
	rvector pos = zpi.bpi.PickPos + normal * 5;
	rvector up = abs(normal.z) != 1 ? rvector(0, 0, 1) : rvector(1, 0, 0);

	ZPostPortal(n, pos, normal, up);
}

bool Portal::RedirectPos(rvector &from, rvector &to)
{
	if (!PortalSetExists)
		return false;

	rvector hit;
	auto ppi = LineIntersection(from, to, &hit);
	if (!ppi)
		return false;

	from = hit * ppi->Transform;
	auto dir = Normalized(to - from) * ppi->Rot;
	to = from + dir * 10000;

	return true;
}

void Portal::DrawFakeCharacter(ZCharacter *pZChar)
{
	if (!PortalSetExists || DontDrawChar)
		return;

	PortalInfo *ppi;

	if (!CheckIntersection(pZChar->GetPosition(), 100, 190, &ppi))
		return;
	
	// The position affects the bounding box used for occlusion culling.
	auto& pos = pZChar->m_Position;
	// The world matrix affects the rendering of the character.
	auto& mat = pZChar->GetVisualMesh()->m_WorldMat;

	auto oldpos = pos;
	auto oldmat = mat;

	mat *= ppi->Transform;
	pos *= ppi->Transform;

	DontDrawChar = true;
	pZChar->Draw();
	DontDrawChar = false;

	pos = oldpos;
	mat = oldmat;
}

bool Portal::Move(ZObject *pObj, rvector &diff)
{
	if (!PortalSetExists)
		return false;

	rvector &origin = pObj->m_Position;
	rvector target = origin + diff;

	PortalInfo *ppi;

	if (!CheckIntersection(target, pObj->GetCollRadius(), pObj->GetCollHeight(), &ppi))
		return false;

	if (DotProduct(target, ppi->Normal) + ppi->d < 0)
	{
		origin = target * ppi->Transform;
		pObj->SetDirection(pObj->GetDirection() * ppi->Rot);
		pObj->SetVelocity(pObj->GetVelocity() * ppi->Rot);

		ZGetGameInterface()->GetGameInput()->lastanglex = ZGetCamera()->m_fAngleX;
		ZGetGameInterface()->GetGameInput()->lastanglez = ZGetCamera()->m_fAngleZ;

		RCameraPosition *= ppi->Transform;
		RCameraDirection *= ppi->Rot;
		RUpdateCamera();
	}
	else
		origin = target;

	return true;
}

bool Portal::Move(ZMovingWeapon & Obj, v3 & diff)
{
	if (!PortalSetExists)
		return false;

	v3& origin = Obj.m_Position;
	v3 target = origin + diff;

	PortalInfo* pi = LineIntersection(origin, target);

	if (!pi)
		return false;

	if (DotProduct(target, pi->Normal) + pi->d < 0)
	{
		origin *= pi->Transform;
		Obj.m_Dir *= pi->Rot;
		Obj.m_Velocity *= pi->Rot;
		diff *= pi->Rot;
	}

	return true;
}

struct RedirectCameraSubRet
{
	bool Intersection;
	v3 NewPos, NewDir, NewUp;
	bool MakeNearProjectionMatrix;
};

RedirectCameraSubRet Portal::RedirectCameraSub()
{
	RedirectCameraSubRet ret{};

	auto target = MyChar()->GetPosition() + ZCamera::GetTargetOffset(CameraDir, MyChar()->GetScale());

	bool InsidePortal = false;

	PortalInfo *ppi;
	if (ZObjectPortalIntersection(ZGetGame()->m_pMyCharacter, &ppi))
	{
		if (DotProduct(target - ZGetCamera()->m_fDist * CameraDir, ppi->Normal) + ppi->d < 0)
		{
			InsidePortal = true;
		}
	}

	if (InsidePortal)
	{
		ret.NewDir = CameraDir * ppi->Rot;
		ret.NewPos = target * ppi->Transform;
		ret.NewPos -= ZGetCamera()->m_fDist * ret.NewDir;

		ret.MakeNearProjectionMatrix = DotProduct(ret.NewPos, ppi->Other.Normal) + ppi->Other.d < 15;
	}
	else
	{
		auto dir = -ZGetCamera()->GetCurrentDir();
		auto idealpos = target + dir * ZGetCamera()->m_fDist;

		ZPICKINFO zpi;
		if (!(ZGetGame()->Pick(ZGetGame()->m_pMyCharacter, target, dir, &zpi,
			RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET, 0)
			&& zpi.bBspPicked
			&& Magnitude(target - zpi.bpi.PickPos) < ZGetCamera()->m_fDist)) {
			return{};
		}

		const auto& pick = zpi.bpi.PickPos;

		rvector hit;
		ppi = LineIntersection(target, pick, &hit);
		if (!ppi)
			return{};

		int axis = -1;

		for (int i = 0; i < 3; i++)
		{
			if (fabs(hit[i] - idealpos[i]) > 1)
			{
				axis = i;
				break;
			}
		}

		if (axis == -1)
			return{};

		auto diff = idealpos - target;
		auto distoutside = (idealpos[axis] - hit[axis]) / (idealpos[axis] - target[axis]) * Magnitude(diff);

		auto disp = hit * ppi->Transform;

		ret.NewDir = -dir * ppi->Rot;
		ret.NewPos = disp - ret.NewDir * distoutside;

		ret.MakeNearProjectionMatrix = distoutside < 15;
	}

	ret.NewUp = CameraUp * ppi->Rot;

	ret.Intersection = true;
	
	return ret;
}

void Portal::RedirectCamera()
{
	if(!PortalSetExists)
		return;

	MakeNearProjectionMatrix = false;
	LookingThroughPortal = false;

	CameraPos = RCameraPosition;
	CameraDir = RCameraDirection;
	CameraUp = RCameraUp;

	auto ret = RedirectCameraSub();
	if (!ret.Intersection)
		return;

	CameraPos = ret.NewPos;
	CameraDir = ret.NewDir;
	CameraUp = ret.NewUp;
	MakeNearProjectionMatrix = ret.MakeNearProjectionMatrix;

	auto View = ViewMatrix(CameraPos, CameraDir, CameraUp);
	RSetTransform(D3DTS_VIEW, View);

	LookingThroughPortal = true;

	RViewFrustum = MakeViewFrustum(
		View,
		CameraPos, CameraDir,
		GetFOV(), ComputeVerticalFOV(GetFOV(), RGetAspect()),
		MakeNearProjectionMatrix ? 0.1 : 5.0, g_fFarZ);
}

PortalInfo* Portal::LineIntersection(const v3& l1, const v3& l2, v3* hit)
{
	for (auto& pi : ValidPortals())
	{
		v3 vhit;
		if (LinePortalIntersection(l1, l2, pi, vhit))
		{
			if (hit)
				*hit = vhit;
			return &pi;
		}
	}

	return nullptr;
}

bool Portal::CheckIntersection(const rvector &target, float fRadius, float fHeight, PortalInfo **pppi)
{
	rvector mins = target - rvector(fRadius, fRadius, 0);
	rvector maxs = target + rvector(fRadius, fRadius, fHeight);

	for (auto &pair : PortalList)
	{
		for (int i = 0; i < 2; i++)
		{
			auto& portalinfo = pair.second[i];

			if (AABBPortalIntersection(mins, maxs, portalinfo))
			{
				if(pppi)
					*pppi = &portalinfo;

				return true;
			}
		}
	}

	return false;
}

bool Portal::ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi)
{
	return CheckIntersection(pObj->m_Position, pObj->GetCollRadius(), pObj->GetCollHeight(), retppi);
}

bool Portal::LinePortalIntersection(const rvector &L1, const rvector &L2,
	const PortalInfo &portalinfo, rvector &Hit) const
{
	return LineOBBIntersection(portalinfo.World, portalinfo.InvWorld, PortalExtents, L1, L2, Hit);
}

bool Portal::AABBPortalIntersection(const rvector &B1, const rvector &B2, const PortalInfo &ppi) const
{
	return IsIntersect(ppi.BoundingBox, { B1, B2 });
}

void Portal::DeletePlayer(ZCharacter *pZChar)
{
	auto it = PortalList.find(pZChar);
	if(it != PortalList.end())
		PortalList.erase(it);
}

static rfrustum MakePortalFrustum(const PortalInfo& pi, const v3& CamPos)
{
	return{
		PlaneFromPoints(CamPos, pi.TopLeft, pi.TopRight),
		PlaneFromPoints(CamPos, pi.TopRight, pi.BottomRight),
		PlaneFromPoints(CamPos, pi.BottomRight, pi.BottomLeft),
		PlaneFromPoints(CamPos, pi.BottomLeft, pi.TopLeft),
		pi.Near,
		pi.Far
	};
}

void Portal::Update(RecursionContext& rc)
{
	for (auto&& p : rc.Portals)
	{
		for (auto&& Context : p)
		{
			const auto& pi = Context.pi;

			Context.IsVisible = isInViewFrustum(pi.BoundingBox, rc.Cam.Frustum) &&
				DotProduct(pi.Normal, rc.Cam.Pos) + pi.d > 0;

			Context.Frustum = MakePortalFrustum(pi, rc.Cam.Pos * pi.Other.Transform);
		}
	}
}

struct ContextSaver {
	rmatrix World, View, Projection;
	rfrustum Frustum;

	ContextSaver()
	{
		World = RGetTransform(D3DTS_WORLD);
		View = RGetTransform(D3DTS_VIEW);
		Projection = RGetTransform(D3DTS_PROJECTION);

		Frustum = RViewFrustum;
	}

	~ContextSaver()
	{
		RSetTransform(D3DTS_WORLD, World);
		RSetTransform(D3DTS_VIEW, View);
		RSetTransform(D3DTS_PROJECTION, Projection);

		RViewFrustum = Frustum;
	}
};

void Portal::RenderPortals(const RecursionContext& rc)
{
	ContextSaver Orig;

	auto& Contexts = rc.Portals;

	for (size_t ctx = 0; ctx < Contexts.size(); ctx++)
	{
		auto RenderPortal = [&](int i)
		{
			const auto& Context = Contexts[ctx][i];
			const auto& portalinfo = Context.pi;

			if (!Context.IsVisible)
				return;

			if (rc.Depth > 1)
			{
				MLog("Depth %d, %d visible\n", rc.Depth, i);
			}

			RSetTransform(D3DTS_WORLD, portalinfo.World);

			// Draw black surface
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

			// Set color argument to black constant color
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0);

			EdgeMesh.Draw();

			// Reset color argument to texture
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			// Write portal outline to stencil buffer
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_STENCILREF, 0);
			RGetDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER);
			RGetDevice()->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR);
			RGetDevice()->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			// Turn off color and depth writing. Only interested in the stencil.
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			EdgeMesh.Draw();

			// The stencil of the portal outline is now written to the buffer.
			// Now, draw the game world into the stencil.

			// Turn color and depth writing back on.
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED |
				D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

			// Enable stencil usage
			RGetDevice()->SetRenderState(D3DRS_STENCILREF, 1);
			RGetDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
			RGetDevice()->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			RGetDevice()->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

			// World
			RSetTransform(D3DTS_WORLD, IdentityMatrix());

			// View
			auto pos = rc.Cam.Pos * portalinfo.Transform;
			auto dir = rc.Cam.Dir * portalinfo.Rot;
			auto up = rc.Cam.Up * portalinfo.Rot;

			auto mView = ViewMatrix(pos, dir, up);
			RSetTransform(D3DTS_VIEW, mView);

			// Projection
			auto mProjection = MakeObliquelyClippingProjectionMatrix(mView,
				portalinfo.Other.Pos, portalinfo.Other.Normal);
			RSetTransform(D3DTS_PROJECTION, mProjection);

			RViewFrustum = Context.Other.Frustum;

			ForceProjection = true;
			DontDraw = true;

			ZGetGame()->Draw();

			DontDraw = false;
			ForceProjection = false;

			// Clear the stencil buffer since the next portal
			// will be drawn in a different place.
			RGetDevice()->Clear(0, nullptr, D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);

			// Disable stencil usage.
			RGetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		};

		int PortalIndex = 0;

		// The furthest away portal needs to be drawn first, so that
		// it doesn't overwrite the image of the nearby one.
		if (Magnitude(Contexts[ctx][1].pi.Pos - rc.Cam.Pos) >
			Magnitude(Contexts[ctx][0].pi.Pos - rc.Cam.Pos)) {
			PortalIndex = 1;
		}

		RenderPortal(PortalIndex);

		RSetTransform(D3DTS_VIEW, Orig.View);
		RSetTransform(D3DTS_PROJECTION, Orig.Projection);

		RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		RenderPortal(!PortalIndex);
	}
}

void Portal::WriteDepth(const RecursionContext& rc)
{
	ContextSaver Orig;

	RGetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f);

	for (const auto& pair : rc.Portals)
	{
		for (int i = 0; i < 2; i++)
		{
			const auto& pc = pair[i];

			if (!pc.IsVisible)
				continue;

			const auto& pi = pc.pi;

			RSetTransform(D3DTS_WORLD, pi.World);

			RGetDevice()->SetTexture(0, nullptr);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

			EdgeMesh.Draw();

			RGetDevice()->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED |
				D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		}

		for (int i = 0; i < 2; i++)
		{
			if (pair[i].IsVisible)
				RenderEdge(pair[i].pi);
		}
	}
}

static void LogPortal(const PortalInfo& pi)
{
#define LOG DMLog
#define LOG_VECTOR(name) LOG(#name " = %f, %f, %f\n", EXPAND_VECTOR(pi.name))
#define LOG_MATRIX(name) LOG("Matrix " #name "\n"); LogMatrix(pi.name)
#define LOG_PLANE(name) LOG(#name " = %f, %f, %f, %f\n", pi.name.a, pi.name.b, pi.name.c, pi.name.d)

	LOG("Portal %d\n", pi.Index);

	LOG_VECTOR(Pos);
	LOG_VECTOR(Normal);
	LOG_VECTOR(Up);
	LOG("d = %f\n", pi.d);

	LOG_MATRIX(World);
	LOG_MATRIX(InvWorld);
	LOG_MATRIX(Rot);
	LOG_MATRIX(Transform);

	LOG_PLANE(Near);
	LOG_PLANE(Far);

	LOG_VECTOR(TopRight);
	LOG_VECTOR(TopLeft);
	LOG_VECTOR(BottomRight);
	LOG_VECTOR(BottomLeft);

	LOG_VECTOR(BoundingBox.vmin);
	LOG_VECTOR(BoundingBox.vmax);

#undef LOG
#undef LOG_VECTOR
#undef LOG_MATRIX
#undef LOG_PLANE
}

void Portal::CreatePortal(ZCharacter *Owner, int n, const rvector &Pos,
	const rvector &Normal, const rvector &Up)
{
	auto& portalpair = PortalList[Owner];
	auto& pi = portalpair[n];

	pi.Pos = Pos;
	pi.Normal = Normal;
	pi.Up = Up;

	pi.d = -DotProduct(pi.Normal, pi.Pos);

	pi.World = MakeWorldMatrix(pi.Pos, pi.Normal, pi.Up);
	pi.InvWorld = Inverse(pi.World);

	for (auto&& pi : portalpair)
	{
		auto UnrotateThis = Inverse(MakeOrientationMatrix(pi.Normal, pi.Up));
		auto RotateToOther = MakeOrientationMatrix(-pi.Other.Normal, pi.Other.Up);

		pi.Rot = UnrotateThis * RotateToOther;

		auto UntranslateThis = TranslationMatrix(-pi.Pos);
		auto TranslateToOther = TranslationMatrix(pi.Other.Pos);

		pi.Transform = UntranslateThis * pi.Rot * TranslateToOther;
	}

	pi.Set = true;

	if (pi.Set && pi.Other.Set)
		PortalSetExists = true;

	pi.TopRight = PortalExtents * pi.World;
	pi.TopLeft = rvector(-PortalExtents.x, PortalExtents.y, PortalExtents.z) * pi.World;
	pi.BottomLeft = rvector(-PortalExtents.x, -PortalExtents.y, PortalExtents.z) * pi.World;
	pi.BottomRight = rvector(PortalExtents.x, -PortalExtents.y, PortalExtents.z) * pi.World;

	pi.BoundingBox.vmin = pi.BoundingBox.vmax = pi.TopRight;

	for (auto&& PortalCorner : { pi.TopRight, pi.BottomLeft })
	{
		for (int i = 0; i < 3; ++i)
		{
			pi.BoundingBox.vmin[i] = std::min(pi.BoundingBox.vmin[i], PortalCorner[i]);
			pi.BoundingBox.vmax[i] = std::max(pi.BoundingBox.vmax[i], PortalCorner[i]);
		}
	}

	pi.Near = PlaneFromPointNormal(pi.Pos, pi.Normal);
	pi.Far  = PlaneFromPointNormal(pi.Pos + pi.Normal * g_fFarZ, -pi.Normal);

	if (pi.Set && pi.Other.Set)
	{
		LogPortal(portalpair[0]);
		LogPortal(portalpair[1]);
	}
}

struct FrustraDrawerType
{
	bool Enabled{};
	bool Lock{};
	v3 CamPositions[2]{};
	v3 LineEndpoints[2][4]{};

	void CalcLines(int index, const v3& CamPos, const v3& PortalDir, const rfrustum& Frustum)
	{
		CamPositions[index] = CamPos;

		for (int i = 0; i < 4; ++i)
		{
			auto&& a = Frustum[i];
			auto&& b = Frustum[(i + 1) % 4];
			v3 dir, pos;
			auto success = GetIntersectionOfTwoPlanes(&dir, &pos, a, b);
			if (!success)
			{
				DMLog("failed intersection between {%f, %f, %f, %f} and {%f, %f, %f, %f}\n",
					a.a, a.b, a.c, a.d,
					b.a, b.b, b.c, b.d);
			}

			if (DotProduct(dir, PortalDir) < 0)
				dir = -dir;
			LineEndpoints[index][i] = CamPos + dir * 1000;
		}
	}

	void Update(const RecursionContext& rc)
	{
		if (!Enabled || Lock)
			return;

		for (auto& pair : rc.Portals)
		{
			for (int PortalIndex = 0; PortalIndex < 2; ++PortalIndex)
			{
				auto&& portal = pair[PortalIndex];
				auto CamPos = rc.Cam.Pos * portal.pi.Other.Transform;

				for (auto&& end : LineEndpoints[PortalIndex])
				{
					CalcLines(PortalIndex, CamPos, portal.pi.Normal, portal.Frustum);
				}
			}
		}
	}

	void Draw(PortalPair& pair)
	{
		if (!Enabled)
			return;

		for (int PortalIndex = 0; PortalIndex < 2; ++PortalIndex)
		{
			RSetTransform(D3DTS_WORLD, IdentityMatrix());
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
			RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

			u32 Color = PortalIndex ? 0xFF0000FF : 0xFFFF0000;
			auto Draw = [&](const v3& vec)
			{
				auto&& CamPos = CamPositions[PortalIndex];
				auto Dir = Normalized(vec - CamPos);
				RDrawLine(CamPos, CamPos + Dir * 10000, Color);
			};

			for (auto&& end : LineEndpoints[PortalIndex])
			{
				Draw(end);
			}

			Color = 0xFF00FF00;

			auto& pi = pair[PortalIndex];
			Draw(pi.TopLeft);
			Draw(pi.TopRight);
			Draw(pi.BottomLeft);
			Draw(pi.BottomRight);
		}
	}
};

static FrustraDrawerType FrustraDrawer;

void SetPortalFrustraDrawLock(bool Value) { FrustraDrawer.Lock = Value; }
bool GetPortalFrustraDrawLock() { return FrustraDrawer.Lock; }
void SetPortalFrustraDrawEnabled(bool Value) { FrustraDrawer.Enabled = Value; }
bool GetPortalFrustraDrawEnabled() { return FrustraDrawer.Enabled; }

void Portal::UpdateAndRender(const RecursionContext* PrevContext)
{
	if (!PrevContext)
		RecursionCount = 0;

	if (RecursionCount >= 1)
		return;

	RecursionCount++;

	auto NumContexts = PortalList.size();

	auto it = PortalList.begin();
	auto Contexts = MAKE_STACK_ARRAY(PortalContextPair, NumContexts,
		[&](PortalContextPair* p, size_t pos) {
			new (p) PortalContextPair{ it->second };
			it++;
	});

	RecursionContext rc;
	rc.Depth = RecursionCount;
	rc.Portals = ArrayView<PortalContextPair>{ Contexts.get(), NumContexts };
	rc.Parent = PrevContext;

	CameraContext CurCamera;
	if (!PrevContext)
	{
		CurCamera.Pos = CameraPos;
		CurCamera.Dir = CameraDir;
		CurCamera.Up = CameraUp;
		CurCamera.Frustum = RViewFrustum;
	}
	else
	{
		CurCamera = PrevContext->Cam;
	}

	rc.Cam = CurCamera;

	auto Run = [this](RecursionContext& rc)
	{
		rmatrix OrigView;
		rmatrix View;
		rfrustum OrigFrustum;

		if (rc.Depth > 1)
		{
			OrigFrustum = RViewFrustum;

			OrigView = RGetTransform(D3DTS_VIEW);

			auto pos = rc.Cam.Pos;
			auto dir = rc.Cam.Dir;

			rvector up = rc.ViewingPortal->Up;

			View = ViewMatrix(pos, dir, up);
		}

		Update(rc);

		UpdateAndRender(&rc);

		if (rc.Depth > 1)
		{
			RSetTransform(D3DTS_VIEW, View);
		}

		RenderPortals(rc);

		WriteDepth(rc);

		if (rc.Depth > 1)
		{
			RSetTransform(D3DTS_VIEW, OrigView);

			RViewFrustum = OrigFrustum;
		}

		FrustraDrawer.Update(rc);
	};

	if (!PrevContext)
	{
		Run(rc);
		return;
	}

	for (size_t i = 0; i < Contexts.size(); i++)
	{
		for (int PortalIndex = 0; PortalIndex < 2; PortalIndex++)
		{
			rc.ViewingPortal = &Contexts[i][PortalIndex].pi;

			if (!rc.Portals[i][PortalIndex].IsVisible)
				continue;

			rc.Cam.Pos = CurCamera.Pos * rc.ViewingPortal->Transform;
			rc.Cam.Dir = CurCamera.Dir * rc.ViewingPortal->Rot;
			rc.Cam.Up = CurCamera.Up * rc.ViewingPortal->Rot;

			rc.Cam.Frustum = MakePortalFrustum(rc.ViewingPortal->Other, rc.Cam.Pos);

			Run(rc);
		}
	}
}

Range<ValidPortalIterator> Portal::ValidPortals() {
	return MakeRange(
		ValidPortalIterator{ PortalList, PortalList.begin() },
		ValidPortalIterator{ PortalList, PortalList.end() });
}

void Portal::PreDraw()
{
	if (!PortalSetExists || DontDraw)
		return;

	if (!RIsStencilBuffer())
		return;

	RedirectCamera();

	UpdateAndRender();

	if (MakeNearProjectionMatrix)
	{
		rmatrix mProjection = GetDefaultProjectionMatrix(0.01);
		RSetTransform(D3DTS_PROJECTION, mProjection);
		ForceProjection = true;
	}
	else
	{
		if (LookingThroughPortal)
		{
			rmatrix mProjection = GetDefaultProjectionMatrix();
			RSetTransform(D3DTS_PROJECTION, mProjection);
			ForceProjection = true; // Setting the projection again would reset the frustum
		}
		else
		{
			ForceProjection = false;
		}
	}
}

void Portal::PostDraw()
{
	if (DontDraw || !PortalSetExists || !RIsStencilBuffer())
		return;

	if (!PortalList.empty())
		FrustraDrawer.Draw(PortalList.begin()->second);
}

#endif