#include "stdafx.h"
#include "rquaternion.h"

rquaternion::rquaternion(const D3DXQUATERNION& quat)
	: x{ quat.x }, y{ quat.y }, z{ quat.z }, w{ quat.w } {}

rquaternion::operator D3DXQUATERNION () const { return{ x, y, z, w }; }