#include "stdafx.h"
#include "MStateMachine.h"

bool MState::SetNextState(int nID)
{
	return m_pSM->SetNextState(nID);	// CallStack에 의한 오류 방지를 위해 SetState()를 쓰지 않는다.
}

MStateStackMachine::MStateStackMachine(void)
{
	m_pCurrState = NULL;
	m_nNextState = NULLSTATE;
}

MStateStackMachine::~MStateStackMachine(void)
{
	if(m_pCurrState!=NULL){
		m_pCurrState->OnDestroy();
	}
}

void MStateStackMachine::AddState(int nID, MState* pState)
{
	_ASSERT(nID>NULLSTATE);
	if(nID<=NULLSTATE) return;

	pState->m_pSM = this;

	m_States.insert(m_States.end(), MSTATEMAPVALTYPE(nID, pState));
}

void MStateStackMachine::DelState(int nID)
{
	_ASSERT(nID>NULLSTATE);
	if(nID<=NULLSTATE) return;

	MSTATEMAPITOR i = m_States.find(nID);
	if(i!=m_States.end()){
		MState* pState = (*i).second;
		delete pState;
		m_States.erase(i);
	}
}

bool MStateStackMachine::SetState(int nID)
{
	if(m_pCurrState!=NULL) m_pCurrState->OnDestroy();

	if(nID<=NULLSTATE){
		m_pCurrState = NULL;
		return true;
	}

	MSTATEMAPITOR i = m_States.find(nID);
	if(i==m_States.end()) return false;

	m_pCurrState = (*i).second;

	if(m_pCurrState->OnCreate()==false) return false;

	return true;
}

bool MStateStackMachine::SetNextState(int nID)
{
	m_nNextState = nID;
	return true;
}

bool MStateStackMachine::Run(void)
{
	if(m_pCurrState!=NULL){
		if(m_pCurrState->OnRun()==false){
			m_pCurrState->OnDestroy();
			return true;
		}
	}

	if(m_nNextState!=NULLSTATE){
		SetState(m_nNextState);
		m_nNextState = NULLSTATE;
	}

	return true;
}
