#include "stdafx.h"
#include "ZServerView.h"
#include "ZApplication.h"


#define SELECTBOX_SIZEX		220
#define SELECTBOX_SIZEY		17


ZServerView::ZServerView(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_cServerList.clear();
	m_nSelectNum = -1;
	m_nTextOffset = 0;
}


ZServerView::~ZServerView(void)
{
	ClearServerList();
}

void ZServerView::OnDraw(MDrawContext* pDC)
{
	MRECT rect;
	rect = GetClientRect();
	int nRow    = rect.w / SELECTBOX_SIZEX;
	int nColumn = rect.h / SELECTBOX_SIZEY;

	int nDebugCount = 1;
	int nMatchCount = 1;
	int nClanCount  = 1;
	int nQuestCount = 1;
	int nEventCount = 1;
	SERVERLIST::iterator itr = m_cServerList.begin();

	for ( int y = 0;  y < nColumn;  y++)
	{
		for ( int x = 0;  x < nRow;  x++)
		{
			if ( itr == m_cServerList.end())
				break;

			if ( (*itr)->nType == 0)		// Void region
			{
				itr++;
				continue;
			}


			// Set region
			MRECT rectBox;
			rectBox.x = x * SELECTBOX_SIZEX;
			rectBox.y = y * SELECTBOX_SIZEY;
			rectBox.w = SELECTBOX_SIZEX - 1;
			rectBox.h = SELECTBOX_SIZEY - 1;


			// Check select
			bool bSelected = false;
			if ( (x + y * nRow) == m_nSelectNum)
				bSelected = true;

			bool bFulled = false;
			if ( ((*itr)->nNumOfUser >= (*itr)->nCapacity) || ( !(*itr)->bIsLive))
				bFulled = true;


            // Draw select bar
//			if ( bSelected && !bFulled)
			if ( bSelected)
			{
				pDC->SetColor( MCOLOR(0xFF454545));
				pDC->FillRectangle( rectBox);
			}

            
			// Draw Icon shadow
			pDC->SetColor( MCOLOR(0xFF000000));
			pDC->FillRectangle( rectBox.x + 4, rectBox.y + 7, 7, 4);
			pDC->FillRectangle( rectBox.x + 5, rectBox.y + 6, 5, 6);


			// Set server name
			char szText[ 100];
			switch ( (*itr)->nType)
			{
				case 1 :	// Debug server
					sprintf_safe( szText, (*itr)->szName, nDebugCount++);
					if ( bSelected)		pDC->SetColor( MCOLOR(0xFFFF0098));
					else				pDC->SetColor( MCOLOR(0xFFAC5387));
					break;

				case 2 :	// Match server
					sprintf_safe( szText, (*itr)->szName, nMatchCount++);
					if ( bSelected)		pDC->SetColor( MCOLOR(0xFF00E9FF));
					else				pDC->SetColor( MCOLOR(0xFF53A4AC));
					break;

				case 3 :	// Clan server
					sprintf_safe( szText, (*itr)->szName, nClanCount++);
					if ( bSelected)		pDC->SetColor( MCOLOR(0xFFFFBE00));
					else				pDC->SetColor( MCOLOR(0xFFAC9553));
					break;

				case 4 :	// Quest server
					sprintf_safe( szText, (*itr)->szName, nQuestCount++);
					if ( bSelected)		pDC->SetColor( MCOLOR(0xFF44FF00));
					else				pDC->SetColor( MCOLOR(0xFF6BAC53));
					break;

				case 5 :	// Event server
					sprintf_safe( szText, (*itr)->szName, nEventCount++);
					if ( bSelected)		pDC->SetColor( MCOLOR(0xFF8800FF));
					else				pDC->SetColor( MCOLOR(0xFF8453AC));
					break;
			}

			if ( (*itr)->bIsLive)
			{
				if ( (*itr)->nNumOfUser >= (*itr)->nCapacity)
					sprintf_safe( szText, "%s (FULL / %d)", (*itr)->szName, (*itr)->nCapacity);
				else
					sprintf_safe( szText, "%s (%d / %d)", (*itr)->szName, (*itr)->nNumOfUser, (*itr)->nCapacity);
			}
			else
			{
				sprintf_safe( szText, "%s (checking)", (*itr)->szName);

				pDC->SetColor( MCOLOR(0xFF505050));
			}


			// Draw Icon
			pDC->FillRectangle( rectBox.x + 3, rectBox.y + 6, 7, 4);
			pDC->FillRectangle( rectBox.x + 4, rectBox.y + 5, 5, 6);


			// Draw name
			pDC->SetColor( MCOLOR(0xFF000000));
			MRECT rectText = rectBox;
			rectText.x += 14;
			rectText.y += m_nTextOffset + 1;
			pDC->Text( rectText, szText, MAM_LEFT | MAM_VCENTER);		// Shadow

			if ( (*itr)->bIsLive)
			{
				if ( bFulled)
				{
					if ( bSelected)
						pDC->SetColor( MCOLOR(0xFFD00000));
					else
						pDC->SetColor( MCOLOR(0xFF900000));
				}
				else
				{
					if ( bSelected)
						pDC->SetColor( MCOLOR(0xFFFFFFFF));
					else
						pDC->SetColor( MCOLOR(0xFFA0A0A0));
				}
			}
			else
			{
				pDC->SetColor( MCOLOR(0xFF505050));
			}

			rectText.x--;
			rectText.y--;
			pDC->Text( rectText, szText, MAM_LEFT | MAM_VCENTER);


			itr++;
		}
	}
}


