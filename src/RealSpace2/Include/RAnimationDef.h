#pragma once

#include "RMeshUtil.h"
#include <vector>
#include <string>

enum AnimationType{
	RAniType_TransForm = 0,
	RAniType_Vertex,
	RAniType_Bone,
	RAniType_Tm,
};

enum AnimationLoopType {
	RAniLoopType_Normal = 0,
	RAniLoopType_Loop,
	RAniLoopType_OnceIdle,
	RAniLoopType_HoldLastFrame,
	RAniLoopType_OnceLowerBody,
};

enum AnimationFileType {

	RAniFileType_None = 0,

	RAniFileType_Idle,
	RAniFileType_Run,
	RAniFileType_RunB,
	RAniFileType_RunW,
	RAniFileType_RunW_downF,
	RAniFileType_RunW_downB,
	RAniFileType_RunLW,
	RAniFileType_RunLW_down,
	RAniFileType_RunRW,
	RAniFileType_RunRW_down,
	RAniFileType_JumpU,
	RAniFileType_JumpD,
	RAniFileType_Bind,
	RAniFileType_Die,
	RAniFileType_Die2,
	RAniFileType_Load,
	RAniFileType_TumbleF,
	RAniFileType_TumbleB,
	RAniFileType_TumbleR,
	RAniFileType_TumbleL,
	RAniFileType_JumpwallB,
	RAniFileType_JumpwallF,
	RAniFileType_JumpwallL,
	RAniFileType_JumpwallR,
	RAniFileType_Blast,
	RAniFileType_Blast_fall,
	RAniFileType_Blast_drop,
	RAniFileType_Blast_stand,
	RAniFileType_Blast_dagger,
	RAniFileType_Blast_drop_dagger,
	RAniFileType_Attack1,
	RAniFileType_Attack1_ret,
	RAniFileType_Attack2,
	RAniFileType_Attack2_ret,
	RAniFileType_Attack3,
	RAniFileType_Attack3_ret,
	RAniFileType_Attack4,
	RAniFileType_Attack_Jump,
	RAniFileType_Uppercut,
	RAniFileType_Damage,
	RAniFileType_Damage2,
	RAniFileType_Damage_down,
	RAniFileType_Blast_airmove,
	RAniFileType_Login_intro,
	RAniFileType_Login_idle,
	RAniFileType_Login_walk,
	RAniFileType_Guard_start,
	RAniFileType_Guard_idle,
	RAniFileType_Guard_block1,
	RAniFileType_Guard_block1_ret,
	RAniFileType_Guard_block2,
	RAniFileType_Guard_cancel,
	RAniFileType_Taunt,
	RAniFileType_Pit,
	RAniFileType_Bow,
	RAniFileType_Wave,
	RAniFileType_Cry,
	RAniFileType_Laugh,
	RAniFileType_Dance,

	RAniFileType_End
};
