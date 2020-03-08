#include "stdafx.h"
#include "ZTaskManager.h"
#include "ZTask.h"
#include "ZTask_MoveToPos.h"
#include "ZTask_RotateToDir.h"
#include "ZTask_AttackMelee.h"
#include "ZTask_AttackRange.h"
#include "ZTask_Skill.h"

#ifdef _DEBUG
static bool g_bTaskDebug = true;
#endif

ZTaskManager::ZTaskManager() : m_pCurrTask(NULL), m_pOnFinishedFunc(NULL)
{

}

ZTaskManager::~ZTaskManager()
{
	if (m_pCurrTask)
	{
		delete m_pCurrTask;
		m_pCurrTask = NULL;
	}

	for (list<ZTask*>::iterator itor = m_Tasks.begin(); itor != m_Tasks.end(); ++itor)
	{
		delete (*itor);
	}
	m_Tasks.clear();
}


void ZTaskManager::Clear()
{
#ifdef _DEBUG
	if (g_bTaskDebug)
	{
		char szCurrTask[256] = "";
		if (m_pCurrTask) strcpy_safe(szCurrTask, m_pCurrTask->GetTaskName());
		mlog("TASK: Clear(queue=%d, currtask=%s)\n", (int)m_Tasks.size(), szCurrTask);
	}
#endif

	CancelCurrTask();

	for (list<ZTask*>::iterator itor = m_Tasks.begin(); itor != m_Tasks.end(); ++itor)
	{
		delete (*itor);
	}
	m_Tasks.clear();
}


void ZTaskManager::Push(ZTask* pTask)
{
#ifdef _DEBUG
	if (g_bTaskDebug)
	{
		char szTask[256] = "";
		mlog("TASK: Push(queue=%d, task=%s)\n", (int)m_Tasks.size(), pTask->GetTaskName());
	}
#endif

	m_Tasks.push_back(pTask);
}

void ZTaskManager::PushFront(ZTask* pTask)
{
#ifdef _DEBUG
	if (g_bTaskDebug)
	{
		char szTask[256] = "";
		mlog("TASK: PushFront(task=%s)\n", pTask->GetTaskName());
	}
#endif

	if (m_pCurrTask)
	{
		m_pCurrTask->Cancel();
		m_Tasks.push_front(m_pCurrTask);
	}

	m_pCurrTask = pTask;
	m_pCurrTask->Start();
}

void ZTaskManager::Run(float fDelta)
{
	if (m_pCurrTask)
	{
		// 현재 태스크가 있으면 실행
		ZTaskResult ret = m_pCurrTask->Run(fDelta);

		switch (ret)
		{
		case ZTR_RUNNING:
			{

			}
			break;
		case ZTR_COMPLETED:
			{
				CompleteCurrTask();
			}
			break;
		case ZTR_CANCELED:
			{
				CancelCurrTask();
			}
			break;
		}
	}
	else
	{
		if (PopTask())
		{
			// 현재 태스크가 없으면 새로운 태스크를 꺼내서 시작한다.
			m_pCurrTask->Start();
		}
	}
}

bool ZTaskManager::PopTask()
{
	if (m_Tasks.empty()) return false;

	list<ZTask*>::iterator itor = m_Tasks.begin();
	m_pCurrTask = (*itor);
	m_Tasks.erase(itor);

	return true;
}


bool ZTaskManager::CancelCurrTask()
{
	if (!m_pCurrTask) return true;

#ifdef _DEBUG
	if (g_bTaskDebug)
	{
		char szTask[256] = "";
		mlog("TASK: Cancel(task=%s)\n", m_pCurrTask->GetTaskName());
	}
#endif

	if (m_pCurrTask->Cancel())
	{
		delete m_pCurrTask;
		m_pCurrTask = NULL;
		return true;
	}

	return false;
}

void ZTaskManager::CompleteCurrTask()
{
	if (!m_pCurrTask) return;

#ifdef _DEBUG
	if (g_bTaskDebug)
	{
		char szTask[256] = "None";
		mlog("TASK: Complete(task=%s)\n", m_pCurrTask->GetTaskName());
	}
#endif

	m_pCurrTask->Complete();

	if (m_pOnFinishedFunc)
	{
		m_pOnFinishedFunc(m_pCurrTask->GetParent(), m_pCurrTask->GetID());
	}

	delete m_pCurrTask;
	m_pCurrTask = NULL;
}

bool ZTaskManager::Cancel()
{
	return CancelCurrTask();
}

ZTASK_ID ZTaskManager::GetCurrTaskID()
{
	if (m_pCurrTask)
	{
		return m_pCurrTask->GetID();
	}
	return ZTID_NONE;
}

//////////////////////////////////////////////////////////////////
ZTask* ZTaskManager::CreateMoveToPos(ZActor* pParent, rvector& vTarPos, bool bChained)
{
	ZTask* pNew = new ZTask_MoveToPos(pParent, vTarPos, bChained);
	return pNew;
}

ZTask* ZTaskManager::CreateRotateToDir(ZActor* pParent, rvector& vTarDir)
{
	ZTask* pNew = new ZTask_RotateToDir(pParent, vTarDir);
	return pNew;

}

ZTask* ZTaskManager::CreateAttackMelee(ZActor* pParent)
{
	ZTask* pNew = new ZTask_AttackMelee(pParent);
	return pNew;
}

ZTask* ZTaskManager::CreateAttackRange(ZActor* pParent, rvector& dir)
{
	ZTask* pNew = new ZTask_AttackRange(pParent, dir);
	return pNew;
}

ZTask* ZTaskManager::CreateSkill(ZActor* pParent,int nSkill,MUID& uidTarget,rvector& targetPosition)
{
	ZTask* pNew = new ZTask_Skill(pParent,nSkill,uidTarget,targetPosition);
	return pNew;
}

