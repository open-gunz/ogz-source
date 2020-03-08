#pragma once

//  [10/13/2003]
//////////////////////////////////////////////////////////////////////////
// RSphere
// Bound Sphere
// Magicbell
//////////////////////////////////////////////////////////////////////////

#include "RBoundary.h"
#include "RTypes.h"

class RSphere :	public RBoundary
{
public:
	rvector mCentre;
	float mRadius;

public:
	bool isCollide(	CDInfo* data_, CDInfoType cdType_ );
	inline void setSphere( const rvector& centre_, float radius_ );
	void setSphere( const rvector& centre_ )
	{
		mCentre	= centre_;
	}
	void setSphere( float radius_ )
	{
		mRadius	= radius_;
	}

	rmatrix mWorld;
	inline void setTransform( rmatrix& world_ );

public:
	RSphere();
	~RSphere();
};

void RSphere::setSphere( const rvector& centre_, float radius_ )
{
	mCentre = centre_;
	mRadius = radius_;
	mWorld = TranslationMatrix(centre_ + v3{ 0, 100, 0 });
}

void RSphere::setTransform( rmatrix& world_ )
{
	mWorld = TranslationMatrix(mCentre);
	mWorld *= world_;
}