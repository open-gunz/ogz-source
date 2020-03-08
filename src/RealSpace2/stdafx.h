#pragma once

#include "targetver.h"

#define POINTER_64 __ptr64

#include <list>
#include <vector>
#include <map>
#include <string>
#include <list>

#ifdef _WIN32
#include <d3d9.h>
#include <d3dx9math.h>
#endif

#include "MDebug.h"
#include "MZFileSystem.h"
#include "FileInfo.h"
#include "MXml.h"
#include "RTypes.h"
#ifdef _WIN32
#include "RMesh.h"
#endif

#include "MTime.h"
#include "SafeString.h"

#include <cassert>
#define ASSERT assert
