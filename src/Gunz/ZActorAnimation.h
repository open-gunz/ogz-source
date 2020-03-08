#ifndef _ZACTORANIMCONTROLLER_H
#define _ZACTORANIMCONTROLLER_H

#include "ZStateMachine.h"

//! 에니메이션 세트
enum ZA_ANIM_STATE {

	ZA_ANIM_NONE = 0,
	ZA_ANIM_IDLE,				// 정지
	ZA_ANIM_WALK,				// 걷기
	ZA_ANIM_RUN,				// 뛰기
	ZA_ANIM_ATTACK_MELEE,		// 근접 공격
	ZA_ANIM_ATTACK_RANGE,		// 원거리 공격
	ZA_ANIM_RANGE_DAMAGED,		// 원거리 피격
	ZA_ANIM_MELEE_DAMAGED1,		// 원거리 피격
	ZA_ANIM_MELEE_DAMAGED2,		// 원거리 피격
	ZA_ANIM_LIGHTNING_DAMAGED,	// 라이트닝 데미지
	ZC_ANIM_DAMAGED_DOWN,		// 다운
	ZC_ANIM_STAND,				// 일어서기

	ZA_ANIM_BLAST,				// 날라가기
	ZA_ANIM_BLAST_FALL,			// 떨어지기
	ZA_ANIM_BLAST_DROP,			// 땅에 부딪히기
	ZA_ANIM_DIE,

	ZA_ANIM_BLAST_DAGGER,		// 단검찌르기에 날라가는
	ZA_ANIM_BLAST_DAGGER_DROP,	// 단검찌르기에 넘어지는

	ZA_ANIM_SPECIAL1,			// 스킬이나 마법
	ZA_ANIM_SPECIAL2,			// 스킬이나 마법
	ZA_ANIM_SPECIAL3,
	ZA_ANIM_SPECIAL4,

	ZA_ANIM_END
};

enum ZA_ANIM_INPUT {
	// actor input
	ZA_INPUT_NONE = 0,
	ZA_INPUT_WALK,				// a
	ZA_INPUT_RUN,				// b
	ZA_INPUT_ROTATE,			// 방향을 바꾸려 한다.
	ZA_INPUT_WALK_DONE,
	ZA_INPUT_ATTACK_MELEE,		// c
	ZA_INPUT_ATTACK_RANGE,		// d
	ZA_INPUT_RISE,				// j

	// event
	ZA_EVENT_DETECT_ENEMY,		// b
	ZA_EVENT_RANGE_DAMAGED,		// d
	ZA_EVENT_MELEE_DAMAGED1,	// d
	ZA_EVENT_MELEE_DAMAGED2,	// d
	ZA_EVENT_LIGHTNING_DAMAGED,	// d
	ZA_EVENT_BLAST,				// e
	ZA_EVENT_BLAST_DAGGER,		// f
	ZA_EVENT_FALL,				// g
	ZA_EVENT_REACH_GROUND,		// i
	ZA_EVENT_REACH_GROUND_DAGGER,// i
	ZA_EVENT_REACH_PEAK,		
	ZA_EVENT_DEATH,				// l
	ZA_EVENT_SPECIAL1,
	ZA_EVENT_SPECIAL2,
	ZA_EVENT_SPECIAL3,
	ZA_EVENT_SPECIAL4,

	// anim event
	ZA_ANIM_DONE,				// k

	ZA_INPUT_END
};

class ZActor;

class ZActorAnimation
{
protected:
	//! 에니메이션 정보
	static struct ANIMATION_INFO {		// ZCharacter의 ZANIMATIONINFO에서 가져옴
		int		nID;
		char*	szName;
		bool	bEnableCancel;
		bool	bLoop;
		bool	bMove;
	} m_AnimationTable[ZA_ANIM_END];

	ZActor*					m_pBody;
	ZStateMachine			m_AniFSM;				///< 애니메이션 상태기계
	ZA_ANIM_STATE			m_nCurrState;			///< 현재 애니메이션 상태

	void InitAnimationStates();
public:
	ZActorAnimation();
	virtual ~ZActorAnimation();
	void Init(ZActor* pBody);
	void Set(ZA_ANIM_STATE nAnim, bool bReset=false);
	bool Input(ZA_ANIM_INPUT nInput);
	void ForceAniState(int nAnimState);				///< 애니메이션 상태를 강제로 바꾼다. 

	ZA_ANIM_STATE GetCurrState() { return m_nCurrState; }
	static bool IsAttackAnimation(ZA_ANIM_STATE nAnimState);
	static bool IsSkippableDamagedAnimation(ZA_ANIM_STATE nAnimState);
};












#endif