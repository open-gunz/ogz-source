#include "stdafx.h"
#include "ZWorld.h"
#include "ZMap.h"
#include "ZMapDesc.h"
#include "ZSkyBox.h"
#include "ZInitialLoading.h"
#include "RUtil.h"
#include "RBspObject.h"

ZWorld::ZWorld()
{
	m_szName[0] = 0;
	m_szBspName[0] = 0;
}

ZWorld::~ZWorld() = default;

void ZWorld::Update(float fDelta)
{
	m_waters.Update();
	m_flags.Update();
}

void ZWorld::Draw()
{
	auto Shaders = SaveAndDisableShaders();

	if (m_pSkyBox)
		m_pSkyBox->Render();

	SetShaders(Shaders);

	if (m_bFog)
		RGetViewFrustum()[5] = ComputeZPlane(m_fFogFar, -1, RCameraPosition, RCameraDirection);

	m_pBsp->Draw();

	RealSpace2::g_poly_render_cnt = 0;

	__BP(16,"ZGame::Draw::flags");

	m_flags.Draw();

	__EP(16);
}

static void ZWorldProgressCallBack(void *pUserParam, float fProgress)
{
	ZLoadingProgress *pLoadingProgress = (ZLoadingProgress*)pUserParam;
	pLoadingProgress->UpdateAndDraw(fProgress);
}

bool ZWorld::Create(ZLoadingProgress *pLoading)
{
	if (m_bCreated)
	{
		assert(false);
		return true;
	}

	m_pBsp = std::make_unique<RBspObject>();
	if (!m_pBsp->Open(m_szBspName, RBspObject::ROpenMode::Runtime, ZWorldProgressCallBack, pLoading))
	{
		MLog("Error while loading map %s!\n", m_szName);
		return false;
	}

	m_pBsp->OptimizeBoundingBox();

	char szMapPath[64]; szMapPath[0] = 0;
	ZGetCurrMapPath(szMapPath);

	ZWater*		water_instance;
	RMapObjectList* map_object_list		= m_pBsp->GetMapObjectList();
	RMeshMgr* mesh_mgr					= m_pBsp->GetMeshManager();
	
	auto it = map_object_list->begin();
	while (it != map_object_list->end())
	{
		ROBJECTINFO* object_info = &*it;
		RMesh* pMesh = mesh_mgr->GetFast(object_info->nMeshID);
		if (pMesh->m_data_num <= 0)
		{
			++it;
			continue;
		}
		RMeshNode* pMeshNode = pMesh->m_data[0];

		auto* object_name = object_info->name.c_str();

		auto len = strlen(m_szName) + 1;
		object_name += len;

		if (pMeshNode->m_point_color_num > 0)
		{
			ZClothEmblem* new_instance = new ZClothEmblem;
			new_instance->CreateFromMeshNode(pMeshNode, this);
			m_flags.Add(new_instance, object_name);
			it = map_object_list->erase(it);
			continue;
		}

		if (m_pSkyBox == NULL)
		{
			if (strncmp(object_name, "obj_sky_", 8) == 0 ||
				strncmp(object_name, "obj_ef_sky", 10) == 0)
			{
				m_pSkyBox = std::make_unique<ZSkyBox>(std::unique_ptr<RVisualMesh>{pMesh->m_pVisualMesh});
				it->pVisualMesh.release();
				it = map_object_list->erase(it);
				continue;
			}
		}

		int nWater = 0;

		if (!strncmp(object_name, "obj_water", 9))	nWater = 1;
		if (!strncmp(object_name, "obj_water2", 10))	nWater = 3;
		if (!strncmp(object_name, "obj_sea", 7))		nWater = 2;

		if (nWater) {
			m_bWaterMap = true;
			m_fWaterHeight = pMeshNode->m_mat_base._42;
		}
		else {
			m_bWaterMap = false;
			m_fWaterHeight = 0.f;
		}

		if(nWater)	
		{
			int id = object_info->nMeshID;

			RMesh* mesh = mesh_mgr->GetFast(id);
			RMeshNode* node = mesh->m_data[0];

			water_instance = new ZWater;

			water_instance->SetMesh(node);
			m_waters.push_back( water_instance );

				 if(nWater==1) water_instance->m_nWaterType = WaterType1;
			else if(nWater==3) water_instance->m_nWaterType = WaterType2;


			if(nWater==2) 
			{
				water_instance->m_isRender = false;
				pMesh->m_LitVertexModel = true;	
			}
			else 
			{
				it = map_object_list->erase(it);
				continue;
			}
		}

		++it;
	}

	char szBuf[128];
	
	if (m_flags.size() > 0)
	{
		sprintf_safe(szBuf, "%s%s/flag.xml", szMapPath,
			ZGetGameClient()->GetMatchStageSetting()->GetMapName());
		m_flags.InitEnv(szBuf);
	}

	m_pMapDesc = std::make_unique<ZMapDesc>();
	m_pMapDesc->Open(m_pBsp.get());

	sprintf_safe(szBuf, "%s%s/smoke.xml", szMapPath,
		ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	m_pMapDesc->LoadSmokeDesc(szBuf);

	FogInfo finfo = GetBsp()->GetFogInfo();
	m_bFog = finfo.bFogEnable;
	m_fFogNear = finfo.fNear;
	m_fFogFar = finfo.fFar;
	m_dwFogColor = finfo.dwFogColor;

	m_bCreated = true;

	return true;
}

void ZWorld::OnInvalidate()
{
	m_pBsp->OnInvalidate();
	m_flags.OnInvalidate();
}

void ZWorld::OnRestore()
{
	m_pBsp->OnRestore();
	m_flags.OnRestore();
}

void ZWorld::SetFog(bool bFog)
{
	if(bFog) {
		RSetFog(m_bFog, m_fFogNear, m_fFogFar, m_dwFogColor);
	}
	else {
		RSetFog(FALSE);
	}
}