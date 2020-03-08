#pragma once

#include "MBaseStringResManager.h"

class ZStringResManager : public MBaseStringResManager
{
protected:
	MStringRes<int>			m_Messages;
	virtual bool OnInit() override;
public:
	ZStringResManager();
	virtual ~ZStringResManager();
	static void MakeInstance();

	const char* GetMessageStr(int nID);
	MStringRes<int>*	GetMessages()	{ return &m_Messages; }

};

inline ZStringResManager* ZGetStringResManager()
{
	return (ZStringResManager*)MBaseStringResManager::GetInstance();
}

inline const char* ZMsg(const int nID) { return ZGetStringResManager()->GetMessageStr(nID); }
inline const char* ZErrStr(const int nID) { return ZGetStringResManager()->GetErrorStr(nID); }
inline const char* ZStr(string& key) { return ZGetStringResManager()->GetString(key); }

template<size_t size>
inline bool ZTransMsg(char (&poutStr)[size], int nMsgID, const int argnum = 0, const char* arg1 = NULL, ...) {
	va_list args;
	va_start(args, arg1);
	bool ret = ZGetStringResManager()->GetMessages()->Translate(poutStr, size, nMsgID, argnum, arg1, args);
	va_end(args);
	return ret;
}

inline bool ZTransMsg(char* poutStr, int maxlen, int nMsgID, const int argnum=0, const char* arg1=NULL, ... )
{
	va_list args;
	va_start(args, arg1);
	bool ret = ZGetStringResManager()->GetMessages()->Translate(poutStr, maxlen, nMsgID, argnum, arg1, args);
	va_end(args);
	return ret;
}
