#include "stdafx.h"
/*

	2001-11, Turner의 쏘스를 기반으로 만들어졌음.

	참고해야할 문서.

	Mark Duchaineau, LLNL, Murray Wolinsky, LANL, David E. Sigeti, LANL, 
	Mark C. Miller, LLNL, Charles Aldrich, LANL, Mark B. Mineev-Weinstein; 
		ROAMing Terrain: Real-time Optimally Adapting Meshes
	http://www.llnl.gov/graphics/ROAM/roam.pdf 

	Turner, Bryan; 
		Real-Time Dynamic Level of Detail Terrain Rendering with ROAM
	http://www.gamasutra.com/features/20000403/turner_01.htm

	Yordan Gyurchev; 
		ROAM Implementation Optimizations 
	http://www.flipcode.com/tutorials/tut_roamopt.shtml
*/

#include <math.h>
#include "MDebug.h"
#include "RealSpace2.h"
#include "RRoam.h"

_NAMESPACE_REALSPACE2_BEGIN

#define FOV pi/2
#define INDEXMAX	16384
#define VERTEXMAX	16384

// Beginning frame varience (should be high, it will adjust automatically)
int gDesiredTris = 3000;
float gFrameVariance = 50;
int gNumTrisRendered;
float gClipAngle;

#define USEVB

// -------------------------------------------------------------------------------------------------
//	PATCH CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Split a single Triangle and link it into the mesh.
// Will correctly force-split diamonds.
//
void Patch::Split(TriTreeNode *tri)
{
	// We are already split, no need to do it again.
	if (tri->LeftChild)
		return;

	// If this triangle is not in a proper diamond, force split our base neighbor
	if ( tri->BaseNeighbor && (tri->BaseNeighbor->BaseNeighbor != tri) )
		Split(tri->BaseNeighbor);

	// Create children and link into mesh
	tri->LeftChild  = Landscape::AllocateTri();
	tri->RightChild = Landscape::AllocateTri();

	// If creation failed, just exit.
	if ( !tri->LeftChild )
		return;

	// Fill in the information we can get from the parent (neighbor pointers)
	tri->LeftChild->BaseNeighbor  = tri->LeftNeighbor;
	tri->LeftChild->LeftNeighbor  = tri->RightChild;

	tri->RightChild->BaseNeighbor  = tri->RightNeighbor;
	tri->RightChild->RightNeighbor = tri->LeftChild;

	// Link our Left Neighbor to the new children
	if (tri->LeftNeighbor != NULL)
	{
		if (tri->LeftNeighbor->BaseNeighbor == tri)
			tri->LeftNeighbor->BaseNeighbor = tri->LeftChild;
		else if (tri->LeftNeighbor->LeftNeighbor == tri)
			tri->LeftNeighbor->LeftNeighbor = tri->LeftChild;
		else if (tri->LeftNeighbor->RightNeighbor == tri)
			tri->LeftNeighbor->RightNeighbor = tri->LeftChild;
		else
			;// Illegal Left Neighbor!
	}

	// Link our Right Neighbor to the new children
	if (tri->RightNeighbor != NULL)
	{
		if (tri->RightNeighbor->BaseNeighbor == tri)
			tri->RightNeighbor->BaseNeighbor = tri->RightChild;
		else if (tri->RightNeighbor->RightNeighbor == tri)
			tri->RightNeighbor->RightNeighbor = tri->RightChild;
		else if (tri->RightNeighbor->LeftNeighbor == tri)
			tri->RightNeighbor->LeftNeighbor = tri->RightChild;
		else
			;// Illegal Right Neighbor!
	}

	// Link our Base Neighbor to the new children
	if (tri->BaseNeighbor != NULL)
	{
		if ( tri->BaseNeighbor->LeftChild )
		{
			tri->BaseNeighbor->LeftChild->RightNeighbor = tri->RightChild;
			tri->BaseNeighbor->RightChild->LeftNeighbor = tri->LeftChild;
			tri->LeftChild->RightNeighbor = tri->BaseNeighbor->RightChild;
			tri->RightChild->LeftNeighbor = tri->BaseNeighbor->LeftChild;
		}
		else
			Split( tri->BaseNeighbor);  // Base Neighbor (in a diamond with us) was not split yet, so do that now.
	}
	else
	{
		// An edge triangle, trivial case.
		tri->LeftChild->RightNeighbor = NULL;
		tri->RightChild->LeftNeighbor = NULL;
	}
}

