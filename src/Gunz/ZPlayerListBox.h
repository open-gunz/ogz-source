#pragma once

#include "MListBox.h"
#include "map"
#include "vector"
#include "MUID.h"
#include "ZEmblemInterface.h"
#include "ZApplication.h"

class MBitmap;
class MScrollBar;

enum ePlayerState
{
	PS_LOGOUT = 0,
	PS_FIGHT,
	PS_WAIT,
	PS_LOBBY,
	PS_END,
};

class ZPlayerListItem : public MListItem {
public:
	ZPlayerListItem() {
		m_szName[0] = 0;
		m_szClanName[0] = 0;
		m_szLevel[0] = 0;
	}

	ZPlayerListItem(const MUID& UID, MMatchUserGradeID Grade,
		const char* szName, const char* szClanName, const char* szLevel = 0) :
		m_PlayerUID{ UID }, m_Grade{ Grade }
	{
		assert(m_szName != nullptr);

		auto CopyString = [&](auto&& dest, auto&& src) {
			strcpy_safe(dest, src ? src : "");
		};

		CopyString(m_szName, szName);
		CopyString(m_szClanName, szClanName);
		CopyString(m_szLevel, szLevel);
	}

	void SetColor(MCOLOR c) { m_Color = c; }
	MCOLOR GetColor() const { return m_Color; }
	const MUID& GetUID() const { return m_PlayerUID; }

	MUID				m_PlayerUID{};
	MMatchUserGradeID	m_Grade = MMUG_FREE;
	MCOLOR				m_Color = XRGB(0xCD);

	char			m_szName[MATCHOBJECT_NAME_LENGTH];
	char			m_szClanName[CLAN_NAME_LENGTH];
	char			m_szLevel[128];

};

class ZLobbyPlayerListItem : public ZPlayerListItem {
protected:
	MBitmap* m_pBitmap{};
	unsigned int m_nClanID{};

public:
	ePlayerState m_nLobbyPlayerState = PS_LOGOUT;

public:
	ZLobbyPlayerListItem(const MUID& puid, MBitmap* pBitmap, unsigned int nClanID,
		const char* szLevel, const char* szName, const char *szClanName,
		ePlayerState nLobbyPlayerState, MMatchUserGradeID Grade) :
		ZPlayerListItem{ puid, Grade, szName, szClanName, szLevel }
	{
		m_pBitmap = pBitmap;
		m_nLobbyPlayerState = nLobbyPlayerState;
		m_nClanID = nClanID;
		ZGetEmblemInterface()->AddClanInfo(m_nClanID);
	}

	virtual ~ZLobbyPlayerListItem() override {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanID);
	}

	virtual const char* GetString() override
	{
		return m_szName;
	}

	virtual const char* GetString(int i) override
	{
		if (i == 1)
			return m_szLevel;
		else if (i == 2)
			return m_szName;
		else if (i == 4)
			return m_szClanName;

		return nullptr;
	}

	virtual MBitmap* GetBitmap(int i) override
	{
		if (i == 0) {
			return m_pBitmap;
		}
		else if (i == 3)
		{
			if (strcmp(m_szClanName, "") == 0) {
				return nullptr;
			}
			else {
				return ZGetEmblemInterface()->GetClanEmblem(m_nClanID);
			}
		}

		return nullptr;
	}
};

class ZFriendPlayerListItem : public ZPlayerListItem {
protected:
	char		m_szLocation[128];
	MBitmap*	m_pBitmap{};

public:
	ePlayerState	m_nLobbyPlayerState = PS_LOGOUT;

	ZFriendPlayerListItem()
	{
		m_szLocation[0] = 0;
	}

	ZFriendPlayerListItem(const MUID& puid, MBitmap* pBitmap, const char* szName, const char* szClanName,
		const char* szLocation, ePlayerState nLobbyPlayerState, MMatchUserGradeID Grade) : 
		ZPlayerListItem{ puid, Grade, szName, szClanName }
	{
		m_pBitmap = pBitmap;
		m_nLobbyPlayerState = nLobbyPlayerState;
		strcpy_safe(m_szLocation, szLocation ? szLocation : "");
	}

	virtual const char* GetString() override
	{
		return m_szName;
	}

	virtual const char* GetString(int i) override
	{
		if(i==1)
			return m_szName;

		return nullptr;
	}

	virtual MBitmap* GetBitmap(int i) override
	{
		if (i == 0)
			return m_pBitmap;

		return nullptr;
	}

	const char* GetLocation() { return m_szLocation; }
};

class ZClanPlayerListItem : public ZPlayerListItem{
protected:
	MBitmap* m_pBitmap{};
	MMatchClanGrade	m_ClanGrade = MCG_NONE;

public:
	ePlayerState	m_nLobbyPlayerState = PS_LOGOUT;

public:
	ZClanPlayerListItem() = default;

	ZClanPlayerListItem(const MUID& puid, MBitmap* pBitmap, const char* szName, const char* szClanName,
		const char* szLevel, ePlayerState nLobbyPlayerState, MMatchClanGrade clanGrade) :
		ZPlayerListItem{ puid, MMUG_FREE, szName, szClanName, szLevel }
	{
		m_pBitmap = pBitmap;
		m_nLobbyPlayerState = nLobbyPlayerState;
		m_ClanGrade = clanGrade;
	}

