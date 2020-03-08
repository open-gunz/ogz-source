#include "stdafx.h"
#include "RSphere.h"
#include "RealSpace2.h"

using namespace RealSpace2;

RSphere::RSphere()
{
	mCentre	= rvector(0,0,0);
	mRadius	=  10.f;
}

RSphere::~RSphere()
{
}

bool RSphere::isCollide( CDInfo* data_, CDInfoType cdType_ )
{
	rvector distance = *data_->clothCD.v - mCentre;
	if (MagnitudeSq(distance) < mRadius * mRadius)
	{
		*data_->clothCD.pos = mCentre + ( mRadius * *data_->clothCD.n);
		return true;
	}
	return false;
}