// ---------------------------------------------------------------------
// Tessellate a Patch.
// Will continue to split until the variance metric is met.
//
void Patch::RecursTessellate( TriTreeNode *tri,
							 int leftX,  int leftY,
							 int rightX, int rightY,
							 int apexX,  int apexY,
							 int node )
{
#define SQR(x) ((x) * (x))

	float TriVariance;
	int centerX = (leftX + rightX)>>1; // Compute X coordinate of center of Hypotenuse
	int centerY = (leftY + rightY)>>1; // Compute Y coord...

	if ( node < (1<<VARIANCE_DEPTH) )
	{
		rvector cameraposition=RCameraPosition;
		// Extremely slow distance metric (sqrt is used).
		// Replace this with a faster one!
		float distance = 1.0f + sqrtf( SQR((float)centerX - cameraposition.x) +
									   SQR((float)centerY - cameraposition.y) +
									   SQR((float)m_HeightMap[(centerY*MAP_SIZE)+centerX] - cameraposition.z));
		
		// Egads!  A division too?  What's this world coming to!
		// This should also be replaced with a faster operation.
		TriVariance = ((float)m_CurrentVariance[node] * MAP_SIZE * 2)/distance;	// Take both distance and variance into consideration
	}

	if ( (node >= (1<<VARIANCE_DEPTH)) ||	// IF we do not have variance info for this node, then we must have gotten here by splitting, so continue down to the lowest level.
		 (TriVariance > gFrameVariance))	// OR if we are not below the variance tree, test for variance.
	{
		Split(tri);														// Split this triangle.
		
		if (tri->LeftChild &&											// If this triangle was split, try to split it's children as well.
			((abs(leftX - rightX) >= 3) || (abs(leftY - rightY) >= 3)))	// Tessellate all the way down to one vertex per height field entry
		{
			RecursTessellate( tri->LeftChild,   apexX,  apexY, leftX, leftY, centerX, centerY,    node<<1  );
			RecursTessellate( tri->RightChild, rightX, rightY, apexX, apexY, centerX, centerY, 1+(node<<1) );
		}
	}
}

