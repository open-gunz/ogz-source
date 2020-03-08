#pragma once

//  [10/7/2003]
//////////////////////////////////////////////////////////////////////////
// Temporary Class - to Test Cloth Animation
//
// magicbell
//////////////////////////////////////////////////////////////////////////

#include "rmesh.h"
#include "realspace2.h"
#include "rmeshutil.h"

#define RIGHT_LEG 0x01
#define	LEFT_LEG 0x02

class TBCylinder
{
public:
	TBCylinder(void);
	~TBCylinder(void);

private:
	int miNumVertex;
	int* vertexIndex;
	
	int miNumRenderVertices;
	RVertex* mVertices;
	LPDIRECT3DVERTEXBUFFER8 mVertexBuffer;
	LPDIRECT3DINDEXBUFFER8 mIndexBuffer;		

public:
	rvector base_centre;
	float height;
	float radius;
	RealSpace2::RMeshNode* mpMesh;

private:

public:
	// 어떤 vertex들을 이용해서 실린더를 구성할 것인지를 결정
	bool setBCylinder( RealSpace2::RMeshNode * pMeshNode_, DWORD flag_ ); 
	// update
	void update( rvector* point_list_ );
	// check collistion
	// 만약 충돌 아니면 리턴 false
	// 대략 충돌이면 리턴 true 하고 조정되야할 위치 pos_ 반환
	bool isCollide( rvector& v_, rvector& n_, rvector& pos_ );

	void vertex_setup();
	void render(rmatrix* pWorld_);
};
	