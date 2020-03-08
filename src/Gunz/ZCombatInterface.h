#pragma once

#include "ZInterface.h"
#include "MPicture.h"
#include "MEdit.h"
#include "MListBox.h"
#include "MLabel.h"
#include "MAnimation.h"
#include "ZObserver.h"
#include "ZCombatChat.h"
#include "ZCrossHair.h"
#include "ZMiniMap.h"
#include "ZVoteInterface.h"

class ZCharacter;
class ZScreenEffect;
class ZWeaponScreenEffect;
class ZMiniMap;
class ZCombatQuestScreen;

struct ZResultBoardItem {
	char szName[64];
	char szClan[CLAN_NAME_LENGTH];
	int nClanID;
	int nTeam;
	int nScore;
	int nKills;
	int nDeaths;
	int	nAllKill;
	int	nExcellent;
	int	nFantastic;
	int	nHeadShot;
	int	nUnbelievable;
	bool bMyChar;
	bool bGameRoomUser;

	ZResultBoardItem() { }
	ZResultBoardItem(const char *_szName, const char *_szClan, int _nTeam, int _nScore,
		int _nKills, int _nDeaths, bool _bMyChar = false, bool _bGameRoomUser = false) {
		strcpy_safe(szName, _szName);
		strcpy_safe(szClan, _szClan);
		nTeam = _nTeam;
		nScore = _nScore;
		nKills = _nKills;
		nDeaths = _nDeaths;
		nAllKill = 0;
		nExcellent = 0;
		nFantastic = 0;
		nHeadShot = 0;
		nUnbelievable = 0;
		bMyChar = _bMyChar;
		bGameRoomUser = _bGameRoomUser;
	}
};

class ZResultBoardList : public std::list<ZResultBoardItem*>
{
public:
	void Destroy() {
		while (!empty())
		{
			delete *begin();
			erase(begin());
		}
	}
};

class ZCombatInterface final : public ZInterface
{
public:
	ZCombatInterface(const char* szName = NULL, MWidget* pParent = NULL, MListener* pListener = NULL);
	~ZCombatInterface();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnDraw(MDrawContext* pDC) override;
	virtual void OnDrawCustom(MDrawContext* pDC);
	int DrawVictory(MDrawContext* pDC, int x, int y, int nWinCount, bool bGetWidth = false);

	virtual bool IsDone() override;

	void OnAddCharacter(ZCharacter *pChar);

	void Resize(int w, int h);

	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;

	static MFont *GetGameFont();
	MPOINT GetCrosshairPoint() { return MPOINT(MGetWorkspaceWidth() / 2, MGetWorkspaceHeight() / 2); }

	void ShowMenu(bool bVisible = true);
	void ShowInfo(bool bVisible = true);
	void EnableInputChat(bool bInput = true, bool bTeamChat = false);

	void SetDrawLeaveBattle(bool bShow, int nSeconds);

	void ShowChatOutput(bool bShow);
	bool IsChatVisible() const { return m_Chat.IsChat(); }
	bool IsTeamChat() const { return m_Chat.IsTeamChat(); }
	bool IsMenuVisible() const { return m_bMenuVisible; }

	void Update();
	void SetPickTarget(bool bPick, ZCharacter* pCharacter = NULL);

	void Finish();
	bool IsFinish();

	ZCharacter* GetTargetCharacter();
	MUID		GetTargetUID();

	void SetObserverMode(bool bEnable);
	bool GetObserverMode() const { return m_Observer.IsVisible(); }
	ZObserver* GetObserver() { return &m_Observer; }
	ZCrossHair* GetCrossHair() { return &m_CrossHair; }

	ZVoteInterface* GetVoteInterface() { return &m_VoteInterface; }

	void ShowCrossHair(bool bVisible) { m_CrossHair.Show(bVisible); }
	void OnGadget(MMatchWeaponType nWeaponType);
	void OnGadgetOff();

	bool IsShowResult() const { return m_bShowResult; }
	bool IsShowScoreBoard() const { return m_bDrawScoreBoard; }

	ZCombatChat			m_Chat;
	ZCombatChat			m_AdminMsg;
	u32					m_nReservedOutTime;

protected:
	ZWeaponScreenEffect*		m_pWeaponScreenEffect;

	ZScreenEffect*		m_pResultPanel;
	ZScreenEffect*		m_pResultPanel_Team;
	ZResultBoardList	m_ResultItems;
	ZScreenEffect*		m_pResultLeft;
	ZScreenEffect*		m_pResultRight;

	int					m_nClanIDRed;
	int					m_nClanIDBlue;
	char				m_szRedClanName[32];
	char				m_szBlueClanName[32];

	ZCombatQuestScreen*	m_pQuestScreen;

	ZObserver			m_Observer;
	ZCrossHair			m_CrossHair;
	ZVoteInterface		m_VoteInterface;

	ZIDLResource*		m_pIDLResource;

	MLabel*				m_pTargetLabel;
	MBitmap*			m_ppIcons[ZCI_END];
	MBitmapR2*			m_pResultBgImg;

	bool				m_bMenuVisible;

	bool				m_bPickTarget;
	char				m_szTargetName[256];

	MMatchItemDesc*		m_pLastItemDesc;

	int					m_nBullet;
	int					m_nBulletAMagazine;
	int					m_nMagazine;

	int					m_nBulletImageIndex;
	int					m_nMagazineImageIndex;

	char				m_szItemName[256];

	bool				m_bReserveFinish;
	u32					m_nReserveFinishTime;

	bool				m_bDrawLeaveBattle;
	int					m_nDrawLeaveBattleSeconds;

	bool				m_bOnFinish;
	bool				m_bShowResult;

	bool				m_bDrawScoreBoard;

	float				m_fOrgMusicVolume;


	void SetBullet(int nBullet);
	void SetBulletAMagazine(int nBulletAMagazine);
	void SetMagazine(int nMagazine);

	void SetItemImageIndex(int nIndex);

	void SetItemName(const char* szName);
	void UpdateCombo(ZCharacter* pCharacter);

	void OnFinish();

	void GameCheckPickCharacter();

	void IconRelative(MDrawContext* pDC, float x, float y, int nIcon);

	void DrawFriendName(MDrawContext* pDC);
	void DrawEnemyName(MDrawContext* pDC);
	void DrawAllPlayerName(MDrawContext* pDC);

	void DrawScoreBoard(MDrawContext* pDC);
	void DrawResultBoard(MDrawContext* pDC);
	void DrawSoloSpawnTimeMessage(MDrawContext* pDC);
	void DrawLeaveBattleTimeMessage(MDrawContext* pDC);
	void GetResultInfo(void);

	void DrawTDMScore(MDrawContext* pDC);

	void DrawNPCName(MDrawContext* pDC);
};

void TextRelative(MDrawContext* pDC, float x, float y, const char *szText, bool bCenter = false);