bool ZServerView::OnEvent(MEvent* pEvent, MListener* pListener)
{
	bool bRet = MWidget::OnEvent(pEvent, pListener);


	// Check rect range
	MRECT r = GetClientRect();
	if ( r.InPoint( pEvent->Pos) == false)
		return bRet;


	// LButton down
	if ( pEvent->nMessage == MWM_LBUTTONDOWN)
	{
		MRECT rect;
		rect = GetClientRect();
		int nRow = rect.w / SELECTBOX_SIZEX;

		int nSelect = (pEvent->Pos.x / SELECTBOX_SIZEX) + (pEvent->Pos.y / SELECTBOX_SIZEY) * nRow;
		if ( nSelect < (int)m_cServerList.size())
		{
			SERVERLIST::iterator itr = m_cServerList.begin();
			for ( int i = 0;  i < nSelect;  i++)
				itr++;			// 아 map을 썻어야 했는데...일단 배째~ -ㅇ-; 님아 그러시면 골룸~

			if ( ( (*itr)->nType > 0) && (*itr)->bIsLive)
			{
				m_nSelectNum = nSelect;

				// 사운드 출력
				ZGetSoundEngine()->PlaySound("if_click");
				bRet = true;
			}
		}
	}


	return bRet;
}


void ZServerView::ClearServerList( void)
{
	while ( m_cServerList.size())
	{
		delete *m_cServerList.begin();
		m_cServerList.pop_front();
	}
	m_nSelectNum = -1;
}


bool ZServerView::AddServer( const char* szName, const char* szAddress, int nPort, int nType, int nNumOfUser, int nCapacity, bool IsLive )
{
	ServerInfo* pServerNode = new ServerInfo;
	strcpy_safe( pServerNode->szName, szName);
	strcpy_safe( pServerNode->szAddress, szAddress);
	pServerNode->nPort = nPort;
	pServerNode->nType = nType;
	pServerNode->nNumOfUser = nNumOfUser;
	pServerNode->nCapacity = nCapacity;
	pServerNode->bIsLive = IsLive;


	m_cServerList.push_back( pServerNode);

	return true;
}


const ServerInfo *ZServerView::GetSelectedServer() const
{
	if ( m_nSelectNum < 0)
		return GetFirstServer();

	auto itr = m_cServerList.begin();

	for ( int i = 0;  i < m_nSelectNum;  i++)
	{
		if ( itr == m_cServerList.end())
			return NULL;

		itr++;			// 아  map을 썻어야 했는데... -ㅇ-;
	}

	return (*itr);
}

const ServerInfo *ZServerView::GetFirstServer() const
{
	auto it = m_cServerList.begin();
	if (it == m_cServerList.end())
		return nullptr;

	return *it;
}


void ZServerView::SetCurrSel( int nNumber)
{
	if ( nNumber > (int)m_cServerList.size())
		return;

	if ( nNumber == -1)
		m_nSelectNum = -1;


	SERVERLIST::iterator itr = m_cServerList.begin();
	for ( int i = 0;  i < nNumber;  i++)
		itr++;			// 아 map을 썻어야 했는데...일단 배째~ -ㅇ-; 님아 그러시면 골룸~


//	if ( ( (*itr)->nType > 0) && ( (*itr)->nNumOfUser < (*itr)->nCapacity) && (*itr)->bIsLive)
	if ( (itr!=m_cServerList.end()) && ( (*itr)->nType > 0) && (*itr)->bIsLive)
		m_nSelectNum = nNumber;
	else
		m_nSelectNum = -1;
}

int ZServerView::GetCurrSel() const
{
	if( m_cServerList.empty() ) return -1;
	auto itr = m_cServerList.begin();
	for ( int i = 0;  i < m_nSelectNum;  i++)
		itr++;


	int bRet = -1;
	int nType = (*itr)->nType;
	int nNumOfUser = (*itr)->nNumOfUser;
	int nCapacity = (*itr)->nCapacity;
	bool nLive = (*itr)->bIsLive;


	if ( ( (*itr)->nType > 0) && ( (*itr)->nNumOfUser < (*itr)->nCapacity) && (*itr)->bIsLive)
		bRet = m_nSelectNum;

	return bRet;
}