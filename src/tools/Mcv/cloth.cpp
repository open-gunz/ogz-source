//  [10/7/2003]
//////////////////////////////////////////////////////////////////////////
// CLOTH CPP
// 
// MAGICBELL
//
// OPTI : 파티클 간의 거리 계산에서 Length를 이용하는 것보다 Length Square를 이용하는 것이 더 빠를 것임
//
//////////////////////////////////////////////////////////////////////////

// INCLUDE
#include "stdafx.h"
#include "mdebug.h"
#include "RealSpace2.h"
#include "RMeshUtil.h"
//#include "RMesh.h"
#include "tbcylinder.h"
#include "Cloth.h"


// DEFINE
#define GRAVITY -9.18

// GLOBAL
//DEBUG
bool collision = true;


// 생성자/소면자
CCloth::CCloth()
:mX(0), mOldX(0), mForce(0), mConst(0), mfTimeStep(0), mpMesh(0), mHolds(0)
{
}


CCloth::~CCloth()
{
	SAFE_DELETE_ARRAY( mX );
	SAFE_DELETE_ARRAY( mOldX );
	SAFE_DELETE_ARRAY( mForce );
	SAFE_DELETE_ARRAY( mConst );
	SAFE_DELETE_ARRAY( mVertices );
	SAFE_DELETE_ARRAY( mHolds );
	SAFE_RELEASE( mVertexBuffer );

	if( mCylinders.size() != 0 )
	{
		for( int i = 0 ; i < mCylinders.size(); ++i )
		{
			//delete mCylinders[i];
		}
	}
	mCylinders.clear();
}


// XXX
// Test Code - 삭제 및 수정요구 사항
void CCloth::setColth( RealSpace2::RMeshNode* mesh_ )
{
	int i;
	rvector vecDistance;

	// vertex copy
	miNumParticles = mesh_->m_point_num;
	mX = new rvector[miNumParticles];
	mOldX = new rvector[miNumParticles];
	for( i = 0 ; i < miNumParticles; ++i )
	{
		mX[i] = mesh_->m_point_list[i];
		mOldX[i] = mesh_->m_point_list[i];
	}

	mForce = new rvector[miNumParticles];
	memset( mForce, 0, sizeof(rvector)* miNumParticles );

	mHolds = new bool[miNumParticles];
	memset( mHolds, false, sizeof(bool) * miNumParticles );

    miNumConst = mesh_->m_face_num * 3 ;
	mConst = new sConstraint[miNumConst];
	for( i = 0 ; i < mesh_->m_face_num; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			mConst[ i*3 + j ].refA = mesh_->m_face_list[i].m_point_index[j];
			if( j + 1 >= 3 )
			{
				mConst[ i*3 + j ].refB = mesh_->m_face_list[i].m_point_index[0];
			}
			else
			{
				mConst[ i*3 + j ].refB = mesh_->m_face_list[i].m_point_index[j+1];
			}
			vecDistance = mesh_->m_point_list[mConst[ i*3 + j ].refA] - mesh_->m_point_list[mConst[ i*3 + j ].refB];
			mConst[ i*3 + j ].restLength = D3DXVec3Length(&vecDistance);
		}
	}

	// Render Vertex setup
	miNumVertices = 3 * mesh_->m_face_num;
	mVertices = new RVertex[ miNumVertices ];

	// Vertex Buffer Setup
	if( FAILED(RealSpace2::RGetDevice()->CreateVertexBuffer( sizeof(RVertex)*miNumVertices, D3DUSAGE_WRITEONLY, RVertexType,  D3DPOOL_MANAGED, &mVertexBuffer )))
	{
		_ASSERT( !"Fail to Create vertex buffer" );
	}

	// Set Pointer
	mpMesh = mesh_;

	init();

	//mpVertexList = mX;

	//create( mpMesh->m_face_list, mpMesh->m_face_num, mpMesh->m_point_num );
}

