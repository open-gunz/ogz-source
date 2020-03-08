#include "stdafx.h"
#include "ZBehavior.h"
#include "ZBrain.h"
#include "ZBehavior_Idle.h"
#include "ZBehavior_Attack.h"

ZBehavior::ZBehavior() : m_pCurrState(NULL), m_pBrain(NULL)
{

}

ZBehavior::~ZBehavior()
{

}

void ZBehavior::Init(ZBrain* pBrain)
{
	m_pBrain = pBrain;

	ZState* pState = new ZBehavior_Idle(pBrain);
	pState->AddTransition(ZBEHAVIOR_INPUT_ATTACKED,		ZBEHAVIOR_STATE_ATTACK);
	m_FSM.AddState(pState);

	pState = new ZBehavior_Attack(pBrain);
	m_FSM.AddState(pState);

	
	// ±âº»Àº idle
	ForceState(ZBEHAVIOR_STATE_IDLE);
}

void ZBehavior::Run(float fDelta)
{
	if (m_pCurrState) m_pCurrState->Run(fDelta);
}

bool ZBehavior::Input(ZBEHAVIOR_INPUT nInput)
{
	int nextState = m_FSM.StateTransition(nInput);
	if (nextState == ZStateMachine::INVALID_STATE) return false;

	ZBEHAVIOR_STATE nNextBehaviorState = ZBEHAVIOR_STATE(nextState);
	ChangeBehavior(nNextBehaviorState);

	return true;
}

void ZBehavior::ForceState(ZBEHAVIOR_STATE nState)
{
	ChangeBehavior(nState);
}


void ZBehavior::ChangeBehavior(ZBEHAVIOR_STATE nState)
{
	if (m_pCurrState)
	{
		((ZBehaviorState*)m_pCurrState)->Exit();
	}

	m_FSM.SetState(nState);

	m_pCurrState = (ZBehaviorState*)m_FSM.GetCurrState();
	m_pCurrState->Enter();
}