#include "stdafx.h"
#include "MMatrix.h"

static float _matIdentity[] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
} ;

const MMatrix MMatrix::IDENTITY(_matIdentity);
