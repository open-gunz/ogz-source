#pragma once

#include <map>
#include <string>
#include <list>

class MCommand;
class MCommandDesc;
using MCommandDescMap = std::map<int, MCommandDesc*>;
using MCommandList = std::list<MCommand*>;
using MCommandAliasMap = std::map<std::string, std::string>;

class MCommandManager{
protected:
	MCommandDescMap		m_CommandDescs;
	// Queue for posted commands
	MCommandList		m_CommandQueue;
	MCommandAliasMap	m_CommandAlias;
protected:
	void InitializeCommandDesc();
	void InitializeCommandMemPool();
	void FinalizeCommandMemPool();
public:
	MCommandManager();
	virtual ~MCommandManager();

	void Initialize();

	int GetCommandDescCount() const;
	int GetCommandQueueCount() const;
	MCommandDesc* GetCommandDesc(int i);
	MCommandDesc* GetCommandDescByID(int nID);
	void AssignDescs(MCommandManager* pTarCM);

	void AddCommandDesc(MCommandDesc* pCD);

	bool Post(MCommand* pNew);

	MCommand* GetCommand();
	MCommand* PeekCommand();

	template <size_t size> void GetSyntax(char(&szSyntax)[size], const MCommandDesc *pCD) {
		GetSyntax(szSyntax, size, pCD);
	}
	void GetSyntax(char* szSyntax, int maxlen, const MCommandDesc* pCD);

	bool ParseMessage(MCommand* pCmd, char* szErrMsg, int nErrMsgMaxLength, const char* szMsg);
	void AddAlias(std::string szName, std::string szText);
};
