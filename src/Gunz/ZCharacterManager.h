#pragma once

#include "MUID.h"
#include "RVisualMeshMgr.h"
#include "ZObject.h"
#include "ZCharacter.h"
#include "MObjectTypes.h"

_USING_NAMESPACE_REALSPACE2

class ZCharacterManager : public std::map<MUID, ZCharacter*>
{
private:
	int		m_nLiveCount;

public:
	ZCharacterManager();
	virtual ~ZCharacterManager();

	void Add(ZCharacter *pCharacter);
	ZCharacter* Add(MUID uid, rvector pos,bool bMyCharacter=false);

	void Delete(MUID uid);
	void Clear();

	ZCharacter* Find(MUID uid);

	int GetCharacterIndex(const MUID& uid, bool bIncludeHiddenChar=true);		

	void OutputDebugString_CharacterState();

	int GetLiveCount();
	int GetCount() { return (int)size(); }
	ZCharacter* Get(int index);

	bool ToggleClothSimulation();

	void OnInvalidate();
	void OnRestore();

	void InitRound();
};