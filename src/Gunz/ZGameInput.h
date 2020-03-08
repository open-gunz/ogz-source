#pragma once

#include <list>

class MEvent;
class MListener;

struct ZACTIONKEYITEM {
	ZACTIONKEYITEM(float time, bool pressed, int key) {
		fTime = time; bPressed = pressed; nActionKey = key;
	}
	float fTime;
	bool bPressed;
	int nActionKey;
};

struct ZKEYSEQUENCEITEM {
	bool bPressed;
	int nActionKey;
};

struct ZKEYSEQUENCEACTION {
	ZKEYSEQUENCEACTION(float time, int count, ZKEYSEQUENCEITEM *key) {
		fTotalTime = time; nKeyCount = count; pKeys = key;
	}

	float fTotalTime;
	int nKeyCount;
	ZKEYSEQUENCEITEM *pKeys;
};

class ZGameInput final
{
public:
	ZGameInput();
	~ZGameInput();

	static bool OnEvent(MEvent* pEvent);
	void Update(float fElapsed);
	
private:
	friend class Portal;

	void GameCheckSequenceKeyCommand();
	bool OnDebugEvent(MEvent* pEvent);

	float lastanglex, lastanglez;

	bool m_bCTOff;
	static ZGameInput* m_pInstance;
	std::list<ZACTIONKEYITEM> m_ActionKeyHistory;
	std::vector<ZKEYSEQUENCEACTION> m_SequenceActions;
};