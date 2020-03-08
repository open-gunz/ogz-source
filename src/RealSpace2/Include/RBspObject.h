#pragma once

#include <cstdio>
#include <list>
#include <array>

#include "MUtil.h"
#include "RTypes.h"
#include "RLightList.h"
#include "RSolidBsp.h"
#include "RMaterialList.h"
#include "RNavigationMesh.h"
#include "ROcclusionList.h"
#include "RAnimationMgr.h"
#include "RDummyList.h"
#include "rapidxml.hpp"
#include "LightmapGenerator.h"
#ifdef _WIN32
#include "RMeshMgr.h"
#include "VulkanMaterial.h"
#include "RBspObjectDraw.h"
#include "RVisualMesh.h"

#define BSP_FVF	(D3DFVF_XYZ | D3DFVF_TEX2)
#define BSP_NORMAL_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
#define LIGHT_BSP_FVF (D3DFVF_XYZ | D3DFVF_TEX2 | D3DFVF_DIFFUSE)
#endif

class MZFile;
class MZFileSystem;
class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RMATERIAL;
class RMaterialList;
class RDummyList;
class RBaseTexture;
struct RSBspNode;
class BulletCollision;

struct RDEBUGINFO {
	int nCall, nPolygon;
	int nMapObjectFrustumCulled;
	int nMapObjectOcclusionCulled;
	RSolidBspNode* pLastColNode{};
};

struct BSPVERTEX {
	float x, y, z;  // World position
	float tu1, tv1; // Texture coordinates for diffuse map
	float tu2, tv2; // Texture coordinates for lightmap

	rvector *Coord() { return (rvector*)&x; }
};

struct BSPNORMALVERTEX {
	v3 Position;
	v3 Normal;
	v2 TexCoord0;
	v2 TexCoord1;
};

struct RPOLYGONINFO {
	rplane plane;
	int	nMaterial;
	int	nConvexPolygon;
	int	nLightmapTexture;
	int	nPolygonID;
	u32 dwFlags;

	BSPVERTEX *pVertices;
	int		nVertices;
	int		nIndicesPos;
};

struct RCONVEXPOLYGONINFO {
	rplane	plane;
	rvector *pVertices;
	rvector *pNormals;
	int	nVertices;
	int	nMaterial;
	float fArea;
	u32 dwFlags;
};

struct ROBJECTINFO {
	std::string name;
	int nMeshID{};
#ifdef _WIN32
	std::unique_ptr<RVisualMesh> pVisualMesh;
#endif
	RLIGHT* pLight{};
	float fDist{};
};

struct RBSPPICKINFO {
	RSBspNode* pNode;
	int nIndex;
	rvector PickPos;
	RPOLYGONINFO* pInfo;
};

using RMapObjectList = std::vector<ROBJECTINFO>;

struct RDrawInfo {
	RDrawInfo() = default;
	RDrawInfo(RDrawInfo&& src) = delete;
	~RDrawInfo() {
		SAFE_DELETE(pVertices);
		SAFE_DELETE(pPlanes);
		SAFE_DELETE(pUAxis);
		SAFE_DELETE(pVAxis);
	}

	int	nVertice = 0;
	union
	{
		BSPVERTEX* pVertices = nullptr;
		BSPNORMALVERTEX* pNormalVertices;
	};
	int	nIndicesOffset = 0;
	int	nTriangleCount = 0;
	rplane * pPlanes = nullptr;
	rvector * pUAxis = nullptr;
	rvector * pVAxis = nullptr;
};

struct RSBspNode
{
	int				nPolygon;
	RPOLYGONINFO	*pInfo;
	RDrawInfo		*pDrawInfo;

	int				nFrameCount;

	RSBspNode		*m_pPositive, *m_pNegative;

	rplane plane;
	rboundingbox	bbTree;

	RSBspNode();
	~RSBspNode();

	RSBspNode *GetLeafNode(const rvector &pos);
	void DrawWireFrame(int nFace, u32 color);
	void DrawBoundingBox(u32 color);
};

struct RBSPMATERIAL : public RMATERIAL {
#ifdef _WIN32
	union
	{
		RBaseTexture *texture = nullptr;
		VulkanMaterial VkMaterial;
	};
#endif

	RBSPMATERIAL() = default;
	RBSPMATERIAL(RMATERIAL *mat)
	{
		Ambient = mat->Ambient;
		Diffuse = mat->Diffuse;
		DiffuseMap = mat->DiffuseMap;
		dwFlags = mat->dwFlags;
		Name = mat->Name;
		Power = mat->Power;
		Specular = mat->Specular;
	};

