#include "stdafx.h"
#include "ZEmblemInterface.h"
#include "MResourceManager.h"
#include "FileInfo.h"
#include "ZGameClient.h"

ZEmblemInterface::~ZEmblemInterface()
{
	Destroy();
}

void ZEmblemInterface::Create()
{
	m_pBitmapNoEmblem = MBitmapManager::Get( "no_emblem.png");
}

void ZEmblemInterface::Destroy()
{
	ClearClanInfo();
}

bool ZEmblemInterface::AddClanInfo( UINT nClanID)
{
	if(nClanID==0) return false;

	EmblemInfoMapList::iterator Iterator;
	if ( FindClanInfo( nClanID, &Iterator))
	{
		(*Iterator).second.m_nNumOfClan++;
		return false;
	}
	
	ZEmblemInfoNode EmblemNode;
	EmblemNode.m_nClanID		= nClanID;
	EmblemNode.m_nNumOfClan		= 1;

	m_EmblemInfoMap.insert( EmblemInfoMapList::value_type( nClanID, EmblemNode));

	ReloadClanInfo(nClanID);

	return true;
}

bool ZEmblemInterface::ReloadClanInfo(UINT nClanID)
{
	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator)) return false;

	ZEmblemInfoNode &EmblemNode = Iterator->second;

	char szFilePath[256];
	if(!ZGetGameClient()->GetEmblemManager()->GetEmblemPathByCLID(nClanID,szFilePath)) 
		return false;

	SAFE_DELETE(EmblemNode.m_pBitmapEmblem);

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
#endif

	MBitmapR2 *pBitmap = new MBitmapR2;
	pBitmap->Create("clanEmblem",RGetDevice(),szFilePath,false);
	EmblemNode.m_pBitmapEmblem = pBitmap;

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

	return true;
}

bool ZEmblemInterface::DeleteClanInfo( UINT nClanID)
{
	if(nClanID==0) return false;

	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator))
		return ( false);

	(*Iterator).second.m_nNumOfClan--;

	if ( (*Iterator).second.m_nNumOfClan == 0)
		m_EmblemInfoMap.erase( Iterator);

	return true;
}

bool ZEmblemInterface::ClearClanInfo( void)
{
	m_EmblemInfoMap.clear();

	return true;
}

MBitmap* ZEmblemInterface::GetClanEmblem( UINT nClanID)
{
	if(nClanID==0) return NULL;

	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator))
	{
		return m_pBitmapNoEmblem;
	}

	return Iterator->second.m_pBitmapEmblem;
}

bool ZEmblemInterface::FindClanInfo( UINT nClanID, EmblemInfoMapList::iterator* pIterator)
{
	EmblemInfoMapList::iterator Iterator;
	Iterator = m_EmblemInfoMap.find( nClanID);

	if (Iterator == m_EmblemInfoMap.end())
		return false;

	*pIterator = Iterator;

	return true;
}
