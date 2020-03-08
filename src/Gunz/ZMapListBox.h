#ifndef ZMAPLISTBOX_H
#define ZMAPLISTBOX_H

#include "MWidget.h"

class MListBox;
class MZFileSystem;
class MTextArea;

class ZMapListBox : public MWidget{
protected:
	MListBox*	m_pListBox;
	MBitmap*	m_pThumbnail;

protected:
	virtual void OnDraw(MDrawContext* pDC);
	virtual bool OnShow(void);
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);
public:
	ZMapListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZMapListBox(void);

	void Refresh(MZFileSystem* pFS);

	const char* GetSelItemString(void);

	void SetSelIndex(int i);
};

#endif