// Initialize
void CCloth::init()
{
	// set Gravity
	mgForce.x = 0;
	mgForce.z = 0;
	mgForce.y = GRAVITY;

	// set Timestep (defualt)
	mfTimeStep = 0.1;

	// set Number of Iteration
	miNumIter = 3;

	// 일정 높이 이상의 점들은 모두 정지!!
	for( int i = 0 ; i < miNumParticles; ++i )
	{
		if( mX[i].y > -20 )
		{
			mHolds[i] = true;
		}
	}

	// 정지된 vertex가 아니면 힘을 적용시킨다...
	for( i = 0 ; i < miNumParticles; ++i )
	{
		if( !mHolds[i] )
		{
			mForce[i].x = 0;
			mForce[i].y = GRAVITY;
			mForce[i].z = 0;
		}
	}

}

// Update
void CCloth::Update()
{
	
	accumulateForces();
	varlet();
	satisfyConstraints();
	//*/
}

// AccumulateForces
void CCloth::accumulateForces()
{

}

// Varlet
void CCloth::varlet()
{
	rvector* swapTemp;

	for( int i = 0 ; i < miNumParticles; ++i )
	{
		if( mHolds[i] )
		{
			continue;
		}
		//mOldX[i] = 2 * mX[i] - mOldX[i] + mgForce * mfTimeStep * mfTimeStep;
		mOldX[i] = 2 * mX[i] - mOldX[i] + mForce[i] * mfTimeStep * mfTimeStep;
	}

	swapTemp = mX;
	mX = mOldX;
	mOldX = swapTemp;

//	mpVertexList = mX;
}

// Satisfy Constraints
void CCloth::satisfyConstraints()
{
	sConstraint* c;
	rvector* x1;
	rvector* x2;
	rvector delta;
	float deltaLegth;
	float diff;

	for( int i = 0 ; i < miNumIter; ++i )
	{
		
		// TODO : Do Collision Check Here
		if( collision )
		{
			for( int z = 0 ; z < mCylinders.size(); ++z )
			{
				for( int x = 0 ; x < miNumParticles; ++x )
				{
					if( mHolds[x] )
					{
						continue;
					}
					rvector newPos;
					if( mCylinders[z]->isCollide( mX[x], mpMesh->m_point_normal_list[x], newPos ) )
					{
						mX[x] = newPos;
						//mOldX[x] = newPos;
					}
				}
			}
		}

		for( int j = 0 ; j < miNumConst; ++j )
		{
			/*
			c = &mConst[j];
			x1 = &mX[ c->refA ];
			x2 = &mX[ c->refB ];
			delta = *x2 - *x1;
			deltaLegth = D3DXVec3Length( &delta );
			diff = (float) ( ( deltaLegth - c->restLength ) / deltaLegth );
			*x1 += delta * 0.5 * diff;
			*x2 -= delta * 0.5 * diff; 
			//*/
			float w1 = 0.5, w2 = 0.5;
			c = &mConst[j];
			x1 = &mX[ c->refA ];
			x2 = &mX[ c->refB ];

			if( mHolds[c->refA])
			{
				w1 = 0;
			}
			if( mHolds[c->refB])
			{
				w2 = 0;
			}

			if( w1 == 0 && w2 == 0 )
			{
				continue;
			}

			delta = *x2 - *x1;
			deltaLegth = D3DXVec3Length( &delta );
			diff = (float) ( ( deltaLegth - c->restLength ) / deltaLegth *  (w1 + w2) );
			*x1 += delta * 0.5 * diff;
			*x2 -= delta * 0.5 * diff; 
		}
		// TODO : 위치 고정
		
		for( int k = 0 ; k < miNumParticles; ++k )
		{
			if(mHolds[k])
			{
				mX[k] = mOldX[k];
			}
		}

		
	}
}

