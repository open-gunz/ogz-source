#pragma once

#include "mwidget.h"
#include "map"

class MButton;

#define SMI_MAPNAME_LENGTH		128
#define SMI_ROOMNAME_LENGTH		128
#define	NUM_DISPLAY_ROOM		8

enum GameState
{
	GAME_PLAYING = 0,
	GAME_WAITING,
	GMAE_CLOSED,
	GAME_STATE_NUM,
};

struct sMapInfo
{
	bool		IsEmpty;
	int			RoomNumber;
	MUID		uidStage;
	char		map_name[SMI_MAPNAME_LENGTH];
	char		room_name[SMI_ROOMNAME_LENGTH];
	int			nPeople;
	int			nMaxPeople;
	bool		bForcedEnter;
	bool		bLimitLevel;
	int			nMasterLevel;
	int			nLimitLevel;
	bool		bPrivate;
	GameState	roomState;
	MMATCH_GAMETYPE		nGame_Type;
	
	sMapInfo()
	{
		uidStage			= MUID(0,0);
		IsEmpty			= true;
		bForcedEnter	= false;
		bPrivate		= false;
	}
};
class ZRoomListBox :	public MWidget
{
protected:
	int						m_nPrevStageCount;
	int						m_nNextStageCount;

	int						m_iNumRoom;
	float					m_RoomWidth;
	float					m_RoomHeight;
	sMapInfo				m_pMapInfo[NUM_DISPLAY_ROOM];
	map<string, MBitmap*>	m_pMapImage;
	map<MMATCH_GAMETYPE, MBitmap*>	m_pIconImage;
	MBitmap*				m_pRoomFrame;

	int						m_Selection;
	int						m_currPage;

	int						m_iGapWidth;
	int						m_iGapHeight;
	int						m_iGapCenter;

	// interesting check this (Michael)
	//MScrollBar*		m_pScrollBar;
	MUID					m_uidSelectedPrivateStageUID;
protected:
	virtual void	OnDraw( MDrawContext* pDC );
	virtual bool	OnShow( void );
	virtual bool	OnCommand( MWidget* pWidget, const char* szMassage );
	virtual bool	OnEvent(MEvent* pEvent, MListener* pListener);
protected:
	MUID GetSelRoomUID();
	MUID GetSelectedPrivateStageUID();
public:
	void	SetBannerImage(char* pBannerName, MBitmap* pBitmap);
	void	SetIconImage(MMATCH_GAMETYPE type, MBitmap* pBitmap);
	void	SetFrameImage(MBitmap* pBitmap){m_pRoomFrame = pBitmap;};
	void	SetWidth( float width ) { m_RoomWidth	= width; }
	void	SetHeight( float height ) { m_RoomHeight	= height; }
	void	SetRoomName( int i, char* pRoomName, sMapInfo* info );
	void	Resize( float w, float h );
	

	void Clear();
	void SetScroll(int nPrevStageCount, int nNextStageCount);

	struct _RoomInfoArg
	{
		int nIndex;
		int nRoomNumber;
		MUID uidStage;
        char* szRoomName;
		// maybe changing this can also fix banner prob
		char* szMapName;

		int nMaxPlayers;
		int nCurrPlayers;
		bool bPrivate;
		bool bForcedEntry;
        bool bLimitLevel;
		int nMasterLevel;
		int nLimitLevel;
        MMATCH_GAMETYPE nGameType;
		STAGE_STATE nStageState;
	};
	void SetRoom(const _RoomInfoArg* pRoomInfo);
	void SetEmptyRoom(int nIndex);
	void SetPage();
	int GetFirstStageCursor();
	int GetLastStageCursor();
	void RequestSelStageJoin();
	void RequestSelPrivateStageJoin();
	const sMapInfo* GetSelMapInfo();
	void SetPrivateStageUID(const MUID& uidStage);
public:
	ZRoomListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZRoomListBox(void);

};