#pragma once

//  [10/13/2003]
//////////////////////////////////////////////////////////////////////////
// RCylinder 
// Bounding Cylinder
//
// Magicbell
//////////////////////////////////////////////////////////////////////////

#include "RBoundary.h"
#include "RTypes.h"

//Declare

//class RCylinder 
class RCylinder : public RBoundary
{
public:
	rvector mTopCentre;
	rvector mBottomCentre;
	float mHeight;
	float mRadius;

	rmatrix mWorld;

public:
	bool isCollide(	CDInfo* data_, CDInfoType cdType_ );
	inline void setTopCentre( const rvector& in_ );
	inline void setBottomCentre( const rvector& in_ );
	inline void setHeight( const float& in_ );
	inline void setRadius( const float& in_ );
	inline void setTransform( rmatrix& world_ );

public:
	RCylinder();
	virtual ~RCylinder();
};

void RCylinder::setTopCentre(const rvector& in_) {
	mTopCentre = in_; auto temp = mTopCentre - mBottomCentre;  mHeight = Magnitude(temp); }
void RCylinder::setBottomCentre( const rvector& in_ ) {
	mBottomCentre = in_; auto temp = mTopCentre - mBottomCentre;  mHeight = Magnitude(temp); }
void RCylinder::setHeight( const float& in_ ) { mHeight = in_; }
void RCylinder::setRadius( const float& in_ ) { mRadius = in_; }
void RCylinder::setTransform( rmatrix& world_ ) { mWorld = world_; }


//////////////////////////////////////////////////////////////////////////
// Helper Function
// Shortest Distance Between a Line and a Point
// return : true - shortest point is exist in a line , false - does not
// intersection : 점과 line 의 가장 가까운 라인위의 교점 - 현재 쓸모없음
// direction : line의 방향 - 현재 쓸모없음
// distacne : 라인과 점과의 거리
// TODO (OPTI)  : 쓸모없는 input을 없애고 연산을 줄인다 
//////////////////////////////////////////////////////////////////////////
bool getDistanceBetLineSegmentAndPoint( const rvector& lineStart_, 
									   const rvector& lineEnd_, 
									   rvector* point_, 
									   rvector* intersection_, 
									   rvector* direction_, 
									   float& distance_ );