#ifndef _ZTASKMANAGER_H
#define _ZTASKMANAGER_H


typedef void (*ZTM_ONFINISHED)(ZActor* pActor, ZTASK_ID nLastID);

/// 태스크 관리자
class ZTaskManager
{
protected:
	list<ZTask*>			m_Tasks;
	ZTask*					m_pCurrTask;
	ZTM_ONFINISHED			m_pOnFinishedFunc;

	bool PopTask();
	bool CancelCurrTask();
	void CompleteCurrTask();
public:
	ZTaskManager();
	~ZTaskManager();
	void Clear();
	void Push(ZTask* pTask);
	void PushFront(ZTask* pTask);
	bool Cancel();						///< 현재 태스크를 취소한다.
	void Run(float fDelta);

	// interface functions
	bool IsEmpty() { return m_Tasks.empty(); }
	int GetCount() { return (int)m_Tasks.size(); }
	ZTask* GetCurrTask()	{ return m_pCurrTask; }
	ZTASK_ID GetCurrTaskID();
public:
	static ZTask* CreateMoveToPos(ZActor* pParent, rvector& vTarPos, bool bChained=false);
	static ZTask* CreateRotateToDir(ZActor* pParent, rvector& vTarDir);
	static ZTask* CreateAttackMelee(ZActor* pParent);
	static ZTask* CreateAttackRange(ZActor* pParent, rvector& dir);
	static ZTask* CreateSkill(ZActor* pParent,int nSkill,MUID& uidTarget,rvector& targetPosition);
	void SetOnFinishedCallback(ZTM_ONFINISHED pCallback) { m_pOnFinishedFunc = pCallback; }
};





#endif