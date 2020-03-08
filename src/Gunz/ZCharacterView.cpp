#include "stdafx.h"

#include "ZCharacterView.h"
#include "RealSpace2.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZGameClient.h"
#include <algorithm>

extern bool Enable_Cloth;

u32 GetVisualWeaponID(u32 nMeleeItemID, u32 nPrimaryItemID,
									u32 nSecondaryItemID, u32 nCustom1ItemID,
									u32 nCustom2ItemID)
{
	if (nPrimaryItemID != 0) return nPrimaryItemID;
	else if (nSecondaryItemID != 0) return nSecondaryItemID;
	else if (nMeleeItemID != 0) return nMeleeItemID;
	else if (nCustom1ItemID != 0) return nCustom1ItemID;
	else if (nCustom2ItemID != 0) return nCustom2ItemID;

	return 0;
}

void ZCharacterView::RepositionItemSlots()
{
	MRECT r = GetClientRect();

#define ARMORLIST_TOP_SPACE	10
#define ARMORLIST_BOTTOM_SPACE	50
#define LEFTITEM_COUNT	5
	int nItemHeight = (r.h-ARMORLIST_TOP_SPACE-ARMORLIST_BOTTOM_SPACE)/LEFTITEM_COUNT;
	int nItemWidth = nItemHeight;

#define ITEM_SPACE	6
	for(int i=0; i<LEFTITEM_COUNT; i++){
		MRECT ir(r.x+r.w-nItemWidth-ARMORLIST_TOP_SPACE, r.y+10+nItemHeight*i, nItemWidth, nItemHeight);
		ir.w-=ITEM_SPACE;
		ir.h-=ITEM_SPACE;
		m_ItemSlots[i].m_Rect = ir;
	}

	for(int i=0; i<MMCIP_END-LEFTITEM_COUNT; i++){
		MRECT ir(r.x+nItemWidth*(i<3?i:i-3)+ARMORLIST_TOP_SPACE, r.y+r.h-nItemHeight*(i<3?2:1), nItemWidth, nItemHeight);
		ir.w-=ITEM_SPACE;
		ir.h-=ITEM_SPACE;
		m_ItemSlots[i+LEFTITEM_COUNT].m_Rect = ir;
	}
}

void ZCharacterView::OnSize(int w, int h)
{
	RepositionItemSlots();
}

void ZCharacterView::OnDraw(MDrawContext* pDC)
{
	DWORD dwCurTime = GetGlobalTimeMS();
	if ( !m_bLButtonDown && m_bAutoRotate && m_bEnableRotate)
		RotateRight( (float)( dwCurTime - m_dwTime) / 20.0f);

	m_dwTime = dwCurTime;

	if( !Enable_Cloth )
	{
		m_pTVisualMesh.m_pVisualMesh->DestroyCloth();
	}
	else if( m_pTVisualMesh.m_pVisualMesh->isChestClothMesh() )
	{
		srand( GetGlobalTimeMS());
		int rint = rand() % 10;
		force.x += rint - 7;
		force.x = min(max(force.x, 5.f), maxForce * 0.3f);
		srand( GetGlobalTimeMS());
		rint = rand() % (int)(maxForce*0.3);
		force.y += rint - 4;	
		force.y = min(max(force.y, 0.f), maxForce);
 		force.z = -90;

		m_pTVisualMesh.m_pVisualMesh->SetClothForce( force );
	}

	bool bGame = g_pGame ? true:false;
	m_pTVisualMesh.m_pVisualMesh->SetClothValue(bGame,0.f);

	ZMeshView::OnDraw(pDC);
}

bool ZCharacterView::IsDropable(MWidget* pSender)
{
	return false;
}

bool ZCharacterView::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return false;
}

