#include "stdafx.h"
#include "LightmapGenerator.h"
#include "RBspObject.h"
#include "RVersions.h"
#include "RealSpace2.h"

#define MAX_LIGHTMAP_SIZE		1024
#define MAX_LEVEL_COUNT			10

_NAMESPACE_REALSPACE2_BEGIN

bool SaveMemoryBmp(int x, int y, void *data, void **retmemory, int *nsize);

class RBspLightmapManager {
public:
	RBspLightmapManager();

	int GetSize() const { return m_nSize; }
	auto * GetData() { return m_pData.get(); }

	void SetSize(int nSize) { m_nSize = nSize; }
	void SetData(std::unique_ptr<u32[]> pData) { m_pData = std::move(pData); }

	bool Add(const u32 * data, int nSize, FreeBlock * retpoint);
	bool GetFreeRect(int nLevel, FreeBlock *pt);

	void Save(const char *filename);

	float CalcUnused();
	float m_fUnused;

protected:
	std::unique_ptr<RFREEBLOCKLIST[]> m_pFreeList;
	std::unique_ptr<u32[]> m_pData;
	int m_nSize;
};

struct RLIGHTMAPTEXTURE {
	int nSize;
	std::unique_ptr<u32> data;
	bool bLoaded;
	FreeBlock position;
	int	nLightmapIndex;
};

RBspLightmapManager::RBspLightmapManager()
	: m_pData{ new u32[MAX_LIGHTMAP_SIZE*MAX_LIGHTMAP_SIZE] },
	m_pFreeList{ new RFREEBLOCKLIST[MAX_LEVEL_COUNT + 1] }
{
	m_nSize = MAX_LIGHTMAP_SIZE;

	m_pFreeList[MAX_LEVEL_COUNT].push_back({ 0, 0 });
}

float RBspLightmapManager::CalcUnused()
{
	float fUnused = 0.f;

	for (int i = 0; i <= MAX_LEVEL_COUNT; i++) {
		float fThisLevelSize = pow(0.25f, (MAX_LEVEL_COUNT - i));
		fUnused += (float)m_pFreeList[i].size()*fThisLevelSize;
	}

	return fUnused;
}

bool RBspLightmapManager::GetFreeRect(int nLevel, FreeBlock *pt)
{
	if (nLevel > MAX_LEVEL_COUNT) return false;

	if (m_pFreeList[nLevel].empty())
	{
		FreeBlock point;
		if (!GetFreeRect(nLevel + 1, &point))
			return false;

		int nSize = 1 << nLevel;

		FreeBlock newpoint;

		newpoint.x = point.x + nSize; newpoint.y = point.y;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x + nSize; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		*pt = point;
	}
	else
	{
		*pt = *m_pFreeList[nLevel].begin();
		m_pFreeList[nLevel].erase(m_pFreeList[nLevel].begin());
	}

	return true;
}

bool RBspLightmapManager::Add(const u32 *data, int nSize, FreeBlock *retpoint)
{
	int nLevel = 0, nTemp = 1;
	while (nSize > nTemp)
	{
		nTemp = nTemp << 1;
		nLevel++;
	}
	_ASSERT(nSize == nTemp);

	FreeBlock pt;
	if (!GetFreeRect(nLevel, &pt))
		return false;

	for (int y = 0; y < nSize; y++)
	{
		for (int x = 0; x < nSize; x++)
		{
			m_pData[(y + pt.y)*GetSize() + (x + pt.x)] = data[y*nSize + x];
		}
	}
	*retpoint = pt;
	return true;
}

void RBspLightmapManager::Save(const char *filename)
{
#ifdef _WIN32
	RSaveAsBmp(GetSize(), GetSize(), m_pData.get(), filename);
#endif
}

