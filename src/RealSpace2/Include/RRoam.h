#ifndef __RROAM_H
#define __RROAM_H

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

typedef struct _LITVERTEX {
	float x, y, z;		// world position
	DWORD Diffuse;   // diffuse color    
	DWORD Specular;  // specular color    
	float tu1, tv1;  // texture coordinates
} LITVERTEX, *LPLITVERTEX; 
#define RSLFVF	( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )

// Depth of variance tree: should be near SQRT(PATCH_SIZE) + 1
#define VARIANCE_DEPTH (9)

//
// TriTreeNode Struct
// Store the triangle tree data, but no coordinates!
//
struct TriTreeNode
{
	TriTreeNode *LeftChild;
	TriTreeNode *RightChild;
	TriTreeNode *BaseNeighbor;
	TriTreeNode *LeftNeighbor;
	TriTreeNode *RightNeighbor;

	WORD index; 
	
};

class Patch {
protected:

	unsigned char *m_HeightMap;									// Pointer to height map to use
	int m_WorldX, m_WorldY;										// World coordinate offset of this patch.

	unsigned char m_VarianceLeft[ 1<<(VARIANCE_DEPTH)];			// Left variance tree
	unsigned char m_VarianceRight[1<<(VARIANCE_DEPTH)];			// Right variance tree

	unsigned char *m_CurrentVariance;							// Which varience we are currently using. [Only valid during the Tessellate and ComputeVariance passes]
	unsigned char m_VarianceDirty;								// Does the Varience Tree need to be recalculated for this Patch?
	unsigned char m_isVisible;									// Is this patch visible in the current frame?

	TriTreeNode m_BaseLeft;										// Left base triangle tree node
	TriTreeNode m_BaseRight;									// Right base triangle tree node

public:
	// Some encapsulation functions & extras
	TriTreeNode *GetBaseLeft()  { return &m_BaseLeft; }
	TriTreeNode *GetBaseRight() { return &m_BaseRight; }
	char isDirty()     { return m_VarianceDirty; }
	int  isVisibile( ) { return m_isVisible; }
	void SetVisibility();

	// The static half of the Patch Class
	virtual void Init( int heightX, int heightY, int worldX, int worldY, unsigned char *hMap );
	virtual void Destroy();
	
	virtual void Reset();
	virtual void Tessellate();
	virtual void Render();
	virtual void ComputeVariance();
	virtual void ComputeBoundingBox();

	// The recursive half of the Patch Class
	virtual void			Split( TriTreeNode *tri);
	virtual void			RecursTessellate( TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY, int node );
	virtual void			RecursRender( TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY,
												WORD *ileft,WORD *iright,WORD *iapex);
	virtual unsigned char	RecursComputeVariance(	int leftX,  int leftY,  unsigned char leftZ,
													int rightX, int rightY, unsigned char rightZ,
													int apexX,  int apexY,  unsigned char apexZ,
													int node);
protected:
	rboundingbox m_BoundingBox;
	WORD itl,itr,idl,idr; 	

};

// How many TriTreeNodes should be allocated?
#define POOL_SIZE (50000)

// ------- 1024x1024 MAP -------
#define MAP_SIZE (1024)
#define NUM_PATCHES_PER_SIDE (16)

// Some more definitions
#define RROAM_PATCH_SIZE (MAP_SIZE/NUM_PATCHES_PER_SIDE)
#define RROAM_TEXTURE_SIZE (128)

//
// Landscape Class
// Holds all the information to render an entire landscape.
//
class Landscape
{
	friend Patch;
protected:
	static LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	static WORD m_nVertices;
	static LITVERTEX *m_pVertices;

	static LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	static int m_nIndicies;
	static WORD *m_pIndicies;

	unsigned char *m_HeightMap;										// HeightMap of the Landscape
	Patch m_Patches[NUM_PATCHES_PER_SIDE][NUM_PATCHES_PER_SIDE];	// Array of patches

	static int	m_NextTriNode;										// Index to next free TriTreeNode
	static TriTreeNode m_TriPool[POOL_SIZE];						// Pool of TriTree nodes for splitting

	static int GetNextTriNode() { return m_NextTriNode; }
	static void SetNextTriNode( int nNextNode ) { m_NextTriNode = nNextNode; }

public:
	static WORD PutVertex(float x,float y,float z);

	static TriTreeNode *AllocateTri();

	virtual void Init(unsigned char *hMap);
	virtual void Destroy();
	
	virtual void Reset();
	virtual void Tessellate();
	virtual void Render();
};

_NAMESPACE_REALSPACE2_END

#endif
