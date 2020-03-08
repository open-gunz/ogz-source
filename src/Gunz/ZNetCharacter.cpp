#include "stdafx.h"

#include "ZNetCharacter.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZApplication.h"

MImplementRTTI(ZNetCharacter, ZCharacter);

void ZNetCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized == false) return;
	if (!IsVisible()) return;

	ZCharacter::OnUpdate(fDelta);

	if (IsMoveAnimation())
	{
		rvector origdiff = fDelta*GetVelocity();

		rvector diff = m_AnimationPositionDiff;
		diff.z += origdiff.z;
		if (GetDistToFloor() < 0 && diff.z < 0) diff.z = -GetDistToFloor();

		Move(diff);
	}

	UpdateHeight(fDelta);
};