/***********************************************************************
  ZMonsterBookInterface.cpp
  
  용  도 : 몬스터 도감 인터페이스
  작성일 : 29, MAR, 2004
  작성자 : 임동환
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZMonsterBookInterface.h"			// Include ZMonsterBookInterface.h
#include "ZGameInterface.h"					// Include ZGameInterface.h
#include "ZQuest.h"							// Include ZQuest.h
#include "ZToolTip.h"						// Include ZToolTip.h


/***********************************************************************
  ZMonsterBookInterface : public
  
  desc : 생성자
************************************************************************/
ZMonsterBookInterface::ZMonsterBookInterface( void)
{
	m_pBookBgImg = NULL;
	m_pIllustImg = NULL;
}


/***********************************************************************
  ~ZMonsterBookInterface : public
  
  desc : 소멸자
************************************************************************/
ZMonsterBookInterface::~ZMonsterBookInterface( void)
{
}


/***********************************************************************
  OnCreate : public
  
  desc : 몬스터 도감 보이기
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnCreate( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 변수 초기화
	m_nCurrentPage = 0;			// 0 페이지로 초기화
	m_bIsFirstPage = true;


	// 퀘스트에 필요한 정보 로딩
	ZGetQuest()->Load();
	ReadQuestItemXML();


	// 로비 UI 감추기
	MWidget* pWidget = pResource->FindWidget( "Lobby");
	if ( pWidget)
		pWidget->Show( false);


	// 페이지를 그린다
	DrawPage();

	// 몬스터 도감 보이기
	pWidget = pResource->FindWidget( "MonsterBook");
	if ( pWidget)
		pWidget->Show( true);

	
	// 스트립 이미지 애니메이션
	MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_StripBottom");
 	if( pPicture)
		pPicture->SetAnimation( 0, 1000.0f);
	pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_StripTop");
	if( pPicture)
		pPicture->SetAnimation( 1, 1000.0f);
}


/***********************************************************************
  OnDestroy : public
  
  desc : 몬스터 도감 감추기
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnDestroy( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 몬스터 도감 감추기
	MWidget* pWidget = pResource->FindWidget( "MonsterBook");
	if ( pWidget)
		pWidget->Show( false);


	// 배경 책 이미지를 메모리로부터 삭제한다
	if ( m_pBookBgImg != NULL)
	{
		// 배경 책 이미지를 보여주는 위젯의 비트맵 이미지 포인터를 리셋한다
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_BookBG");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pBookBgImg;
		m_pBookBgImg = NULL;
	}


	// 퀘스트 아이템 리스트 삭제
	m_QuestItemDesc.clear();


	// 일러스트 이미지 삭제
	DeleteIllustImage();


	// 로비 UI 보이기
	pWidget = pResource->FindWidget( "Lobby");
	if ( pWidget)
		pWidget->Show( true);
}


/***********************************************************************
  OnPrevPage : public
  
  desc : 이전 페이지 넘기기 버튼을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnPrevPage( void)
{
	// 보여줄 페이지 번호를 구한다
	if ( m_nCurrentPage == 0)
		m_nCurrentPage = ZGetQuest()->GetNumOfPage();
	else
		m_nCurrentPage--;


	// 페이지를 그린다
	DrawPage();
}


/***********************************************************************
  OnNextPage : public
  
  desc : 다음 페이지 넘기기 버튼을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnNextPage( void)
{
	// 보여줄 페이지 번호를 구한다
	if ( m_nCurrentPage == ZGetQuest()->GetNumOfPage())
		m_nCurrentPage = 0;
	else
		m_nCurrentPage++;


	// 페이지를 그린다
	DrawPage();
}


/***********************************************************************
  DrawPage : protected
  
  desc : 페이지를 그린다
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::DrawPage( void)
{
	// 만약 표지이면(0페이지) 페이지를 그린다.
	if ( m_nCurrentPage == 0)
	{
		DrawFirstPage();
		return;
	}


	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 배경 책 이미지를 로딩한다
	if ( m_bIsFirstPage == true)
	{
		if ( m_pBookBgImg != NULL)
		{
			delete m_pBookBgImg;
			m_pBookBgImg = NULL;
		}

		m_pBookBgImg = new MBitmapR2;
		((MBitmapR2*)m_pBookBgImg)->Create( "monsterIllust.png", RGetDevice(), "interface/MonsterIllust/book_bg.jpg");
		if ( m_pBookBgImg)
		{
			// 읽어온 비트맵 이미지 포인터를 해당 위젯에 넘겨줘서 표시한다
			MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_BookBG");
			if ( pPicture)
				pPicture->SetBitmap( m_pBookBgImg->GetSourceBitmap());
		}
	}

	// 페이지 번호를 업데이트 한다
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_PageNumber");
	if ( pLabel)
	{
		char szPageNum[ 20];
		sprintf_safe( szPageNum, "- %d -", m_nCurrentPage);
		pLabel->SetText( szPageNum);
	}


	// 달성률
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_Complete");
	if ( pLabel)
		pLabel->SetText( "");


	// 현재 페이지에 대한 몬스터 정보를 얻어온다
	MQuestNPCInfo* pMonsterInfo = ZGetQuest()->GetNPCPageInfo( m_nCurrentPage - 1);


	// 몬스터의 드롭 아이템에 대한 정보를 업데이트 한다
	int nCount = 0;
	int nMatchedItem = 0;
	if ( pMonsterInfo)
	{
		MQuestDropSet* pDropItem = ZGetQuest()->GetDropTable()->Find( pMonsterInfo->nDropTableID);

		if ( pDropItem)
		{
			for ( set<int>::iterator i = pDropItem->GetQuestItems().begin();  i != pDropItem->GetQuestItems().end();  i++)
			{
				char szWidgetName[ 50];
				sprintf_safe( szWidgetName, "MonsterBook_DropItem%d", nCount);
				MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetName);
				if ( pPicture)
				{
					ZMyQuestItemMap::iterator itr = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( *i);
					if ( itr != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end())
					{
						map< int, MQuestItemSimpleDesc>::iterator Iterator;
						if ( ReadSimpleQuestItemDesc( *i, &Iterator))
						{
							pPicture->AttachToolTip( (*Iterator).second.m_szName);
							pPicture->SetBitmap( ZApplication::GetGameInterface()->GetQuestItemIcon( *i, true));
							nMatchedItem++;
						}
					}
					else
					{
						pPicture->AttachToolTip( "?????");
						pPicture->SetBitmap( MBitmapManager::Get( "slot_icon_unknown.tga"));
					}
					pPicture->Show( true);
				}
				nCount++;
			}
		}
	}
	
	int nPercentage = (int)( (float)nMatchedItem / (float)nCount * 100.0f + 0.5);
//	if ( nCount == 0)
		nPercentage = 100;

	for ( ;  nCount < 10;  nCount++)
	{
		char szWidgetName[ 50];
		sprintf_safe( szWidgetName, "MonsterBook_DropItem%d", nCount);
		MWidget* pWidget = pResource->FindWidget( szWidgetName);
		if ( pWidget)
			pWidget->Show( false);
	}


	// 해당 몬스터의 이름을 업데이트 한다
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterName");
	if (pLabel)
		pLabel->SetText( (pMonsterInfo) ? pMonsterInfo->szName : "");


	// 해당 몬스터의 등급을 업데이트 한다
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterGrade");
	if ( pLabel)
	{
		if ( nPercentage >= 20)
		{
			if  ( pMonsterInfo)
			{
				char szGrade[ 128];
				sprintf_safe( szGrade, "%s : ", ZMsg(MSG_WORD_GRADE));
				switch ( pMonsterInfo->nGrade)
				{
					case NPC_GRADE_REGULAR :
						strcat_safe( szGrade, ZMsg(MSG_WORD_REGULAR));
						break;

					case NPC_GRADE_LEGENDARY :
						strcat_safe( szGrade, ZMsg(MSG_WORD_LEGENDARY));
						break;

					case NPC_GRADE_BOSS :
						strcat_safe( szGrade, ZMsg(MSG_WORD_BOSS));
						break;

					case NPC_GRADE_ELITE :
						strcat_safe( szGrade, ZMsg(MSG_WORD_ELITE));
						break;

					case NPC_GRADE_VETERAN :
						strcat_safe( szGrade, ZMsg(MSG_WORD_VETERAN));
						break;
				}
				pLabel->SetText( szGrade);
			}
			else
				pLabel->SetText( "");
		}
		else
		{
			char szGrade[ 128];
			sprintf_safe( szGrade, "%s : ?????", ZMsg(MSG_WORD_GRADE));
			pLabel->SetText( szGrade);
		}
	}

	MTextArea* pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_MonsterDesc");
	if ( pTextArea)
	{
		pTextArea->Clear();

		if ( nPercentage >= 20)
			pTextArea->AddText( (pMonsterInfo) ? pMonsterInfo->szDesc : "", MCOLOR( 0xFF321E00));
		else
			pTextArea->AddText( "?????", MCOLOR( 0xFF321E00));
	}

	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterHP");
	if ( pLabel)
	{
		if ( nPercentage >= 50)
		{
			if  ( pMonsterInfo)
			{
				char szHP[ 128];
				strcpy_safe( szHP, "HP : ");

				if ( pMonsterInfo->nMaxHP > 200)
					strcat_safe( szHP, ZMsg(MSG_WORD_VERYHARD));
				else if ( pMonsterInfo->nMaxHP > 120)
					strcat_safe( szHP, ZMsg(MSG_WORD_HARD));
				else if ( pMonsterInfo->nMaxHP > 80)
					strcat_safe( szHP, ZMsg(MSG_WORD_NORAML));
				else if ( pMonsterInfo->nMaxHP > 30)
					strcat_safe( szHP, ZMsg(MSG_WORD_WEAK));
				else
					strcat_safe( szHP, ZMsg(MSG_WORD_VERYWEAK));

				pLabel->SetText( szHP);
			}
			else
				pLabel->SetText( "");
		}
		else
			pLabel->SetText( "HP : ?????");
	}

	pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_Attacks");
	if ( pTextArea)
	{
		pTextArea->Clear();

		if ( nPercentage >= 80)
		{
			if ( pMonsterInfo)
			{
				for ( int i = 0;  i < pMonsterInfo->nSkills;  i++)
				{
					ZSkillManager::iterator iterator = ZGetApplication()->GetSkillManager()->find( pMonsterInfo->nSkillIDs[ i]);
					if ( iterator != ZGetApplication()->GetSkillManager()->end())
						pTextArea->AddText( (*iterator).second->szName, MCOLOR( 0xFF321E00));
				}
			}
		}
		else
			pTextArea->AddText( "?????", MCOLOR( 0xFF321E00));
	}

	if ( nPercentage >= 100)
	{
		char szFileName[ 256];
		if ( pMonsterInfo)
			sprintf_safe( szFileName, "monster_Illust%02d.jpg", (int)pMonsterInfo->nID);
		SetIllustImage( (pMonsterInfo) ? szFileName : "");
	}
	else
		DeleteIllustImage();

	MWidget* pWidget = pResource->FindWidget( "MonsterBook_PrevPageButton");
	if ( pWidget)
		pWidget->Show( true);

	pWidget = pResource->FindWidget( "MonsterBook_NextPageButton");
	if ( pWidget)
	{
		if ( m_nCurrentPage == ZGetQuest()->GetNumOfPage())
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	m_bIsFirstPage = false;
}

void ZMonsterBookInterface::DrawFirstPage( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_pBookBgImg != NULL)
	{
		delete m_pBookBgImg;
		m_pBookBgImg = NULL;
	}

	m_pBookBgImg = new MBitmapR2;
	((MBitmapR2*)m_pBookBgImg)->Create( "monsterIllust.png", RGetDevice(), "interface/MonsterIllust/book_firstbg.jpg");
	if ( m_pBookBgImg)
	{
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_BookBG");
		if ( pPicture)
			pPicture->SetBitmap( m_pBookBgImg->GetSourceBitmap());
	}

	DeleteIllustImage();

	DrawComplete();

	MLabel* pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_PageNumber");
	if ( pLabel)
		pLabel->SetText( "");

	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterName");
	if (pLabel)
		pLabel->SetText( "");

	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterGrade");
	if ( pLabel)
		pLabel->SetText( "");

	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterHP");
	if ( pLabel)
		pLabel->SetText( "");

	MTextArea* pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_MonsterDesc");
	if ( pTextArea)
		pTextArea->Clear();

	pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_Attacks");
	if ( pTextArea)
		pTextArea->Clear();

	for ( int i = 0;  i < 10;  i++)
	{
		char szWidgetName[ 128];
		sprintf_safe( szWidgetName, "MonsterBook_DropItem%d", i);
		MWidget* pWidget = pResource->FindWidget( szWidgetName);
		if ( pWidget)
			pWidget->Show( false);
	}

	MWidget* pWidget = pResource->FindWidget( "MonsterBook_PrevPageButton");
	if ( pWidget)
		pWidget->Show( false);

	pWidget = pResource->FindWidget( "MonsterBook_NextPageButton");
	if ( pWidget)
		pWidget->Show( true);


	m_bIsFirstPage = true;
}

bool ZMonsterBookInterface::SetIllustImage( const char* szFileName)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	DeleteIllustImage();

	// Error check
	if ( strlen( szFileName) == 0)
		return false;

	m_pIllustImg = new MBitmapR2;
	char szFullFileName[256];
	sprintf_safe( szFullFileName, "interface/MonsterIllust/%s", szFileName);
	((MBitmapR2*)m_pIllustImg)->Create( "monsterIllust.png", RGetDevice(), szFullFileName);

	if ( !m_pIllustImg)
		return false;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_MonsterIllust");
	if ( pPicture)
		pPicture->SetBitmap( m_pIllustImg->GetSourceBitmap());

	return true;
}


/***********************************************************************
  DeleteIllustImage : public
  
  desc : 읽어왔던 몬스터 일러스트 이미지를 메모리에서 삭제한다
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZMonsterBookInterface::DeleteIllustImage( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 일러스트를 보여주는 위젯의 비트맵 이미지 포인터를 리셋한다
	MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_MonsterIllust");
	if ( pPicture)
		pPicture->SetBitmap( NULL);


	// 일러스트 이미지를 메모리로부터 삭제한다
	if ( m_pIllustImg)
	{
		delete m_pIllustImg;
		m_pIllustImg = NULL;
	}

	return true;
}


/***********************************************************************
  ReadQuestItemXML : protected
  
  desc : 퀘스트 희생 아이템 XML을 읽는다
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZMonsterBookInterface::ReadQuestItemXML()
{
	auto Filename = "System/zquestitem.xml";

	MXmlDocument xmlQuestItemDesc;
	if( !xmlQuestItemDesc.LoadFromFile(Filename, ZApplication::GetFileSystem()))
	{
		return false;
	}

	MXmlElement rootElement = xmlQuestItemDesc.GetDocumentElement();
	for ( int i = 0;  i < rootElement.GetChildNodeCount();  i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode( i);

		char szTagName[ 256];
		chrElement.GetTagName( szTagName);

		if ( szTagName[ 0] == '#')
			continue;

		bool bFindPage = false;
		if ( !_stricmp( szTagName, "ITEM"))
		{
			char szAttrName[64];
			char szAttrValue[256];
			int nItemID = 0;
			MQuestItemSimpleDesc SimpleDesc;
			
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				if ( !_stricmp( szAttrName, "id"))				// ID
					nItemID = atoi( szAttrValue);

				else if ( !_stricmp( szAttrName, "name"))		// Name
					strcpy_safe( SimpleDesc.m_szName, szAttrValue);
			}

			m_QuestItemDesc.insert( map< int, MQuestItemSimpleDesc>::value_type( nItemID, SimpleDesc));
		}
	}

	xmlQuestItemDesc.Destroy();

	return true;
}


/***********************************************************************
  ReadSimpleQuestItemDesc : protected
  
  desc : 희생 아이템 리스트로부터 아이템 정보를 얻는다
  arg  : nItemID = 아이템 ID
         pIterator* = 리턴받을 Iterrator
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZMonsterBookInterface::ReadSimpleQuestItemDesc( int nItemID,
	map< int, MQuestItemSimpleDesc>::iterator* pIterator)
{
	map< int, MQuestItemSimpleDesc>::iterator iterator;
	iterator = m_QuestItemDesc.find( nItemID);

	if ( iterator == m_QuestItemDesc.end())
		return false;

	*pIterator = iterator;

	return true;
}


/***********************************************************************
  DrawComplete : protected
  
  desc : 희생 아이템 리스트로부터 아이템 정보를 얻는다
  arg  : nItemID = 아이템 ID
         pIterator* = 리턴받을 Iterrator
  ret  : true(=success) or false(=fail)
************************************************************************/
void ZMonsterBookInterface::DrawComplete( void)
{
	int nMatchedItemNum = 0;
	int nTotalItemNum = 0;
	for ( int i = 0;  i < ZGetQuest()->GetNumOfPage(); i++)
	{
		// 현재 페이지에 대한 몬스터 정보를 얻어온다
		MQuestNPCInfo* pMonsterInfo = ZGetQuest()->GetNPCPageInfo( i);
		if ( pMonsterInfo)
		{
			MQuestDropSet* pDropItem = ZGetQuest()->GetDropTable()->Find( pMonsterInfo->nDropTableID);

			if ( pDropItem)
			{
				for ( set<int>::iterator itr = pDropItem->GetQuestItems().begin();  itr != pDropItem->GetQuestItems().end();  itr++)
				{
					if ( ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( *itr) != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end())
						nMatchedItemNum++;

					nTotalItemNum++;
				}
			}
		}
	}

	int nPercentage = (int)( (float)nMatchedItemNum / (float)nTotalItemNum * 100.0f + 0.5);


	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "MonsterBook_Complete");
	if ( pLabel)
	{
		char szComplete[ 128];
		sprintf_safe( szComplete, "%s : %d%%", ZMsg(MSG_WORD_RATE), nPercentage);
		pLabel->SetText( szComplete);
	}
}