// ---------------------------------------------------------------------
// Render the tree.  Simple no-fan method.
//
void Patch::RecursRender( TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY,
							WORD *ileft,WORD *iright,WORD *iapex)
{
	if ( tri->LeftChild )					// All non-leaf nodes have both children, so just check for one
	{
		int centerX = (leftX + rightX)>>1;	// Compute X coordinate of center of Hypotenuse
		int centerY = (leftY + rightY)>>1;	// Compute Y coord...

		RecursRender( tri->LeftChild,  apexX,   apexY, leftX, leftY, centerX, centerY ,
						iapex,ileft,&(tri->index));
		RecursRender( tri->RightChild, rightX, rightY, apexX, apexY, centerX, centerY ,
						iright,iapex,&(tri->index));
	}
	else									// A leaf node!  Output a triangle to be rendered.
	{
		// Actual number of rendered triangles...
		
		gNumTrisRendered++;

		float leftZ  = m_HeightMap[(leftY *MAP_SIZE)+leftX ];
		float rightZ = m_HeightMap[(rightY*MAP_SIZE)+rightX];
		float apexZ  = m_HeightMap[(apexY *MAP_SIZE)+apexX ];

#ifdef USEVB
		if (*ileft==0xffff) *ileft=Landscape::PutVertex((float)leftX,(float)leftY,(float)leftZ);
		if (*iright==0xffff) *iright=Landscape::PutVertex((float)rightX,(float)rightY,(float)rightZ);
		if (*iapex==0xffff) *iapex=Landscape::PutVertex((float)apexX,(float)apexY,(float)apexZ);
		
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*ileft;
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*iright;
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*iright;
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*iapex;
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*iapex;
		Landscape::m_pIndicies[Landscape::m_nIndicies++]=*ileft;

		
		if(Landscape::m_nIndicies>=2048)
		{
	Landscape::m_pVertexBuffer->Unlock();
	Landscape::m_pIndexBuffer->Unlock();
	RGetDevice()->SetFVF(RSLFVF);
	RGetDevice()->SetStreamSource(0,Landscape::m_pVertexBuffer,0,sizeof(LITVERTEX));
	RGetDevice()->SetIndices(Landscape::m_pIndexBuffer);
			HRESULT hr=RGetDevice()->DrawIndexedPrimitive(D3DPT_LINELIST,0,0,Landscape::m_nVertices,0,Landscape::m_nIndicies/2);
			MASSERT(hr==D3D_OK);
			Landscape::m_nIndicies=0;
	Landscape::m_pVertexBuffer->Lock(NULL,VERTEXMAX*sizeof(LITVERTEX),(VOID**)&Landscape::m_pVertices,NULL);
	Landscape::m_pIndexBuffer->Lock(NULL,INDEXMAX*sizeof(WORD),(VOID**)&Landscape::m_pIndicies,D3DLOCK_DISCARD);
		}

		MASSERT(Landscape::m_nIndicies<INDEXMAX);
#else
		LITVERTEX verts[4];
		verts[0].x=leftX;verts[0].y=leftY;verts[0].z=leftZ;
		verts[1].x=rightX;verts[1].y=rightY;verts[1].z=rightZ;
		verts[2].x=apexX;verts[2].y=apexY;verts[2].z=apexZ;
		memcpy(verts+3,verts,sizeof(LITVERTEX));
		verts[0].Diffuse=verts[3].Diffuse=0xff0000;
		verts[1].Diffuse=0xff00;
		verts[2].Diffuse=0xff;

		RGetDevice()->SetFVF(RSLFVF);
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,3,verts,sizeof(LITVERTEX));
#endif

	}
}

// ---------------------------------------------------------------------
// Computes Variance over the entire tree.  Does not examine node relationships.
//
unsigned char Patch::RecursComputeVariance( int leftX,  int leftY,  unsigned char leftZ,
										    int rightX, int rightY, unsigned char rightZ,
											int apexX,  int apexY,  unsigned char apexZ,
											int node)
{
	//        /|\
	//      /  |  \
	//    /    |    \
	//  /      |      \
	//  ~~~~~~~*~~~~~~~  <-- Compute the X and Y coordinates of '*'
	//
	int centerX = (leftX + rightX) >>1;		// Compute X coordinate of center of Hypotenuse
	int centerY = (leftY + rightY) >>1;		// Compute Y coord...
	unsigned char myVariance;

	// Get the height value at the middle of the Hypotenuse
	unsigned char centerZ  = m_HeightMap[(centerY * MAP_SIZE) + centerX];

	// Variance of this triangle is the actual height at it's hypotenuse midpoint minus the interpolated height.
	// Use values passed on the stack instead of re-accessing the Height Field.
	myVariance = abs((int)centerZ - (((int)leftZ + (int)rightZ)>>1));

	// Since we're after speed and not perfect representations,
	//    only calculate variance down to an 8x8 block
	if ( (abs(leftX - rightX) >= 8) ||
		 (abs(leftY - rightY) >= 8) )
	{
		// Final Variance for this node is the max of it's own variance and that of it's children.
		myVariance = max( myVariance, RecursComputeVariance( apexX,   apexY,  apexZ, leftX, leftY, leftZ, centerX, centerY, centerZ,    node<<1 ) );
		myVariance = max( myVariance, RecursComputeVariance( rightX, rightY, rightZ, apexX, apexY, apexZ, centerX, centerY, centerZ, 1+(node<<1)) );
	}

	// Store the final variance for this node.  Note Variance is never zero.
	if (node < (1<<VARIANCE_DEPTH))
		m_CurrentVariance[node] = 1 + myVariance;

	return myVariance;
}