static v3 InterpolatedVector(const v3 &a, const v3 &b, float x)
{
	auto ab = min(max(DotProduct(a, b), -1.0f), 1.0f);
	if (ab == 1.0f) return b;

	auto theta = acos(ab);

	auto theta1 = theta * x;
	auto theta2 = theta*(1.0f - x);
	auto costheta1 = cos(theta1);
	auto costheta2 = cos(theta2);
	auto u = costheta1 - ab * costheta2;
	auto v = costheta2 - ab * costheta1;
	auto D = (1.0f - Square(ab));
	if (D == 0) return a;

	auto vReturn = (1.0f / D*(u*a + v*b));
	return vReturn;
}

v3 GetNormal(const RCONVEXPOLYGONINFO *poly, const rvector &position,
	int au, int av)
{
	int nSelPolygon = -1, nSelEdge = -1;
	float fMinDist = FLT_MAX;

	if (poly->nVertices == 3)
		nSelPolygon = 0;
	else
	{
		rvector pnormal(poly->plane.a, poly->plane.b, poly->plane.c);

		for (int i = 0; i < poly->nVertices - 2; i++)
		{
			const auto& a = poly->pVertices[0];
			const auto& b = poly->pVertices[i + 1];
			const auto& c = poly->pVertices[i + 2];

			if (IntersectTriangle(a, b, c, position + pnormal, -pnormal, nullptr))
			{
				nSelPolygon = i;
				nSelEdge = -1;
				break;
			}
			else
			{
				float dist = GetDistance(position, a, b);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 0; }
				dist = GetDistance(position, b, c);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 1; }
				dist = GetDistance(position, c, a);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 2; }
			}
		}
	}

	auto& v0 = poly->pVertices[0];
	auto& v1 = poly->pVertices[nSelPolygon + 1];
	auto& v2 = poly->pVertices[nSelPolygon + 2];

	auto& n0 = poly->pNormals[0];
	auto& n1 = poly->pNormals[nSelPolygon + 1];
	auto& n2 = poly->pNormals[nSelPolygon + 2];

	v3 pos;
	if (nSelEdge != -1)
	{
		auto& e0 = nSelEdge == 0 ? v0 : nSelEdge == 1 ? v1 : v2;
		auto& e1 = nSelEdge == 0 ? v1 : nSelEdge == 1 ? v2 : v0;

		auto dir = Normalized(e1 - e0);

		pos = e0 + DotProduct(dir, position - e0) * dir;
	}
	else
		pos = position;

	auto a = v1 - v0;
	auto b = v2 - v1;
	auto x = pos - v0;

	float f = b[au] * x[av] - b[av] * x[au];
	if (IS_ZERO(f))
		return n0;

	float t = (a[av] * x[au] - a[au] * x[av]) / f;

	auto tem = InterpolatedVector(n1, n2, t);

	auto inter = a + t*b;

	int axisfors;
	if (fabs(inter.x) > fabs(inter.y) && fabs(inter.x) > fabs(inter.z))
		axisfors = 0;
	else if (fabs(inter.y) > fabs(inter.z))
		axisfors = 1;
	else
		axisfors = 2;

	float s = x[axisfors] / inter[axisfors];
	return InterpolatedVector(n0, tem, s);
}

