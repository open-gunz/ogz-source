#pragma once

#include "RCloth.h"
#include "RTypes.h"

#define GRENADE_SPEAR_EMBLEM_POWER	1000 //2000
#define BULLET_SPEAR_EMBLEM_POWER	1500 //3000
#define SHOTGUN_SPEAR_EMBLEM_POWER	2000 //3500
#define ROCKET_SPEAR_EMBLEM_POWER	2500 //5000
#define EXPLOSION_EMBLEM_POWER		3000 //5000

enum RESTRICTION_AXIS
{
	AXIS_X, AXIS_Y, AXIS_Z,
};
enum RESTRICTION_COMPARE
{
	COMPARE_GREATER, COMPARE_LESS,
};

struct sRestriction
{
	RESTRICTION_AXIS	axis;
	float							position;
	RESTRICTION_COMPARE compare;
};

class ZClothEmblem : public RCloth
{
private:
	static unsigned int	msRef;

protected:
	ZWorld*			m_pWorld;
	RMeshNode*		mpMeshNode;
	RBaseTexture*	mpTex;
	rvector*		mpWind;
	float			mfBaseMaxPower;
	rvector			mBaseWind;
	D3DLIGHT9*		mpLight;

	RWindGenerator	mWndGenerator;

	RealSpace2::rboundingbox	mAABB;
	list<sRestriction*> mRestrictionList;
	DWORD		mMyTime;
	bool		mbIsInFrustrum;

protected:
	virtual void	accumulateForces();
	virtual void	varlet();
	virtual void	satisfyConstraints();

public:

	float			m_fDist;
	rmatrix			mWorldMat;
	
public:
	virtual void	update();
	virtual void	render();
	virtual void	UpdateNormal();

	void setOption( int nIter_, float power_, float inertia_ );
	void CreateFromMeshNode( RMeshNode* pMeshNdoe_ , ZWorld* pWorld);

	void setForce( float x_, float y_, float z_ )
	{
		mpWind->x	= x_;
		mpWind->y	= y_;
		mpWind->z	= z_;
	};
	void setExplosion( rvector& pos_, float power_ );
	void CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ );

	void OnInvalidate();
	void OnRestore();

	void SetBaseWind( rvector& w_ )
	{
		mBaseWind	= w_;
	}
	void SetBaseMaxPower( float p_ )
	{
		mfBaseMaxPower	= p_;
	}
	void AddRestriction( sRestriction* rest_ )
	{
		mRestrictionList.push_back( rest_ );
	}
	RWindGenerator* GetWndGenerator(){ return &mWndGenerator; }

	static int GetRefCount() { return msRef;}

public:
	ZClothEmblem();
	virtual ~ZClothEmblem();

};

class ZEmblemList final : public std::list<ZClothEmblem*>
{
protected:
	std::map<std::string, ZClothEmblem*> mEmblemMap;
	std::map<std::string, ZClothEmblem*>::iterator mEmblemMapItor;

public:
	~ZEmblemList();

public:
	void	Update();
	void    Draw();
	ZClothEmblem*	Get( int i_ );
	void	SetExplosion( rvector& pos_, float power_ );
	void	CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ );

	void	OnInvalidate();
	void	OnRestore();	
	void	Clear();
	void Add( ZClothEmblem* p_, const char* pName_ );

	void	InitEnv( char* pFileName_ );
};