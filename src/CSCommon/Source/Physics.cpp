#include "stdafx.h"

#include "Physics.h"

v3 ParabolicMotion(const v3& InitialVelocity, float fSec)
{
	return (0.5f * v3(0, 0, -1)*GRAVITY_ACCELERATION * pow(fSec, 2) + InitialVelocity * fSec);
}