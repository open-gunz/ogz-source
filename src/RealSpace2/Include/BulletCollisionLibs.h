#pragma once

#ifndef _DEBUG
#pragma comment(lib, "../sdk/bullet/lib/Release/BulletCollision.lib")
#pragma comment(lib, "../sdk/bullet/lib/Release/LinearMath.lib")
#else
#pragma comment(lib, "../sdk/bullet/lib/Debug/BulletCollision.lib")
#pragma comment(lib, "../sdk/bullet/lib/Debug/LinearMath.lib")
#endif