static void CalcLightmapUV(
	RSBspNode * pNode,
	const int * pSourceLightmap,
	std::vector<RLIGHTMAPTEXTURE>& SourceLightmaps,
	std::vector<RBspLightmapManager>& LightmapList,
	const std::vector<RCONVEXPOLYGONINFO>& ConvexPolygons)
{
	if (pNode->nPolygon)
	{
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			int is = pNode->pInfo[i].nConvexPolygon;
			int nSI = pSourceLightmap[is];

			auto* poly = &ConvexPolygons[is];

			rboundingbox bbox;

			bbox.vmin = bbox.vmax = poly->pVertices[0];
			for (int j = 1; j < poly->nVertices; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
					bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
				}
			}

			auto& DestLightmap = SourceLightmaps[nSI];

			int lightmapsize = DestLightmap.nSize;

			rvector diff = float(lightmapsize) / float(lightmapsize - 1)*(bbox.vmax - bbox.vmin);

			for (int k = 0; k<3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
			}

			int au, av, ax;

			if (fabs(poly->plane.a)>fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0; // yz
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz
			else
				ax = 2;	// xy

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			RPOLYGONINFO *pInfo = &pNode->pInfo[i];
			for (int j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 = ((*pInfo->pVertices[j].Coord())[au] - bbox.vmin[au]) / diff[au];
				pInfo->pVertices[j].tv2 = ((*pInfo->pVertices[j].Coord())[av] - bbox.vmin[av]) / diff[av];
			}

			auto* CurrentLightmap = LightmapList.size() ? &LightmapList[LightmapList.size() - 1] : NULL;

			if (!DestLightmap.bLoaded)
			{
				FreeBlock pt;

				while (!CurrentLightmap || !CurrentLightmap->Add(DestLightmap.data.get(), DestLightmap.nSize, &pt))
				{
					LightmapList.emplace_back();
					CurrentLightmap = &LightmapList.back();
				}
				DestLightmap.bLoaded = true;
				DestLightmap.position = pt;
				DestLightmap.nLightmapIndex = LightmapList.size() - 1;
			}

			pNode->pInfo[i].nLightmapTexture = DestLightmap.nLightmapIndex;

			float fScaleFactor = (float)DestLightmap.nSize / (float)CurrentLightmap->GetSize();
			for (int j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 =
					pInfo->pVertices[j].tu2 * fScaleFactor +
					(float)DestLightmap.position.x / (float)CurrentLightmap->GetSize();
				pInfo->pVertices[j].tv2 =
					pInfo->pVertices[j].tv2 * fScaleFactor +
					(float)DestLightmap.position.y / (float)CurrentLightmap->GetSize();
			}
		}
	}

	auto CalcNodeUV = [&](auto* node) {
		if (node)
			CalcLightmapUV(node,
				pSourceLightmap, SourceLightmaps,
				LightmapList, ConvexPolygons);
	};
	CalcNodeUV(pNode->m_pPositive);
	CalcNodeUV(pNode->m_pNegative);
}

v3 LightmapGenerator::CalcDiffuse(const rboundingbox& bbox,
	const RCONVEXPOLYGONINFO* poly,
	const v3& polynormal, const v3& diff,
	const RLIGHT* plight,
	int lightmapsize,
	int j, int k,
	int au, int av, int ax)
{
	int nShadowCount = 0;

	for (int m = 0; m < 4; m++)
	{
		if (isshadow[(k + m % 2)*(lightmapsize + 1) + j + m / 2])
			nShadowCount++;
	}

	if (nShadowCount >= 4)
		return{ 0, 0, 0 };

	v3 color{ 0, 0, 0 };

	if (nShadowCount > 0)
	{
		v3 tempcolor{ 0, 0, 0 };

		for (int m = 0; m < Supersample; m++)
		{
			for (int n = 0; n < Supersample; n++)
			{
				rvector position;
				position[au] = bbox.vmin[au] + ((k + (n + .5f) / Supersample) / lightmapsize)*diff[au];
				position[av] = bbox.vmin[av] + ((j + (m + .5f) / Supersample) / lightmapsize)*diff[av];
				position[ax] = (-poly->plane.d - polynormal[au] * position[au] - polynormal[av] * position[av]) / polynormal[ax];

				bool bShadow = false;

				float fDistanceToPolygon = Magnitude(position - plight->Position);

				RBSPPICKINFO bpi;
				if (bsp.PickShadow(plight->Position, position, &bpi))
				{
					float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);
					if (fDistanceToPolygon > fDistanceToPickPos + Tolerance)
						bShadow = true;
				}

				if (!bShadow)
				{
					rvector dpos = plight->Position - position;
					float fdistance = Magnitude(dpos);
					float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
					fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
					Normalize(dpos);

					auto normal = GetNormal(poly, position, au, av);

					float fDot;
					fDot = DotProduct(dpos, normal);
					fDot = max(0.f, fDot);

					tempcolor += fIntensity*plight->fIntensity*fDot*plight->Color;
				}
			}
		}
		tempcolor *= 1.f / Square(Supersample);
		color += tempcolor;
	}
	else
	{
		rvector position;
		position[au] = bbox.vmin[au] + (((float)k + .5f) / (float)lightmapsize)*diff[au];
		position[av] = bbox.vmin[av] + (((float)j + .5f) / (float)lightmapsize)*diff[av];
		position[ax] = (-poly->plane.d - polynormal[au] * position[au] - polynormal[av] * position[av]) / polynormal[ax];

		rvector dpos = plight->Position - position;
		float fdistance = Magnitude(dpos);
		float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
		fIntensity = min(max(1.0f - fIntensity, 0.f), 1.f);
		Normalize(dpos);

		auto normal = GetNormal(poly, position, au, av);

		float fDot;
		fDot = DotProduct(dpos, normal);
		fDot = max(0.f, fDot);

		color += fIntensity*plight->fIntensity*fDot*plight->Color;
	}

	return color;
}

