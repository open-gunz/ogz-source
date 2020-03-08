#include "stdafx.h"
#include "MMatchObjectCacheBuilder.h"
#include "MMatchObjCache.h"
#include "MMatchObject.h"
#include "MSharedCommandTable.h"
#include "MCommandCommunicator.h"
#include "MBlobArray.h"
#include "MMatchTransDataType.h"
#include "MMatchServer.h" 


MMatchObjectCacheBuilder::MMatchObjectCacheBuilder()
{
}

MMatchObjectCacheBuilder::~MMatchObjectCacheBuilder()
{
	Reset();
}

void MMatchObjectCacheBuilder::AddObject(MMatchObject* pObj)
{
	MMatchObjCache* pCache = new MMatchObjCache;
	int nItemID = 0;
	MMatchItemDesc* pItemDesc = NULL;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	//	_ASSERT(pCharInfo);
	if (pCharInfo == NULL)
	{
		delete pCache;
		return;
	}

	pCache->SetInfo(pObj->GetUID(), pObj->GetName(), pCharInfo->m_ClanInfo.m_szClanName,
		pCharInfo->m_nLevel, pObj->GetAccountInfo()->m_nUGrade, pObj->GetAccountInfo()->m_nPGrade);
	pCache->SetCLID(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);

	MMatchClan* pClan = MMatchServer::GetInstance()->GetClanMap()->GetClan(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);
	if (pClan)
		pCache->SetEmblemChecksum(pClan->GetEmblemChecksum());
	else
		pCache->SetEmblemChecksum(0);

	pCache->GetCostume()->nSex = pObj->GetCharInfo()->m_nSex;
	pCache->GetCostume()->nHair = pObj->GetCharInfo()->m_nHair;
	pCache->GetCostume()->nFace = pObj->GetCharInfo()->m_nFace;

	for (int i = 0; i < MMCIP_END; i++)
	{
		if (!pObj->GetCharInfo()->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
		{
			pCache->GetCostume()->nEquipedItemID[i] =
				pObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetDescID();
		}
		else
		{
			pCache->GetCostume()->nEquipedItemID[i] = 0;
		}
	}

	pCache->SetFlags(pObj->GetPlayerFlags());

	m_ObjectCacheList.push_back(pCache);
}

void MMatchObjectCacheBuilder::Reset()
{
	MMatchObjCacheList::iterator itor;
	while ((itor = m_ObjectCacheList.begin()) != m_ObjectCacheList.end()) {
		MMatchObjCache* pObjCache = (*itor);
		m_ObjectCacheList.pop_front();
		delete pObjCache;
	}
}

MCommand* MMatchObjectCacheBuilder::GetResultCmd(MATCHCACHEMODE nMode, MCommandCommunicator* pCmdComm)
{
	MCommand* pCmd = pCmdComm->CreateCommand(MC_MATCH_OBJECT_CACHE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUChar(nMode));
	int nCount = (int)m_ObjectCacheList.size();
	void* pCacheArray = MMakeBlobArray(sizeof(MMatchObjCache), nCount);
	int nIndex = 0;
	for (MMatchObjCacheList::iterator itor = m_ObjectCacheList.begin(); itor != m_ObjectCacheList.end(); itor++) {
		MMatchObjCache* pTrgCache = (MMatchObjCache*)MGetBlobArrayElement(pCacheArray, nIndex++);
		MMatchObjCache* pSrcCache = (*itor);


		pTrgCache->SetInfo(pSrcCache->GetUID(), pSrcCache->GetName(), pSrcCache->GetClanName(),
			pSrcCache->GetLevel(), pSrcCache->GetUGrade(), pSrcCache->GetPGrade());

		pTrgCache->SetFlags(pSrcCache->GetFlags());
		pTrgCache->SetCLID(pSrcCache->GetCLID());
		pTrgCache->SetEmblemChecksum(pSrcCache->GetEmblemChecksum());

		pTrgCache->AssignCostume(pSrcCache->GetCostume());
	}
	pCmd->AddParameter(new MCmdParamBlob(pCacheArray, MGetBlobArraySize(pCacheArray)));
	MEraseBlobArray(pCacheArray);

	return pCmd;
}
