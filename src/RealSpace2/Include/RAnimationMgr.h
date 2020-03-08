#pragma once

#include "RAnimation.h"

_NAMESPACE_REALSPACE2_BEGIN

#define MAX_ANIMATION_NODE 1000

typedef RHashList<RAnimationFile*>				RAnimationFileHashList;
typedef RHashList<RAnimationFile*>::iterator	RAnimationFileHashList_Iter;

typedef RHashList<RAnimation*>					RAnimationHashList;
typedef RHashList<RAnimation*>::iterator		RAnimationHashList_Iter;

class RAnimationFileMgr
{
public:
	RAnimationFileMgr();
	~RAnimationFileMgr();

	static RAnimationFileMgr* GetInstance();
	
	void Destroy();

	RAnimationFile* Add(const char* filename);
	RAnimationFile* Get(const char* filename);

	RAnimationFileHashList m_list;
};

inline RAnimationFileMgr* RGetAnimationFileMgr() { return RAnimationFileMgr::GetInstance(); }

class RAnimationMgr {
public:
	RAnimationMgr();
	~RAnimationMgr();

	bool LoadAnimationFileList(const char* filename) {
		return true;
	}

	inline RAnimation* Add(const char* name, const char* filename,int sID,int MotionTypeID = -1) {
		return AddAnimationFile(name,filename,sID,false,MotionTypeID);
	}

	inline RAnimation* AddGameLoad(const char* name,char* filename,int sID,int MotionTypeID = -1) {
		return AddAnimationFile(name,filename,sID,true,MotionTypeID);
	}

	RAnimation* AddAnimationFile(const char* name, const char* filename,int sID,bool notload,int MotionTypeID = -1);

	void DelAll(); 

	RAnimation* GetAnimation(const char* name,int MotionTypeID=-1);
	RAnimation* GetAnimation(int sID,int MotionTypeID=-1);
	RAnimation* GetAnimationListMap(const char* name,int wtype);

	void ReloadAll();

	void MakeListMap(int size);

public:

	int	m_id_last;
	int	m_list_map_size;

	RAnimationHashList  m_list;
	RAnimationHashList* m_list_map;

	std::vector<RAnimation*> m_node_table;
};

_NAMESPACE_REALSPACE2_END
