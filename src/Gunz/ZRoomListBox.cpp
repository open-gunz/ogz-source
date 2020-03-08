#include "stdafx.h"

#include "ZGameClient.h"
#include ".\zroomlistbox.h"
#include "MButton.h"
#include "MBitmap.h"
#include "RTypes.h"
#include "MDrawContext.h"
#include "MDebug.h"
#include "MScrollBar.h"
#include "ZApplication.h"
#include "ZPost.h"

#define		ROOM_WIDTH					8
#define		ROOM_HEIGHT					7
const char* EMPTY_ROOM		=		"Empty";

template < class F, class S>
class string_key_equal
{
private:
	F string_key;
public:
	string_key_equal( const F& str ) : string_key(str) {}
	bool operator() (pair<const F, S> elem)
	{		return ( _stricmp(elem.first.c_str(), string_key.c_str())== 0);		}
};

ZRoomListBox::ZRoomListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_nPrevStageCount = m_nNextStageCount = -1;
	m_iNumRoom		= 0;
	m_RoomWidth		= 0;
	m_RoomHeight	= 0;
	m_pRoomFrame	= 0;
	m_Selection		= -1;
	m_currPage		=	0;
	m_iGapWidth		= ROOM_WIDTH;
	m_iGapHeight	= ROOM_HEIGHT;
	m_iGapCenter	= 10;
	m_uidSelectedPrivateStageUID	= MUID(0,0);
}

ZRoomListBox::~ZRoomListBox(void)
{
	for( map<string, MBitmap*>::iterator	iter = m_pMapImage.begin(); iter != m_pMapImage.end(); )
	{
		m_pMapImage.erase(iter++);
	}
}

