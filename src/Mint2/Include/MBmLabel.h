#pragma once

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MBmLabel : public MWidget{
protected:
	MBitmap*		m_pLabelBitmap;
	MSIZE			m_CharSize;

	virtual void OnDraw(MDrawContext* pDC) override;
public:
	MBmLabel(const char* szName = nullptr, MWidget* pParent = nullptr, MListener* pListener = nullptr);

	void SetLabelBitmap(MBitmap* pLabelBitmap);
	void SetCharSize(MSIZE &size);
#define MINT_BMLABEL	"BmLabel"
	virtual const char* GetClassName() override { return MINT_BMLABEL; }
};
