#pragma once

#include "MWidget.h"
#include "MLookNFeel.h"

class MGroup;
class MGroupLook {
public:
	virtual void OnDraw(MGroup* pGroup, MDrawContext* pDC);
	virtual MRECT GetClientRect(MGroup* pGroup, const MRECT& r);
};

class MGroup : public MWidget{
	DECLARE_LOOK(MGroupLook)
	DECLARE_LOOK_CLIENT()
public:
	MGroup(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MGroup();

#define MINT_GROUP	"Group"
	virtual const char* GetClassName() override { return MINT_GROUP; }
};