void ZRoomListBox::OnDraw( MDrawContext* pDC )
{
	MBitmap* pBitmap;
	MRECT		rect;
	map<string, MBitmap*>::iterator iter;
	map<MMATCH_GAMETYPE, MBitmap*>::iterator iterIcon;
	int pressed_reposition = 0;

 	int index = 0;
	
	for( int i = 0; i < NUM_DISPLAY_ROOM; ++i )
	{
		bool	bRoomFull = false;

		if( m_Selection == i )			pressed_reposition = 1; 
		else							pressed_reposition = 0;

		const char*	mapName = MGetBannerName( m_pMapInfo[i].map_name);
		if( m_pMapInfo[i].IsEmpty )
		{
			continue;
		}
		rect = GetInitialClientRect();
		
		int width  = (int)( m_iGapWidth + ( m_RoomWidth + m_iGapWidth*2 + m_iGapCenter ) * ( index%2 ) );
		int height = (int)( m_iGapHeight + ( m_RoomHeight + m_iGapHeight ) * (int)(index*0.5f));
 		
 		if( m_pMapInfo[i].nPeople >= m_pMapInfo[i].nMaxPeople )
		{
			bRoomFull = true;
		}
		
		iter = find_if( m_pMapImage.begin(), m_pMapImage.end(), string_key_equal<string, MBitmap*>(mapName));
		if( iter != m_pMapImage.end() )
		{
			pBitmap	= iter->second;
			if( pBitmap != 0 )
			{
				if( ( m_pMapInfo[i].roomState == GMAE_CLOSED ) || bRoomFull )	
 					pDC->SetBitmapColor(125,125,125,255);
				else					
 					pDC->SetBitmapColor(255,255,255,255);

				pDC->SetBitmap( pBitmap ); 
				pDC->Draw( width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight );
			}

			if(m_pRoomFrame!=0)
			{
				pDC->SetBitmap(m_pRoomFrame);
 				pDC->Draw(width + pressed_reposition, height + pressed_reposition, m_RoomWidth * 0.75, m_RoomHeight, 0, 0, 512, 32 );
			}
		}

		char szBuf[128];
		MRECT r;

 		r.x = width + m_RoomWidth*0.01f	+ pressed_reposition;
		r.y = height + m_RoomHeight*0.1f + pressed_reposition;
		r.w = m_RoomWidth*0.1;
		r.h = m_RoomHeight *0.5;
     	sprintf_safe(szBuf,"%03d", m_pMapInfo[i].RoomNumber);

		pDC->SetFont( MFontManager::Get("FONTc8b") );
		pDC->SetColor( 0,0,0);
		pDC->Text( MRECT( r.x+1, r.y+1, r.w, r.h), szBuf);
		if(  m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull )
			pDC->SetColor( 115,146,173 );
		else
			pDC->SetColor( 181, 247, 66 );
		pDC->Text( r, szBuf );

		r.x = width + m_RoomWidth*0.01f + pressed_reposition;
		r.y = height + m_RoomHeight*0.5f + pressed_reposition;
		r.w = m_RoomWidth*0.1f;
		r.h = m_RoomHeight*0.5f;
		sprintf_safe(szBuf,"%d/%d", m_pMapInfo[i].nPeople, m_pMapInfo[i].nMaxPeople );

		pDC->SetColor( 0,0,0);
		pDC->Text( MRECT( r.x+1, r.y+1, r.w, r.h), szBuf);
		if(  m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull )
			pDC->SetColor( 115,146,173 );
		else
			pDC->SetColor( 181, 247, 66 );
		pDC->Text( r, szBuf );

		// 방 이름
		pDC->SetFont(MFontManager::Get("FONTc8b"));
		r.x = width + m_RoomWidth * 0.12 + pressed_reposition;
		r.y = height - m_RoomHeight * 0.15 + pressed_reposition;
		r.w = m_RoomWidth * 0.75;
		r.h = m_RoomHeight/**0.85*/;

		MFont * pFont = pDC->GetFont();
		char szBufTemp[SMI_MAPNAME_LENGTH];
		int strLength = 0;
		int RoomWidth = pFont->GetWidth(m_pMapInfo[i].room_name);
		if (RoomWidth > m_RoomWidth*0.7)
		{
			while (strLength < 29)
			{
				if (m_pMapInfo[i].map_name[strLength] == '0') // 이름의 끝
					strcpy(szBufTemp, m_pMapInfo[i].room_name);
				if (((unsigned char)m_pMapInfo[i].room_name[strLength]) > 127)
					strLength += 2;
				else
					++strLength;
			}
			strncpy(szBufTemp, m_pMapInfo[i].room_name, strLength * sizeof(char));
			szBufTemp[strLength] = '\0';
			strcat(szBufTemp, "...");

			pDC->SetColor(0, 0, 0);
			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBufTemp, MAM_LEFT);
			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
				pDC->SetColor(115, 146, 173);
			else
				pDC->SetColor(255, 255, 255);
			pDC->Text(r, szBufTemp, MAM_LEFT);
		}
		else
		{
			pDC->SetColor(0, 0, 0);
			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), m_pMapInfo[i].room_name, MAM_LEFT);
			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
				pDC->SetColor(115, 146, 173);
			else
				pDC->SetColor(255, 255, 255);
			pDC->Text(r, m_pMapInfo[i].room_name, MAM_LEFT);
		}

		pDC->SetFont(MFontManager::Get("FONTc8b"));
		r.x = width + m_RoomWidth * 0.12 + pressed_reposition;
		r.y = height + m_RoomHeight * 0.2 + pressed_reposition;
		r.w = m_RoomWidth * 0.75;
		r.h = m_RoomHeight/**0.85*/;

		int RoomWidth1 = pFont->GetWidth(m_pMapInfo[i].map_name);
		if (RoomWidth1 > m_RoomWidth*0.7)
		{
			while (strLength < 29)
			{
				if (m_pMapInfo[i].map_name[strLength] == '0') // 이름의 끝
					strcpy_safe(szBufTemp, m_pMapInfo[i].map_name);
				if (((unsigned char)m_pMapInfo[i].map_name[strLength]) > 127)
					strLength += 2;
				else
					++strLength;
			}
			strncpy_safe(szBufTemp, m_pMapInfo[i].map_name, strLength * sizeof(char));
			szBufTemp[strLength] = '\0';
			strcat_safe(szBufTemp, "...");

			pDC->SetColor(0, 0, 0);
			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBufTemp, MAM_LEFT);
			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
				pDC->SetColor(115, 146, 173);
			else
				pDC->SetColor(255, 255, 255);
			pDC->Text(r, szBufTemp, MAM_LEFT);
		}
		else
		{
			pDC->SetColor(0, 0, 0);
			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), m_pMapInfo[i].map_name, MAM_LEFT);
			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
				pDC->SetColor(115, 146, 173);
			else
				pDC->SetColor(255, 255, 255);
			pDC->Text(r, m_pMapInfo[i].map_name, MAM_LEFT);
		}

		r.x = width + m_RoomWidth*0.75f + pressed_reposition;
		r.y = height + pressed_reposition;
		r.w = m_RoomWidth * 0.2f ;
		r.h = m_RoomHeight;

		if( m_pMapInfo[i].bLimitLevel )
		{
			char szBufTemp[64];
			sprintf_safe( szBufTemp, "%d~%d", max(m_pMapInfo[i].nMasterLevel - m_pMapInfo[i].nLimitLevel,1), m_pMapInfo[i].nMasterLevel + m_pMapInfo[i].nLimitLevel);

			pDC->SetColor( 0,0,0);
			pDC->Text( MRECT( r.x+1, r.y+1, r.w, r.h), szBufTemp);
			if( m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull )
				pDC->SetColor( 100, 100, 100 );
			else
				pDC->SetColor( 181, 247, 66 );
			pDC->Text( r, szBufTemp );
		}
		
		if (m_pMapInfo[i].bPrivate)
		{
			// ºñ¹Ð¹æÀÌ¸é °ÔÀÓ¸ðµå ¾ÆÀÌÄÜ´ë½Å ¿­¼è¾ÆÀÌÄÜÀÌ ³ª¿Â´Ù.
			// ¿­¼è¾ÆÀÌÄÜÀº ÇÏµåÄÚµåµÊ. ³ªÁß¿¡ ÀÏ¹ÝÈ­ÇØ¼­ xml·Î »©³ö¾ßÇÒ µí..

			#define FN_ROOMLIST_PRIVATE_KEY		"in_key.png"
			pBitmap = MBitmapManager::Get(FN_ROOMLIST_PRIVATE_KEY);
			if (pBitmap != NULL) 
			{
				float x, y;
				x = width + m_RoomWidth*0.9 + pressed_reposition;
				y = height + pressed_reposition + ((m_RoomHeight-pBitmap->GetHeight())/2);
				pDC->SetBitmap( pBitmap );
				pDC->Draw(x, y, pBitmap->GetWidth(), pBitmap->GetHeight());
			}
		}
		else
		{
			iterIcon = m_pIconImage.find( m_pMapInfo[i].nGame_Type );
			if( iterIcon != m_pIconImage.end() )
			{
				pBitmap = iterIcon->second;
				if( pBitmap != 0)
				{
 					r.x = width + m_RoomWidth*0.9 + pressed_reposition;
					r.y = height + m_RoomHeight/4.7 + pressed_reposition;

					// ¾ÆÀÌÄÜ
					if( m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull )
					{
						pDC->SetBitmapColor(100,100,100,100);
					}
					else
					{
  						pDC->SetBitmapColor(255,255,255,255);
					}

					pDC->SetBitmap( pBitmap );
					pDC->Draw(r.x, height + pressed_reposition, m_RoomHeight, m_RoomHeight);
				}			
			}
		}


		// ÇÃ·¹ÀÌ ÁßÀÌ¸é ÇÃ·¹ÀÌ ¾ÆÀÌÄÜÀ» Ç¥½ÃÇÑ´Ù.
		if ( m_pMapInfo[i].roomState == GAME_PLAYING)
		{
			// ¿ª½Ã ÇÃ·¹ÀÌ ¾ÆÀÌÄÜµµ ÇÏµåÄÚµù... -_-;
			#define FN_ROOMLIST_PLAYICON		"icon_play.tga"
			pBitmap = MBitmapManager::Get( FN_ROOMLIST_PLAYICON);
			if (pBitmap != NULL) 
			{
				float x, y;
				x = width  + (m_RoomWidth  * 0.955) + pressed_reposition;
				y = height + (m_RoomHeight * 0.54)  + pressed_reposition;
				pDC->SetBitmap( pBitmap );
				pDC->Draw(x, y, (m_RoomHeight * 0.5), (m_RoomHeight * 0.5));
			}
		}

		//¿Ó±¸(Å×µÎ¸®)
		if( i == m_Selection)
		{
			if( m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull) pDC->SetColor(115,146,173);
			else pDC->SetColor( 181, 247, 66 );
		}
		else pDC->SetColor(128,128,128,255);
		pDC->Rectangle(width+pressed_reposition, height+pressed_reposition, m_RoomWidth, m_RoomHeight );

		++index;
	}
	pDC->SetOpacity( 255 );
	pDC->SetBitmapColor(255,255,255,255);
}

