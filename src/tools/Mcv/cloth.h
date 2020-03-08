#pragma once

// [10/7/2003]
//////////////////////////////////////////////////////////////////////////
// CLOTH HEADER
// VALET INTERPOLATION 알고리즘을 이용한 CLOTH ANIMATION
//
// MAGICBELL
//////////////////////////////////////////////////////////////////////////

// INCLUDE
#include "RMesh.h"
//#include "RHalfEdgeMesh.h"
// DEFINE

#include "vector"

using namespace std;

typedef struct constraint
{
	int refA, refB;
	float restLength;
}
sConstraint;


struct rvector;
struct RVertex;
class TBCylinder;

typedef vector<TBCylinder*> Cylinder;


class CCloth //: RealSpace2::RHalfEdgeMesh
{
public:
	CCloth();
	~CCloth();

private:

	// Rendering Primitives
	int miNumVertices;
	RVertex* mVertices;
	LPDIRECT3DVERTEXBUFFER8 mVertexBuffer;

	// number of partices
	int miNumParticles;
	// Current Position
	rvector* mX;
	// Last Position
	rvector* mOldX;
	// Force ( include Gravity )
	rvector* mForce;
	// or Use only one Force ( Gravity ) - apply same force to all particles
	rvector mgForce;
	// Constraints
	int miNumConst;
	sConstraint* mConst;
	// Time Steps
	float	mfTimeStep;
	// Number of Iteration - more iteration is more accurate and slow
	int miNumIter;

	// Hold Particles
	bool* mHolds;
	
	// Collistion Object List
	// 원규씨과 상의하여 결정.. 현재 모든 파티클은 모든 컬리젼 오브젝트와 충돌검사한다
	// 속도 향상을 위해 part별로 대상 오브젝트를 구별할 수 있도록 할수 있다
	//sRCObj* mDestObject;

public:
	// point to Mesh Node - to Render!!
	RealSpace2::RMeshNode *mpMesh;	
	// TODO : 테스트 실린더들...
	Cylinder mCylinders;

private:
	// 힘 계산 - 중력 이외에 쓰이지 않므면 같으면 없애 버림.. 현재 고려되는 상황 없음
	void accumulateForces();
	// Interpolation bet. current frame and last frame
	void varlet();
	// satisfy constraints - 충돌 및 제약 조건을 만족시킨다
	void satisfyConstraints();
	//xxx calc vertex normal
	void calcVertexNormal();

public:
	// initialize - set Gravity.. 
	void init();
	// XXX Test 용 코드.. 메쉬 노드를 받아 cloth를 생성한다
	// automate constraints -> 반드시 수동으로 바꿔야 한다
	void setColth( RealSpace2::RMeshNode* mesh_ );
	// update - call every frame
	void Update();
	// update position - vertex들의 자체적인 움직임
	void updatePosition( RealSpace2::RMeshNode* mesh_, rmatrix* pMatrix_ );
	// render
	void render();

	//XXX to Test Force relative things 
	void setForces( rvector& inc_ );
};