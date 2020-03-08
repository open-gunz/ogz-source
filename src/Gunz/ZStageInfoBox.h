#ifndef _ZSTAGEINFOBOX_H
#define _ZSTAGEINFOBOX_H

#include "MListBox.h"
#include "MBListBoxLook.h"

class ZStageInfoBox;

class ZStageInfoBoxLook : public MListBoxLook {
public:
	virtual void OnDraw(ZStageInfoBox* pBox, MDrawContext* pDC);
};

class ZStageInfoBox : public MListBox{
//	DECLARE_LOOK(ZStageInfoBoxLook)
	MBListBoxLook *m_pLook;
public:
	ZStageInfoBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZStageInfoBox(void);

	virtual void OnDraw(MDrawContext* pDC);
	void SetLook(MBListBoxLook *pLook) { m_pLook=pLook; }
};


#endif