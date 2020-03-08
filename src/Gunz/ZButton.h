#ifndef ZBUTTON_H
#define ZBUTTON_H

#include "MButton.h"
#include "MBmButton.h"

class ZButton : public MButton{
protected:
	unsigned char	m_nIllumination;
	DWORD			m_dwCurrentTime;
	DWORD			m_dwLastTime;
	DWORD			m_dwClickedTime;
	bool			m_bClicked;

	virtual void OnMouseIn(void);
	virtual void OnMouseOut(void);
	virtual void OnButtonDown(void);
	virtual void OnButtonUp(void);
	virtual void OnButtonClick(void);

	virtual bool OnShow(void);
	virtual void OnHide(void);

	virtual void OnDraw(MDrawContext* pDC);

public:
	ZButton(const char* szName, MWidget* pParent, MListener* pListener);
	virtual ~ZButton(void);
};

class ZBmButton: public MBmButton 
{
protected:
	unsigned char	m_nIllumination;
	DWORD			m_dwCurrentTime;
	DWORD			m_dwLastTime;
	DWORD			m_dwClickedTime;
	bool			m_bClicked;

	virtual void OnMouseIn(void);
	virtual void OnMouseOut(void);
	virtual void OnButtonDown(void);
	virtual void OnButtonUp(void);
	virtual void OnButtonClick(void);

	virtual bool OnShow(void);
	virtual void OnHide(void);

	virtual void OnDraw(MDrawContext* pDC);

public:
	ZBmButton(const char* szName, MWidget* pParent, MListener* pListener);
	virtual ~ZBmButton(void);
};

#endif