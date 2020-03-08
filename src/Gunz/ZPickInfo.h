#pragma once

#include "RBspObject.h"

class ZObject;

struct ZPICKINFO {
	ZObject*	pObject;
	RPickInfo	info;

	bool bBspPicked;
	RBSPPICKINFO bpi;
};