#pragma once

#include "vector"

using namespace std;

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////////////////////////////
//	DEFINE
//////////////////////////////////////////////////////////////////////////
struct sHEVertex;
struct sHEEdge;
struct sHEFace;
class RMeshNode;
class RHEMesh;

//////////////////////////////////////////////////////////////////////////
//	ENUM
//////////////////////////////////////////////////////////////////////////
enum	eSubdivisionType
{
	BUTTERFLY,
};

//////////////////////////////////////////////////////////////////////////
//	HALF EDGE MESH
//////////////////////////////////////////////////////////////////////////
class RHEMesh
{
protected:
	sHEEdge*	mEdges;
	sHEVertex*	mVertices;
	sHEFace*	mFaces;

	int			miNumEdges;
	int			miNumVertices;
	int			miNumFaces;
	
protected:
	void		AddEdge( sHEVertex* pVertex_, sHEFace* pFace_, sHEEdge* pPrev_, sHEEdge* pNext_, sHEEdge* pPair_, sHEEdge* nEdge_ );
	void		SplitEdge( sHEEdge* pEdge_, sHEVertex* pVertex_, sHEEdge* nEdge_, sHEEdge* npEdge_ = NULL );
	sHEFace*	SplitFaceLocal( sHEFace* pFace_, sHEFace* nFace_, sHEEdge* nEdge1_, sHEEdge* nEdge2_ );

	virtual bool	Validate_HEMESH();		// TEST!!!!

public:
	void GetSubdivisionMesh( RMeshNode* pMeshNode_, eSubdivisionType eType_, rvector* pPointList_ = 0 );
	virtual void Draw();

public:
	RHEMesh();
	~RHEMesh();
};

//////////////////////////////////////////////////////////////////////////
//	SUBDIVISION MESH
//////////////////////////////////////////////////////////////////////////
class RSubdivisionMesh : public RHEMesh
{
protected:
	bool	mbNormal;
	bool	mbColor;
	bool	mbTexCoord;

	sHEEdge*	mSubEdge;
	sHEVertex*	mSubVertex;
	sHEFace*	mSubFace;

	sHEEdge*	mSubDestEdge;
	sHEVertex*	mSubDestVertex;
	sHEFace*	mSubDestFace;

	int			miSubNumEdges;
	int			miSubNumVertices;
	int			miSubNumFaces;

	int			miSubLevel;

	int			miSubVertexSize;
	int			miSubEdgeSize;
	int			miSubFaceSize;

	bool		mbIsFirstTime;

protected:
	virtual	void	Refinement() = 0;
	virtual void	Smoothing() = 0;

	int		GetValence( sHEVertex* pVertex_ );
	void	init( int subLevel_ );
	void	SplitFace( sHEFace* pFace_ );
	
	virtual bool	Validate();		// TEST!!!!

public:
	virtual void Subdivide( int subLevel_ );
	
public:
	RSubdivisionMesh();
	~RSubdivisionMesh();	
};


//////////////////////////////////////////////////////////////////////////
//	BUTTERFLY MESH
//////////////////////////////////////////////////////////////////////////
class RButterflyMesh : public RSubdivisionMesh
{
protected:
	virtual void	Refinement();
	virtual void	Smoothing();

	void	CalAddedVertex( sHEEdge* peCurr_, sHEVertex* pvCurr_, sHEVertex* pvDest_, 
		int valence_ /* pvDest¿« valence*/, sHEVertex* nVertex_ );	
	
public:
	void			initialize( int level_, RMeshNode* pMeshNode_, rvector* pPointList_ = 0 )
	{
		GetSubdivisionMesh( pMeshNode_, BUTTERFLY, pPointList_ );
		init( level_ );
	};
	virtual void	Draw();

	void	Reset( rvector* pPointList_, rvector* pNormalList_ = 0 );

	sHEVertex*	GetVertexList() const
	{
		return	mSubDestVertex;
	}
	int	GetNumPrimitive() const
	{
		return	miSubNumFaces;
	}
	int	GetNumVertices() const
	{
		return	miSubNumVertices;
	}
	
public:
	RButterflyMesh( int level_, RMeshNode* pMeshNode_, rvector* pPointList_ = 0 ) 
	{
		initialize( level_, pMeshNode_, pPointList_ );
	};
	RButterflyMesh() {};
	~RButterflyMesh(){};
};

_NAMESPACE_REALSPACE2_END