#pragma once

#include "MXml.h"

#include "RNameSpace.h"
#include "MUtil.h"
#include "RTypes.h"
#include "RealSpace2.h"
_USING_NAMESPACE_REALSPACE2

#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "Mint4R2.h"
#include "RParticleSystem.h"

#include "MObject.h"
#include "MMatchItem.h"
#include "MMatchMap.h"
#include "MSafeUDP.h"
#include "MMatchTransDataType.h"
#include "MErrorTable.h"
#include "Config.h"

#include "ZGlobal.h"
#include "ZMessages.h"

//#define _BIRDCAMERA


#ifndef _PUBLISH
//	#define _FASTDEBUG
//	#define _BIRDTEST



#define __BP(i,n) MBeginProfile(i,n);
#define __EP(i) MEndProfile(i);
#define __SAVEPROFILE(i) MSaveProfile(i);

#else
	#define _DO_NOT_USE_PROFILER
	#define __BP(i,n) ;
	#define __EP(i) ;
	#define __SAVEPROFILE(i) ;
#endif