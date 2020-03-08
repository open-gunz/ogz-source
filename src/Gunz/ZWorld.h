#pragma once

#include "ZWater.h"
#include "ZMapDesc.h"
#include "ZClothEmblem.h"

class ZWorldManager;
class ZMapDesc;
class ZSkyBox;
class ZLoadingProgress;

class ZWorld
{
public:
	void Update(float fDelta);
	void Draw();
	bool Create(ZLoadingProgress *pLoading);

	void OnInvalidate();
	void OnRestore();

	RBspObject	*GetBsp() { return m_pBsp.get(); }
	ZMapDesc	*GetDesc() { return m_pMapDesc.get(); }
	ZEmblemList	*GetFlags() { return &m_flags; }
	ZWaterList	*GetWaters() { return &m_waters; }

	void SetFog(bool bFog);
	bool IsWaterMap() { return m_bWaterMap; }
	float GetWaterHeight() { return m_fWaterHeight; }
	bool IsFogVisible()		{ return m_bFog; }

private:
	friend ZWorldManager;

	ZWorld();
	~ZWorld();

	bool			m_bWaterMap{};
	float			m_fWaterHeight{};
	ZWaterList		m_waters;
	ZEmblemList		m_flags;
	std::unique_ptr<ZSkyBox> m_pSkyBox;

	bool m_bFog{};
	u32	m_dwFogColor = -1;
	float m_fFogNear{};
	float m_fFogFar{};

	char m_szName[64];
	char m_szBspName[_MAX_PATH];
	std::unique_ptr<RBspObject> m_pBsp;
	std::unique_ptr<ZMapDesc> m_pMapDesc;

	int m_nRefCount = 1;
	bool m_bCreated{};
};