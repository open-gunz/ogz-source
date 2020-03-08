#include "stdafx.h"
#include "ZBrain.h"
#include "ZActor.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZMyCharacter.h"
#include "ZRangeWeaponHitDice.h"
#include "ZModule_Skills.h"
#include "MMath.h"
#include "RNavigationMesh.h"
#include "RBspObject.h"
#include "ZPickInfo.h"

ZBrain* ZBrain::CreateBrain(MQUEST_NPC nNPCType)
{
	switch (nNPCType)
	{
	case NPC_GOBLIN_KING:
		{
			return new ZBrain_GoblinKing();
		}
	break;
	};

	return new ZBrain();
}

float ZBrain::MakePathFindingUpdateTime(char nIntelligence)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fPathFinding_ShakingRatio;
	float fTime = pGlobalAIValue->m_fPathFindingUpdateTime[nIntelligence-1];

	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if (fMinTime < 0.0f) fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber(fMinTime, fMaxTime);
}

float ZBrain::MakeAttackUpdateTime(char nAgility)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fAttack_ShakingRatio;
	float fTime = pGlobalAIValue->m_fAttackUpdateTime[nAgility-1];

	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if (fMinTime < 0.0f) fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber(fMinTime, fMaxTime);
}

float ZBrain::MakeSpeed(float fSpeed)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fSpeed_ShakingRatio;

	float fExtraValue = fSpeed * fShakingRatio;
	float fMinSpeed = max((fSpeed - fExtraValue), 0.0f);
	float fMaxSpeed = fSpeed + fExtraValue;

	return RandomNumber(fMinSpeed, fMaxSpeed);
}

float ZBrain::MakeDefaultAttackCoolTime()
{
	if (!m_pBody->GetNPCInfo()) return 0.0f;

	float fShakingRatio = 0.3f;
	float fCoolTime = m_pBody->GetNPCInfo()->fAttackCoolTime;

	float fExtraValue = fCoolTime * fShakingRatio;
	float fMinCoolTime = max((fCoolTime - fExtraValue), 0.01f);
	float fMaxCoolTime = fCoolTime + fExtraValue;

	return RandomNumber(fMinCoolTime, fMaxCoolTime);

}

ZBrain::ZBrain() : m_pBody(NULL), m_uidTarget(MUID(0,0))
					
{
	
}

ZBrain::~ZBrain()
{

}

void ZBrain::Init(ZActor* pBody)
{
	m_pBody = pBody;

	m_Behavior.Init(this);

	if (m_pBody->GetNPCInfo())
	{
		float fDefaultPathFindingUpdateTime = ZBrain::MakePathFindingUpdateTime(m_pBody->GetNPCInfo()->nIntelligence);
		float fAttackUpdateTime = ZBrain::MakeAttackUpdateTime(m_pBody->GetNPCInfo()->nAgility);
		float fDefaultAttackUpdateTime = m_pBody->GetNPCInfo()->fAttackCoolTime;

		m_PathFindingTimer.Init(fDefaultPathFindingUpdateTime);
		m_AttackTimer.Init(fAttackUpdateTime);
		m_DefaultAttackTimer.Init(fDefaultAttackUpdateTime);
	}
}

#include "ZGame.h"

void ZBrain::Think(float fDelta)
{
	m_Behavior.Run(fDelta);

	// 길찾기
	ProcessBuildPath(fDelta);

	// 공격
	ProcessAttack(fDelta);


	#ifdef _DEBUG
	{
		DrawDebugPath();
	}
	#endif
}

void ZBrain::ProcessAttack(float fDelta)
{
	bool bDefaultAttackEnabled = true;
	if ((m_pBody->GetNPCInfo()) && (m_pBody->GetNPCInfo()->fAttackCoolTime != 0.0f))
	{
		bDefaultAttackEnabled = m_DefaultAttackTimer.Update(fDelta);
	}

	if ((!m_AttackTimer.Update(fDelta)) && (!bDefaultAttackEnabled)) return;

	if (!m_pBody->IsAttackable()) return;
	MQUEST_NPC_ATTACK nNpcAttackType = CheckAttackable();

	if (bDefaultAttackEnabled && ((nNpcAttackType == NPC_ATTACK_MELEE) || (nNpcAttackType == NPC_ATTACK_RANGE)))
	{
		float fNextCoolTime = MakeDefaultAttackCoolTime();
		m_DefaultAttackTimer.Init(fNextCoolTime);

		DefaultAttack(nNpcAttackType);
	}

	// 스킬이 사용가능하면 스킬을 쓴다
	if(nNpcAttackType==NPC_ATTACK_NONE) {
		int nSkill;
		MUID uidTarget;
		rvector targetPosition;

		if(CheckSkillUsable(&nSkill,&uidTarget,&targetPosition)) 
		{
			UseSkill(nSkill, uidTarget, targetPosition);
		}
	}
}