bool ZRoomListBox::OnShow()
{
	return true;
}

bool ZRoomListBox::OnCommand( MWidget* pWidget, const char* szMassage )
{
	SetPage();
	return true;
}

bool ZRoomListBox::OnEvent( MEvent* pEvent, MListener* pListener )
{
	MRECT r = GetInitialClientRect();

	MPOINT ClickPos = pEvent->Pos;

	MRECT	rect;

	switch(pEvent->nMessage)
	{
		if(r.InPoint(ClickPos)==true)
		{
			case MWM_LBUTTONDOWN:
			{	
				for( int i = 0 ; i < NUM_DISPLAY_ROOM; ++i )
				{
					int width = (int)((m_RoomWidth + ROOM_WIDTH)*(i%2)+ ROOM_WIDTH);
					int height = (int)((m_RoomHeight + ROOM_HEIGHT)*((int)i/2) + ROOM_HEIGHT);
					rect.x = width;
					rect.y = height;
					rect.w = m_RoomWidth;
					rect.h = m_RoomHeight;
					if(	rect.InPoint( ClickPos ) )
					{
						if( m_pMapInfo[i].IsEmpty )
							continue;
						m_Selection	= i;
						SetPrivateStageUID(MUID(0,0));

						return true;
					}
					m_Selection = -1;
				}
				return true;
			}
			case MWM_LBUTTONDBLCLK:
			{
				for( int i = 0 ; i < NUM_DISPLAY_ROOM; ++i )
				{
					int width = (int)((m_RoomWidth + ROOM_WIDTH)*(i%2)+ ROOM_WIDTH);
					int height = (int)((m_RoomHeight + ROOM_HEIGHT)*((int)i/2) + ROOM_HEIGHT);
					rect.x = width;
					rect.y = height;
					rect.w = m_RoomWidth;
					rect.h = m_RoomHeight;
					if( rect.InPoint( ClickPos ) )
					{
						if( m_pMapInfo[i].IsEmpty )
							continue;
						m_Selection	= i;
						SetPrivateStageUID(MUID(0,0));

						RequestSelStageJoin();
						return true;
					}
					m_Selection = -1;
				}
				return true;
			}
		}
	}
	return false;
}