// -------------------------------------------------------------------------------------------------
//	PATCH CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Initialize a patch.
//
void Patch::Init( int heightX, int heightY, int worldX, int worldY, unsigned char *hMap )
{
	// Clear all the relationships
	m_BaseLeft.RightNeighbor = m_BaseLeft.LeftNeighbor = m_BaseRight.RightNeighbor = m_BaseRight.LeftNeighbor =
	m_BaseLeft.LeftChild = m_BaseLeft.RightChild = m_BaseRight.LeftChild = m_BaseLeft.LeftChild = NULL;

	// Attach the two m_Base triangles together
	m_BaseLeft.BaseNeighbor = &m_BaseRight;
	m_BaseRight.BaseNeighbor = &m_BaseLeft;

	// Store Patch offsets for the world and heightmap.
	m_WorldX = worldX;
	m_WorldY = worldY;

	// Store pointer to first byte of the height data for this patch.
	m_HeightMap = &hMap[heightY * MAP_SIZE + heightX];

	// Initialize flags
	m_VarianceDirty = 1;
	m_isVisible = 0;
}

void Patch::Destroy()
{
}

// ---------------------------------------------------------------------
// Reset the patch.
//
void Patch::Reset()
{
	// Assume patch is not visible.
	m_isVisible = 0;

	// Reset the important relationships
	m_BaseLeft.LeftChild = m_BaseLeft.RightChild = m_BaseRight.LeftChild = m_BaseLeft.LeftChild = NULL;

	// Attach the two m_Base triangles together
	m_BaseLeft.BaseNeighbor = &m_BaseRight;
	m_BaseRight.BaseNeighbor = &m_BaseLeft;

	// Clear the other relationships.
	m_BaseLeft.RightNeighbor = m_BaseLeft.LeftNeighbor = m_BaseRight.RightNeighbor = m_BaseRight.LeftNeighbor = NULL;

	itl=itr=idl=idr=0xffff;
	m_BaseLeft.index=m_BaseRight.index=0xffff;
}

// ---------------------------------------------------------------------
// Compute the variance tree for each of the Binary Triangles in this patch.
//
void Patch::ComputeVariance()
{
	// Compute variance on each of the base triangles...

	m_CurrentVariance = m_VarianceLeft;
	RecursComputeVariance(	0,          RROAM_PATCH_SIZE, m_HeightMap[RROAM_PATCH_SIZE * MAP_SIZE],
							RROAM_PATCH_SIZE, 0,          m_HeightMap[RROAM_PATCH_SIZE],
							0,          0,          m_HeightMap[0],
							1);

	m_CurrentVariance = m_VarianceRight;
	RecursComputeVariance(	RROAM_PATCH_SIZE, 0,          m_HeightMap[ RROAM_PATCH_SIZE],
							0,          RROAM_PATCH_SIZE, m_HeightMap[ RROAM_PATCH_SIZE * MAP_SIZE],
							RROAM_PATCH_SIZE, RROAM_PATCH_SIZE, m_HeightMap[(RROAM_PATCH_SIZE * MAP_SIZE) + RROAM_PATCH_SIZE],
							1);

	// Clear the dirty flag for this patch
	m_VarianceDirty = 0;
}

