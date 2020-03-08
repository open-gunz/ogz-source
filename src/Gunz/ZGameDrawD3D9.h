#pragma once

#include "MUtil.h"

class ZGameDrawD3D9
{
public:
	ZGameDrawD3D9(class ZGame& Game);

	void Draw();
	void OnInvalidate() {}
	void OnRestore() {}

private:
	void DrawScene();

	ZGame& Game;
};