	virtual const char* GetString() override
	{
		return m_szName;
	}

	virtual const char* GetString(int i) override
	{
		if (i == 1)
			return m_szName;
		else if (i == 3)
			return m_szLevel;

		return nullptr;
	}

	virtual MBitmap* GetBitmap(int i) override
	{
		if (i == 0)
			return m_pBitmap;

		return nullptr;
	}
};

enum eStagePlayerState
{
	SPS_NONE = 0,
	SPS_SHOP,
	SPS_EQUIP,
	SPS_READY,
	SPS_END
};

struct sStagePlayerInfo
{
	int Level;
	char szName[128];
	int state;
	bool isMaster;
	int	nTeam;
};

class ZStagePlayerListItem : public ZPlayerListItem {
public:
	MMatchTeam Team = MMT_ALL;
	bool	m_bEnableObserver{};
	unsigned int m_nClanID{};
	MBitmap* m_pBitmap{};

	ZStagePlayerListItem() = default;

	ZStagePlayerListItem(const MUID& puid, MBitmap* pBitmap, unsigned int nClanID,
		const char* szName, const char* szClanName, const char* szLevel, MMatchUserGradeID Grade) :
		ZPlayerListItem{ puid, Grade, szName, szClanName, szLevel }
	{
		m_pBitmap = pBitmap;
		m_nClanID = nClanID;
		ZGetEmblemInterface()->AddClanInfo(m_nClanID);
	}

	virtual ~ZStagePlayerListItem() override
	{
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanID);
	}

	virtual const char* GetString() override
	{
		return m_szName;
	}

	virtual const char* GetString(int i) override
	{
		if (i == 1)
			return m_szLevel;
		else if (i == 2)
			return m_szName;
		else if (i == 4)
			return m_szClanName;

		return nullptr;
	}

	virtual MBitmap* GetBitmap(int i) override
	{
		if (i == 0) return m_pBitmap;
		else if (i == 3)
		{
			if ( strcmp( m_szClanName, "") == 0)
				return nullptr;
			else
				return ZGetEmblemInterface()->GetClanEmblem(m_nClanID);
		}
		return nullptr;
	}
};

class ZPlayerListBoxLook : public MListBoxLook
{
public:
	virtual void OnItemDraw2(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color,
		bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual void OnItemDraw2(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected,
		bool bFocus, int nAdjustWidth);

	virtual void OnDraw(MListBox* pListBox, MDrawContext* pDC) override;
	virtual MRECT GetClientRect(MListBox* pListBox, const MRECT& r) override { return r; }
};

class ZPlayerListBox : public MListBox
{
public:
	enum class PlayerListMode {
		Channel = 0,
		Stage,
		ChannelFriend,
		StageFriend,
		ChannelClan,
		StageClan,
		End,
	};

	enum class ModeCategory {
		Unknown,
		Channel, // PlayerListMode::Channel
		Stage,   // PlayerListMode::Stage
		Friend,  // PlayerListMode::ChannelFriend or PlayerListMode::StageFriend
		Clan,    // PlayerListMode::ChannelClan or PlayerListMode::StageClan
	};

private:
	bool LegalState(const char* szName, ModeCategory ExpectedCategory);

	std::unique_ptr<MBmButton> m_pButton;

	std::vector<MUID> mPlayerOrder;

	int				mSelectedPlayer{};
	int				mStartToDisplay{};
	float			m_SlotWidth{};
	float			m_SlotHeight{};

	int m_nOldW{};
	PlayerListMode m_nMode{};

protected:
	void SetupButton(const char *szOn, const char *szOff);

public:
	ZPlayerListBox(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	virtual ~ZPlayerListBox() override;

	DECLARE_LOOK(ZPlayerListBoxLook)
	DECLARE_LOOK_CLIENT()

	virtual void OnSize(int w, int h) override;

	void InitUI(PlayerListMode nMode);
	void RefreshUI();

	PlayerListMode GetMode() const { return m_nMode; }
	void SetMode(PlayerListMode mode);

	void AddPlayerChannel(const MUID& puid, ePlayerState state, int nLevel,
		const char* szName, const char* szClanName, unsigned int nClanID, MMatchUserGradeID nGrade);

	void AddPlayerStage(const MUID& puid, MMatchObjectStageState state, int nLevel,
		const char* szName, const char* szClanName, unsigned int nClanID,
		bool isMaster, MMatchTeam nTeam);

	void AddPlayerFriend(ePlayerState state, char* szName, char* szLocation);

	void AddPlayerClan(const MUID& puid, ePlayerState state, char* szName, int nLevel, MMatchClanGrade nGrade);

	void DelPlayer(const MUID& puid);
	void UpdatePlayer(const MUID& puid, MMatchObjectStageState state, bool isMaster, MMatchTeam nTeam);

	void UpdateList();

	ZPlayerListItem* GetUID(MUID uid);
	const char* GetPlayerName( int nIndex);

	MUID GetSelectedPlayerUID();
	void SelectPlayer(MUID);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) override;

	void SetWidth(float t) { m_SlotWidth = t; }
	void SetHeight(float t) { m_SlotHeight = t; }

	int m_nTotalPlayerCount{};
	int m_nPage{};
};
