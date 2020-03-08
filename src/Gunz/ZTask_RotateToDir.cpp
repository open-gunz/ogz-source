#include "stdafx.h"
#include "ZTask_RotateToDir.h"

ZTask_RotateToDir::ZTask_RotateToDir(ZActor* pParent, rvector& vDir) 
					: ZTask(pParent), m_TargetDir(vDir), m_bRotated(false)
{

}

ZTask_RotateToDir::~ZTask_RotateToDir()
{

}

void ZTask_RotateToDir::OnStart()
{

}

ZTaskResult ZTask_RotateToDir::OnRun(float fDelta)
{
	rvector dir = m_TargetDir;

	if (!m_bRotated)
	{
		rmatrix mat;
		rvector vBodyDir = m_pParent->GetDirection();
		float fAngle=GetAngleOfVectors(dir, vBodyDir);
		float fRotAngle = m_pParent->GetNPCInfo()->fRotateSpeed * (fDelta / 1.0f);

		if (fAngle > 0.0f) fRotAngle = -fRotAngle;
		if (fabs(fRotAngle) > fabs(fAngle)) 
		{
			fRotAngle = -fAngle;
			m_bRotated = true;
			return ZTR_COMPLETED;
		}
		mat = RGetRotZ(ToDegree(fRotAngle));

		m_pParent->RotateTo(vBodyDir * mat);
	}
	else
	{
		return ZTR_COMPLETED;
	}

	

	return ZTR_RUNNING;
}

void ZTask_RotateToDir::OnComplete()
{
	
}

bool ZTask_RotateToDir::OnCancel()
{
	return true;
}
