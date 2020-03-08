#include "stdafx.h"
#include "ZTask_MoveToPos.h"

ZTask_MoveToPos::ZTask_MoveToPos(ZActor* pParent, rvector& vTarPos, bool bChained) 
					: ZTask(pParent), m_TargetPos(vTarPos), m_bRotated(false), m_bChained(bChained)
{
	m_TargetPos.z = 0.0f;
}

ZTask_MoveToPos::~ZTask_MoveToPos()
{

}

void ZTask_MoveToPos::OnStart()
{

}

ZTaskResult ZTask_MoveToPos::OnRun(float fDelta)
{
	rvector p1 = m_TargetPos;
	rvector p2 = m_pParent->GetPosition();
	p1.z = p2.z = 0.0f;
	float diff = Magnitude(p1 - p2);

	if (diff <= 20.0f)
	{
		return ZTR_COMPLETED;
	}

	rvector dir = m_TargetPos - m_pParent->GetPosition();
	Normalize(dir);

	if (!m_bRotated)
	{
		// 방향이 맞지 않으면 제자리에서 회전한다.
		rmatrix mat;
		rvector vBodyDir = m_pParent->GetDirection();
		float fAngle=GetAngleOfVectors(dir, vBodyDir);
		float fRotAngle = m_pParent->GetNPCInfo()->fRotateSpeed * (fDelta / 1.0f);

		if (fAngle > 0.0f) fRotAngle = -fRotAngle;
		if (fabs(fRotAngle) > fabs(fAngle)) 
		{
			fRotAngle = -fAngle;
			m_bRotated = true;
		}
		mat = RGetRotZ(ToDegree(fRotAngle));

		m_pParent->RotateTo(vBodyDir * mat);
	}
	else
	{
		m_pParent->RunTo(dir);
	}

	

	return ZTR_RUNNING;
}

void ZTask_MoveToPos::OnComplete()
{
	m_pParent->Stop(!m_bChained);
}

bool ZTask_MoveToPos::OnCancel()
{
	m_pParent->Stop(false);
	return true;
}