void Patch::ComputeBoundingBox()
{
	m_BoundingBox.minx = (float)m_WorldX; m_BoundingBox.maxx = (float)m_WorldX + RROAM_PATCH_SIZE;
	m_BoundingBox.miny = (float)m_WorldY; m_BoundingBox.maxy = (float)m_WorldY + RROAM_PATCH_SIZE;
	m_BoundingBox.minz = 255; m_BoundingBox.maxz = 0;
	
	int i,j;
	for (i = 0; i < RROAM_PATCH_SIZE; i++)
	{
		for (j = 0; j < RROAM_PATCH_SIZE; j++)
		{
			m_BoundingBox.minz = min(m_BoundingBox.minz, float(m_HeightMap[(i* MAP_SIZE) + j]));
			m_BoundingBox.maxz = max(m_BoundingBox.maxz, float(m_HeightMap[(i* MAP_SIZE) + j]));
		}
	}
}

// ---------------------------------------------------------------------
// Discover the orientation of a triangle's points:
//
// Taken from "Programming Principles in Computer Graphics", L. Ammeraal (Wiley)
//
inline int orientation( int pX, int pY, int qX, int qY, int rX, int rY )
{
	int aX, aY, bX, bY;
	float d;

	aX = qX - pX;
	aY = qY - pY;

	bX = rX - pX;
	bY = rY - pY;

	d = (float)aX * (float)bY - (float)aY * (float)bX;
	return (d < 0) ? (-1) : (d > 0);
}

// ---------------------------------------------------------------------
// Set patch's visibility flag.
//
void Patch::SetVisibility()
{
	m_isVisible = isInViewFrustum(m_BoundingBox, RGetViewFrustum());
//	mlog("%d\n",m_isVisible);
//	m_isVisible=1;
}

// ---------------------------------------------------------------------
// Create an approximate mesh.
//
void Patch::Tessellate()
{
	// Split each of the base triangles
	m_CurrentVariance = m_VarianceLeft;
	RecursTessellate (	&m_BaseLeft,
						m_WorldX,				m_WorldY+RROAM_PATCH_SIZE,
						m_WorldX+RROAM_PATCH_SIZE,	m_WorldY,
						m_WorldX,				m_WorldY,
						1 );
					
	m_CurrentVariance = m_VarianceRight;
	RecursTessellate(	&m_BaseRight,
						m_WorldX+RROAM_PATCH_SIZE,	m_WorldY,
						m_WorldX,				m_WorldY+RROAM_PATCH_SIZE,
						m_WorldX+RROAM_PATCH_SIZE,	m_WorldY+RROAM_PATCH_SIZE,
						1 );
}

// ---------------------------------------------------------------------
// Render the mesh.
//
void Patch::Render()
{
	RSetTransform(D3DTS_WORLD,
		unmove(TranslationMatrix({static_cast<float>(m_WorldX), static_cast<float>(m_WorldY), 0 })));

#ifdef USEVB
	Landscape::m_nIndicies=0;
	Landscape::m_pIndexBuffer->Lock(NULL,INDEXMAX*sizeof(WORD),(VOID**)&Landscape::m_pIndicies,D3DLOCK_DISCARD);
	Landscape::m_pVertexBuffer->Lock(NULL,VERTEXMAX*sizeof(LITVERTEX),(VOID**)&Landscape::m_pVertices,NULL);
#endif

		RecursRender (	&m_BaseLeft,
			0,				RROAM_PATCH_SIZE,
			RROAM_PATCH_SIZE,		0,
			0,				0,
			&idl,&itr,&itl);
		
		RecursRender(	&m_BaseRight,
			RROAM_PATCH_SIZE,		0,
			0,				RROAM_PATCH_SIZE,
			RROAM_PATCH_SIZE,		RROAM_PATCH_SIZE,
			&itr,&idl,&idr);
	
#ifdef USEVB
	Landscape::m_pVertexBuffer->Unlock();
	Landscape::m_pIndexBuffer->Unlock();

	if(Landscape::m_nIndicies)
	{
		RGetDevice()->SetFVF(RSLFVF);
		RGetDevice()->SetStreamSource(0,Landscape::m_pVertexBuffer, 0,sizeof(LITVERTEX));
		RGetDevice()->SetIndices(Landscape::m_pIndexBuffer);
		HRESULT hr=RGetDevice()->DrawIndexedPrimitive(D3DPT_LINELIST,0,0,Landscape::m_nVertices,0,Landscape::m_nIndicies/2);
		MASSERT(hr==D3D_OK);
	}
#endif

}

