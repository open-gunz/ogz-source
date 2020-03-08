#ifndef MCOMMANDLOGFRAME_H
#define MCOMMANDLOGFRAME_H

#include "MFrame.h"
#include "MListBox.h"

class MCommand;

class MCommandLogFrame : public MFrame{
protected:
	MListBox*	m_pCommandList;
protected:
	void OnSize(int w, int h);
public:
	MCommandLogFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MCommandLogFrame(void);
	void AddCommand(u32 nGlobalClock, MCommand* pCmd);
	MListBox* GetCommandList() { return m_pCommandList; }
};

#endif