bool LightmapGenerator::CheckShadow(const RLIGHT* plight,
	const RCONVEXPOLYGONINFO* poly,
	const rboundingbox& bbox,
	const v3& pnormal, const v3& diff,
	int lightmapsize,
	int j, int k,
	int au, int av, int ax)
{
	if ((plight->dwFlags & RM_FLAG_CASTSHADOW) == 0 ||
		(poly->dwFlags & RM_FLAG_RECEIVESHADOW) == 0) return false;

	rvector position;
	position[au] = bbox.vmin[au] + ((float)k / (float)lightmapsize)*diff[au];
	position[av] = bbox.vmin[av] + ((float)j / (float)lightmapsize)*diff[av];
	position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

	float fDistanceToPolygon = Magnitude(position - plight->Position);

	RBSPPICKINFO bpi;
	if (bsp.PickShadow(plight->Position, position, &bpi))
	{
		float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);

		if (fDistanceToPolygon > fDistanceToPickPos + Tolerance)
			return true;
	}

#ifdef _WIN32
	for (auto& ObjectInfo : bsp.m_ObjectList)
	{
		if (!ObjectInfo.pVisualMesh) return false;

		rmatrix inv = Inverse(ObjectInfo.pVisualMesh->m_WorldMat);

		rvector origin = plight->Position * inv;
		rvector target = position * inv;

		rvector dir = target - origin;
		rvector dirorigin = position - plight->Position;

		rvector vOut;

		rboundingbox bbox;
		bbox.vmin = ObjectInfo.pVisualMesh->m_vBMin;
		bbox.vmax = ObjectInfo.pVisualMesh->m_vBMax;

		auto bBBTest = IntersectLineAABB(origin, dir, bbox);
		float t;
		if (bBBTest &&
			ObjectInfo.pVisualMesh->Pick(plight->Position, dirorigin, &vOut, &t))
		{
			rvector PickPos = plight->Position + vOut*t;
			return true;
		}
	}
#else
	assert(false);
#endif

	return false;
}

