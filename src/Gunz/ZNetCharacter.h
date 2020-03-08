#pragma once

#include "MRTTI.h"
#include "ZCharacter.h"


class ZNetCharacter : public ZCharacter
{
	MDeclareRTTI;
private:
	virtual void OnUpdate(float fDelta) override;
};