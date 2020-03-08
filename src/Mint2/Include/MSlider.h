#ifndef MSLIDER_H
#define MSLIDER_H

#include "MScrollBar.h"
#include "MLookNFeel.h"
//#include "MBSliderLook.h"

class MSliderThumb;
class MBSliderThumbLook;

class MSliderThumbLook{
protected:
public:
	virtual void OnDraw(MSliderThumb* pThumb, MDrawContext* pDC);
	virtual MRECT GetClientRect(MSliderThumb* pThumb, const MRECT& r);
	virtual MSIZE GetDefaultSize(MSliderThumb* pThumb);
public:
	MSliderThumbLook(){};
};

class MSliderThumb : public MThumb{
	DECLARE_LOOK(MSliderThumbLook)
	DECLARE_LOOK_CLIENT()
public:
	MSliderThumb(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual MSIZE GetDefaultSize(void);
};

class MSlider : public MScrollBar{
	DECLARE_LOOK(MScrollBarLook)
	DECLARE_LOOK_CLIENT()
protected:
	virtual int GetThumbSize(void);
	void Initialize(void);
public:
	MSlider(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	MSlider(MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MSlider(void);

#define MINT_SLIDER	"Slider"
	virtual const char* GetClassName(void){ return MINT_SLIDER; }
};

#endif