ZCharacterView::ZCharacterView(const char* szName, MWidget* pParent, MListener* pListener)
	: ZMeshView(szName, pParent, pListener)
{
	SetDrawInfo(false);

	m_fDist = 235.0f;
	m_fMaxDist = 400.0f;
	m_fMinDist = 120.0f;

	m_Eye = rvector(0.0f, 92, -m_fDist);
	m_At = rvector(0, 92, 0);
	m_Up = rvector(0, 1, 0);

	memset(&m_Info, 0, sizeof(ZCharacterViewInfo));

	m_bVisibleEquipment = false;
	m_nVisualWeaponParts = MMCIP_PRIMARY;


	m_bAutoRotate = false;
	m_dwTime = GetGlobalTimeMS();


	for(int i=0; i<MMCIP_END; i++)
	{
		m_ItemSlots[i].m_nItemID = 0;
	}

	RepositionItemSlots();

}

ZCharacterView::~ZCharacterView(void)
{

}

void ZCharacterView::InitCharParts(ZCharacterView* pCharView, MMatchCharItemParts nVisualWeaponParts)
{
	if(pCharView) {

		u32 itemids[MMCIP_END];
		for (int i = 0; i < MMCIP_END; i++) itemids[i] = pCharView->m_ItemSlots[i].m_nItemID;

		InitCharParts(pCharView->m_Info.nSex, pCharView->m_Info.nHair, pCharView->m_Info.nFace,
						itemids, nVisualWeaponParts);
	}
}

void ZCharacterView::InitCharParts(MMatchSex nSex, unsigned int nHair, unsigned int nFace, 
				   u32* nEquipItemIDs, MMatchCharItemParts nVisualWeaponParts)
{
	RMesh* pMesh = NULL;
	if (nSex == MMS_MALE)
	{
		pMesh = ZGetMeshMgr()->Get("heroman1");
	}
	else
	{
		pMesh = ZGetMeshMgr()->Get("herowoman1");
	}

	if (m_pTVisualMesh.GetVMesh(false)) {
		m_pTVisualMesh.Create();
		m_pTVisualMesh.GetVMesh()->Create(pMesh);
	}

	m_Info.nSex = nSex;
	m_Info.nHair = nHair;
	m_Info.nFace = nFace;

	for (int i = 0; i < MMCIP_END; i++)
	{
		m_ItemSlots[i].m_nItemID = nEquipItemIDs[i];
	}

	u32 nWeaponID = 0;

	if (nVisualWeaponParts == MMCIP_PRIMARY)
	{
		nWeaponID= GetVisualWeaponID(nEquipItemIDs[MMCIP_MELEE],
			nEquipItemIDs[MMCIP_PRIMARY], nEquipItemIDs[MMCIP_SECONDARY],
			nEquipItemIDs[MMCIP_CUSTOM1], nEquipItemIDs[MMCIP_CUSTOM2]);
	}
	else
	{
		nWeaponID = nEquipItemIDs[nVisualWeaponParts];
	}

	if (m_pTVisualMesh.GetVMesh())
	{
		ZChangeCharParts( m_pTVisualMesh.GetVMesh() , nSex, nHair, nFace, nEquipItemIDs );
		ZChangeCharWeaponMesh( m_pTVisualMesh.GetVMesh(), nWeaponID);

		m_pTVisualMesh.GetVMesh()->SetAnimation("login_idle");
		m_pTVisualMesh.GetVMesh()->SetCheckViewFrustum(false);
	}

	if( Enable_Cloth )
	{
		m_pTVisualMesh.m_pVisualMesh->ChangeChestCloth(0.9f,1);
	}
}


