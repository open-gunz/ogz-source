#pragma once

#include "targetver.h"

#ifdef MFC
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>

#include <afxdtctl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif
#include <afxrich.h>
#include <afxcview.h>
#endif

#include <stdio.h>

#include "MSharedCommandTable.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MMatchGameType.h"

#include "SafeString.h"

#include <cassert>
#define ASSERT assert

#define _QUEST
#define _QUEST_ITEM
#define _MONSTER_BIBLE