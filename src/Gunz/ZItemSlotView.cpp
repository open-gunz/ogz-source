#include "stdafx.h"

#include "ZItemSlotView.h"
#include "ZEquipmentListBox.h"
#include "ZIDLResource.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "ZPost.h"
#include "MComboBox.h"


ZItemSlotView::ZItemSlotView(const char* szName, MWidget* pParent, MListener* pListener)
: MButton(szName, pParent, pListener)
{
	m_nParts = MMCIP_END;
	m_pBackBitmap = NULL;
	m_bSelectBox = false;
	m_bKindable = false;
	m_bDragAndDrop = false;
	m_szItemSlotPlace[0] = '\0';
	SetFocusEnable(true);
}

ZItemSlotView::~ZItemSlotView(void)
{
}


void ZItemSlotView::SetDefaultText(MMatchCharItemParts nParts)
{
	switch (nParts)
	{
	case MMCIP_HEAD: SetText("< Head >"); break;
	case MMCIP_CHEST: SetText("< Chest >"); break;
	case MMCIP_HANDS: SetText("< Hands >"); break;
	case MMCIP_LEGS: SetText("< Legs >"); break;
	case MMCIP_FEET: SetText("< Feet >"); break;
	case MMCIP_FINGERL: SetText("< Left Finger >"); break;
	case MMCIP_FINGERR: SetText("< Right Finger >"); break;
	case MMCIP_MELEE: SetText("< Melee >"); break;
	case MMCIP_PRIMARY: SetText("< Primary Weapon >"); break;
	case MMCIP_SECONDARY: SetText("< Secondary Weapon >"); break;
	case MMCIP_CUSTOM1: SetText("< Item 1 >"); break;
	case MMCIP_CUSTOM2: SetText("< Item 2 >"); break;
	default : SetText(""); break;
	}
}

void ZItemSlotView::OnDraw(MDrawContext* pDC)
{
	if ( m_nParts == MMCIP_END)
		return;

	u32 nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
	MMatchItemDesc* pItemDesc = NULL;
	if (nItemID != 0) 
		pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	MRECT r;

	// Draw select box
	if ( m_bSelectBox)
	{
		r = GetClientRect();
		r.x -= 5;
		r.y -= 5;
		r.w += 10;
		r.h += 10;
		pDC->SetColor( MCOLOR( 0x60808080));
		pDC->FillRectangle( r);
	}

	// Draw BItmap
	int w, h;
	r = GetRect();
	w = min ( r.w, r.h);
	h = min ( r.w, r.h);

	if ( (nItemID == 0) || (pItemDesc == NULL))
	{
		if ( m_pBackBitmap != NULL)
		{
			pDC->SetBitmap(m_pBackBitmap);
			pDC->Draw(0, 0, w, h);
		}
	}
	else
	{
		MBitmap* pIconBitmap = GetItemIconBitmap(pItemDesc);

		if (pIconBitmap)
		{
			pDC->SetBitmap(pIconBitmap);
			pDC->Draw( 0, 0, w, h);
		}
	}

	// Draw Item box
	if ( m_bKindable)
	{
		pDC->SetColor( MCOLOR( 0xFFFFFFFF));
		pDC->Rectangle( 0, 0, w, h);
	}
	else
	{
		pDC->SetColor( MCOLOR( 0xFF303030));
		pDC->Rectangle( 0, 0, w, h);
	}


	// Draw Name
	r = GetClientRect();
	r.x += w + 10;
	r.w -= (w + 10);
	if ( pItemDesc != NULL)
	{
		pDC->SetColor( MCOLOR(0xFFC0C0C0));
		pDC->Text( r, pItemDesc->m_szName, MAM_VCENTER);
	}
	else
	{
//		SetDefaultText( m_nParts);

		if ( m_bSelectBox)
			pDC->SetColor( MCOLOR(0xFF202020));
		else
			pDC->SetColor( MCOLOR(0xFF404040));

		pDC->Text( r, m_szName, MAM_VCENTER);
	}
}

void ZItemSlotView::SetParts(MMatchCharItemParts nParts)
{
	SetDefaultText(nParts);
	m_nParts = nParts;
}

bool ZItemSlotView::IsDropable(MWidget* pSender)
{
	if (pSender == NULL)
		return false;

	return true;
}


bool ZItemSlotView::IsEquipableItem(u32 nItemID, int nPlayerLevel, MMatchSex nPlayerSex)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;

	// 성별 제한 조건
	if (pItemDesc->m_nResSex != -1)
	{
		if (pItemDesc->m_nResSex != int(nPlayerSex)) return false;
	}

	// 레벨 제한 조건
	if (pItemDesc->m_nResLevel > nPlayerLevel) return false;


	return true;
}