void ZBrain::UseSkill(int nSkill, MUID& uidTarget, rvector& vTargetPos)
{
	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ((nTaskID == ZTID_NONE) || 
		(nTaskID == ZTID_SKILL) ||
		(nTaskID == ZTID_ROTATE_TO_DIR)) return;

#ifdef _DEBUG
	mlog("Use Skill(id=%d, skill=%d)\n", m_pBody->GetUID().Low, nSkill);
#endif

	rvector dir = vTargetPos - m_pBody->GetPosition();
	Normalize(dir);


	m_pBody->m_TaskManager.Clear();
	ZTask* pNew;
	pNew = ZTaskManager::CreateRotateToDir(m_pBody, dir);
	m_pBody->m_TaskManager.Push(pNew);
	pNew = ZTaskManager::CreateSkill(m_pBody, nSkill, uidTarget, vTargetPos);
	m_pBody->m_TaskManager.Push(pNew);

}

void ZBrain::DefaultAttack(MQUEST_NPC_ATTACK nNpcAttackType)
{
	switch (nNpcAttackType)
	{
	case NPC_ATTACK_MELEE:
		{
			ZTask* pNew = ZTaskManager::CreateAttackMelee(m_pBody);
			m_pBody->m_TaskManager.PushFront(pNew);
		}
		break;
	case NPC_ATTACK_RANGE:
		{
			ZObject* pTarget = GetTarget();
			if (pTarget)
			{
				ZRangeWeaponHitDice dice;
				dice.BuildSourcePosition(m_pBody->GetPosition());		// 총구 위치를 정확히 알아야 하는디
				dice.BuildTargetPosition(pTarget->GetPosition());
				dice.BuildTargetBounds(pTarget->GetCollRadius(), pTarget->GetCollHeight());
				float fTargetSpeed = Magnitude(pTarget->GetVelocity());
				dice.BuildTargetSpeed(fTargetSpeed);
				dice.BuildGlobalFactor(m_pBody->GetHitRate());
				
				rvector shot_dir = dice.ReturnShotDir();

				ZTask* pNew = ZTaskManager::CreateAttackRange(m_pBody, shot_dir);
				m_pBody->m_TaskManager.PushFront(pNew);
			}
		}
		break;
	}
}

void ZBrain::ProcessBuildPath(float fDelta)
{
	if (!m_PathFindingTimer.Update(fDelta)) return;
	
	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ((nTaskID == ZTID_ATTACK_MELEE) || 
		(nTaskID == ZTID_ATTACK_RANGE) || 
		(nTaskID == ZTID_ROTATE_TO_DIR) ||
		(nTaskID == ZTID_SKILL)) return;

	FindTarget();

	ZObject* pTarget = GetTarget();
	if (pTarget)
	{
		// 원거리 맙일 경우 공격가능하면 다가가지 않고 바라만 본다.
		if (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_RANGE)
		{
			if (CheckAttackable() == NPC_ATTACK_RANGE)
			{
				m_pBody->m_TaskManager.Clear();
				m_pBody->Stop();

				return;
			}
		}

		rvector tarpos = pTarget->GetPosition();
		BuildPath(tarpos);
		PushPathTask();
	}
	else
	{
		// 없으면 지금은 그냥 가만히 있자.
		m_pBody->m_TaskManager.Clear();
		m_pBody->Stop();
	}
}

void ZBrain::OnBody_AnimEnter(ZA_ANIM_STATE nAnimState)
{
}

void ZBrain::OnBody_AnimExit(ZA_ANIM_STATE nAnimState)
{

}

void ZBrain::OnBody_CollisionWall()
{

}

void ZBrain::OnBody_OnTaskFinished(ZTASK_ID nLastID)
{
	if ((nLastID == ZTID_MOVE_TO_POS) || (nLastID == ZTID_MOVE_TO_DIR) || (nLastID == ZTID_MOVE_TO_TARGET))
	{
		if (GetTarget())
		{
			m_PathFindingTimer.Force();
		}
	}
}

