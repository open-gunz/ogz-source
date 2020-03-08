#pragma once

/// 캐릭터 속성 - 이 값은 변하지 않는다.
struct ZCharacterProperty
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	float		fMaxHP;		// *10 된값이다.
	float		fMaxAP;
	int			nMoveSpeed;
	int			nWeight;
	int			nMaxWeight;
	int			nSafeFall;
	ZCharacterProperty() : nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1),
		fMaxHP(1000.f),
		fMaxAP(1000.f),
		nMoveSpeed(100),
		nWeight(0),
		nMaxWeight(100),
		nSafeFall(100)
	{
		szName[0] = 0;
		szClanName[0] = 0;
	}
	void SetName(const char* name) { strcpy_safe(szName, name); }
	void SetClanName(const char* name) { strcpy_safe(szClanName, name); }
	const char* GetName() { return szName; }
	const char* GetClanName() { return szClanName; }
};

/// 캐릭터 상태값
struct ZCharacterStatus
{
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;	// 처음 방에 들어올때 로딩이 다 되었는지의 퍼센트 100이 되면 로딩이 다된것
	int			nCombo;
	int			nMaxCombo;
	int			nAllKill;
	int			nExcellent;
	int			nFantastic;
	int			nHeadShot;
	int			nUnbelievable;
	int			nExp;

	ZCharacterStatus() :
		nLife(10),
		nKills(0),
		nDeaths(0),
		nLoadingPercent(0),
		nCombo(0),
		nMaxCombo(0),
		nAllKill(0),
		nExcellent(0),
		nFantastic(0),
		nHeadShot(0),
		nUnbelievable(0),
		nExp(0)
	{  }

	void AddKills(int nAddedKills = 1) { nKills += nAddedKills; }
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths; }
	void AddExp(int _nExp = 1) { nExp += _nExp; }
};