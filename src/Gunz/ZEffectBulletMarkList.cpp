#include "stdafx.h"

#include "ZGame.h"

#include "ZEffectBillboard.h"
#include "ZEffectBulletMarkList.h"
#include "RealSpace2.h"
#include "ZMyCharacter.h"

#include <crtdbg.h>

//#define BULLETMARK_DISCARD_COUNT	1024
#define BULLETMARK_FLUSH_COUNT		256

ZEffectBulletMarkList::ZEffectBulletMarkList(void)
{
	m_fLifeTime=3.f;
	m_fVanishTime=1.f;
}

#define BULLETMARK_SCALE	5.0f

#define BULLETMART_CULL_DISTACNE_SQ_1 640000
#define BULLETMART_CULL_DISTACNE_SQ_2 250000

void ZEffectBulletMarkList::Add(const rvector &pos, const rvector &normal)
{
	// Early Culling
	auto vec = g_pGame->m_pMyCharacter->GetPosition() - pos;
	float fDistanceSq = MagnitudeSq(vec);
	int nLevel = GetEffectLevel();
	if( nLevel >= 1 && ( fDistanceSq > BULLETMART_CULL_DISTACNE_SQ_1 ) ) return;
	if( nLevel == 2 && ( fDistanceSq > BULLETMART_CULL_DISTACNE_SQ_2 ) ) return;

	ZEFFECTBULLETMARKITEM *pNewItem = new ZEFFECTBULLETMARKITEM;
	push_back(pNewItem);

	// Transform
	rmatrix matTranslation;
	rmatrix matWorld;

	rvector dir = normal;

	rvector up = rvector(0, 0, 1);

	float dot = DotProduct(dir,up);

	if(dot > 0.99f || dot < -0.99f)
		up = rvector(0,1,0);

	auto right = Normalized(CrossProduct(up, dir));
	up = Normalized(CrossProduct(right, dir));

	rmatrix mat;
	GetIdentityMatrix(mat);
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=up.x;mat._22=up.y;mat._23=up.z;
	mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

	matTranslation = TranslationMatrix(pos);

	matWorld = mat * matTranslation;


	static ZEFFECTCUSTOMVERTEX v[] = {
		{{-1, -1, 0}, 0xFFFFFFFF, 1, 0 },
		{{-1,  1, 0}, 0xFFFFFFFF, 1, 1},
		{{ 1,  1, 0}, 0xFFFFFFFF, 0, 1},
		{{ 1, -1, 0}, 0xFFFFFFFF, 0, 0},
	};
    
	for(int i=0;i<4;i++)
	{
		rvector worldv = BULLETMARK_SCALE * v[i].pos * matWorld;
		pNewItem->v[i].pos = worldv;
		pNewItem->v[i].tu = v[i].tu;
		pNewItem->v[i].tv = v[i].tv;
		pNewItem->v[i].color= 0xffffffff;
	}

	pNewItem->fElapsedTime=0;
}

void ZEffectBulletMarkList::Update(float fElapsed)
{
	for(iterator i=begin();i!=end();)
	{
		ZEFFECTBULLETMARKITEM *p = (ZEFFECTBULLETMARKITEM*)*i;
		p->fElapsedTime+=fElapsed;
		if( p->fElapsedTime > m_fLifeTime )
		{
			delete p;
			i=erase(i);
			continue;
		}

		float fVanish = min(1.f, max(0.f, (m_fLifeTime - p->fElapsedTime) / m_fVanishTime));
		u32 color = ((u32)(fVanish * 255))<<24 | 0xffffff;
		p->v[0].color = color;
		p->v[1].color = color;
		p->v[2].color = color;
		p->v[3].color = color;
		i++;
	}
}

void ZEffectBulletMarkList::BeginState()
{
	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	pDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	ZEffectBase::BeginState();
}

bool ZEffectBulletMarkList::Draw()
{
	if(!m_pVB) return false;

	if( size()==0 ) return true;

	BeginState();

	HRESULT hr;

	auto RemainNum = size();

	iterator itr = begin();
	
	while (RemainNum)
	{
		if(m_dwBase >= EFFECTBASE_DISCARD_COUNT)
			m_dwBase = 0;

		DWORD dwThisNum = min(RemainNum, static_cast<u32>(BULLETMARK_FLUSH_COUNT));

		dwThisNum = min(dwThisNum, EFFECTBASE_DISCARD_COUNT - m_dwBase);

		BYTE *pVertices;
		if( FAILED( hr = m_pVB->Lock( m_dwBase * sizeof(ZEFFECTCUSTOMVERTEX) * 4, dwThisNum * sizeof(ZEFFECTCUSTOMVERTEX) * 4,
			(VOID**)&pVertices, m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
		{
			return false;
		}

		BYTE *pInd;
		if( FAILED( hr = m_pIB->Lock( m_dwBase * sizeof(WORD) * 6, dwThisNum * sizeof(WORD) * 6,
			(VOID**)&pInd, m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
		{
			return false;
		}
		
		for(DWORD j=0;j<dwThisNum;j++)
		{
			ZEFFECTBULLETMARKITEM *p=(ZEFFECTBULLETMARKITEM*)*itr;
			
			_ASSERT(p != 0);
			if(p == 0) mlog("Fatal Error : ZEffectBulletMarkList : Draw() %d\n", __LINE__ );

			memcpy(pVertices,p->v,sizeof(ZEFFECTCUSTOMVERTEX)*4);
			pVertices+=sizeof(ZEFFECTCUSTOMVERTEX)*4;

			WORD inds[] = { 0,1,2,0,2,3 };
			for(int k=0;k<6;k++)
			{
				inds[k]+=(m_dwBase+j)*4;
			}
			memcpy(pInd,inds,sizeof(inds));
			pInd+=sizeof(inds);

			itr++;
		}

		m_pVB->Unlock();
		m_pIB->Unlock();

		if(FAILED( hr = RGetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_dwBase*4,dwThisNum*4,m_dwBase*6,dwThisNum*2) ))
			return false;

		m_dwBase+=dwThisNum;
		RemainNum-=dwThisNum;

	}

	RGetDevice()->SetStreamSource( 0, NULL , 0, 0 );	
	RGetDevice()->SetIndices(NULL);

	return true;
}