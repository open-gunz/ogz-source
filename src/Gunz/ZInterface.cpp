#include "stdafx.h"

#include "ZInterface.h"
#include "MListBox.h"
#include "MEdit.h"
#include "ZPost.h"

ZInterface::ZInterface(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName,pParent,pListener)
{
}

ZInterface::~ZInterface()
{
}

bool ZInterface::IsDone()
{
	return false;
}

bool ZInterface::OnCreate()
{
	return false;
}

void ZInterface::OnDraw(MDrawContext* pDC)
{
}

void ZInterface::OnDestroy()
{
}
