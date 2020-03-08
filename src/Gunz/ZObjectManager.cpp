#include "stdafx.h"
#include "ZObjectManager.h"
#include "ZShadow.h"
#include "ZNetCharacter.h"
#include "ZActor.h"
#include "ZEnemy.h"

ZObjectManager::ZObjectManager() : m_nGenerate(0), m_nOnDrawCnt(0), m_nRenderedCnt(0)
{

}

ZObjectManager::~ZObjectManager()
{
	Clear();
}

void ZObjectManager::Add(ZObject *pObject)
{
	Insert(pObject);

	if (pObject->IsNPC())
	{
		m_NPCObjectMap.Insert(pObject->GetUID(), pObject);
	}
}

void ZObjectManager::Delete(ZObject* pObject)
{
	iterator itor = find(pObject->GetUID());

	if (itor != end()) {
		erase(itor);
	}

	if (pObject->IsNPC())
	{
		m_NPCObjectMap.Delete(pObject->GetUID());
	}
}

void ZObjectManager::Clear()
{
	clear();
	ClearNPC();
}

void ZObjectManager::ClearNPC()
{
	while(!m_NPCObjectMap.empty()) {
		Delete(m_NPCObjectMap.begin()->second);
	}
}

void ZObjectManager::Update(float fDelta)
{
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		pObject->UpdateModules(fDelta);
		pObject->Update(fDelta);
	}
}

