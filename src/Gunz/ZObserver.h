#pragma once

#include "ZPrerequisites.h"
#include "MDrawContext.h"

class ZCharacter;
class ZCamera;
class ZIDLResource;

enum ZObserverType
{
	ZOM_NONE = 0,

	ZOM_ANYONE,
	ZOM_BLUE,
	ZOM_RED,

	ZOM_MAX
};

#define ZFREEOBSERVER_RADIUS	30.f
#define OBSERVER_QUICK_TAGGER_TARGET_KEY			'0'

class ZObserverQuickTarget {
protected:
	MUID	m_arrayPlayers[10];

public:
	ZObserverQuickTarget()	{ Clear(); }
	~ZObserverQuickTarget()	{ Clear(); }
	void Clear() {
		std::fill(std::begin(m_arrayPlayers), std::end(m_arrayPlayers), MUID{ 0, 0 });
	}

	bool ConvertKeyToIndex(char nKey, int* nIndex);
	void StoreTarget(int nIndex, const MUID& uidChar) { m_arrayPlayers[nIndex] = uidChar; }
	auto& GetTarget(int nIndex) const { return m_arrayPlayers[nIndex]; }
};

class ZObserver final
{
public:
	ZObserver();
	~ZObserver();
	bool Create(ZCamera* pCamera, ZIDLResource*	pIDLResource);
	void Destroy();
	void ChangeToNextTarget();
	bool SetFirstTarget();

	auto IsVisible() const { return m_bVisible; }
	void Show(bool bVisible);
	void OnDraw(MDrawContext* pDC);
	auto GetDelay() const { return m_fDelay; }

	void SetType(ZObserverType nType);
	auto GetType() const { return m_nType; }
	
	auto* GetTargetCharacter() { return m_pTargetCharacter; }

	void SetFreeLookTarget(const rvector& tar) { m_FreeLookTarget = tar; }
	auto& GetFreeLookTarget() const { return m_FreeLookTarget; }

	void NextLookMode();

	bool OnKeyEvent(bool bCtrl, char nKey);

private:
	void ShowInfo(bool bShow);
	void SetTarget(ZCharacter* pCharacter);
	void SetTarget(const MUID& muid);
	bool IsVisibleSetTarget(ZCharacter* pCharacter);
	void CheckDeadTarget();

	float					m_fDelay;
	bool					m_bVisible;
	ZObserverType			m_nType;
	ZCharacter*				m_pTargetCharacter;
	ZCamera*				m_pCamera;
	ZIDLResource*			m_pIDLResource;
	rvector					m_FreeLookTarget;
	ZObserverQuickTarget	m_QuickTarget;
};