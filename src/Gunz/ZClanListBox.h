#ifndef _ZCLANLISTBOX_H
#define _ZCLANLISTBOX_H

#include "MWidget.h"
#include "MMatchGlobal.h"

#define NUM_DISPLAY_CLAN	4

struct ZCLANINFO {
	int nClanEmblemID;
	char szClanName[CLAN_NAME_LENGTH];
	int nPlayers;		// 대기중인 사람수
	bool bEmpty;		// 비어있는방인지
};

class ZClanListBox : public MWidget {
protected:
	int					m_nPrevStageCount;
	int					m_nNextStageCount;

	int					m_iNumRoom;
	float				m_RoomWidth;
	float				m_RoomHeight;
	ZCLANINFO			m_pClanInfo[NUM_DISPLAY_CLAN];
	MBitmap*			m_pRoomFrame;

	int					m_Selection;
	int					m_currPage;

protected:
	virtual void	OnDraw( MDrawContext* pDC );

public:
	void	SetWidth( float width ) { m_RoomWidth	= width; }
	void	SetHeight( float height ) { m_RoomHeight	= height; }

public:
	ZClanListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZClanListBox(void);

	void SetInfo(int nIndex, int nEmblemID, const char *szName, int nPlayers);
	void Clear(int nIndex);
	void ClearAll();
};

#endif