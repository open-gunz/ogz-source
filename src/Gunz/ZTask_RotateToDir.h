#ifndef _ZTASK_ROTATE_TO_DIR_H
#define _ZTASK_ROTATE_TO_DIR_H

#include "ZTask.h"

class ZTask_RotateToDir : public ZTask
{
private:
	// data
	rvector		m_TargetDir;		// ¸ñÀûÁö
	bool		m_bRotated;
protected:
	virtual void OnStart();
	virtual ZTaskResult OnRun(float fDelta);
	virtual void OnComplete();
	virtual bool OnCancel();
public:
	DECLARE_TASK_ID(ZTID_ROTATE_TO_DIR);

	ZTask_RotateToDir(ZActor* pParent, rvector& vDir);
	virtual ~ZTask_RotateToDir();
	virtual const char* GetTaskName() { return "RotateToDir"; }
};







#endif