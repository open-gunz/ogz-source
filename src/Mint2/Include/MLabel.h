#pragma once

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MLabel;
class MLabelLook{
public:
	virtual void OnDraw(MLabel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MLabel* pLabel, const MRECT& r);
};

class MLabel : public MWidget{
protected:
	MCOLOR			m_TextColor;
	MAlignmentMode	m_AlignmentMode;

	DECLARE_LOOK(MLabelLook)
	DECLARE_LOOK_CLIENT()

public:
	MLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	void SetTextColor(MCOLOR color);
	MCOLOR GetTextColor(void);

	MAlignmentMode GetAlignment(void);
	MAlignmentMode SetAlignment(MAlignmentMode am);


#define MINT_LABEL	"Label"
	virtual const char* GetClassName() override{ return MINT_LABEL; }
};
