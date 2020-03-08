#include "stdafx.h"
#include "Rcylinder.h"
#include "RealSpace2.h"

using namespace RealSpace2;

RCylinder::RCylinder() :
	mTopCentre(0, 0, 0), mBottomCentre(0, 0, 0), mHeight(0), mRadius(0)
{
	GetIdentityMatrix(mWorld);
}

RCylinder::~RCylinder()
{
}

bool RCylinder::isCollide( CDInfo* data_, CDInfoType cdType_ )
{
	rvector intersection;
	float distance;

	if( !getDistanceBetLineSegmentAndPoint( mTopCentre, mBottomCentre, data_->clothCD.v, 
		&intersection , NULL, distance ) )
	{
		return false;
	}

	if( distance > mRadius )
	{
		return false;
	}

	*data_->clothCD.pos = intersection + ( (*data_->clothCD.n) * mRadius * 1.2f );

	return true;
}

bool getDistanceBetLineSegmentAndPoint( const rvector& lineStart_, 
									   const rvector& lineEnd_, 
									   rvector* point_, 
									   rvector* intersection_, 
									   rvector* direction_, 
									   float& distance_ )
{
	rvector line = lineEnd_ - lineStart_;
	rvector cross_line = *point_ - lineStart_;
	float line_length_square = MagnitudeSq(line);

	float u = DotProduct(cross_line, line) / line_length_square;

	if( u < 0.0f || u > 1.0f )
	{
		return false;
	}

	rvector intersection = lineStart_ + u * (lineEnd_ - lineStart_);

	if( intersection_ != NULL )
	{
		*intersection_	= intersection;
	}

	if( direction_ != NULL )
	{
		auto vec = *point_ - intersection;
		*direction_ = Normalized(vec);
	}

	distance_ = Magnitude(*point_ - intersection);

	return true;
}