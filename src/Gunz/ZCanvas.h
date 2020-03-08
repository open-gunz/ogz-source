#ifndef _ZCANVAS_H
#define _ZCANVAS_H


#include "MWidget.h"


typedef void (*ZC_ONDRAW)(void* pSelf, MDrawContext* pDC);

class ZCanvas : public MWidget
{
private:
protected:
	ZC_ONDRAW			m_pOnDrawFunc;
	virtual void OnDraw(MDrawContext* pDC);
public:
	ZCanvas(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZCanvas();
	void SetOnDrawCallback(ZC_ONDRAW pCallback) { m_pOnDrawFunc = pCallback; }

	#define MINT_CANVAS			"Canvas"
	virtual const char* GetClassName(void){ return MINT_CANVAS; }
};






#endif