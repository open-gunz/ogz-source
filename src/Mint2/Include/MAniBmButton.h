#ifndef MANIBMBUTTON_H
#define MANIBMBUTTON_H

#include "MButton.h"
#include "MAnimation.h"

class MAniBmButton : public MButton{
	MAnimation*		m_pUpAnimation;
	MAnimation*		m_pOverAnimation;
protected:
	virtual void OnDownDraw(MDrawContext* pDC);
	virtual void OnUpDraw(MDrawContext* pDC);
	virtual void OnOverDraw(MDrawContext* pDC);
	virtual void OnSize(int w, int h);
public:
	MAniBmButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MAniBmButton(void);
	void SetUpAniBitmap(MAniBitmap* pAniBitmap);
	void SetOverAniBitmap(MAniBitmap* pAniBitmap);

#define MINT_ANIBMBUTTON	"AniBmButton"
	virtual const char* GetClassName(void){ return MINT_ANIBMBUTTON; }
};

#endif
