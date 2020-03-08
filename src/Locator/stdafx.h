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
#include <afxcview.h>
#endif

#include <cassert>
#define ASSERT assert

#include "SafeString.h"
#include "MDebug.h"