	~RBSPMATERIAL();
};

struct FogInfo
{
	bool bFogEnable;
	u32 dwFogColor;
	float fNear;
	float fFar;
	FogInfo() { bFogEnable = false; }
};

struct AmbSndInfo
{
	int itype;
	char szSoundName[64];
	rvector min;
	rvector center;
	rvector max;
	float radius;
};

#define AS_AABB		0x01
#define AS_SPHERE	0x02
#define AS_2D		0x10
#define AS_3D		0x20

using RGENERATELIGHTMAPCALLBACK = bool(*)(float fProgress);

struct PickInfo;
struct BspCounts;

class RBspObject
{
public:
	enum class ROpenMode {
		Runtime,
		Editor
	} m_OpenMode;

	RBspObject(bool PhysOnly = false);
	RBspObject(const RBspObject&) = delete;
	RBspObject(RBspObject&&) = default;
	~RBspObject();

	void ClearLightmaps();

	bool Open(const char *, ROpenMode nOpenFlag = ROpenMode::Runtime,
		RFPROGRESSCALLBACK pfnProgressCallback = nullptr,
		void *CallbackParam = nullptr, bool PhysOnly = false);

	bool OpenDescription(const char *);
	bool OpenRs(const char *, BspCounts&);
	bool OpenBsp(const char *, const BspCounts&);
	bool OpenLightmap();
	bool OpenCol(const char *);
	bool OpenNav(const char *);

	void OptimizeBoundingBox();

	bool IsVisible(const rboundingbox &bb) const;

#ifdef _WIN32
	bool Draw();
	void DrawObjects();

	bool DrawLight(RSBspNode *pNode, int nMaterial);
	void DrawLight(D3DLIGHT9 *pLight);
#endif

	bool GenerateLightmap(const char *filename, int nMaxLightmapSize, int nMinLightmapSize, int nSuperSample,
		float fToler, v3 AmbientLight, RGENERATELIGHTMAPCALLBACK pProgressFn = nullptr);

	void SetWireframeMode(bool bWireframe) { m_bWireframe = bWireframe; }
	bool GetWireframeMode() { return m_bWireframe; }
	void SetShowLightmapMode(bool bShowLightmap) { m_bShowLightmap = bShowLightmap; }
	bool GetShowLightmapMode() { return m_bShowLightmap; }

	bool Pick(const rvector &pos, const rvector &dir, RBSPPICKINFO *pOut,
		u32 dwPassFlag = DefaultPassFlag);
	bool PickTo(const rvector &pos, const rvector &to, RBSPPICKINFO *pOut,
		u32 dwPassFlag = DefaultPassFlag);;
	bool PickOcTree(const rvector &pos, const rvector &dir, RBSPPICKINFO *pOut,
		u32 dwPassFlag = DefaultPassFlag);

	u32 GetLightmap(rvector &Pos, RSBspNode *pNode, int nIndex);

	RBSPMATERIAL *GetMaterial(RSBspNode *pNode, int nIndex) {
		return GetMaterial(pNode->pInfo[nIndex].nMaterial); }

	int	GetMaterialCount() const { return Materials.size(); }
	RBSPMATERIAL *GetMaterial(int nIndex);

	RMapObjectList* GetMapObjectList() { return &m_ObjectList; }
	RDummyList* GetDummyList() { return &m_DummyList; }
	RBaseTexture* GetBaseTexture(int n);

	RLightList& GetMapLightList() { return StaticMapLightList; }
	RLightList& GetObjectLightList() { return StaticObjectLightList; }
	RLightList& GetSunLightList() { return StaticSunLightList; }

	RSBspNode* GetOcRootNode() { return OcRoot.empty() ? nullptr : OcRoot.data(); }
	RSBspNode* GetRootNode() { return BspRoot.empty() ? nullptr : BspRoot.data(); }

	rvector GetDimension() const;

	int	GetVertexCount() const { return OcVertices.size(); }
	int	GetPolygonCount() const { return OcInfo.size(); }
	int GetNodeCount() const { return OcRoot.size(); }
	int	GetBspPolygonCount() const { return BspInfo.size(); }
	int GetBspNodeCount() const { return BspRoot.size(); }
	int GetConvexPolygonCount() const { return ConvexPolygons.size(); }
#ifdef _WIN32
	int GetLightmapCount() const { return LightmapTextures.size(); }
#endif