// Render
void CCloth::render()
{
	int i, mtr_i;
	LPDIRECT3DDEVICE8 dev =	RealSpace2::RGetDevice(); // Get Device Pointer

	RMtrlMgr* pMtrlMgr = &mpMesh->m_pParentMesh->m_mtrl_list_ex;
	RMtrl* pMtrl = pMtrlMgr->Get_s(mpMesh->m_mtrl_id,-1);
	int num_mtrl = pMtrl->m_sub_mtrl_num;

	RMtrl* psMtrl;

	/*
	for( int i = 0 ; i < mpMesh->m_face_num; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			mVertices[3*i +j].p = mX[mpMesh->m_face_list[i].m_point_index[j]];
		}
	}
	void* pVertices;
	mVertexBuffer->Lock(0, sizeof(RVertex)*miNumVertices, (BYTE**)&pVertices, 0 );

	memcpy( pVertices, mVertices, sizeof(RVertex)*miNumVertices);

	mVertexBuffer->Unlock();

	dev->SetTexture( 0, NULL );
	dev->SetStreamSource(0, mVertexBuffer, sizeof(RVertex) );
	dev->SetVertexShader( RVertexType );
	dev->SetRenderState( D3DRS_LIGHTING, false );
	dev->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	dev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, mpMesh->m_face_num );
	dev->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	//*/

	rmatrix identity;
	D3DXMatrixIdentity( &identity );

	dev->SetStreamSource(0, mVertexBuffer, sizeof(RVertex) );
	dev->SetVertexShader( RVertexType );
	//dev->SetRenderState( D3DRS_LIGHTING, TRUE );
	dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	dev->SetTransform( D3DTS_WORLD, &identity );

	int point_index;		// 현재 버텍스의 인덱스

	calcVertexNormal();

	if( 0 == num_mtrl )
	{
		for( i = 0 ; i < mpMesh->m_face_num ; ++i )
		{
			for( int j = 0 ; j < 3; ++j )
			{
				point_index = mpMesh->m_face_list[i].m_point_index[j];
				mVertices[3*i+j].p = mX[point_index];
				mVertices[3*i+j].tu = mpMesh->m_face_list[i].m_point_tex[j].x;
				mVertices[3*i+j].tv = mpMesh->m_face_list[i].m_point_tex[j].y;
				mVertices[3*i+j].n = mpMesh->m_point_normal_list[point_index];
				//mVertices[ 3*i + j ].n = getVertexNormal( point_index );
			}
		}

		// Copy Begin
		void *Buffer;
		if( FAILED( mVertexBuffer->Lock( 0, sizeof(RVertex) * mpMesh->m_face_num * 3, (BYTE**)&Buffer, 0 )))
		{
			_ASSERT( !"FAIL TO LOCK" );
		}

		memcpy( Buffer, mVertices, sizeof(RVertex) * mpMesh->m_face_num * 3 );

		mVertexBuffer->Unlock();
		// Copy End

		dev->SetTexture( 0, pMtrl->m_tex );
		dev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, mpMesh->m_face_num );
	}
	else
	{
		int face_mtrl_id;
		int vertex_index;
		for( mtr_i = 0 ; mtr_i < num_mtrl; ++mtr_i )
		{
			vertex_index = 0;

			if(pMtrl == NULL)
			{
				continue;
			}

			for( i = 0 ; i < mpMesh->m_face_num; ++i )
			{

				psMtrl = pMtrlMgr->Get_s( mpMesh->m_mtrl_id, i );

				face_mtrl_id = mpMesh->m_face_list[i].m_mtrl_id;
				if( face_mtrl_id >= num_mtrl )
				{
					face_mtrl_id -= num_mtrl;
				}
				if( mtr_i != face_mtrl_id )
				{
					continue;	// Not this face for this material
				}
				for( int j = 0 ; j < 3; ++j )
				{
					point_index = mpMesh->m_face_list[i].m_point_index[j];
					mVertices[vertex_index].p = mX[point_index];
					mVertices[vertex_index].tu = mpMesh->m_face_list[i].m_point_tex[j].x;
					mVertices[vertex_index].tv = mpMesh->m_face_list[i].m_point_tex[j].y;
					mVertices[vertex_index].n = mpMesh->m_point_normal_list[point_index];
					//mVertices[ 3*i + j ].n = getVertexNormal( point_index );

					++vertex_index;
				} // end vertex loof				
			}// end Face loof

			if( vertex_index == 0 )
				continue;

			// copy begin
			void *Buffer;
			if( FAILED( mVertexBuffer->Lock( 0, sizeof(RVertex) * vertex_index, (BYTE**)&Buffer, 0 )))
			{
				_ASSERT( !"FAIL TO LOCK" );
			}

			memcpy( Buffer, mVertices, sizeof(RVertex ) * vertex_index );

			mVertexBuffer->Unlock();

			dev->SetTexture( 0, psMtrl->m_tex );
			dev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, vertex_index / 3);
			// copy end
		}// end material loof
		//*/
	}
}

