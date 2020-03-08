#include "stdafx.h"
#include "RAnimationMgr.h"
#include "RealSpace2.h"
#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

RAnimationFileMgr::RAnimationFileMgr() = default;
RAnimationFileMgr::~RAnimationFileMgr()
{
	Destroy();
}

void RAnimationFileMgr::Destroy()
{
	RAnimationFileHashList_Iter  node;

	for(node = m_list.begin(); node != m_list.end(); ) {
		delete (*node);
		node = m_list.erase(node);
	}

	m_list.Clear();
}

RAnimationFileMgr* RAnimationFileMgr::GetInstance()
{
	static RAnimationFileMgr m_AniMgr;
	return &m_AniMgr;
}

RAnimationFile* RAnimationFileMgr::Add(const char* filename)
{
	RAnimationFile* pFile = Get(filename);

	if( pFile ) {
		pFile->AddRef();
		return pFile;
	}

	pFile = new RAnimationFile;

	pFile->LoadAni( filename );

	pFile->SetName(filename);

	m_list.PushBack( pFile );

	return pFile;
}

RAnimationFile* RAnimationFileMgr::Get(const char* filename)
{
	return m_list.Find(filename);
}

RAnimationMgr::RAnimationMgr() {
	m_id_last = 0;
	m_list_map = NULL;
	m_list_map_size = 0;
}

RAnimationMgr::~RAnimationMgr() {
	DelAll();
}


RAnimation* RAnimationMgr::AddAnimationFile(const char* name, const char* filename,int sID,bool notload,int MotionTypeID) {

	RAnimation* node = new RAnimation;

	if(notload) {
		node->SetLoadDone(false);
	}
	else {

		if (!node->LoadAni(filename)) {
			mlog("elu %s file loading failure !!!\n",filename);
			delete node;
			return NULL;
		}
		node->SetLoadDone(true);
	}

	node->SetFileName(filename);
	node->SetName(name);
	node->m_sID = sID;
	node->SetWeaponMotionType(MotionTypeID);

	m_node_table.push_back(node);
	m_id_last++;

	if(m_id_last > MAX_ANIMATION_NODE)
		mlog("에니메이션 노드 예약 사이즈를 늘리는것이 좋겠음..\n",filename);

	m_list.PushBack(node);

	if(m_list_map) {
		if(MotionTypeID != -1) {
			if(m_list_map_size > MotionTypeID) {
				m_list_map[MotionTypeID].PushBack(node);
			}
		}
	}

	return node;
}

void RAnimationMgr::DelAll() {

	RAnimationHashList_Iter node;

	if(m_list_map_size) {

		for(int i=0;i<m_list_map_size;i++) {
			m_list_map[i].Clear();
		}

		delete [] m_list_map;
		m_list_map	= NULL;
		m_list_map_size = 0;
	}

	for(node = m_list.begin(); node != m_list.end(); ) {
		delete (*node);
		node = m_list.erase(node);
	}

	m_list.Clear();

	if(!m_node_table.empty())
		m_node_table.clear();

	m_id_last = 0;
}

void RAnimationMgr::ReloadAll()
{
	RAnimationHashList_Iter node;
	RAnimation* pANode = NULL;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		pANode = *node;
		if(pANode) {
			if(pANode->IsLoadDone()==false) {
				if (!pANode->LoadAni(pANode->m_filename)) {
					mlog("elu %s file loading failure !!!\n",pANode->m_filename);
				}
				pANode->SetLoadDone(true);
			}
		}
	}
}

void RAnimationMgr::MakeListMap(int size)
{
	if(m_list_map)
		return;

	if( size > 100 ) {
		mlog("RAnimationMgr::MakeListMap %d 는 너무과한거 아닌가?\n",size);
	}

	m_list_map = new RAnimationHashList[size];
	m_list_map_size = size;
}

RAnimation* RAnimationMgr::GetAnimationListMap(const char* name,int wtype) {

	if(m_list_map_size==0) return NULL;

	if(m_list_map_size-1 < wtype) {
		return NULL;
	}

	RAnimation* pAni = m_list_map[wtype].Find(name);

	return pAni;
}

RAnimation* RAnimationMgr::GetAnimation(const char* name,int wtype) 
{
	if(!name) 
		return NULL;

	if(name[0]==0) 
		return NULL;

	if(m_list.empty())
		return NULL;

	RAnimation* pAni = NULL;

	if(wtype != -1)
		pAni = GetAnimationListMap(name,wtype);

	if(pAni) {
		return pAni;
	}

	pAni = m_list.Find(name);
	return pAni;
}

RAnimation* RAnimationMgr::GetAnimation(int sID,int wtype) {

	RAnimationHashList_Iter node;

	if( m_list.empty() ) 
		return NULL;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		if( (*node)->CheckWeaponMotionType(wtype) )
			if( (*node)->m_sID == sID )
				return *node;
	}
	return NULL;
}

_NAMESPACE_REALSPACE2_END