bool ZObjectManager::DrawObject(ZObject* pObject)
{
	if ((pObject->GetInitialized()) && (pObject->IsVisible()))
	{
		pObject->Draw();

		if(pObject->IsRendered())
		{
			if ((MIsExactlyClass(ZNetCharacter, pObject)) || 
				(MIsExactlyClass(ZMyCharacter, pObject)) ||
				(MIsExactlyClass(ZHumanEnemy , pObject)) 	)
			{
				MStaticCast(ZCharacterObject, pObject)->DrawShadow();
			}

			m_nOnDrawCnt++;
			m_nRenderedCnt++;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////
// 에러 테스트용..

char g_upper_string[20][20] =
{
	{"none"},
	{"shot"},		
	{"reload"},		
	{"load"},
	{"start"},
	{"idle"},
	{"block1"},
	{"block1_ret"},
	{"block2"},
	{"cancel"},
};

char g_lower_string[100][20] =
{
	{"none"},
	{"idle1"},
	{"idle2"},
	{"idle3"},
	{"idle4"},
	{"forward"},
	{"back"},
	{"left"},
	{"right"},
	{"jump_up"},
	{"jump_down"},
	{"die1"},
	{"die2"},
	{"die3"},
	{"die4"},
	{"wall_left"},
	{"wall_left_down"},
	{"wall"},
	{"wall_down_forward"},
	{"wall_down"},
	{"wall_right"},
	{"wall_right_down"},
	{"tumble_forward"},
	{"tumble_back"},
	{"tumble_right"},
	{"tumble_left"},
	{"bind"},
	{"jump_wall_forward"},
	{"jump_wall_back"},
	{"jump_wall_left"},
	{"jump_wall_right"},
	{"attack1"},
	{"attack1_ret"},
	{"attack2"},
	{"attack2_ret"},
	{"attack3"},
	{"attack3_ret"},
	{"attack4"},
	{"attack4_ret"},
	{"attack5"},
	{"jump_attack"},
	{"upppercut"},
	{"guard_start"},
	{"guard_idle"},
	{"guard_block1"},
	{"guard_block1_ret"},
	{"guard_block2"},
	{"guard_cancel"},
	{"blast"},
	{"blast_fall"},
	{"blast_drop"},
	{"blast_stand"},
	{"blast_airmove"},
	{"blast_dagger"},
	{"blast_drop_dagger"},
	{"damage"},
	{"damage2"},
	{"damage_down"},
	{"taunt"},
	{"bow"},
	{"wave"},
	{"laugh"},
	{"cry"},
	{"dance"},
	{"run_wall_cancle"}
};

void ZObjectManager::Draw()
{
	m_nRenderedCnt = 0;
	m_nOnDrawCnt = 0;

	ZObject* pMyCharacter = (ZObject*)g_pGame->m_pMyCharacter;

	for (iterator itor = begin(); itor != end(); ++itor) 
	{
		ZObject* pObject = (*itor).second;
		if (pObject == NULL)			continue;
		if (pObject == pMyCharacter)	continue;

		DrawObject(pObject);

	}

	if( pMyCharacter ) 
	{ 
		RVisualMesh* pVMesh = pMyCharacter->GetVisualMesh();

		if(pVMesh) {
		
			if(pVMesh->GetVisibility() != 1.f) {

				pVMesh->SetSpRenderMode(1);

				DrawObject( pMyCharacter );

				pVMesh->SetSpRenderMode(2);
			}

			pMyCharacter->m_bRendered = DrawObject( pMyCharacter );

			pVMesh->SetSpRenderMode(0);
		}
	}
}

ZObject* ZObjectManager::Pick(ZObject* pMyObject,rvector& pos,rvector& dir, RPickInfo* pInfo)
{
	ZObject* pPickObject = NULL;
	float best_dist = 10000.f;
	RPickInfo PickInfo;

	memset(&PickInfo,0,sizeof(RPickInfo));

	ZObject* pObject;

	for (iterator itor = begin(); itor != end(); ++itor) {

		pObject = (*itor).second;

		if(pObject != pMyObject) {

			if(pObject->Pick(pos, dir, &PickInfo)) {

				if(PickInfo.t < best_dist) {
					pPickObject = pObject;
					*pInfo = PickInfo;
					best_dist = PickInfo.t;
				}
			}

		}
	}
	return pPickObject;
}

ZObject* ZObjectManager::Pick(int x,int y,RPickInfo* pInfo)
{
	ZObject* pPickObject = NULL;
	float best_dist = 10000.f;
	RPickInfo PickInfo;

	ZObject* pObject;

	for (iterator itor = begin(); itor != end(); ) {

		pObject = (*itor).second;

		if(pObject->Pick(x,y,&PickInfo)) {
			if(PickInfo.t < best_dist) {
				pPickObject = pObject;
				*pInfo = PickInfo;
				best_dist = PickInfo.t;
			}
		}
		++itor;
	}
	return pPickObject;
}

ZObject* ZObjectManager::Pick(rvector& pos,rvector& dir, RPickInfo* pInfo)
{
	ZObject* pPickObject = NULL;
	float best_dist = 10000.f;
	RPickInfo PickInfo;

	for (iterator itor = begin(); itor != end(); ++itor) {

		ZObject* pObject = (*itor).second;

		if(pObject->Pick(pos, dir, &PickInfo)) {

			if(PickInfo.t < best_dist) {
				pPickObject = pObject;
				*pInfo = PickInfo;
				best_dist = PickInfo.t;
			}
		}
	}
	return pPickObject;

}

ZObject* ZObjectManager::Pick( rvector& pos, float Radius )
{
	ZObject* pPickObject = NULL;
	ZObject* pObject = NULL;
	float best_dist = Radius;

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		pObject	= iter->second;

		auto vec = pObject->GetPosition() - pos;
		if (Magnitude(vec) < best_dist)
		{
			pPickObject	= pObject;
		}
	}
	return pPickObject;
}

void ZObjectManager::Insert(ZObject* pObject)
{
	insert(value_type(pObject->GetUID(), pObject));
}

ZObject* ZObjectManager::GetObject(const MUID& uid)
{
	iterator itor = find(uid);
	if (itor == end()) return NULL;

	return (*itor).second;
}

ZActor* ZObjectManager::GetNPCObject(const MUID& uidNPC)
{
	return (ZActor*)m_NPCObjectMap.Find(uidNPC);
}