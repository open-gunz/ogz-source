#include "stdafx.h"

#include "ZGame.h"
#include "ZClothEmblem.h"
#include "RealSpace2.h"
#include "RLightList.h"
#include "RDynamicLight.h"
#include "MDebug.h"
#include "RBspObject.h"

#define Gravity							-7
#define MAX_NUM_CLOTH_PARTICLE			165
#define LIGHT_DISTANCE					100
#define RESERVED_SPACE					10

static RVertex					g_Cloth_Buffer[MAX_NUM_CLOTH_PARTICLE*3];
static LPDIRECT3DVERTEXBUFFER9	g_hw_Buffer			= 0;
unsigned int			ZClothEmblem::msRef = 0;

static bool bHardwareBuffer = true;

struct testv
{
	rvector n;
	DWORD c;
};

#define testvFVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)

void	ZClothEmblem::CreateFromMeshNode( RMeshNode* pMeshNdoe_ , ZWorld* pWorld)
{
	int			i, nIndices;
	rvector		vecDistance;

	mpMeshNode	= pMeshNdoe_;

	m_nCntP	= mpMeshNode->m_point_num;
	m_nCntC	= mpMeshNode->m_face_num * 3 ;

	// Initialize
	m_pX		= new rvector[m_nCntP];
	m_pOldX		= new rvector[m_nCntP];
	m_pForce	= new rvector[m_nCntP];
	m_pHolds	= new int    [m_nCntP];
	m_pWeights	= new float  [m_nCntP];
	m_pNormal	= new rvector[m_nCntP];

	m_pConst	= new sConstraint[m_nCntC];

	memset( m_pForce    , 0, sizeof(rvector)* m_nCntP );
	memset( m_pHolds    , 0, sizeof(bool)   * m_nCntP );
	memset( m_pWeights  , 0, sizeof(float)  * m_nCntP );

	nIndices = pMeshNdoe_->m_face_num*3;

	rmatrix World;
	// I don't know how this nonsense works but somehow it does '__'
	rvector Pos{ mpMeshNode->m_mat_base(0, 0), mpMeshNode->m_mat_base(0, 1), mpMeshNode->m_mat_base(0, 2) };
	rvector Dir = rvector(0,-1,0);
	rvector Up  = rvector(0,0,1);

	MakeWorldMatrix(&World, Pos, Dir, Up);

	mWorldMat = mpMeshNode->m_mat_result * World;

	D3DXVECTOR4 rVec;
	for( i = 0 ; i < mpMeshNode->m_point_num; ++i )
	{
		mpMeshNode->m_point_list[i] = Transform(mpMeshNode->m_point_list[i], mWorldMat);
	}

	//	Copy Vertex
	memcpy( m_pX, mpMeshNode->m_point_list, sizeof(rvector) * m_nCntP );
	memcpy( m_pOldX, mpMeshNode->m_point_list, sizeof(rvector) * m_nCntP );

	//	Build Constraints
	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			m_pConst[ i*3 + j ].refA = mpMeshNode->m_face_list[i].m_point_index[j];
			if( j + 1 >= 3 )
			{
				m_pConst[ i*3 + j ].refB = mpMeshNode->m_face_list[i].m_point_index[0];
			}
			else
			{
				m_pConst[ i*3 + j ].refB = mpMeshNode->m_face_list[i].m_point_index[j+1];
			}
			vecDistance = mpMeshNode->m_point_list[m_pConst[ i*3 + j ].refA] - mpMeshNode->m_point_list[m_pConst[ i*3 + j ].refB];
			m_pConst[ i*3 + j ].restLength = Magnitude(vecDistance);
		}
	}

	_ASSERT( mpMeshNode->m_point_color_num );

	for( i = 0 ; i < m_nCntP; ++i )
	{
		m_pHolds[i]	= CLOTH_VALET_ONLY;
		// Exclusive with other Attribute...
		if( mpMeshNode->m_point_color_list[i].x != 0 )
		{
			m_pHolds[i]	= CLOTH_HOLD;
		}
		else 
		{
			{
				m_pHolds[i] |= CLOTH_FORCE;
			}

			if( mpMeshNode->m_point_color_list[i].y != 0 )
			{
				m_pHolds[i] |= CLOTH_COLLISION;
			}
		}
	}

	m_pWorld = pWorld;

	float	minDistance = 999999;
	float	fTemp;
	RLIGHT	*pSelectedLight = nullptr;
	
	for(auto& Light : m_pWorld->GetBsp()->GetObjectLightList())
	{
		auto vec = Light.Position - m_pX[0];
		fTemp	= Magnitude(vec);
		if( fTemp < minDistance )
		{
			minDistance		= fTemp;
			pSelectedLight	= &Light;
		}
	}

	mpLight	= new D3DLIGHT9;
	memset( mpLight, 0, sizeof(D3DLIGHT9));

	mpLight->Ambient.r = 0.3;
	mpLight->Ambient.g = 0.3;
	mpLight->Ambient.b = 0.3;

	if( pSelectedLight!=0 &&  minDistance < pSelectedLight->fAttnEnd	)
	{	
		mpLight->Type		= D3DLIGHT_POINT;

		mpLight->Diffuse.r	= pSelectedLight->Color.x * pSelectedLight->fIntensity;
		mpLight->Diffuse.g	= pSelectedLight->Color.y * pSelectedLight->fIntensity;
		mpLight->Diffuse.b	= pSelectedLight->Color.z * pSelectedLight->fIntensity;

		mpLight->Position	= pSelectedLight->Position;

		mpLight->Range		= pSelectedLight->fAttnEnd;
		mpLight->Attenuation1	= 0.0001f;
	}
	else
	{
		mpLight->Type		= D3DLIGHT_DIRECTIONAL;
		
		mpLight->Diffuse.r	= 0.1f;
		mpLight->Diffuse.g	= 0.1f;
		mpLight->Diffuse.b	= 0.1f;

		mpLight->Direction	= rvector( 1, 1, 1 );
	
		mpLight->Attenuation1	= 0.0f;
		mpLight->Attenuation0	= 0.0f;	

		mpLight->Range		= 0.0f;
	}

	//PreCalculate AABB
	float	mostSmallX, mostSmallY, mostSmallZ;
	float	mostBigX, mostBigY, mostBigZ;

	mostSmallX	= mostSmallY	= mostSmallZ	= 9999999;
	mostBigX	= mostBigY		= mostBigZ		= -9999999;

	rvector* pCurr;
	for( i = 0 ; i < m_nCntP; ++i )
	{
		pCurr	= &mpMeshNode->m_point_list[i];
		
		mostSmallX	= min( mostSmallX, pCurr->x );
		mostSmallY	= min( mostSmallY, pCurr->y );
		mostSmallZ	= min( mostSmallZ, pCurr->z );

		mostBigX	= max( mostBigX, pCurr->x );
		mostBigY	= max( mostBigY, pCurr->y );
		mostBigZ	= max( mostBigZ, pCurr->z );
	}

	mAABB.vmin = rvector( mostSmallX - RESERVED_SPACE, mostSmallY - RESERVED_SPACE, mostSmallZ - RESERVED_SPACE );
	mAABB.vmax = rvector( mostBigX + RESERVED_SPACE, mostBigY + RESERVED_SPACE, mostBigZ + RESERVED_SPACE );

	mMyTime	= 0;
}