	// TODO: Make a separate output parameter
	bool CheckWall(const rvector &origin, rvector &targetpos, float fRadius, float fHeight = 0.f,
		RCOLLISIONMETHOD method = RCW_CYLINDER, int nDepth = 0, rplane *pimpactplane = nullptr);

	bool CheckSolid(const rvector &pos, float fRadius, float fHeight = 0.f,
		RCOLLISIONMETHOD method = RCW_CYLINDER);

	rvector GetFloor(const rvector &origin, float fRadius, float fHeight, rplane *pimpactplane = nullptr);

	void OnInvalidate();
	void OnRestore();

	void SetObjectLight(const rvector& pos);

	bool GetShadowPosition(const rvector& pos_, const rvector& dir_, rvector* outNormal_, rvector* outPos_);

#ifdef _WIN32
	auto* GetMeshManager() { return &m_MeshList; }
#endif

	void test_MakePortals();

	void DrawBoundingBox();
	void DrawOcclusions();
	void DrawNormal(int nIndex, float fSize = 1.f);

	void DrawCollision_Polygon();
	void DrawCollision_Solid();

	void DrawSolid();
	void DrawSolidNode();
	void DrawColNodePolygon(const rvector &pos);

	void DrawNavi_Polygon();
	void DrawNavi_Links();

	RSolidBspNode* GetColRoot() { return &ColRoot[0]; }

	void LightMapOnOff(bool b);
	static void SetDrawLightMap(bool b);

	FogInfo GetFogInfo() { return m_FogInfo; }
	std::vector<AmbSndInfo>& GetAmbSndList() { return AmbSndInfoList; }

	void GetNormal(int nConvexPolygon, const rvector &position, rvector *normal);

	static bool CreateShadeMap(const char *szShadeMap);
	static void DestroyShadeMap();

	RDEBUGINFO *GetDebugInfo() { return &m_DebugInfo; }
	RNavigationMesh* GetNavigationMesh() { return &m_NavigationMesh; }

	void SetMapObjectOcclusion(bool b) { m_bNotOcclusion = b; }

#ifdef _WIN32
	u32 GetFVF() const { return RenderWithNormal ? BSP_NORMAL_FVF : BSP_FVF; }
#endif
	size_t GetStride() const { return RenderWithNormal ? sizeof(BSPNORMALVERTEX) : sizeof(BSPVERTEX); }

	void UpdateUBO();

#ifdef _WIN32
	RBspObjectDraw DrawObj;
#endif

private:
#ifdef _WIN32
	friend class RBspObjectDrawVulkan;
	friend class RBspObjectDrawD3D9;
#endif
	friend struct LightmapGenerator;

	bool LoadRS2Map(rapidxml::xml_node<>&);
	bool LoadRS3Map(rapidxml::xml_node<>&, const std::string& directory);

#ifdef _WIN32
	void Draw(RSBspNode *Node, int Material);
	void DrawNoTNL(RSBspNode *Node, int Material);
#endif

	template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags>
	void DrawNodes(int LoopCount);
	template <u32 Flags, bool ShouldHaveFlags, bool SetAlphaTestFlags, bool SetTextures>
	void DrawNodesImpl(int LoopCount);

	void SetDiffuseMap(int nMaterial);

	bool PickShadow(const rvector &pos, const rvector &to, RBSPPICKINFO *pOut);

	template <bool Shadow = false>
	bool Pick(RSBspNode *pNode, const rvector &v0, const rvector &v1, PickInfo&);
	template <bool Shadow>
	bool CheckLeafNode(RSBspNode* pNode, const v3& v0, const v3& v1, PickInfo&);
	template <bool Shadow>
	bool CheckBranches(RSBspNode* pNode, const v3& v0, const v3& v1, PickInfo&);

	template <bool Shadow = false>
	bool Pick(std::vector<RSBspNode>& Nodes,
		const v3& src, const v3& dest, const v3& dir,
		u32 PassFlag, RBSPPICKINFO* Out);

	template <bool UseOccluders>
	void ChooseNodes(RSBspNode *bspNode, const rfrustum& LocalViewFrustum);
	int ChooseNodes(RSBspNode *bspNode, const rvector &center, float fRadius);

	auto* GetLeafNode(const rvector &pos) { return BspRoot[0].GetLeafNode(pos); }

