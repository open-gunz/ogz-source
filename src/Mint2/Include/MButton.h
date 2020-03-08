#pragma once

#include "MWidget.h"
#include "MLookNFeel.h"
#include <memory>

#define MBUTTON_DEFAULT_ALIGNMENT_MODE	(MAM_HCENTER|MAM_VCENTER)

class MMsgBox;
class MButtonGroup;

enum MButtonType {
	MBT_NORMAL = 0,
	MBT_PUSH,
	MBT_PUSH2,
};

enum MButtonKeyAssigned {
	MBKA_NONE = 0,
	MBKA_ENTER,
	MBKA_ESC,
};

class MButton;

class MButtonLook {
protected:
	bool	m_bWireLook{};

	virtual void OnDownDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnUpDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnOverDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnDisableDraw(MButton* pButton, MDrawContext* pDC);
	virtual void OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed){};

public:
	virtual void OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC);
	virtual void OnDraw(MButton* pButton, MDrawContext* pDC);
	virtual MRECT GetClientRect(MButton* pButton, const MRECT& r);
	void SetWireLook(bool b) {	m_bWireLook = b;	}
	bool GetWireLook() const{	return m_bWireLook; }
};

class MButton : public MWidget {
protected:
	bool		m_bMouseOver{};
	bool		m_bLButtonDown{};
	bool		m_bRButtonDown{};
	MCOLOR		m_TextColor{ 0, 255, 0 };
	bool		m_bShowText = true;
	MAlignmentMode	m_AlignmentMode = MBUTTON_DEFAULT_ALIGNMENT_MODE;
	MButtonType		m_Type = MBT_NORMAL;
	bool		m_bChecked{};
	bool		m_bComboDropped{};
	bool		m_bStretch{};
	
	MButtonGroup	*m_pButtonGroup{};
	int				m_nIndexInGroup{};

public:
	bool		m_bEnableEnter = true;
	bool		m_bHighlight = true;
	MBitmap*	m_pIcon{};
	MButtonKeyAssigned	m_nKeyAssigned = MBKA_NONE;
	std::unique_ptr<MMsgBox> m_pMsgBox;

	MPOINT		m_ClickPos{};
	MPOINT		m_LDragStartClickPos{};
	int			m_LDragVariationX{};
	int			m_LDragVariationY{};

protected:
	virtual void OnMouseIn() {}
	virtual void OnMouseOut() {}
	virtual void OnButtonDown() {}
	virtual void OnButtonUp() {}
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;

	virtual void OnButtonClick();	

	virtual bool OnShow() override;
	virtual void OnHide() override;

public:
	MButton(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);
	MButton(const MButton&) = delete;
	MButton& operator=(const MButton&) = delete;
	~MButton();

	void SetTextColor(MCOLOR color);
	MCOLOR GetTextColor();
	void ShowText(bool bShow = true);
	virtual bool DefaultCommand() override;

	MAlignmentMode GetAlignment();
	MAlignmentMode SetAlignment(MAlignmentMode am);

	void SetType(MButtonType t);
	MButtonType GetType();

	void SetCheck(bool bCheck);
	bool GetCheck();

	bool IsButtonDown();
	bool IsMouseOver();

	void SetConfirmMessageBox(const char* szMessage);

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) override;

	int GetLDragVariationX() const { return m_LDragVariationX; }
	int GetLDragVariationY() const { return m_LDragVariationY; }

	DECLARE_LOOK(MButtonLook)
	DECLARE_LOOK_CLIENT()

	void SetComboDropped(bool b) { m_bComboDropped = b; }
	bool GetComboDropped() const { return m_bComboDropped; }

	void SetStretch(bool b) { m_bStretch = b; }
	bool GetStretch() const { return m_bStretch; }
	void SetButtonGroup(MButtonGroup *pGroup);
	int GetIndexInGroup() const { return m_nIndexInGroup; }

#define MINT_BUTTON	"Button"
	virtual const char* GetClassName(){ return MINT_BUTTON; }
};

#define MBTN_CLK_MSG		"click"
#define MBTN_RCLK_MSG		"rclick"
#define MBTN_DN_MSG			"down"
#define MBTN_UP_MSG			"up"
#define MBTN_RDN_MSG		"rdown"
#define MBTN_RUP_MSG		"rup"
#define MBTN_IN_MSG			"in"
#define MBTN_OUT_MSG		"out"

class MButtonGroup {
protected:
	friend MButton;

	int		m_nCount{};
	MButton *m_pPrevious{};
};