//////////////////////////////////////////////////////////////////////////
//	setOption
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::setOption( int nIter_, float power_, float inertia_ )
{
	m_nCntIter = nIter_;
	m_fTimeStep = power_;
	m_AccelationRatio = inertia_;
}

//////////////////////////////////////////////////////////////////////////
//	update
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::update()
{
	if (!mbIsInFrustrum)
		return;

	DWORD currTime = GetGlobalTimeMS();
	if ( mMyTime - currTime < 33 )
	{
		return;
	}
	mMyTime = GetGlobalTimeMS();

 	accumulateForces();
	varlet();
	memset( m_pForce, 0, sizeof(rvector)*m_nCntP );
	if(mpWind!=NULL) 
	{
		mpWind->x = 0.f;
		mpWind->y = 0.f;
		mpWind->z = 0.f;
	}
	satisfyConstraints();
	mWndGenerator.Update( GetGlobalTimeMS() );
}

//////////////////////////////////////////////////////////////////////////
//	accumulateForces
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::accumulateForces()
{
	if(	mpWind != NULL )
		*mpWind += mWndGenerator.GetWind();
}

//////////////////////////////////////////////////////////////////////////
//	varlet
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::varlet()
{
	for( int i = 0 ; i < m_nCntP; ++i )
	{
		if( m_pHolds[i] & CLOTH_HOLD )
		{
			continue;
		}
		else
		{
			rvector force;
			if( m_pHolds[i] & CLOTH_FORCE && mpWind != NULL )
			{
				force	= m_pForce[i] + (*mpWind );
				force.z = Gravity;
			}
			else
			{
				force = m_pForce[i];
				force.z		+= Gravity;
			}
			m_pOldX[i]	= m_pX[i] + m_AccelationRatio * ( m_pX[i] - m_pOldX[i] ) + force * m_fTimeStep * m_fTimeStep; 
		}
	}	

	rvector* swapTemp;

	swapTemp	= m_pX;
	m_pX		= m_pOldX;
	m_pOldX		= swapTemp;
}

