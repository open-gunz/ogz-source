#include "stdafx.h"
#include "ZStateMachine.h"
#include <crtdbg.h>

const int ZStateMachine::INVALID_STATE = -1;

ZStateMachine::ZStateMachine() : m_nCurrState(INVALID_STATE)
{

}

ZStateMachine::~ZStateMachine()
{
	DeleteAll();
}

ZState* ZStateMachine::CreateState(int nStateID)
{
	ZState* pState = new ZState(nStateID);
	AddState(pState);
	return pState;
}

ZState* ZStateMachine::GetState(int nStateID)
{
	map<int, ZState*>::iterator itor = m_StateMap.find(nStateID);
	if (itor != m_StateMap.end())
	{
		return (*itor).second;
	}

	_ASSERT(0);
	return NULL;
}

ZState* ZStateMachine::GetCurrState()
{
	return GetState(m_nCurrState);
}

void ZStateMachine::AddState(ZState* pState)
{
	m_StateMap.insert(map<int, ZState*>::value_type(pState->GetStateID(), pState));
}

void ZStateMachine::DeleteState(int nStateID)
{
	for (map<int, ZState*>::iterator itor = m_StateMap.begin(); itor != m_StateMap.end(); ++itor)
	{
		if (((*itor).second)->GetStateID() == nStateID)
		{
			delete (*itor).second;
			return;
		}
	}
}

void ZStateMachine::DeleteAll()
{
	for (map<int, ZState*>::iterator itor = m_StateMap.begin(); itor != m_StateMap.end(); ++itor)
	{
		delete (*itor).second;
	}

	m_StateMap.clear();
}

int ZStateMachine::StateTransition(int nInput)
{
	ZState* pState = GetState(m_nCurrState);
	if (pState == NULL) return INVALID_STATE;

	return (pState->GetOutput(nInput));
}


//////////////////////////////////////////////////////////////////////////////////////////
ZState::ZState(int nStateID) : m_nStateID(nStateID)
{

}

ZState::~ZState()
{

}

void ZState::AddTransition(int nInput, int nOutputID)
{
	m_Transitions.insert(map<int, int>::value_type(nInput, nOutputID));
}

void ZState::DeleteTransition(int nOutputID)
{
	for (map<int, int>::iterator itor = m_Transitions.begin(); itor != m_Transitions.end(); ++itor)
	{
		if (nOutputID == (*itor).second)
		{
			m_Transitions.erase(itor);
			return;
		}
	}
}

int ZState::GetOutput(int nInput)
{
	int nRetOutputID = m_nStateID;
	map<int, int>::iterator itor = m_Transitions.find(nInput);
	if (itor != m_Transitions.end())
	{
		nRetOutputID = (*itor).second;
	}

	return nRetOutputID;
}