bool LightmapGenerator::ProcessConvexPolygon(const RCONVEXPOLYGONINFO* poly,
	int PolyIndex, int& lightmapsize)
{
	rboundingbox bbox;

	bbox.vmin = bbox.vmax = poly->pVertices[0];
	for (int j = 1; j < poly->nVertices; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
			bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
		}
	}

	lightmapsize = MaxLightmapSize;

	float targetarea = MaximumArea / 4.f;
	while (poly->fArea < targetarea && lightmapsize > MinLightmapSize)
	{
		targetarea /= 4.f;
		lightmapsize /= 2;
	}

	v3 diff = float(lightmapsize) / float(lightmapsize - 1) * (bbox.vmax - bbox.vmin);

	// 1 texel
	for (int k = 0; k < 3; k++)
	{
		bbox.vmin[k] -= .5f / float(lightmapsize)*diff[k];
		bbox.vmax[k] += .5f / float(lightmapsize)*diff[k];
	}

	v3 pnormal{ poly->plane.a, poly->plane.b, poly->plane.c };

	LightIndex = 0;

	for (auto& Light : bsp.StaticMapLightList)
	{
		if (GetDistance(Light.Position, poly->plane) > Light.fAttnEnd) continue;

		for (int iv = 0; iv < poly->nVertices; iv++)
		{
			if (DotProduct(Light.Position - poly->pVertices[iv], poly->pNormals[iv])>0) {
				pplight[LightIndex] = &Light;
				++LightIndex;
				break;
			}

		}
	}

	int au, av, ax;

	if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
		ax = 0; // yz
	else if (fabs(poly->plane.b) > fabs(poly->plane.c))
		ax = 1;	// xz
	else
		ax = 2;	// xy

	au = (ax + 1) % 3;
	av = (ax + 2) % 3;

	for (int j = 0; j < lightmapsize; j++)			// v 
	{
		for (int k = 0; k < lightmapsize; k++)		// u
		{
			lightmap[j*lightmapsize + k] = AmbientLight;
		}
	}

	for (int l = 0; l < LightIndex; l++)
	{
		RLIGHT *plight = pplight[l];

		for (int j = 0; j < lightmapsize + 1; j++)			// v 
		{
			for (int k = 0; k < lightmapsize + 1; k++)		// u
			{
				isshadow[k*(lightmapsize + 1) + j] = CheckShadow(plight,
					poly,
					bbox,
					pnormal, diff,
					lightmapsize,
					j, k,
					au, av, ax);
			}
		}

		for (int j = 0; j < lightmapsize; j++)
		{
			for (int k = 0; k < lightmapsize; k++)
			{
				lightmap[j*lightmapsize + k] += CalcDiffuse(bbox,
					poly,
					pnormal, diff,
					plight,
					lightmapsize,
					j, k,
					au, av, ax);
			}
		}
	}

	for (int j = 0; j < Square(lightmapsize); j++)
	{
		auto color = lightmap[j];

		color *= 0.25f;
		color.x = min(color.x, 1.f);
		color.y = min(color.y, 1.f);
		color.z = min(color.z, 1.f);
		lightmap[j] = color;
		lightmapdata[j] =
			((u32)(color.x * 255)) << 16 |
			((u32)(color.y * 255)) << 8 |
			((u32)(color.z * 255));
	}

	return true;
}

void LightmapGenerator::InsertLightmap(int lightmapsize,
	int PolyIndex)
{
	bool bConstmap = true;
	for (int j = 0; j < lightmapsize*lightmapsize; j++)
	{
		if (lightmapdata[j] != lightmapdata[0])
		{
			bConstmap = false;
			ConstCount++;
			break;
		}
	}

	bool bNeedInsert = true;
	if (bConstmap)
	{
		lightmapsize = 2;

		auto it = ConstmapTable.find(lightmapdata[0]);
		if (it != ConstmapTable.end())
		{
			SourceLightmap[PolyIndex] = (*it).second;
			bNeedInsert = false;
		}
	}

	if (bNeedInsert)
	{
		int nLightmap = sourcelightmaplist.size();

		SourceLightmap[PolyIndex] = nLightmap;
		if (bConstmap)
			ConstmapTable.insert({ lightmapdata[0], nLightmap });

		sourcelightmaplist.emplace_back();
		auto& new_lightmap = sourcelightmaplist.back();
		new_lightmap.bLoaded = false;
		new_lightmap.nSize = lightmapsize;
		new_lightmap.data = decltype(new_lightmap.data){new u32[Square(lightmapsize)]};
		memcpy(new_lightmap.data.get(), lightmapdata.get(), Square(lightmapsize) * sizeof(u32));
	}
}