//////////////////////////////////////////////////////////////////////////
//	satisfyConstraints
//////////////////////////////////////////////////////////////////////////
void ZClothEmblem::satisfyConstraints()
{
	sConstraint*	c;
	rvector*		x1;
	rvector*		x2;
	rvector			delta;
	float			deltaLegth;
	float			diff;
	int				i, j;

	for (i = 0; i < m_nCntIter; ++i)
	{
		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter*	pCharacter = (*itor).second;

			if (!isInViewFrustum(pCharacter->m_Position, CHARACTER_RADIUS + 10, RGetViewFrustum()))
			{
				continue;
			}

			if (pCharacter->IsDead() && pCharacter->m_bBlastDrop && !pCharacter->IsVisible())
			{
				continue;
			}

			for (j = 0; j < m_nCntP; ++j)
			{
				rvector	pos = pCharacter->m_Position;
				rvector myPos = m_pX[j];

				if (pos.z + 190 < myPos.z || pos.z > myPos.z)
				{
					continue;
				}

				pos.z = 0;
				myPos.z = 0;

				rvector dir = myPos - pos;

				float lengthsq = MagnitudeSq(dir);
				if (lengthsq > CHARACTER_RADIUS*CHARACTER_RADIUS)
				{
					continue;
				}

				Normalize(dir);

				myPos = pos + dir * (CHARACTER_RADIUS);
				m_pX[j].x = myPos.x;
				m_pX[j].y = myPos.y;
			}
		}
	}


	// Restriction
	for (std::list<sRestriction*>::iterator itor = mRestrictionList.begin();
	itor != mRestrictionList.end(); ++itor)
	{
		for (int j = 0; j < m_nCntP; ++j)
		{
			float* p = (float*)&m_pX[j];
			sRestriction* r = *itor;

			p += (int)r->axis;
			if (r->compare == COMPARE_GREATER)
			{
				if (*p > r->position)
				{
					*p = r->position;
				}
			}
			else
			{
				if (*p < r->position - 3)
				{
					*p = r->position;
				}
			}
		}

	}



	// Relaxation
	for (j = 0; j < m_nCntC; ++j)
	{
		c = &m_pConst[j];

		x1 = &m_pX[c->refA];
		x2 = &m_pX[c->refB];

		delta = *x2 - *x1;
		deltaLegth = Magnitude(delta);
		diff = (float)((deltaLegth - c->restLength) / (deltaLegth));

		*x1 += delta * diff * 0.5;
		*x2 -= delta * diff * 0.5;
	}

	for (i = 0; i < m_nCntP; ++i)
	{
		if (m_pHolds[i] & CLOTH_HOLD)
		{
			m_pX[i] = m_pOldX[i];
		}
	}
}

