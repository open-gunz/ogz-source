#pragma once

#include <unordered_map>
#include "RAnimationMgr.h"
#include "RBspObject.h"

class LagCompManager
{
public:
	bool Create();

	RealSpace2::RBspObject* GetBspObject(const char* MapName);

private:
	bool LoadAnimations(const char* filename, int Index);

	RealSpace2::RAnimationMgr AniMgrs[2]; // 0 = male, 1 = female
	std::unordered_map<std::string, RealSpace2::RBspObject> Maps;
};