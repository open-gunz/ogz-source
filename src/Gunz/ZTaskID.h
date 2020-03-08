#ifndef _ZTASKID_H
#define _ZTASKID_H

/// AI가 하는 일
enum ZTASK_ID
{
	ZTID_NONE = 0,
	ZTID_MOVE_TO_TARGET,			///< 목적 대상으로 이동
	ZTID_MOVE_TO_POS,				///< 지정 위치로 이동
	ZTID_MOVE_TO_DIR,				///< 목적 방향으로 이동(길찾기안한다.)
	ZTID_ROTATE_TO_DIR,				///< 목적 방향으로 회전
	ZTID_ATTACK_MELEE,				///< 근접 공격
	ZTID_ATTACK_RANGE,				///< 원근 공격
	ZTID_DELAY,						///< 대기
	ZTID_SKILL,						///< 마법 및 기술
	ZTID_END
};


#endif