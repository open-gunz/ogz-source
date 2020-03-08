#include "stdafx.h"
#include "ZTask.h"
#include "MDebug.h"


void ZTask::ClearParams()
{

}

void ZTask::PushParam(int nValue)
{
	m_Params.push(nValue);
}

void ZTask::PushParam(float fValue)
{
	m_Params.push(*((int*)&fValue));
}

float ZTask::PopParamFloat()
{
	if (m_Params.empty()) 
	{
		_ASSERT(0);
		return 0.0f;
	}

	int ret = m_Params.front();
	m_Params.pop();
	return *((float*)&ret);
}

int ZTask::PopParamInt()
{
	if (m_Params.empty()) 
	{
		_ASSERT(0);
		return 0;
	}

	int ret = m_Params.front();
	m_Params.pop();
	return ret;

}



void ZTask::Start()
{
	OnStart();
}

ZTaskResult ZTask::Run(float fDelta)
{
	return OnRun(fDelta);
}

void ZTask::Complete()
{
	OnComplete();
}

bool ZTask::Cancel()
{
	if (!m_bCancelEnable) return false;

	return OnCancel();
}