void ZRoomListBox::SetRoomName( int i, char* pRoomName, sMapInfo* info )
{
	strcpy_safe( m_pMapInfo[i].map_name,  info->map_name );
	strcpy_safe( m_pMapInfo[i].room_name,  info->room_name );
	m_pMapInfo[i].nPeople		= info->nPeople;
	m_pMapInfo[i].RoomNumber	= info->RoomNumber;
	m_pMapInfo[i].nMaxPeople	= info->nMaxPeople;
	m_pMapInfo[i].roomState		= info->roomState;
}

void ZRoomListBox::SetBannerImage( char* pBannerName, MBitmap* pBitmap )
{
	m_pMapImage.insert( map<string, MBitmap*>::value_type( string(pBannerName), pBitmap) );
}
void ZRoomListBox::SetIconImage( MMATCH_GAMETYPE type, MBitmap* pBitmap )
{
	m_pIconImage.insert( map<MMATCH_GAMETYPE, MBitmap*>::value_type( type, pBitmap ) );
}

void ZRoomListBox::SetRoom(const _RoomInfoArg* pRoomInfo)
{
	int nIndex = pRoomInfo->nIndex; 

	if ((pRoomInfo->nIndex < 0) || (pRoomInfo->nIndex >= NUM_DISPLAY_ROOM)) return;

	m_pMapInfo[nIndex].IsEmpty = false;
	m_pMapInfo[nIndex].uidStage = pRoomInfo->uidStage;
	
	strcpy_safe(m_pMapInfo[nIndex].map_name, pRoomInfo->szMapName);
	strcpy_safe(m_pMapInfo[nIndex].room_name, pRoomInfo->szRoomName);
	m_pMapInfo[nIndex].nMaxPeople = pRoomInfo->nMaxPlayers;
	m_pMapInfo[nIndex].nPeople = pRoomInfo->nCurrPlayers;
	m_pMapInfo[nIndex].RoomNumber = pRoomInfo->nRoomNumber;
	m_pMapInfo[nIndex].nGame_Type	= pRoomInfo->nGameType;
	m_pMapInfo[nIndex].bForcedEnter = pRoomInfo->bForcedEntry;
	m_pMapInfo[nIndex].bLimitLevel	=	pRoomInfo->bLimitLevel;
	m_pMapInfo[nIndex].nMasterLevel = pRoomInfo->nMasterLevel;
	m_pMapInfo[nIndex].nLimitLevel = pRoomInfo->nLimitLevel;
	m_pMapInfo[nIndex].bPrivate = pRoomInfo->bPrivate;

 	if( pRoomInfo->nStageState == STAGE_STATE_STANDBY )
	{
		m_pMapInfo[nIndex].roomState = GAME_WAITING; // ´ë±âÁß
	}
	//else if(	pRoomInfo->nStageState == STAGE_STATE_CLOSE && !pRoomInfo->bForcedEntry )
	//{
	//	m_pMapInfo[nIndex].roomState	= GMAE_CLOSED; // ³­ÀÔ ºÒ°¡
	//}
	else if( pRoomInfo->nStageState == STAGE_STATE_RUN && pRoomInfo->bForcedEntry)
	{
		//m_pMapInfo[nIndex].roomState	= GAME_WAITING;
		m_pMapInfo[nIndex].roomState	= GAME_PLAYING	; // ³­ÀÔ °¡´É
	}
	else
	{
		m_pMapInfo[nIndex].roomState	= GMAE_CLOSED; // ³­ÀÔ ºÒ°¡
	}

}

