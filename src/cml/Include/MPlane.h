#ifndef _MPLANE_H
#define _MPLANE_H

#include "MVector3.h"

class MPlane
{
public:
	float a, b, c, d;

	MPlane() { }
	MPlane(const MVector3& normal, float dis) {
		a = normal.x;
		b = normal.y;
		c = normal.z;
		d = dis;
	}
	MPlane(const MVector3& normal, const MVector3& pos)
	{
		a = normal.x;
		b = normal.y;
		c = normal.z;
		d = - normal.DotProduct(pos);
	}
	const MVector3& Normal() const {
		return *((MVector3*)&a);
	}

	float GetDistanceTo(MVector3& point) const
	{
		MVector3 norm = Normal();
		return (point.DotProduct(norm) + d);
	}
};



#endif