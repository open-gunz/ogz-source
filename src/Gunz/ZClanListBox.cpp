#include "stdafx.h"
#include "ZMessages.h"
#include "ZApplication.h"
#include "ZClanListBox.h"

ZClanListBox::ZClanListBox(const char* szName, MWidget* pParent, MListener* pListener) 
			: MWidget(szName, pParent, pListener)
{
	m_iNumRoom		= 0;
	m_RoomWidth		= 0;
	m_RoomHeight	= 0;

	m_pRoomFrame = MBitmapManager::Get("banner_clan.bmp");

	for (int i = 0; i < NUM_DISPLAY_CLAN; i++)
	{
		ZCLANINFO *pInfo = m_pClanInfo+i;
		pInfo->bEmpty = true;
		pInfo->nClanEmblemID = 0;
	}
}

ZClanListBox::~ZClanListBox()
{
	ClearAll();
}

void ZClanListBox::OnDraw( MDrawContext* pDC )
{
	const int nWidth = 376;	// 원래 위젯 크기
	float fRatio = (float)m_Rect.w / (float)nWidth;

	const int nGap = 7;

	for(int i=0;i<NUM_DISPLAY_CLAN;i++) {

		ZCLANINFO *pInfo = m_pClanInfo+i;

		if(!pInfo->bEmpty) {
			if(m_pRoomFrame)
			{
				// 프레임
				pDC->SetBitmap(m_pRoomFrame);
				int y = (int)(fRatio * (m_pRoomFrame->GetHeight() + nGap ) * i) ;
				pDC->Draw(0, y , (int)(fRatio * m_pRoomFrame->GetWidth()), (int)(fRatio * m_pRoomFrame->GetHeight()));

				// 클랜 emblem
				MBitmap *pBitmap = ZGetEmblemInterface()->GetClanEmblem( pInfo->nClanEmblemID);
				if(pBitmap) {
					int nSize = (int)(.95f * fRatio * m_pRoomFrame->GetHeight());
					int nMargin = (int)(.05f * fRatio * m_pRoomFrame->GetHeight());
					pDC->SetBitmap(pBitmap);
					pDC->Draw(nMargin , y + nMargin , nSize , nSize);
				}

				// 대기중인 클랜 이름
				pDC->SetColor(MCOLOR(0xffffffff));
				pDC->Text((int)(fRatio*40) , (int)(y + fRatio*10) , pInfo->szClanName);

				// 대기중인 사람 수 ( x명 대기중 )
				char szBuffer[256];
				sprintf_safe(szBuffer,ZMsg( MSG_LOBBY_WAITING ),pInfo->nPlayers );
				pDC->Text((int)(fRatio*280) , (int)(y + fRatio*10) , szBuffer);
			}
		}
	}
}

void ZClanListBox::SetInfo(int nIndex, int nEmblemID, const char *szName, int nPlayers)
{
	if(nIndex<0 || nIndex>=NUM_DISPLAY_CLAN) return;

	ZCLANINFO *pInfo = m_pClanInfo+nIndex;

	ZGetEmblemInterface()->AddClanInfo(nEmblemID);
	ZGetEmblemInterface()->DeleteClanInfo(pInfo->nClanEmblemID);
	pInfo->nClanEmblemID = nEmblemID;
	strcpy_safe(pInfo->szClanName , szName);
	pInfo->nPlayers = nPlayers;
	pInfo->bEmpty = false;

}

void ZClanListBox::Clear(int nIndex)
{
	if(nIndex<0 || nIndex>=NUM_DISPLAY_CLAN) return;

	ZCLANINFO *pInfo = m_pClanInfo+nIndex;
	pInfo->bEmpty = true;
	ZGetEmblemInterface()->DeleteClanInfo(pInfo->nClanEmblemID);
	pInfo->nClanEmblemID = 0;
}

void ZClanListBox::ClearAll()
{
	for (int i = 0; i < NUM_DISPLAY_CLAN; i++)
	{
		Clear(i);
	}
}