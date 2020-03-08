#ifndef _RERROR_H
#define _RERROR_H

/// error def

#include "RTypes.h"

enum RERROR
{
	ROK = 0,

	// d3d°ü·Ã
	RERROR_CANNOT_CREATE_D3D	= 1000,
	RERROR_INVALID_DEVICE		= 1001,
	
	
};





//-------------------------------------------------------
void RSetError(int nErrCode);
int RGetLastError(void);


#endif