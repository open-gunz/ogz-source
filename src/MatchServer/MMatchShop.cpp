#include "stdafx.h"
#include "MMatchShop.h"
#include "MMatchConfig.h"

MMatchShop::MMatchShop()
{

}
MMatchShop::~MMatchShop()
{

}
bool MMatchShop::Create(const char* szDescFileName)
{
	return ReadXml(szDescFileName);

	return true;
}


void MMatchShop::Destroy()
{
	Clear();
}


bool MMatchShop::ReadXml(const char* szFileName)
{
	MXmlDocument	xmlDocument;

	xmlDocument.Create();

	if (!xmlDocument.LoadFromFile(szFileName))
	{
		xmlDocument.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlDocument.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_SELL))
		{
			ParseSellItem(chrElement);
		}
	}

	xmlDocument.Destroy();

	return true;
}

void MMatchShop::ParseSellItem(MXmlElement& element)
{
	ShopItemNode* pNewItemNode = new ShopItemNode();

	int nDescID = 0;

	element.GetAttribute(&nDescID, MTOK_SELL_ITEMID);
	element.GetAttribute(&pNewItemNode->nRentPeriodHour, MTOK_SELL_RENT_PERIOD_HOUR, 0);

	pNewItemNode->nItemID = nDescID;
	pNewItemNode->bIsRentItem = (pNewItemNode->nRentPeriodHour > 0);
	

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nDescID);
	if (pItemDesc != NULL)
	{
		// 캐쉬 아이템은 무시
		if (pItemDesc->IsCashItem()) 
		{
			delete pNewItemNode;
			return;
		}

		m_ItemNodeVector.push_back(pNewItemNode);
		m_ItemNodeMap.insert(map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode));
	}
#ifdef _QUEST_ITEM
	else
	{
		if ( QuestTestServer() == false )
		{
			delete pNewItemNode;
			return;
		}

		//MatchItem에서 없을경우 QuestItem에서 다시 한번 검사를 함.
		MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( nDescID );
		if( 0 == pQuestItemDesc )
		{
			delete pNewItemNode;
			return;
		}

		m_ItemNodeVector.push_back( pNewItemNode );
		m_ItemNodeMap.insert( map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode) );
	}
#endif
}

void MMatchShop::Clear()
{
	int nVectorSize = (int)m_ItemNodeVector.size();
	for (int i = 0; i < nVectorSize; i++)
	{
		ShopItemNode* pNode = m_ItemNodeVector[i];
		delete pNode;
	}

	m_ItemNodeVector.clear();
	m_ItemNodeMap.clear();
}


MMatchShop* MMatchShop::GetInstance()
{
	static MMatchShop g_stMatchShop;
	return &g_stMatchShop;
}

bool MMatchShop::IsSellItem(const u32 nItemID)
{
	map<unsigned int, ShopItemNode*>::iterator itor = m_ItemNodeMap.find(nItemID);
	if (itor != m_ItemNodeMap.end())
	{
		return true;
	}

	return false;
}

ShopItemNode* MMatchShop::GetSellItem(int nListIndex)
{
	if ((nListIndex < 0) || (nListIndex >= GetCount())) return NULL;

	return m_ItemNodeVector[nListIndex];
}