#pragma once

#include "RNameSpace.h"
#include "GlobalTypes.h"
#ifdef _WIN32
#include "RMesh.h"
#endif

_NAMESPACE_REALSPACE2_BEGIN

struct RCONVEXPOLYGONINFO;
struct RLightList;
class RBspObject;
struct RLIGHT;
struct RLIGHTMAPTEXTURE;
class RBspLightmapManager;

struct FreeBlock { int x; int y; };
using RFREEBLOCKLIST = std::list<FreeBlock>;

using RGENERATELIGHTMAPCALLBACK = bool(*)(float fProgress);

struct LightmapGenerator
{
	const char* filename{};
	int MaxLightmapSize{}, MinLightmapSize{};
	int Supersample{};
	float Tolerance{};
	v3 AmbientLight{ 0, 0, 0 };

	RGENERATELIGHTMAPCALLBACK pProgressFn{};

	RBspObject& bsp;

	LightmapGenerator(RBspObject& bsp);
	~LightmapGenerator();

	bool Generate();

private:
	void Init();
	bool ProcessConvexPolygon(const RCONVEXPOLYGONINFO* poly, int PolyIndex, int& lightmapsize);
	void InsertLightmap(int lightmapsize, int PolyIndex);
	v3 CalcDiffuse(const rboundingbox& bbox,
		const RCONVEXPOLYGONINFO* poly,
		const v3& polynormal, const v3& diff,
		const RLIGHT* plight,
		int lightmapsize,
		int j, int k,
		int au, int av, int ax);
	bool CheckShadow(const RLIGHT* plight,
		const RCONVEXPOLYGONINFO* poly,
		const rboundingbox& bbox,
		const v3& pnormal, const v3& diff,
		int lightmapsize,
		int j, int k,
		int au, int av, int ax);
	bool SaveToFile();

	float MaximumArea{};
	int ConstCount{};
	int LightIndex{};

	std::unique_ptr<RLIGHT*[]> pplight;
	std::unique_ptr<rvector[]> lightmap;
	std::unique_ptr<u32[]> lightmapdata;
	std::unique_ptr<bool[]> isshadow;
	std::unique_ptr<int[]> SourceLightmap;
	std::map<u32, int> ConstmapTable;

	std::vector<RLIGHTMAPTEXTURE> sourcelightmaplist;
	std::vector<RBspLightmapManager> LightmapList;
};

v3 GetNormal(const RCONVEXPOLYGONINFO *poly, const rvector &position,
	int au, int av);

_NAMESPACE_REALSPACE2_END
