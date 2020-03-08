#pragma once

#include "RAnimationFile.h"

_NAMESPACE_REALSPACE2_BEGIN

class RAnimation : public RBaseObject
{
public:
	RAnimation() { m_filename[0] = 0; m_sound_name[0] = 0; }

	bool LoadAni(const char* filename);

	void  SetFileName(const char* name);
	char* GetFileName();

	AnimationType GetAnimationType();

	int GetMaxFrame();
	int GetAniNodeCount();

	RAnimationNode* GetAniNode(int i);
	RAnimationNode* GetBipRootNode();

	void SetWeaponMotionType(int wtype);
	int  GetWeaponMotionType();

	bool CheckWeaponMotionType(int wtype);

	RAnimationNode* GetNode(const char* name);

	void SetLoadDone(bool b);
	bool IsLoadDone();

	// Sound Link

	void  ClearSoundFile(void);
	bool  SetSoundFileName(const char* pSoundName);
	char* GetSoundFileName();
	bool  IsHaveSoundFile();

	bool  IsSoundRelatedToMap() { return m_bSoundRelatedToMap; }
	void  SetSoundRelatedToMap(bool bValue) { m_bSoundRelatedToMap = bValue; }

	AnimationLoopType	GetAnimationLoopType();
	void				SetAnimationLoopType(AnimationLoopType type);

public:

	RAnimationFile*		m_pAniData{};

	char				m_filename[256];
	char				m_sound_name[256];

	bool				m_bIsHaveSound{};
	bool				m_bSoundRelatedToMap{};
	int					m_sID = -1;

	int					m_NameID = -1;

	int					m_weapon_motion_type = -1;
	
	bool				m_isConnected{};
	bool				m_isLoadDone{};

private:

	AnimationLoopType	m_ani_loop_type = RAniLoopType_Loop;
};

_NAMESPACE_REALSPACE2_END
