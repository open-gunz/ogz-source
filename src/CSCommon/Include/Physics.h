// 물리학에 관련된 함수들
//
// 단위계
// 거리 : 미터
// 시간 : 초
//
#ifndef PHYSICS_H
#define PHYSICS_H

#include "RTypes.h"

/// 중력 가속도
#define GRAVITY_ACCELERATION 9.8f

/// 포물선 운동
v3 ParabolicMotion(const v3& InitialVelocity, float fSec);

template <typename T>
bool MoveMovingWeapon(const v3 & Pos, v3 & Vel, const T& Pick)
{
	return false;
}


#endif