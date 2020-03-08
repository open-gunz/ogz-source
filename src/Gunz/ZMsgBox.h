#pragma once

#include "MMsgBox.h"

class ZMsgBox : public MMsgBox{
protected:
	bool				m_bExclusive;
	u64					m_nShowTime;
	bool				m_bNextVisible;
	MListener*			m_pCustomListener;

	virtual bool OnShow(void);
	virtual void OnDraw(MDrawContext* pDC);

public:
	ZMsgBox(const char* szMessage, MWidget* pParent, MListener* pListener=NULL, MMsgBoxType nType=MT_NOTDECIDED );
	virtual ~ZMsgBox(void);

	void SetCustomListener(MListener* pListener) { m_pCustomListener = pListener; }
	MListener* GetCustomListener() { return m_pCustomListener; }

	void Show(bool bVisible, bool bModal=false);
	virtual void SetText(const char* szText);
};


MListener* ZGetMsgBoxListener(void);
MListener* ZGetConfirmMsgBoxListener(void);