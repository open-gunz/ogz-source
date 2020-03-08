#include "stdafx.h"

#include "RAnimation.h"

#include "RealSpace2.h"


_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

/////////////////////////////////////////////////////////////////////////

AnimationType RAnimation::GetAnimationType() 
{
	if(!m_pAniData)	return RAniType_TransForm;
	return m_pAniData->m_ani_type;
}

int RAnimation::GetMaxFrame() 
{
	if(!m_pAniData)	return 0;
	return m_pAniData->m_max_frame;
}

int RAnimation::GetAniNodeCount() 
{
	if(!m_pAniData)	return 0;
	return m_pAniData->m_ani_node_cnt;
}

RAnimationNode* RAnimation::GetAniNode(int i) 
{
	if(!m_pAniData)	return 0;
	return m_pAniData->m_ani_node[i];
}

RAnimationNode* RAnimation::GetBipRootNode() 
{
	if(!m_pAniData)	return 0;
	return m_pAniData->m_pBipRootNode;
}

void RAnimation::SetWeaponMotionType(int wtype) 
{
	m_weapon_motion_type = wtype;
}

int  RAnimation::GetWeaponMotionType() 
{
	return m_weapon_motion_type;
}

RAnimationNode* RAnimation::GetNode(const char* name)
{
	if(m_pAniData==NULL) 
		return NULL;

	return m_pAniData->GetNode(name);
}

void RAnimation::SetFileName(const char* name)
{
	if(!name[0]) return;
	strcpy_safe(m_filename,name);
}

char* RAnimation::GetFileName()
{
	return m_filename;
}

char* RAnimation::GetSoundFileName() 
{
	return m_sound_name;
}

bool RAnimation::IsHaveSoundFile() 
{
	return m_bIsHaveSound;
}

void RAnimation::SetLoadDone(bool b) 
{
	m_isLoadDone = b;
}

bool RAnimation::IsLoadDone() 
{
	return m_isLoadDone;
}

void RAnimation::ClearSoundFile(void)
{
	m_sound_name[0] = 0;
	m_bIsHaveSound = false;
}

bool RAnimation::SetSoundFileName(const char* pSoundName)
{
	if(!pSoundName[0])
		return false;
	strcpy_safe(m_sound_name,pSoundName);
	m_bIsHaveSound = true;
	return true;
}

bool RAnimation::CheckWeaponMotionType(int wtype) {

	if(wtype == -1)
		return true;

	if(m_weapon_motion_type == wtype)
		return true;
	return false;
}

bool RAnimation::LoadAni(const char* filename)
{
	RAnimationFile* pAnimationFile = RGetAnimationFileMgr()->Add(filename);

	if(pAnimationFile==NULL ) 
		return false;

	m_pAniData = pAnimationFile;

	return true;
}

AnimationLoopType RAnimation::GetAnimationLoopType()
{
#ifdef _WIN32
	if(RMesh::m_bToolMesh)
		return RAniLoopType_Loop;
#endif
	return m_ani_loop_type;
}

void RAnimation::SetAnimationLoopType(AnimationLoopType type)
{
	m_ani_loop_type = type;
}

/////////////////////////////////////////////////////////////////////


_NAMESPACE_REALSPACE2_END
