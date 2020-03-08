#pragma once

#include "MWidget.h"

class MToolTip : public MWidget{
protected:
	bool	m_bUseParentName;

public:
	MToolTip(const char* szName, MWidget* pParent);
	virtual ~MToolTip() override;

	virtual void SetBounds();

	void SetText(const char* szText);

	bool IsUseParentName();

	virtual void OnDraw(MDrawContext* pDC) override;

#define MINT_TOOLTIP	"ToolTip"
	virtual const char* GetClassName() override { return MINT_TOOLTIP; }
};