// -------------------------------------------------------------------------------------------------
//	LANDSCAPE CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Definition of the static member variables
//
int         Landscape::m_NextTriNode;
TriTreeNode Landscape::m_TriPool[POOL_SIZE];
LPDIRECT3DVERTEXBUFFER9 Landscape::m_pVertexBuffer = NULL;
WORD Landscape::m_nVertices=0;
LITVERTEX *Landscape::m_pVertices;

LPDIRECT3DINDEXBUFFER9 Landscape::m_pIndexBuffer=NULL;
int Landscape::m_nIndicies=0;
WORD *Landscape::m_pIndicies=0;

WORD Landscape::PutVertex(float x,float y,float z)
{
	m_pVertices[m_nVertices].x=x;
	m_pVertices[m_nVertices].y=y;
	m_pVertices[m_nVertices].z=z;
	m_pVertices[m_nVertices].Diffuse=0xffffff;
	_ASSERT(m_nVertices<VERTEXMAX);
	return m_nVertices++;
}

// ---------------------------------------------------------------------
// Allocate a TriTreeNode from the pool.
//
TriTreeNode *Landscape::AllocateTri()
{
	TriTreeNode *pTri;

	// IF we've run out of TriTreeNodes, just return NULL (this is handled gracefully)
	if ( m_NextTriNode >= POOL_SIZE )
	{
		MASSERT(false);
		return NULL;
	}

	pTri = &(m_TriPool[m_NextTriNode++]);
	pTri->LeftChild = pTri->RightChild = NULL;
	pTri->index=0xffff;

	return pTri;
}

// ---------------------------------------------------------------------
// Initialize all patches
//
void Landscape::Init(unsigned char *hMap)
{
	D3DCAPS9 caps; 
	RGetDevice()->GetDeviceCaps(&caps);
	int nMaxCount=caps.MaxPrimitiveCount;


	Patch *patch;
	int X, Y;

	// Store the Height Field array
	m_HeightMap = hMap;

	// Initialize all terrain patches
	for ( Y=0; Y < NUM_PATCHES_PER_SIDE; Y++)
		for ( X=0; X < NUM_PATCHES_PER_SIDE; X++ )
		{
			patch = &(m_Patches[Y][X]);
			patch->Init( X*RROAM_PATCH_SIZE, Y*RROAM_PATCH_SIZE, X*RROAM_PATCH_SIZE, Y*RROAM_PATCH_SIZE, hMap );
			patch->ComputeVariance();
			patch->ComputeBoundingBox();
		}
#ifdef USEVB
	HRESULT hr=RGetDevice()->CreateVertexBuffer(sizeof(LITVERTEX)*VERTEXMAX,D3DUSAGE_DYNAMIC,RSLFVF,D3DPOOL_DEFAULT,&m_pVertexBuffer,NULL);
	MASSERT(hr==D3D_OK);

	hr=RGetDevice()->CreateIndexBuffer(sizeof(WORD)*INDEXMAX,D3DUSAGE_DYNAMIC,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pIndexBuffer,NULL);
	MASSERT(hr==D3D_OK);
#endif
}