void ZClothEmblem::render()
{
	if (!isInViewFrustum(mAABB, RGetViewFrustum()) ||
		!m_pWorld->GetBsp()->IsVisible(mAABB)) {
		mbIsInFrustrum = false;
		return;
	}

	mbIsInFrustrum = true;

	int		i, index;

	UpdateNormal();

	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			index	= mpMeshNode->m_face_list[i].m_point_index[j];
			g_Cloth_Buffer[3*i+j].p	= m_pX[index];
			g_Cloth_Buffer[3*i+j].n	= m_pNormal[index];
			g_Cloth_Buffer[3*i+j].tu = mpMeshNode->m_face_list[i].m_point_tex[j].x;
			g_Cloth_Buffer[3*i+j].tv = mpMeshNode->m_face_list[i].m_point_tex[j].y;
		}
	}

	D3DMATERIAL9	mtrl;
	mtrl.Ambient.r	= 1.f;	mtrl.Ambient.g	= 1.f;	mtrl.Ambient.b	= 1.f;	mtrl.Ambient.a	= 0.f;	
	mtrl.Diffuse.r	= 1.0f;	mtrl.Diffuse.g	= 1.0f;	mtrl.Diffuse.b	= 1.0f;	mtrl.Diffuse.a	= 1.f;
	mtrl.Specular.r	= 0.f;	mtrl.Specular.g	= 0.f;	mtrl.Specular.b	= 0.f;	mtrl.Specular.a	= 0.f;
	mtrl.Emissive.r	= 0.f;	mtrl.Emissive.g	= 0.f;	mtrl.Emissive.b	= 0.f;	mtrl.Emissive.a	= 0.f;
	mtrl.Power	= 0.0f;
	RGetDevice()->SetMaterial( &mtrl );

	RMtrlMgr* pMtrlMgr	= &mpMeshNode->m_pParentMesh->m_mtrl_list_ex;
	RMtrl* pMtrl		= pMtrlMgr->Get_s(mpMeshNode->m_mtrl_id,-1);
	RGetDevice()->SetTexture( 0, pMtrl->GetTexture() );
	RGetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

	RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00555555 );
	RGetShaderMgr()->setAmbient( 0x00555555 );

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RGetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	RGetDevice()->SetLight( 0, mpLight );
	RGetDynamicLightManager()->SetLight( m_pX[0], 1, LIGHT_DISTANCE );

	RSetTransform(D3DTS_WORLD, IdentityMatrix());

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
 	RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	RGetDevice()->LightEnable( 0, TRUE );
	RGetDevice()->LightEnable( 1, FALSE );

	RGetDevice()->SetFVF( RVertexType );
	
	if( bHardwareBuffer )
	{
		VOID* pVertex;
		if( FAILED( g_hw_Buffer->Lock( 0,  mpMeshNode->m_point_num * 3 * sizeof(RVertex), (VOID**)&pVertex, D3DLOCK_DISCARD )))
		{
			mlog(" Fail to Lock Emblem hw buffer.. Check Buffer Size.. \n" );
			bHardwareBuffer = false;
			return;
		}
		memcpy( pVertex, g_Cloth_Buffer, mpMeshNode->m_face_num*3 * sizeof(RVertex) );
		if( FAILED( g_hw_Buffer->Unlock() ))
		{
			mlog(" Fail to unLock Emblem hw buffer.. Check Buffer Size.. \n" );
			bHardwareBuffer = false;
			return;
		}

		RGetDevice()->SetStreamSource( 0, g_hw_Buffer, 0, sizeof(RVertex) );
		RGetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST,0,mpMeshNode->m_face_num);
	}
	else
	{
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mpMeshNode->m_face_num, g_Cloth_Buffer, sizeof(RVertex) );
	}	

	RGetDevice()->LightEnable( 0, FALSE );
	RGetDevice()->LightEnable( 1, FALSE );
}

void ZClothEmblem::UpdateNormal()
{
	int	i, j, index, indexTemp[3];
	rvector Point[3];

	memset( m_pNormal, 0, sizeof(rvector)*m_nCntP );
	memset( indexTemp, 0, sizeof(int)*3 );

	for( i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
		for( j = 0 ; j < 3; ++j )
		{
			index		= mpMeshNode->m_face_list[i].m_point_index[j];
			if( index < 0 || index >= m_nCntP )
			{
				_ASSERT(FALSE);
				// TODO: Figure out wtf this even means
				mlog("Index of Particle is not profit to calculate...\n");
				continue;
			}
			Point[j]			= m_pX[index];
			indexTemp[j]= index;
		}
		rvector n;
		CrossProduct(&n, Point[2] - Point[0], Point[2] - Point[1]);
		Normalize(n);

		for( j = 0 ; j < 3; ++j )
		{
			m_pNormal[indexTemp[j]] += n;
		}
	}

	for( i = 0 ; i < m_nCntP; ++i )
	{
		Normalize(m_pNormal[i]);
	}
}

void ZClothEmblem::setExplosion( rvector& pos_, float power_ )
{
	rvector	dir		= m_pX[0] - pos_;
	float lengthsq = MagnitudeSq(dir);
	if( lengthsq	 > 250000 )
	{
		return;
	}

	Normalize(dir);
	*mpWind	+= dir * power_ / sqrt(lengthsq) * 10;
}

