#include "stdafx.h"
#include "ZActorAnimation.h"
#include "ZActor.h"

ZActorAnimation::ANIMATION_INFO ZActorAnimation::m_AnimationTable[ZA_ANIM_END] = 
{
	{ ZA_ANIM_NONE,			""					,true	,true	,false },
	{ ZA_ANIM_IDLE,			"idle"				,true	,true	,false },
	{ ZA_ANIM_WALK,			"run"				,true	,true	,false },
	{ ZA_ANIM_RUN,			"run"				,true	,true	,false },
	{ ZA_ANIM_ATTACK_MELEE,	"melee_attack"		,false	,false	,true },
	{ ZA_ANIM_ATTACK_RANGE,	"range_attack"		,false	,false	,true },
	{ ZA_ANIM_RANGE_DAMAGED,	"range_attacked1"	,false	,false	,false },
	{ ZA_ANIM_MELEE_DAMAGED1,	"melee_attacked1"	,false	,false	,false },
	{ ZA_ANIM_MELEE_DAMAGED2,	"melee_attacked2"	,false	,false	,false },
	{ ZA_ANIM_LIGHTNING_DAMAGED,	"lightning"	,false	,false	,false },
	{ ZC_ANIM_DAMAGED_DOWN,	"damage_down"		,false	,false	,false },
	{ ZC_ANIM_STAND,		"standup"			,false	,false	,false },
	{ ZA_ANIM_BLAST,		"high_timed"		,false	,false	,false },
	{ ZA_ANIM_BLAST_FALL,	"safe_fall_fail"	,false	,false	,false },
	{ ZA_ANIM_BLAST_DROP,	"drop"				,false	,false	,false },
	{ ZA_ANIM_DIE,			"die"				,true	,false	,false },

	{ ZA_ANIM_BLAST_DAGGER,		"blast_d"		,false	,false	,false },
	{ ZA_ANIM_BLAST_DAGGER_DROP,"drop_d"		,false	,false	,false },

	{ ZA_ANIM_SPECIAL1,		"special_attack1"	,true	,false	,false },
	{ ZA_ANIM_SPECIAL2,		"special_attack2"	,true	,false	,false }
};


ZActorAnimation::ZActorAnimation() : m_nCurrState(ZA_ANIM_NONE), m_pBody(NULL)
{

}

ZActorAnimation::~ZActorAnimation()
{

}

void ZActorAnimation::InitAnimationStates()
{
	ZState* pState;
	
	pState = m_AniFSM.CreateState(ZA_ANIM_IDLE);
	pState->AddTransition(ZA_INPUT_WALK,			ZA_ANIM_WALK);
	pState->AddTransition(ZA_INPUT_RUN,				ZA_ANIM_RUN);
	pState->AddTransition(ZA_INPUT_ROTATE,			ZA_ANIM_WALK);
	pState->AddTransition(ZA_INPUT_ATTACK_MELEE,	ZA_ANIM_ATTACK_MELEE);
	pState->AddTransition(ZA_INPUT_ATTACK_RANGE,	ZA_ANIM_ATTACK_RANGE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);

	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);
	pState->AddTransition(ZA_EVENT_SPECIAL1,		ZA_ANIM_SPECIAL1);
	pState->AddTransition(ZA_EVENT_SPECIAL2,		ZA_ANIM_SPECIAL2);
	pState->AddTransition(ZA_EVENT_SPECIAL3,		ZA_ANIM_SPECIAL3);
	pState->AddTransition(ZA_EVENT_SPECIAL4,		ZA_ANIM_SPECIAL4);

	pState = m_AniFSM.CreateState(ZA_ANIM_WALK);
	pState->AddTransition(ZA_INPUT_RUN,				ZA_ANIM_RUN);
	pState->AddTransition(ZA_EVENT_REACH_GROUND,	ZA_ANIM_WALK);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);
	pState->AddTransition(ZA_INPUT_ATTACK_MELEE,	ZA_ANIM_ATTACK_MELEE);
	pState->AddTransition(ZA_INPUT_ATTACK_RANGE,	ZA_ANIM_ATTACK_RANGE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);
	pState->AddTransition(ZA_INPUT_WALK_DONE,		ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_SPECIAL1,		ZA_ANIM_SPECIAL1);
	pState->AddTransition(ZA_EVENT_SPECIAL2,		ZA_ANIM_SPECIAL2);
	pState->AddTransition(ZA_EVENT_SPECIAL3,		ZA_ANIM_SPECIAL3);
	pState->AddTransition(ZA_EVENT_SPECIAL4,		ZA_ANIM_SPECIAL4);

	pState = m_AniFSM.CreateState(ZA_ANIM_RUN);
	pState->AddTransition(ZA_INPUT_RUN,				ZA_ANIM_RUN);
	pState->AddTransition(ZA_EVENT_REACH_GROUND,	ZA_ANIM_RUN);
	pState->AddTransition(ZA_INPUT_ATTACK_MELEE,	ZA_ANIM_ATTACK_MELEE);
	pState->AddTransition(ZA_INPUT_ATTACK_RANGE,	ZA_ANIM_ATTACK_RANGE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);
	pState->AddTransition(ZA_INPUT_WALK_DONE,		ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_SPECIAL1,		ZA_ANIM_SPECIAL1);
	pState->AddTransition(ZA_EVENT_SPECIAL2,		ZA_ANIM_SPECIAL2);
	pState->AddTransition(ZA_EVENT_SPECIAL3,		ZA_ANIM_SPECIAL3);
	pState->AddTransition(ZA_EVENT_SPECIAL4,		ZA_ANIM_SPECIAL4);


	pState = m_AniFSM.CreateState(ZA_ANIM_ATTACK_MELEE);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_ATTACK_RANGE);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	////////////////////////////////////////////////////////////////////////

	pState = m_AniFSM.CreateState(ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_REACH_PEAK,		ZA_ANIM_BLAST_FALL);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_BLAST_FALL);
	pState->AddTransition(ZA_EVENT_REACH_GROUND,	ZA_ANIM_BLAST_DROP);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_BLAST_DROP);
	pState->AddTransition(ZA_ANIM_DONE,				ZC_ANIM_STAND);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	////////////////////////////////////////////////////////////////////////

	pState = m_AniFSM.CreateState(ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_REACH_GROUND_DAGGER,	ZA_ANIM_BLAST_DAGGER_DROP);
	pState->AddTransition(ZA_EVENT_REACH_GROUND,	ZA_ANIM_BLAST_DAGGER_DROP);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_BLAST_DAGGER_DROP);
	pState->AddTransition(ZA_ANIM_DONE,				ZC_ANIM_STAND);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	
	pState = m_AniFSM.CreateState(ZA_ANIM_DIE);

	
	pState = m_AniFSM.CreateState(ZC_ANIM_STAND);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_SPECIAL1);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);

	pState = m_AniFSM.CreateState(ZA_ANIM_SPECIAL2);
	pState->AddTransition(ZA_ANIM_DONE,				ZA_ANIM_IDLE);
	pState->AddTransition(ZA_EVENT_RANGE_DAMAGED,	ZA_ANIM_RANGE_DAMAGED);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED1,	ZA_ANIM_MELEE_DAMAGED1);
	pState->AddTransition(ZA_EVENT_MELEE_DAMAGED2,	ZA_ANIM_MELEE_DAMAGED2);
	pState->AddTransition(ZA_EVENT_LIGHTNING_DAMAGED,ZA_ANIM_LIGHTNING_DAMAGED);
	pState->AddTransition(ZA_EVENT_BLAST,			ZA_ANIM_BLAST);
	pState->AddTransition(ZA_EVENT_BLAST_DAGGER,	ZA_ANIM_BLAST_DAGGER);
	pState->AddTransition(ZA_EVENT_DEATH,			ZA_ANIM_DIE);
}