// ---------------------------------------------------------------------
// Reset all patches, recompute variance if needed
//
void Landscape::Reset()
{
	int X, Y;
	Patch *patch;

	// Set the next free triangle pointer back to the beginning
	SetNextTriNode(0);

	// Reset rendered triangle count.
	gNumTrisRendered = 0;

	// Go through the patches performing resets, compute variances, and linking.
	for ( Y=0; Y < NUM_PATCHES_PER_SIDE; Y++ )
		for ( X=0; X < NUM_PATCHES_PER_SIDE; X++)
//	for ( Y=0; Y < 1; Y++ )
//		for ( X=0; X < 1; X++)
		{
			patch = &(m_Patches[Y][X]);
			
			// Reset the patch
			patch->Reset();
			patch->SetVisibility();
			
			// Check to see if this patch has been deformed since last frame.
			// If so, recompute the varience tree for it.
			if ( patch->isDirty() )
				patch->ComputeVariance();

			if ( patch->isVisibile() )
			{
				// Link all the patches together.
				if ( X > 0 )
					patch->GetBaseLeft()->LeftNeighbor = m_Patches[Y][X-1].GetBaseRight();
				else
					patch->GetBaseLeft()->LeftNeighbor = NULL;		// Link to bordering Landscape here..

				if ( X < (NUM_PATCHES_PER_SIDE-1) )
					patch->GetBaseRight()->LeftNeighbor = m_Patches[Y][X+1].GetBaseLeft();
				else
					patch->GetBaseRight()->LeftNeighbor = NULL;		// Link to bordering Landscape here..

				if ( Y > 0 )
					patch->GetBaseLeft()->RightNeighbor = m_Patches[Y-1][X].GetBaseRight();
				else
					patch->GetBaseLeft()->RightNeighbor = NULL;		// Link to bordering Landscape here..

				if ( Y < (NUM_PATCHES_PER_SIDE-1) )
					patch->GetBaseRight()->RightNeighbor = m_Patches[Y+1][X].GetBaseLeft();
				else
					patch->GetBaseRight()->RightNeighbor = NULL;	// Link to bordering Landscape here..
			}
		}

	m_nVertices=0;
}

// ---------------------------------------------------------------------
// Create an approximate mesh of the landscape.
//
void Landscape::Tessellate()
{
	// Perform Tessellation
	int nCount;
	Patch *patch = &(m_Patches[0][0]);
	for (nCount=0; nCount < NUM_PATCHES_PER_SIDE*NUM_PATCHES_PER_SIDE; nCount++, patch++ )
//	for (nCount=0; nCount < 1; nCount++, patch++ )
	{
		if (patch->isVisibile())
			patch->Tessellate( );
	}
}

// ---------------------------------------------------------------------
// Render each patch of the landscape & adjust the frame variance.
//
void Landscape::Render()
{
	int nCount;
	Patch *patch = &(m_Patches[0][0]);

	// Scale the terrain by the terrain scale specified at compile time.
//	glScalef( 1.0f, MULT_SCALE, 1.0f );

	for (nCount=0; nCount < NUM_PATCHES_PER_SIDE*NUM_PATCHES_PER_SIDE; nCount++, patch++ )
//	for (nCount=0; nCount < 1; nCount++, patch++ )
	{
		if (patch->isVisibile())
			patch->Render();
	}

	/*
	// Check to see if we got close to the desired number of triangles.
	// Adjust the frame variance to a better value.
	if ( GetNextTriNode() != gDesiredTris )
		gFrameVariance += ((float)GetNextTriNode() - (float)gDesiredTris) / (float)gDesiredTris;

#define LIMITVARIANCE 0.5f
	// Bounds checking.
	if ( gFrameVariance < LIMITVARIANCE )
		gFrameVariance = LIMITVARIANCE;
*/
}

void Landscape::Destroy()
{
	int nCount;
	Patch *patch = &(m_Patches[0][0]);
	for (nCount=0; nCount < NUM_PATCHES_PER_SIDE*NUM_PATCHES_PER_SIDE; nCount++, patch++ )
		patch->Destroy();
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}

_NAMESPACE_REALSPACE2_END
