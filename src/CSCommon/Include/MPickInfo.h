#pragma once

#include "RMeshUtil.h"
#include "RBspObject.h"

struct MPICKINFO {
	class MMatchObject*	pObject;
	struct { v3 vOut; float t; RMeshPartsType parts; } info;

	bool bBspPicked;
	RealSpace2::RBSPPICKINFO bpi;
};