void ZClothEmblem::CheckSpearing( rvector& bullet_begin_, rvector& bullet_end_, float power_ )
{	
	if (!mbIsInFrustrum)
		return;

	if (!isInViewFrustum(mAABB, RGetViewFrustum()))
		mbIsInFrustrum = false;

	rvector dir = Normalized(bullet_end_ - bullet_begin_);

	// test line vs AABB
	if (!IntersectLineSegmentAABB(bullet_begin_, dir, mAABB))
		return;

	// line vs triangle test and determine which particle get power
	int index[3], sIndex = -1;
	rvector uvt;
	rvector* v[3];

	for( int i = 0 ; i < mpMeshNode->m_face_num; ++i )
	{
        for( int j = 0 ; j < 3; ++j )
		{
			index[j]= mpMeshNode->m_face_list[i].m_point_index[j];
			v[j]	= &m_pX[index[j]];
		}

		if (IntersectTriangle(*v[0], *v[1], *v[2], bullet_begin_, dir, nullptr, &uvt.x, &uvt.y))
		{
			if( uvt.x + uvt.y < 0.66 )
			{
				sIndex	= index[2];
			}
			else if( uvt.x > uvt.y )
			{
				sIndex	= index[0];
			}
			else
			{
				sIndex	= index[1];
			}

			m_pForce[sIndex]	+= dir * power_;
			break;
		}
	}	
}

void ZClothEmblem::OnInvalidate()
{
	SAFE_RELEASE( g_hw_Buffer );
}

void ZClothEmblem::OnRestore()
{
	if( g_hw_Buffer == 0 )
	{
		if( FAILED( RGetDevice()->CreateVertexBuffer( MAX_NUM_CLOTH_PARTICLE* 3 * sizeof( RVertex ), 
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, RVertexType, D3DPOOL_DEFAULT, &g_hw_Buffer ,NULL) ))
		{
			mlog( "Fail to Restore Vertex Buffer for Emblems..\n" );
			bHardwareBuffer = false;
		}
	}
}

ZClothEmblem::ZClothEmblem(void)
{
	setOption(1, 0.15f,1.0f);
	mpWind	= new rvector(0,0,0);
	mpLight	= 0;
	mpMeshNode	= 0;
	mpTex				= 0;
	mBaseWind	= rvector(0,1,0);
	mfBaseMaxPower	= 0;
	m_fDist = 0.f;
	if(msRef==0) OnRestore();
	++msRef;
	mbIsInFrustrum = true;
}

ZClothEmblem::~ZClothEmblem(void)
{
	SAFE_DELETE(mpWind);
	SAFE_DELETE(mpLight);
	for( list<sRestriction*>::iterator iter = mRestrictionList.begin() ; iter != mRestrictionList.end(); )
	{
		sRestriction* p = *iter;
		SAFE_DELETE(p);
		iter = mRestrictionList.erase(iter);
	}
	--msRef;
	if(msRef==0) OnInvalidate();
}

bool e_clothemblem_sort_float(ZClothEmblem* _a,ZClothEmblem* _b) {
	if( _a->m_fDist > _b->m_fDist )
		return true;
	return false;
}

void ZEmblemList::Draw()
{
	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;
	rvector t_pos;

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;

		if( pCurr == NULL) continue;

		t_pos = GetTransPos(pCurr->mWorldMat);

		t_vec = camera_pos - t_pos;
		pCurr->m_fDist = Magnitude(t_vec);
	}

	sort(e_clothemblem_sort_float);

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		if(pCurr != 0)	pCurr->render();
	}
}

ZClothEmblem* ZEmblemList::Get( int i_ )
{
	if( i_ >= (int)size() )
	{
		return NULL;
	}
	iterator iter = begin();
	for( int i = 0 ; i < i_; ++i )
	{
		++iter;
	}
	return *iter;
}

void ZEmblemList::Update()
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->update();
	}
}

ZEmblemList::~ZEmblemList()
{
	OnInvalidate();
	Clear();
}

void ZEmblemList::Clear()
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		SAFE_DELETE( *iter );
	}
	clear();
}

void ZEmblemList::SetExplosion(  rvector& pos_, float power_ )
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->setExplosion( pos_, power_ );
	}
}

