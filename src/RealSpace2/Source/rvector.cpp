#include "stdafx.h"
#include "rvector.h"
#include <d3d9types.h>
#include <d3dx9math.h>

v3::v3(const D3DVECTOR& vec) : x{ vec.x }, y{ vec.y }, z{ vec.z } {}
v3::operator D3DVECTOR () const { return{ x, y, z }; }

v3::v3(const D3DXVECTOR3& vec) : x{ vec.x }, y{ vec.y }, z{ vec.z } {}
v3::operator D3DXVECTOR3 () const { return{ x, y, z }; }