void ZRoomListBox::SetEmptyRoom(int nIndex)
{
	m_pMapInfo[nIndex].IsEmpty = true;
	m_pMapInfo[nIndex].uidStage = MUID(0,0);
	m_pMapInfo[nIndex].bPrivate = false;
}


void ZRoomListBox::Clear()
{
	m_iNumRoom = 0;
	for (int i = 0; i < NUM_DISPLAY_ROOM; i++)
	{
		SetEmptyRoom(i);
	}
}

void ZRoomListBox::SetPage( )
{
	// ¼­¹ö¿¡ ÇÊ¿äÇÑ ÆäÀÌÁö¸¦ ¿äÃ»ÇÑ´Ù
	//static int oldvalue = 0;
 //	int value = m_pScrollBar->GetValue();
 //	if( abs(oldvalue -value) == 1 )
	//{
	//	if( oldvalue > value )
	//		value -= 19;
	//	else
	//		value += 19;
	//	value = min( max( value, 0 ), 999 );
	//	m_pScrollBar->SetValue( value );
	//}
	//oldvalue = m_pScrollBar->GetValue();
	//int i =min(max((int)m_pScrollBar->GetValue()/100, 0),9);
	//if( i == m_currPage ) return;
	//
	//m_currPage = i;
	//mlog("%d ÆäÀÌÁö ¿äÃ»\n",i);
}

