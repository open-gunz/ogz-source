#include "stdafx.h"
#include "ZTask_Delay.h"

ZTask_Delay::ZTask_Delay(ZActor* pParent) : ZTask(pParent)
{

}

ZTask_Delay::~ZTask_Delay()
{

}


void ZTask_Delay::OnStart()
{

}

ZTaskResult ZTask_Delay::OnRun(float fDelta)
{
	return ZTR_COMPLETED;

}

void ZTask_Delay::OnComplete()
{

}

bool ZTask_Delay::OnCancel()
{
	return true;
}
