#pragma once

#include "targetver.h"

#define POINTER_64 __ptr64

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <cstddef>
#include <stdio.h>

#define _QUEST

#define _QUEST_ITEM

#define _MONSTER_BIBLE 

// stl
#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

// d3d9
#include "d3d9.h"
#include "d3dx9math.h"

#include "fmod.h"

// cml
#include "MXml.h"
#include "MUtil.h"
#include "MDebug.h"
#include "MRTTI.h"
#include "MUID.h"
#include "MemPool.h"

#ifdef GetClassName
#undef GetClassName
#endif

// mint
#include "Mint.h"
#include "MWidget.h"
#include "MBitmap.h"
#include "MButton.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "MFrame.h"
#include "MPopupMenu.h"

// realspace2
#include "RNameSpace.h"
#include "RTypes.h"
#include "RealSpace2.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "Mint4R2.h"

// cscommon
#include "MObject.h"
#include "MMatchItem.h"
#include "MMatchMap.h"
#include "MSafeUDP.h"
#include "MMatchClient.h"
#include "MMatchTransDataType.h"
#include "MErrorTable.h"
#include "Config.h"
#include "MSharedCommandTable.h"
#include "MObjectTypes.h"

// gunz global
#include "ZApplication.h"
#include "ZGlobal.h"
#include "ZMessages.h"
#include "ZStringResManager.h"
#include "ZGameInterface.h"
#include "ZCombatInterface.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZObject.h"
#include "ZIDLResource.h"
#include "ZInterfaceListener.h"
#include "ZColorTable.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "ZNetRepository.h"
#include "ZItem.h"
#include "ZItemDesc.h"
#include "ZPost.h"
#include "ZSoundEngine.h"
#include "ZSoundFMod.h"
#include "ZCamera.h"
#include "ZCharacter.h"
#include "ZActor.h"

#include "SafeString.h"
#include "RGGlobal.h"

#include <cassert>
#define ASSERT assert