bool ZItemSlotView::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || ( m_bDragAndDrop == false))
		return false;

	if (strcmp(pSender->GetClassName(), MINT_EQUIPMENTLISTBOX)==0) {

		// widget id 설정
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		ZEquipmentListBox* pWidget = (ZEquipmentListBox*)pResource->FindWidget("EquipmentList");
		if (pWidget==NULL) return false;

		u32 nItemIndex = 0;
		if (pWidget->IsSelected())
		{
			nItemIndex = pWidget->GetSelIndex();
		}

		MUID uidItem =	ZGetMyInfo()->GetItemList()->GetItemUIDEquip( nItemIndex );
						
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( 
			ZGetMyInfo()->GetItemList()->GetItemIDEquip( nItemIndex )
		);

		if (pItemDesc == NULL) return false;
		if (!IsSuitableItemSlot(pItemDesc->m_nSlot, m_nParts)) return false;
		if (!IsEquipableItem(pItemDesc->m_nID, 99, ZGetMyInfo()->GetSex())) return false;

		ZApplication::GetGameInterface()->Equip( m_nParts, uidItem );

	}
	
	else if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)==0) {	// Equip 슬롯에서 Equip 슬롯으로
		ZItemSlotView* pWidget = (ZItemSlotView*)pSender;
		if (pWidget == this) return false;
		MUID uidItem = ZGetMyInfo()->GetItemList()->GetEquipedItemUID(pWidget->GetParts());
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));
		if (pItemDesc == NULL) return false;
		if (!IsSuitableItemSlot(pItemDesc->m_nSlot, m_nParts)) return false;
		if (!IsEquipableItem(pItemDesc->m_nID, 99, ZGetMyInfo()->GetSex())) return false;

		ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pWidget->GetParts());
		ZApplication::GetGameInterface()->Equip(m_nParts, uidItem);
	}

	if (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0)
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
	else if ( strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0)
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);

	return true;
}

bool ZItemSlotView::OnEvent(MEvent* pEvent, MListener* pListener)
{
	bool bRet = MButton::OnEvent(pEvent, pListener);

	m_bSelectBox = false;

	// Check rect range
	MRECT r = GetClientRect();
	if ( r.InPoint( pEvent->Pos) == false)
		return bRet;


	if ( pEvent->nMessage == MWM_LBUTTONDOWN)
	{
		m_bSelectBox = true;

		// 상점 및 장비 아이템 슬롯일 경우
		u32 nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
		MMatchItemDesc* pItemDesc = NULL;
		if (nItemID != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			// 아이템 설명 업데이트
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem( m_nParts);
			char szItemDescription1[128], szItemDescription2[128], szItemDescription3[128], szItemIcon[128];
			sprintf_safe( szItemDescription1, "%s_ItemDescription1", m_szItemSlotPlace);
			sprintf_safe( szItemDescription2, "%s_ItemDescription2", m_szItemSlotPlace);
			sprintf_safe( szItemDescription3, "%s_ItemDescription3", m_szItemSlotPlace);
			sprintf_safe( szItemIcon,         "%s_ItemIcon",         m_szItemSlotPlace);

			ZGetGameInterface()->SetupItemDescription(  pItemDesc,
														szItemDescription1,
														szItemDescription2,
														szItemDescription3,
														szItemIcon,
														pItemNode);

			// 드래그 & 드롭
			if (pItemDesc && m_bDragAndDrop)
			{
				MBitmap* pIconBitmap = GetItemIconBitmap(pItemDesc, true);
				Mint::GetInstance()->SetDragObject(this, pIconBitmap, pItemDesc->m_szName, pItemDesc->m_szName);
			}

			ZApplication::GetGameInterface()->SetKindableItem( pItemDesc->m_nSlot);
		}


		// 희생 아이템 슬롯일 경우
		if ( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) || (strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0))
		{
			int nSlotNum = (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1;

			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].IsExist())
				return bRet;

			// 드래그 & 드롭
			if ( m_bDragAndDrop)
			{
				Mint::GetInstance()->SetDragObject( this,
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetIconBitmap(),
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetName(),
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetName());
			}
		}
	}

	else if ( pEvent->nMessage == MWM_LBUTTONUP)
	{
		ZApplication::GetGameInterface()->SetKindableItem( MMIST_NONE);
	}
	
	// 더블클릭시 아이템 해제
	else if ( pEvent->nMessage == MWM_LBUTTONDBLCLK)
	{
		u32 nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
		MMatchItemDesc* pItemDesc = NULL;
		if (nItemID != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			if (pItemDesc && m_bDragAndDrop)
			{
				ZPostRequestTakeoffItem( ZGetGameClient()->GetPlayerUID(), m_nParts);
				// The server sends this automatically if UPDATE_STAGE_EQUIP_LOOK is defined.
#ifndef UPDATE_STAGE_EQUIP_LOOK
				ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
#endif
			}
		}

		// 희생 아이템 슬롯일 경우
		if ( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) || (strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0))
		{
			ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
		}
	}


	return bRet;
}


void ZItemSlotView::SetBackBitmap(MBitmap* pBitmap)
{
	m_pBackBitmap = pBitmap;
	if( !GetStretch() && m_pBackBitmap!=NULL)
	{
		SetSize(m_pBackBitmap->GetWidth(), m_pBackBitmap->GetHeight());
	}

}

void ZItemSlotView::SetIConBitmap(MBitmap* pBitmap)
{
}

void ZItemSlotView::EnableDragAndDrop( bool bEnable)
{
	m_bDragAndDrop = bEnable;
}

void ZItemSlotView::SetKindable( bool bKindable)
{
	m_bKindable = bKindable;
}