bool ZBrain::BuildPath(rvector& vTarPos)
{
	RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
	if (pNavMesh != NULL)
	{
		if (pNavMesh->BuildNavigationPath(m_pBody->GetPosition(), vTarPos))
		{
			m_WayPointList.clear();

			for (list<rvector>::iterator itor = pNavMesh->GetWaypointList().begin(); 
				itor != pNavMesh->GetWaypointList().end(); ++itor)
			{
				m_WayPointList.push_back((*itor));
			}

			return true;
		}
	}

	return false;
}

void OutputDebugVector(char* szText, rvector &v)
{
	char text[128];
	sprintf_safe(text, "[%s] %.3f %.3f %.3f\n", szText, v.x, v.y, v.z);
	OutputDebugString(text);
}

void ZBrain::DebugTest()
{
	if (!m_WayPointList.empty())
	{
		float diff=0.0f;
		rvector tar = *m_WayPointList.begin();
		rvector dir = tar - m_pBody->GetPosition();
		Normalize(dir);
		//m_pBody->RunTo(dir);


		list<rvector>::iterator itor = m_WayPointList.begin();
		list<rvector>::iterator itorNext = itor;
		itorNext++;

		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetTexture(1,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		rmatrix birdinitmat;
		GetIdentityMatrix(birdinitmat);
		RSetTransform(D3DTS_WORLD, birdinitmat);


		rvector v1, v2;
		v1 = m_pBody->GetPosition();
		v2 = (*m_WayPointList.begin());
		v1.z=0.0f;
		v2.z=0.0f;
		RDrawLine(v1, v2, 0xFFFFFF00);

		while (itorNext != m_WayPointList.end())
		{
			v1 = (*itor);
			v2 = (*itorNext);
			v1.z=0.0f;
			v2.z=0.0f;
			RDrawLine(v1, v2, 0xFFFFFF00);

			++itor;
			++itorNext;
		}
	}
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

}

void ZBrain::DrawDebugPath()
{
	return;

	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	if (!m_WayPointList.empty())
	{
		list<rvector>::iterator itor = m_WayPointList.begin();
		list<rvector>::iterator itorNext = itor;
		itorNext++;

		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetTexture(1,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		rmatrix birdinitmat;
		GetIdentityMatrix(birdinitmat);
		RSetTransform(D3DTS_WORLD, birdinitmat);

		RDrawLine(rvector(0,0,0), rvector(0,0,100), 0xFFFFFF00);
	}
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}

void ZBrain::PushPathTask()
{
	if (!m_WayPointList.empty())
	{
		m_pBody->m_TaskManager.Clear();

		int nTotal = (int)m_WayPointList.size();
		int cnt=0;
		for (list<rvector>::iterator itor = m_WayPointList.begin(); itor != m_WayPointList.end(); ++itor)
		{
			bool bChained = !((nTotal-1) == cnt);

			ZTask* pNew = ZTaskManager::CreateMoveToPos(m_pBody, (*itor), bChained);
			m_pBody->m_TaskManager.Push(pNew);

			++cnt;
		}
	}
}


ZObject* ZBrain::GetTarget()
{
#ifdef _DEBUG
	// 혼자서 AI 테스트할 경우
	if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) || 
		(ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_AI))
	{
		return (ZObject*)g_pGame->m_pMyCharacter;
	}

#endif

	//mlog("GetTarget\n");
	if (ZGetObjectManager())
	{
		ZObject* pObject = ZGetObjectManager()->GetObject(m_uidTarget);
		return pObject;
	}
	return NULL;
}


MQUEST_NPC_ATTACK ZBrain::CheckAttackable()
{
	ZObject* pTarget = GetTarget();
	if ((pTarget == NULL) || (pTarget->IsDie())) return NPC_ATTACK_NONE;

	// 일단 근접 공격이 가능하면 근접 공격
	if (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_MELEE)
	{
		if (m_pBody->CanAttackMelee(pTarget)) return NPC_ATTACK_MELEE;
	}
	else if (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_RANGE)
	{
		if (m_pBody->CanAttackRange(pTarget)) return NPC_ATTACK_RANGE;
	}
	return NPC_ATTACK_NONE;
}