void ZEmblemList::CheckSpearing( rvector& bullet_begine_, rvector& bullet_end_, float power_ )
{
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZClothEmblem* pCurr	= *iter;
		pCurr->CheckSpearing( bullet_begine_, bullet_end_, power_ );
	}
}

void ZEmblemList::OnInvalidate()
{
	if( ZClothEmblem::GetRefCount() != 0 )
	{
		ZClothEmblem* p = *(begin());
		if(p != 0 ) p->OnInvalidate();
	}
}

void ZEmblemList::OnRestore()
{
	if( ZClothEmblem::GetRefCount() != 0 )
	{
		ZClothEmblem* p = *(begin());
		if(p != 0 ) p->OnRestore();
	}
}

void ZEmblemList::InitEnv( char* pFileName_ )
{
	MXmlDocument Data;
	if (!Data.LoadFromFile(pFileName_, g_pFileSystem))
	{
		MLog("Failed to initialize emblem list environment\n");
		return;
	}

	MXmlElement root, child;
	char TagName[256];
	char Attribute[256];
	root = Data.GetDocumentElement();
	int iCount = root.GetChildNodeCount();	

	for( int i = 0 ; i < iCount; ++i )
	{
		child		= root.GetChildNode(i);
		child.GetTagName( TagName );
		if( TagName[0] == '#' )
		{
			continue;
		}
		child.GetAttribute( Attribute, "NAME" );
		mEmblemMapItor	= mEmblemMap.find( Attribute );
		if( mEmblemMapItor	!= mEmblemMap.end() )
		{
			ZClothEmblem* p		= mEmblemMapItor->second;
			
			if( child.GetAttribute( Attribute, "DIRECTION" ))
			{
				rmatrix RotMat;
				rvector dir = rvector( 0,1,0 );
				int theta;
				sscanf_s( Attribute, "%d", &theta );
				auto up = rvector(0, 0, 1);
				RotMat = RotationMatrix(up, ((float)theta*PI_FLOAT/180) );
				dir = dir*RotMat;
				p->GetWndGenerator()->SetWindDirection( dir );
			}

			if( child.GetAttribute( Attribute, "POWER" ))
			{
				float power;
				sscanf_s( Attribute, "%f", &power );
				p->GetWndGenerator()->SetWindPower( power );
			}
			
			MXmlElement dummy;
			int iDummyNum = child.GetChildNodeCount();
			for( int j = 0 ; j < iDummyNum; ++j )
			{
				dummy = child.GetChildNode( j );
				dummy.GetTagName( TagName );
				if( TagName[0] == '#' )
				{
					continue;
				}
				if( _stricmp( TagName, "RESTRICTION" ) == 0 )
				{
					sRestriction* rest = new sRestriction;
					int iValue = 0;
					float fValue = 0.f;
					if( dummy.GetAttribute( Attribute, "AXIS" ))
					{
						sscanf_s( Attribute, "%d", &iValue );
						rest->axis	=(RESTRICTION_AXIS)iValue;
					}				
					if( dummy.GetAttribute( Attribute, "POSITION") )
					{
						sscanf_s( Attribute, "%f", &fValue );
						rest->position = fValue;
					}
					if( dummy.GetAttribute(Attribute, "COMPARE") )
					{
						sscanf_s( Attribute, "%d", &iValue );
						rest->compare =(RESTRICTION_COMPARE)iValue;
					}
					p->AddRestriction( rest );
				}
				else if( _stricmp( TagName, "WINDTYPE" ) == 0 )
				{
					int iValue = 0;
					if( dummy.GetAttribute( Attribute, "TYPE" ) )
					{
						sscanf_s( Attribute, "%d", &iValue );
						p->GetWndGenerator()->SetWindType( (WIND_TYPE) iValue );
					}
					if( dummy.GetAttribute( Attribute, "DELAY" ))
					{
						sscanf_s( Attribute, "%d", &iValue );
						p->GetWndGenerator()->SetDelayTime( iValue );
					}
				}
			}
		}
	}

	for( list<ZClothEmblem*>::iterator iter = begin(); iter != end(); ++iter )
	{
		for( int i = 0 ; i < 100; ++i )
			(*iter)->update();
	}
}

void ZEmblemList::Add( ZClothEmblem* p_, const char* pName_ )
{
	push_back( p_ );
	mEmblemMap.insert( map<string, ZClothEmblem*>::value_type( pName_, p_ ) );
}