void ZRoomListBox::SetScroll(int nPrevStageCount, int nNextStageCount)
{
	m_nNextStageCount = nNextStageCount;
	m_nPrevStageCount = nPrevStageCount;
}

int ZRoomListBox::GetFirstStageCursor()
{
	for (int i = 0; i < NUM_DISPLAY_ROOM; i++)
	{
		if (m_pMapInfo[i].IsEmpty == false)
		{
			return m_pMapInfo[i].RoomNumber - 1;
			break;
		}
	}

	return 0;
}

int ZRoomListBox::GetLastStageCursor()
{
	for (int i = NUM_DISPLAY_ROOM-1; i >= 0; i--)
	{
		if (m_pMapInfo[i].IsEmpty == false)
		{
			return m_pMapInfo[i].RoomNumber - 1;
			break;
		}
	}

	return 0;
}

MUID ZRoomListBox::GetSelRoomUID()
{
	if ((m_Selection >= 0) && (m_Selection < NUM_DISPLAY_ROOM))
	{
		return m_pMapInfo[m_Selection].uidStage;
	}

	return MUID(0,0);
}

MUID ZRoomListBox::GetSelectedPrivateStageUID()
{
	return m_uidSelectedPrivateStageUID;
}

#include "ZMyInfo.h"
void ZRoomListBox::RequestSelStageJoin()
{
	MUID uidChar = ZGetGameClient()->GetPlayerUID();

	if (GetSelRoomUID() != MUID(0,0))
	{
		if ( (ZGetMyInfo()->IsAdminGrade() == false) && (m_pMapInfo[m_Selection].bPrivate) )
		{
			char szStageName[SMI_ROOMNAME_LENGTH] = "";
			MUID uidStage = MUID(0,0);

			const sMapInfo* pSelRoomInfo = GetSelMapInfo();

			if (pSelRoomInfo != NULL)
			{
				strcpy_safe(szStageName, pSelRoomInfo->room_name);
				uidStage = pSelRoomInfo->uidStage;
			}

			SetPrivateStageUID(uidStage);

			// ºñ¹Ð¹æ
			ZApplication::GetGameInterface()->ShowPrivateStageJoinFrame(szStageName);
		}
		else
		{
			ZPostRequestStageJoin(uidChar, GetSelRoomUID());	// °ø°³¹æ
			ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		}
	}
}

void ZRoomListBox::RequestSelPrivateStageJoin()
{
	MUID uidChar = ZGetGameClient()->GetPlayerUID();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MUID uidStageUID = GetSelectedPrivateStageUID();

	if (uidStageUID == MUID(0,0)) uidStageUID = GetSelRoomUID();

	if (uidStageUID != MUID(0,0))
	{
		MEdit* pPassEdit = (MEdit*)pResource->FindWidget("PrivateStagePassword");
		if (pPassEdit!=NULL)
		{
			char szPassword[256];
			strcpy_safe(szPassword, pPassEdit->GetText());

			ZPostRequestPrivateStageJoin(uidChar, uidStageUID, szPassword);
			ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		}
	}
}

void ZRoomListBox::Resize( float w, float h )
{
	m_RoomWidth	*= w;
	m_RoomHeight	*= h;
	m_iGapCenter	*= w;
	m_iGapWidth	*= w;
	m_iGapHeight	*= h;
}

const sMapInfo* ZRoomListBox::GetSelMapInfo()
{ 
	if ((m_Selection >= 0) && (m_Selection < NUM_DISPLAY_ROOM)) return &m_pMapInfo[m_Selection];
	return NULL;
}


void ZRoomListBox::SetPrivateStageUID(const MUID& uidStage)
{
	m_uidSelectedPrivateStageUID = uidStage;
}