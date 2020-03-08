#ifndef MBButtonLook_H
#define MBButtonLook_H

#include "MButton.h"
#include "MScalableLook.h"

class MBButtonLook : public MButtonLook, public MScalableLook {
public:
	MBitmap*	m_pUpBitmaps[9];
	MBitmap*	m_pDownBitmaps[9];
	MBitmap*	m_pOverBitmaps[9];
	MBitmap*	m_pFocusBitmaps[4];
	MFont*		m_pFont;
	MCOLOR		m_FontColor;
	MCOLOR		m_FontDisableColor;
	MCOLOR		m_FontDownColor;
	MCOLOR		m_FontHighlightColor;
	bool		m_bStretch;
	bool		m_bCustomLook;
	MPOINT		m_FontDownOffset;
	
protected:
	virtual void DrawText(MButton* pButton, MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode a);
	virtual void DrawFocus(MDrawContext* pDC, MRECT& r);
	virtual void OnDownDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnUpDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnOverDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnDisableDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed);
	virtual void OnDraw(	MButton* pButton, MDrawContext* pDC );
public:
	MBButtonLook(void);
	void SetCustomLook(bool b) {
		m_bCustomLook = b;
	}
	bool GetCustomLook() const{
		return m_bCustomLook;
	}

	virtual MRECT GetClientRect(MButton* pButton, MRECT& r);
};

#endif