void ZCharacterView::SetParts(MMatchCharItemParts nParts, unsigned int nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL)
	{
		if (nItemID != 0)
			return;
	}
	else
	{
		if (IsSuitableItemSlot(pItemDesc->m_nSlot, nParts) == false) return;

		if (pItemDesc->m_nResSex != -1)
		{
			if (pItemDesc->m_nResSex != int(ZGetMyInfo()->GetSex())) return;
		}
	}

	m_ItemSlots[nParts].m_nItemID = nItemID;

	if (nParts == MMCIP_PRIMARY)
	{
		ZChangeCharWeaponMesh(m_pTVisualMesh.GetVMesh(), m_ItemSlots[MMCIP_PRIMARY].m_nItemID);
	}
	else
	{
		u32 itemids[MMCIP_END];
		for (int i = 0; i < MMCIP_END; i++) itemids[i] = m_ItemSlots[i].m_nItemID;
		ZChangeCharParts(m_pTVisualMesh.GetVMesh(), m_Info.nSex, m_Info.nHair, m_Info.nFace, itemids);
	}

	if( Enable_Cloth )
		m_pTVisualMesh.m_pVisualMesh->ChangeChestCloth(0.9f,1);
}

void ZCharacterView::ChangeVisualWeaponParts(MMatchCharItemParts nVisualWeaponParts)
{
	if (m_pTVisualMesh.GetVMesh() == NULL) return;

	u32 itemids[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++) itemids[i] = m_ItemSlots[i].m_nItemID;

	u32 nWeaponID;

	if (nVisualWeaponParts == MMCIP_PRIMARY)
	{
		nWeaponID= GetVisualWeaponID(itemids[MMCIP_MELEE],
			itemids[MMCIP_PRIMARY], itemids[MMCIP_SECONDARY],
			itemids[MMCIP_CUSTOM1], itemids[MMCIP_CUSTOM2]);
	}
	else
	{
		nWeaponID = itemids[nVisualWeaponParts];
	}

	ZChangeCharWeaponMesh(m_pTVisualMesh.GetVMesh(), nWeaponID);

	//들고 있는 무기에 따라서 연결..

	m_pTVisualMesh.GetVMesh()->SetAnimation("login_idle");
	m_pTVisualMesh.GetVMesh()->SetCheckViewFrustum(false);

	m_nVisualWeaponParts = nVisualWeaponParts;
}

void ZCharacterView::SetSelectMyCharacter()
{
	ZMyInfo* pmi = ZGetMyInfo();
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	u32 nWeaponID = GetVisualWeaponID(pil->GetEquipedItemID(MMCIP_MELEE),
		pil->GetEquipedItemID(MMCIP_PRIMARY), pil->GetEquipedItemID(MMCIP_SECONDARY),
		pil->GetEquipedItemID(MMCIP_CUSTOM1), pil->GetEquipedItemID(MMCIP_CUSTOM2));

	u32 nItemids[MMCIP_END];

	if(pmi) 
	{
		for (int i = 0; i < MMCIP_END; i++)
		{
			nItemids[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}

		InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nItemids);
	}
}

void ZCharacterView::OnInvalidate()
{
	if(m_pTVisualMesh.m_pVisualMesh) 
		m_pTVisualMesh.m_pVisualMesh->OnInvalidate();
}
void ZCharacterView::OnRestore()
{
	if(m_pTVisualMesh.m_pVisualMesh) 
		m_pTVisualMesh.m_pVisualMesh->OnRestore();
}

void ZCharacterView::SetCharacter( MUID uid  )
{
	m_Info.UID = uid;
	
	MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
	for(MMatchObjCacheMap::iterator itor = pObjCacheMap->begin(); itor != pObjCacheMap->end(); ++itor)
	{
		MMatchObjCache* pObj = (*itor).second;
 		if( pObj->GetUID() == m_Info.UID )
		{
			InitCharParts( pObj->GetCostume()->nSex, pObj->GetCostume()->nHair, pObj->GetCostume()->nFace, 
				pObj->GetCostume()->nEquipedItemID );

			MCOLOR _color;
			char sp_name[255];
			MMatchUserGradeID gid;

			if(GetUserInfoUID(m_Info.UID,_color,sp_name,gid)) {
				SetText( sp_name );
				m_Info.nLevel = 0;
			}
			else {
				SetText( pObj->GetName() );
				m_Info.nLevel = pObj->GetLevel();
			}

			SetDrawInfo( true );
		}
	}	
}