// set force
void CCloth::setForces( rvector& inc_ )
{
	for( int i = 0 ; i < miNumParticles; ++i )
	{
		if( !mHolds[i] )
		{
			mForce[i] += inc_;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update position
//////////////////////////////////////////////////////////////////////////

void CCloth::updatePosition( RealSpace2::RMeshNode* mesh_, rmatrix* pMatrix_ )
{
	static bool  updated = false;

	D3DXVECTOR4 vec;
	for( int i = 0; i < miNumParticles; ++i )
	{
		if((!mHolds[i]) && updated )
		{
			continue;
		}
		D3DXVec3Transform( &vec, &mesh_->m_point_list[i], pMatrix_ );
		mX[i].x = vec.x / vec.w;
		mX[i].y = vec.y / vec.w;
		mX[i].z = vec.z / vec.w;
		mOldX[i].x = vec.x / vec.w;
		mOldX[i].y = vec.y / vec.w;
		mOldX[i].z = vec.z / vec.w;
		
	}
	updated = true;
}

//////////////////////////////////////////////////////////////////////////
// calculate Vertex Normal
//////////////////////////////////////////////////////////////////////////

void CCloth::calcVertexNormal()
{
	D3DXPLANE	plane;
	D3DXVECTOR3 *vv[3];

	int i=0,j=0;

	if(mpMesh->m_face_num) {

		for(i=0;i<mpMesh->m_face_num;i++) {

			vv[0] = &mX[mpMesh->m_face_list[i].m_point_index[0]];
			vv[1] = &mX[mpMesh->m_face_list[i].m_point_index[1]];
			vv[2] = &mX[mpMesh->m_face_list[i].m_point_index[2]];

			D3DXPlaneFromPoints(&plane,vv[0],vv[1],vv[2]);
			D3DXPlaneNormalize(&plane,&plane);

			mpMesh->m_face_normal[i].x = plane.a;
			mpMesh->m_face_normal[i].y = plane.b;
			mpMesh->m_face_normal[i].z = plane.c;

		}
	}

	///////////////////////////////////////////////////

	static int p_cnt[1000];

	memset(p_cnt,0,sizeof(int)*1000);

	for(i=0;i<mpMesh->m_face_num;i++) {
		for(j=0;j<3;j++) {
			mpMesh->m_point_normal_list[ mpMesh->m_face_list[i].m_point_index[j] ] =
				mpMesh->m_point_normal_list[ mpMesh->m_face_list[i].m_point_index[j] ] + mpMesh->m_face_normal[i];

			++p_cnt[ mpMesh->m_face_list[i].m_point_index[j] ];
		}
	}

	for(i=0;i<mpMesh->m_point_num;i++) {
		mpMesh->m_point_normal_list[i] = mpMesh->m_point_normal_list[i] / (float)p_cnt[i];
		D3DXVec3Normalize(&mpMesh->m_point_normal_list[i],&mpMesh->m_point_normal_list[i]);
	}
}