	bool ReadString(MZFile *pfile, char *buffer, int nBufferSize);
	struct OpenNodesState Open_Nodes(RSBspNode *pNode, MZFile *pfile, OpenNodesState State);
	// Returns number of nodes created.
	int Open_ColNodes(RSolidBspNode *pNode, MZFile *pfile, int Depth = 0);
	bool Open_MaterialList(rapidxml::xml_node<>&);
	bool Open_LightList(rapidxml::xml_node<>&);
	bool Open_ObjectList(rapidxml::xml_node<>&);
	bool Open_DummyList(rapidxml::xml_node<>&);
	bool Open_ConvexPolygons(MZFile *pfile);
	bool Open_OcclusionList(rapidxml::xml_node<>&);
	bool Make_LenzFalreList();
	bool Set_Fog(rapidxml::xml_node<>&);
	bool Set_AmbSound(rapidxml::xml_node<>&);

	void CreatePolygonTable();
	void CreatePolygonTable(RSBspNode *pNode, u16** Indices);
	void Sort_Nodes(RSBspNode *pNode);

	bool CreateVertexBuffer();
	bool UpdateVertexBuffer();

	bool CreateIndexBuffer();
	bool UpdateIndexBuffer();

	bool CreateDynamicLightVertexBuffer();
	void InvalidateDynamicLightVertexBuffer();
	bool FlushLightVB();
	bool LockLightVB();

	static constexpr u32 DefaultPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE;

#ifdef _WIN32
	D3DPtr<IDirect3DVertexBuffer9> DynLightVertexBuffer;
#endif

	static RBaseTexture *m_pShadeMap;

	// BSP stuff. Used in Pick.
	std::vector<BSPVERTEX> BspVertices;
	std::vector<RSBspNode> BspRoot;
	std::vector<RPOLYGONINFO> BspInfo;

	// Rendering stuff.
	std::vector<BSPVERTEX> OcVertices;
	std::vector<BSPNORMALVERTEX> OcNormalVertices;
	std::vector<u16> OcIndices;
	std::vector<RSBspNode> OcRoot;
	std::vector<RPOLYGONINFO> OcInfo;

#ifdef _WIN32
	// Vertex and index buffer objects
	D3DPtr<IDirect3DVertexBuffer9> VertexBuffer;
	D3DPtr<IDirect3DIndexBuffer9> IndexBuffer;
#endif

	// Stores material data, i.e. the stuff drawn onto geometry.
	// The first index is special: It's an untextured material
	// that has only a white diffuse color. It is used for materials
	// whose material index didn't map to a valid material.
	// Thus, every material index in the map file maps to
	// the index in this array that is one higher.
	std::vector<RBSPMATERIAL> Materials;

	ROcclusionList m_OcclusionList;

#ifdef _WIN32
	std::vector<RBaseTexturePtr> LightmapTextures;
#endif

	// Convex polygons.
	// Note that these are only used for lightmap generation, and therefore
	// the data is not actually loaded when nOpenMode == Runtime,
	// and only NumConvexPolygons is set in that case.
	std::vector<v3> ConvexVertices;
	std::vector<v3> ConvexNormals;
	int NumConvexPolygons{};
	std::vector<RCONVEXPOLYGONINFO> ConvexPolygons;

	RLightList	StaticMapLightList;
	RLightList	StaticObjectLightList;
	RLightList	StaticSunLightList;

#ifdef _WIN32
	RMeshMgr			m_MeshList;
#endif
	RAnimationMgr		m_AniList;
	RMapObjectList		m_ObjectList;
	bool				m_bNotOcclusion{};

	// Collision stuff.
	// These are only used in GetFloor and CheckSolid, and
	// not in Pick even though that is also involved in collision.
	std::vector<RSolidBspNode> ColRoot;
	std::vector<v3> ColVertices;

	RNavigationMesh m_NavigationMesh;

	RDummyList m_DummyList;

	FogInfo m_FogInfo;

	std::vector<AmbSndInfo>	AmbSndInfoList;

	RDEBUGINFO m_DebugInfo;

	// TODO: Add this as an ROpenMode instead.
	bool PhysOnly{};

	std::string m_filename, m_descfilename;

	bool m_bWireframe{};
	// This is used in the editor to display the lightmap
	// for e.g. debugging, it doesn't control whether
	// the lightmap is on for normal rendering.
	bool m_bShowLightmap{};

	// TODO: Remove this piece of global state
	static bool m_bisDrawLightMap;

	bool RenderWithNormal{};

	bool IsRS3Map{};

#ifdef _WIN32
	std::unique_ptr<BulletCollision> Collision;
#endif
};

#ifdef _DEBUG
extern int g_nPoly, g_nCall;
extern int g_nPickCheckPolygon, g_nRealPickCheckPolygon;
#endif

_NAMESPACE_REALSPACE2_END