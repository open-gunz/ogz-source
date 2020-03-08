#ifndef _ZSTATEMACHINE_H
#define _ZSTATEMACHINE_H

#include <map>
#include <list>
using namespace std;

class ZState;

class ZStateMachine
{
protected:
	map<int, ZState*>		m_StateMap;
	int						m_nCurrState;
public:
	ZStateMachine();
	virtual ~ZStateMachine();
	void AddState(ZState* pState);
	void DeleteState(int nStateID);
	void DeleteAll();
	int StateTransition(int nInput);
	int GetCurrStateID()		{ return m_nCurrState; }
	ZState* GetState(int nStateID);
	ZState* GetCurrState();
	void SetState(int nStateID) { m_nCurrState = nStateID; }
	ZState* CreateState(int nStateID);

	static const int INVALID_STATE;
};


class ZState
{
protected:
	map<int, int>		m_Transitions;
	int					m_nStateID;
public:
	ZState(int nStateID);
	virtual ~ZState();
	void AddTransition(int nInput, int nOutputID);
	void DeleteTransition(int nOutputID);
	int GetOutput(int nInput);
	int GetStateID() { return m_nStateID;}
};

#endif