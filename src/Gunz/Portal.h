#pragma once
#include "Config.h"
#ifdef PORTAL
#include "RTypes.h"
#include <unordered_map>
#include "ArrayView.h"
#include "SimpleMesh.h"
#include "VertexTypes.h"

class ZCharacter;
struct PortalInfo;
struct PortalPair;
struct RecursionContext;
class ValidPortalIterator;
struct RedirectCameraSubRet;
using PortalMap = std::unordered_map<ZCharacter *, PortalPair>;

class Portal
{
public:
	Portal();
	Portal(const Portal&) = delete;
	~Portal();

	void OnLostDevice();
	void OnResetDevice();

	void OnShot();
	bool RedirectPos(rvector &from, rvector &to);

	void DeletePlayer(ZCharacter *pZChar);

	void DrawFakeCharacter(ZCharacter *pZChar);

	bool Move(ZObject *pObj, rvector &diff);
	bool Move(ZMovingWeapon& Obj, v3& diff);

	void PreDraw();
	void PostDraw();

	void CreatePortal(ZCharacter *Owner, int Portal, const rvector &Pos, const rvector &Normal, const rvector &Up);

	bool ForcingProjection() const { return ForceProjection; }
	bool IsDrawingFakeChar() const { return DontDrawChar; }

private:
	void UpdateAndRender(const RecursionContext* PrevContext = nullptr);

	void Update(RecursionContext& rc);
	void RenderPortals(const RecursionContext& rc);

	void WriteDepth(const RecursionContext& rc);

	void RenderEdge(const PortalInfo& portalinfo);

	void RedirectCamera();
	RedirectCameraSubRet RedirectCameraSub();

	bool ZObjectPortalIntersection(const ZObject *pObj, PortalInfo **retppi);
	bool LinePortalIntersection(const rvector &L1, const rvector &L2, const PortalInfo &ppi, rvector &Hit) const;
	bool AABBPortalIntersection(const rvector &B1, const rvector &B2, const PortalInfo &ppi) const;

	PortalInfo* LineIntersection(const v3& l1, const v3& l2, v3 *hit = nullptr);
	bool CheckIntersection(const rvector &target, float fRadius, float fHeight, PortalInfo **retppi);

	Range<ValidPortalIterator> ValidPortals();

	using MeshType = SimpleMesh<WorldSpaceTexVertex, u16, WorldSpaceTexFVF>;
	MeshType RectangleMesh;
	MeshType EdgeMesh;
	RBaseTexturePtr PortalEdgeTex[2];

	PortalMap PortalList;

	PortalInfo* MyPortalInfo{};

	bool MakeNearProjectionMatrix{};
	bool LookingThroughPortal{};

	bool PortalSetExists{};

	rvector CameraPos, CameraDir, CameraUp;

	bool LastLClick{};
	bool LastRClick{};

	static const rvector PortalExtents;

	bool DontDraw{};
	bool DontDrawChar{};
	bool ForceProjection{};

	int RecursionCount{};
	int LastFrameCount{};
};

extern std::unique_ptr<Portal> g_pPortal;
#endif