bool ZBrain::CheckSkillUsable(int *pnSkill, MUID *puidTarget, rvector *pTargetPosition)
{
	if(m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_MAGIC) {
		ZModule_Skills *pmod = (ZModule_Skills *)m_pBody->GetModule(ZMID_SKILLS);
		if(!pmod) return false;

		if (puidTarget) (*puidTarget) = MUID(0,0);
		if (pTargetPosition) (*pTargetPosition) = rvector(0.0f,0.0f,0.0f);

		for(int i=0;i<pmod->GetSkillCount();i++) {
			
			ZSkill *pSkill = pmod->GetSkill(i);

			if(!pSkill->IsReady()) continue; // 쿨타임체크

			ZSkillDesc *pDesc = pmod->GetSkill(i)->GetDesc();
			if(pDesc->IsAlliedTarget()) {	// 대상이 아군인경우

				// 효과가 있는 대상중 가까이 있는 걸 찾는다.
				float fDist = FLT_MAX;
				ZObject *pAlliedTarget = NULL;

				for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
					itor != ZGetObjectManager()->end(); ++itor)
				{
					ZObject *pObject = itor->second;
					if(pObject->IsDie()) continue;
					if(ZGetGame()->IsAttackable(m_pBody,pObject)) continue;	// 적이면 넘어간다
					if (pObject == m_pBody) continue;	// 자기자신이면 넘어간다.

					float dist = MagnitudeSq(pObject->GetPosition() - m_pBody->GetPosition());
					if (pSkill->IsUsable(pObject) && dist < fDist )
					{
						fDist = dist;
						pAlliedTarget = pObject;
					}
				}	

				// 만약 대상이 없으면 자기 자신한테라도 스킬을 건다.
				if ((pAlliedTarget == NULL) && (pSkill->IsUsable(m_pBody)))
				{
					pAlliedTarget = m_pBody;
				}

				if (pAlliedTarget)
				{
					if(pnSkill) *pnSkill=i;
					if(puidTarget) *puidTarget=pAlliedTarget->GetUID();
					if(pTargetPosition) *pTargetPosition = pAlliedTarget->GetCenterPos();
					return true;
				}
			}else {							// 적군이 대상이다

				ZObject* pTarget = GetTarget();
				if (pTarget == NULL) continue;
				if(!pSkill->IsUsable(pTarget)) continue;

				ZPICKINFO pickinfo;
				memset(&pickinfo,0,sizeof(ZPICKINFO));

				rvector pos,dir;
				pos = m_pBody->GetPosition() + rvector(0,0,50);
				dir = pTarget->GetPosition() - m_pBody->GetPosition();
				Normalize(dir);

				const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

				if (ZApplication::GetGame()->Pick(m_pBody, pos, dir, &pickinfo, dwPickPassFlag))
				{
					if (pickinfo.pObject)
					{
						if (pickinfo.pObject == pTarget) {
							if(pnSkill) *pnSkill=i;
							if(puidTarget) *puidTarget=pTarget->GetUID();
							if(pTargetPosition) *pTargetPosition = pTarget->GetCenterPos();
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}
	

bool ZBrain::FindTarget()
{
	MUID uidTarget = MUID(0,0);
	float fDist = FLT_MAX;

	ZCharacter* pTempCharacter = NULL;

	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (*itor).second;
		if (pCharacter->IsDie()) continue;
		
		if (pCharacter->LostConnection())
			continue;

		if (!CheckEnableTargetting(pCharacter)) 
		{
			pTempCharacter = pCharacter;
			continue;
		}

		float dist = MagnitudeSq(pCharacter->GetPosition() - m_pBody->GetPosition());
		if (dist < fDist)
		{
			fDist = dist;
			uidTarget = pCharacter->GetUID();
		}
	}	

	m_uidTarget = uidTarget;

	if ((uidTarget == MUID(0,0)) && (pTempCharacter != NULL))
	{
		m_uidTarget = pTempCharacter->GetUID();
	}


	if (uidTarget != MUID(0,0))
	{
		return true;
	}

	return false;
}

bool ZBrain::CheckEnableTargetting(ZCharacter* pCharacter)
{
	u32 nAttackTypes = m_pBody->GetNPCInfo()->nNPCAttackTypes;
	if (nAttackTypes == NPC_ATTACK_MELEE)
	{
		if ((pCharacter->GetStateLower() == ZC_STATE_LOWER_BIND) &&
			(IS_EQ(MagnitudeSq(pCharacter->GetVelocity()), 0.0f)) &&
			(pCharacter->GetDistToFloor() >= m_pBody->GetCollHeight()) )
		{
			return false;
		}
	}
	return true;
}





///////////////////////////////////////////////////////////////////////////////////////////////////
// ZBrain_Goblin_King /////////////////////////////////////////////////////////////////////////////
ZBrain_GoblinKing::ZBrain_GoblinKing() : ZBrain()
{

}

ZBrain_GoblinKing::~ZBrain_GoblinKing()
{

}