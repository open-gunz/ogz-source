#ifndef ZBMNUMLABEL_H
#define ZBMNUMLABEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

#define BMNUM_NUMOFCHARSET		16

/// 비트맵으로 그린 레이블.	로비에 쓰일 레이블이다.
/// 비트맵에 0~7 / 8 9 , / .   순으로 그려져 있어야 한다.
class ZBmNumLabel : public MWidget{
protected:
	MBitmap*		m_pLabelBitmap;
	MSIZE			m_CharSize;
	MAlignmentMode	m_AlignmentMode;
	int				m_nIndexOffset;
	int				m_nCharMargin[ BMNUM_NUMOFCHARSET];

	virtual void OnDraw(MDrawContext* pDC);
public:
	ZBmNumLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	void SetLabelBitmap(MBitmap* pLabelBitmap);
	void SetCharSize(const MSIZE &size);
	void SetNumber(int n,bool bAddComma = false);
	void SetIndexOffset(int nOffset) { m_nIndexOffset = nOffset; }
	void SetCharMargin( int* nMargin);
	void SetAlignmentMode( MAlignmentMode am)			{ m_AlignmentMode = am; }

#define MINT_ZBMNUMLABEL	"BmNumLabel"
	virtual const char* GetClassName(void){ return MINT_ZBMNUMLABEL; }
};

#endif