void ZActorAnimation::Init(ZActor* pBody)
{
	m_pBody = pBody;	

	InitAnimationStates();

	// 기본 에니메이션 설정
	m_AniFSM.SetState(ZA_ANIM_IDLE);
}

void ZActorAnimation::Set(ZA_ANIM_STATE nAnim, bool bReset)
{
	if ((!bReset) && (GetCurrState() == nAnim)) return;

	ZBrain* pBrain = m_pBody->GetBrain();

	if (pBrain)
	{
		pBrain->OnBody_AnimExit(m_nCurrState);
	}

	if (m_pBody->m_pVMesh)
	{
		if (m_pBody->m_pVMesh->SetAnimation(ani_mode_lower, m_AnimationTable[nAnim].szName, m_AnimationTable[nAnim].bEnableCancel))
		{
			m_nCurrState = nAnim;
		}
	}
	else
	{
		_ASSERT(0);
	}

	if (pBrain) pBrain->OnBody_AnimEnter(m_nCurrState);
}

bool ZActorAnimation::Input(ZA_ANIM_INPUT nInput)
{
	int nextState = m_AniFSM.StateTransition(nInput);
	if (nextState == ZStateMachine::INVALID_STATE) return false;

	ZA_ANIM_STATE nNextAnimState = ZA_ANIM_STATE(nextState);
	m_AniFSM.SetState(nNextAnimState);

	Set(ZA_ANIM_STATE(m_AniFSM.GetCurrStateID()));

	return true;
}

void ZActorAnimation::ForceAniState(int nAnimState)
{
	ZA_ANIM_STATE state = ZA_ANIM_STATE(nAnimState);
	m_AniFSM.SetState(state);
	Set(state);
}

bool ZActorAnimation::IsAttackAnimation(ZA_ANIM_STATE nAnimState)
{
	if ((nAnimState == ZA_ANIM_ATTACK_MELEE) || (nAnimState == ZA_ANIM_ATTACK_RANGE)) return true;
	return false;
}

bool ZActorAnimation::IsSkippableDamagedAnimation(ZA_ANIM_STATE nAnimState)
{
	if( nAnimState == ZA_ANIM_ATTACK_MELEE ||
		nAnimState == ZA_ANIM_ATTACK_RANGE ||
		nAnimState == ZA_ANIM_SPECIAL1 || 
		nAnimState == ZA_ANIM_SPECIAL2 ) return true;

	return false;
}