/////////////////////////////////////////////////////////////
#ifndef _MMatchQuestMonsterGroupDesc_h
#define _MMatchQuestMonsterGroupDesc_h

#include <list>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

class MNPCList : public list<string> {};

class MNPCGroup {
public:
	MNPCList	m_NpcList;
protected:
	int			m_nID;
	string		m_Name;
public:
	MNPCGroup() { }
	virtual ~MNPCGroup() { }
	void SetID(int nID)	{ m_nID = nID; }
	int  GetID() { return m_nID; }

	const char* GetName()	{ return m_Name.c_str(); }
	void SetName(const char* Name) { m_Name = Name; }

	void AddNpc(string NPCName) {
		m_NpcList.push_back(NPCName);
	}

	bool CheckNpcName(string NPCName) {
		for (MNPCList::iterator i=m_NpcList.begin(); i!=m_NpcList.end(); i++) {
			if (_stricmp((*i).c_str(), NPCName.c_str()) == 0)
				return true;
		}
		return false;
	}
};

class MXmlElement;
class MZFileSystem;

class MNPCGroupMgr : public map<string, MNPCGroup*>
{
public:
	MNPCGroupMgr();
	virtual ~MNPCGroupMgr();

	static MNPCGroupMgr* GetInstance();
	void Clear();	

	MNPCGroup* GetGroup(const string& strName);
	MNPCGroup* GetGroup(int nGroupID);

	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);

protected:
	void ParseRule(MXmlElement* element);
};

inline MNPCGroupMgr* MGetNPCGroupMgr() { return MNPCGroupMgr::GetInstance(); }

#endif//_MMatchQuestMonsterGroupDesc_h