bool LightmapGenerator::SaveToFile()
{
	RHEADER header{ R_LM_ID, R_LM_VERSION };

	auto CalcTreeUV = [&](auto* tree) {
		CalcLightmapUV(tree, SourceLightmap.get(),
			sourcelightmaplist, LightmapList, bsp.ConvexPolygons);
	};
	CalcTreeUV(bsp.BspRoot.data());
	CalcTreeUV(bsp.OcRoot.data());

	FILE *file = fopen(filename, "wb+");
	if (!file)
		return false;

	fwrite(&header, sizeof(RHEADER), 1, file);

	u32 nConvexPolygons = bsp.ConvexPolygons.size();
	fwrite(&nConvexPolygons, sizeof(int), 1, file);
	i32 NodeCount = bsp.GetNodeCount();
	fwrite(&NodeCount, sizeof(int), 1, file);

	auto nLightmap = LightmapList.size();
	fwrite(&nLightmap, sizeof(int), 1, file);
	for (size_t i = 0; i < LightmapList.size(); i++)
	{
		char lightfilename[256];
		sprintf_safe(lightfilename, "%s.light%d.bmp", filename, i);
#ifdef _WIN32
		RSaveAsBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), lightfilename);
#endif

		void *memory;
		int nSize;
#ifdef _WIN32
		bool bSaved = SaveMemoryBmp(LightmapList[i].GetSize(), LightmapList[i].GetSize(),
			LightmapList[i].GetData(), &memory, &nSize);
		_ASSERT(bSaved);
#endif
		fwrite(&nSize, sizeof(int), 1, file);
		fwrite(memory, nSize, 1, file);
		delete[] (unsigned char*)memory;
	}

	bsp.Sort_Nodes(bsp.OcRoot.data());

	for (int i = 0; i < bsp.GetPolygonCount(); i++)
		fwrite(&bsp.OcInfo[i].nPolygonID, sizeof(int), 1, file);

	for (int i = 0; i < bsp.GetPolygonCount(); i++)
		fwrite(&bsp.OcInfo[i].nLightmapTexture, sizeof(int), 1, file);

	for (size_t i = 0; i < bsp.OcVertices.size(); i++)
		fwrite(&bsp.OcVertices[i].tu2, sizeof(float), 2, file);

	fclose(file);

	return true;
}

void LightmapGenerator::Init()
{
	for (auto& Poly : bsp.ConvexPolygons)
		MaximumArea = max(MaximumArea, Poly.fArea);

	pplight = std::unique_ptr<RLIGHT*[]>{ new RLIGHT*[bsp.StaticMapLightList.size()] };
	lightmap = std::unique_ptr<rvector[]>{ new rvector[Square(MaxLightmapSize)] };
	lightmapdata = std::unique_ptr<u32[]>{ new u32[Square(MaxLightmapSize)] };
	isshadow = std::unique_ptr<bool[]>{ new bool[Square(MaxLightmapSize + 1)] };
	SourceLightmap = std::unique_ptr<int[]>{ new int[bsp.ConvexPolygons.size()] };
}

bool LightmapGenerator::Generate()
{	
	Init();

	for (size_t i = 0; i < bsp.ConvexPolygons.size(); i++)
	{
		// The progress function returning false
		// indicates that the generation needs to stop.
		if (pProgressFn && !pProgressFn((float)i / (float)bsp.ConvexPolygons.size()))
			return false;

		int lightmapsize{};
		if (!ProcessConvexPolygon(&bsp.ConvexPolygons[i], i, lightmapsize))
		{
			MLog("LightmapGenerator::Generate -- ProcessConvexPolygon failed for polygon index %d\n", i);
			return false;
		}
		InsertLightmap(lightmapsize, i);
	}

	if (!SaveToFile())
	{
		MLog("LightmapGenerator::Generate -- SaveToFile failed\n");
		return false;
	}

	return true;
}

// NOTE: Must be defined in the .cpp so that the vectors
// don't spring an "incomplete type" error.
LightmapGenerator::LightmapGenerator(RBspObject& bsp) : bsp{ bsp } {}
LightmapGenerator::~LightmapGenerator() = default;

_NAMESPACE_REALSPACE2_END
