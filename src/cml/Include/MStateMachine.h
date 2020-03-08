#pragma once

#include <map>

class MStateStackMachine;

class MState{
friend class MStateStackMachine;
private:
	MStateStackMachine*	m_pSM;
protected:

protected:
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual bool OnRun() = 0;

	bool SetNextState(int nID);
public:
	virtual ~MState() = default;
};

typedef std::map<int, MState*>	MSTATEMAP;
typedef MSTATEMAP::iterator		MSTATEMAPITOR;
typedef MSTATEMAP::value_type	MSTATEMAPVALTYPE;

// State Stack Machine
class MStateStackMachine{
protected:
	MSTATEMAP	m_States;
	MState*		m_pCurrState;
	int			m_nNextState;
public:
	MStateStackMachine();
	virtual ~MStateStackMachine();

	void AddState(int nID, MState* pState);
	void DelState(int nID);

	void AddActionOfState(int nActionID, int nStateID);

	bool SetState(int nID);
	bool SetNextState(int nID);

	bool Run();
};

#define NULLSTATE	0