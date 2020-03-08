#include "stdafx.h"

#include "ZGame.h"

#include "ZEffectBillboard.h"
#include "ZEffectBillboardList.h"
#include "RealSpace2.h"

#include <crtdbg.h>

/*
	TODO: 
	
	1. z sort가 전혀 고려되어있지 않다. update등에 sort를 넣어볼만도 하다,

	2. billboard 의  rotation 도 파라미터로 들어갔으면 하는 소망.

	3. 로켓 연기 같은 경우에는 opacity 의 감쇄가 선형대신 급격하게 
		일어나면 (1/x처럼) 연기가 빨려가는듯한 느낌을 없앨수 있을듯.

 */

#define BILLBOARD_FLUSH_COUNT		128

#define BULLETMARK_SCALE	5.0f

//////////////////////////////////////////////////////////////////////////////////////////////

ZEffectShadowList::ZEffectShadowList()
{
	
}

ZEFFECTSHADOWITEM* ZEffectShadowList::Add(rmatrix& m,DWORD _color)
{
	ZEFFECTSHADOWITEM* pNewItem = new ZEFFECTSHADOWITEM;

	push_back(pNewItem);

	pNewItem->dwColor = _color;
	pNewItem->worldmat = m;	

	return pNewItem;
}

void ZEffectShadowList::BeginState()
{
	LPDIRECT3DDEVICE9 pDev = RGetDevice();

	pDev->SetRenderState(D3DRS_LIGHTING, FALSE );

	pDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	pDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	pDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

	pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
	pDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
//	pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );

	pDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	RSetWBuffer(true);

	ZEffectBase::BeginState();
}

void ZEffectShadowList::EndState()
{
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void ZEffectShadowList::Update(float fElapsed)
{

}

bool ZEffectShadowList::Draw()
{
	if(!m_pVB) return false;

	if( empty() ) return true;

	BeginState();

	HRESULT		hr;

	DWORD	dwRemainNum = (DWORD)size();

	iterator itr=begin();

	while(dwRemainNum)
	{
		if(m_dwBase >= EFFECTBASE_DISCARD_COUNT)
			m_dwBase = 0;

		DWORD dwThisNum = min( dwRemainNum , static_cast<DWORD>(BILLBOARD_FLUSH_COUNT) );

		dwThisNum = min( dwThisNum , EFFECTBASE_DISCARD_COUNT - m_dwBase );	

		BYTE* pVertices;

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
			ZEFFECTSHADOWITEM *p = (ZEFFECTSHADOWITEM*)*itr;

			static ZEFFECTCUSTOMVERTEX v[] = {
				{{-0.5f, -0.5f, 0.f}, 0xFFFFFFFF, 0.f, 0.f },
				{{ 0.5f, -0.5f, 0.f}, 0xFFFFFFFF, 1.f, 0.f },
				{{-0.5f,  0.5f, 0.f}, 0xFFFFFFFF, 0.f, 1.f},
				{{ 0.5f,  0.5f, 0.f}, 0xFFFFFFFF, 1.f, 1.f},
			};

			static rvector sv[4] = { 
				rvector(-0.5f,-0.5f , 0.f) , 
				rvector( 0.5f,-0.5f , 0.f) , 
				rvector(-0.5f, 0.5f , 0.f) , 
				rvector( 0.5f, 0.5f , 0.f) ,
			};

			for (size_t i{}; i < 4; ++i)
				v[i].pos = TransformCoord(sv[i], p->worldmat);

			v[0].color = v[1].color = v[2].color = v[3].color = p->dwColor;

			memcpy(pVertices, v, sizeof(ZEFFECTCUSTOMVERTEX) * 4);

			pVertices+=sizeof(ZEFFECTCUSTOMVERTEX)*4;

			WORD inds[] = { 0,1,2,2,1,3 };

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
		dwRemainNum-=dwThisNum;

	}

	RGetDevice()->SetStreamSource( 0, NULL , 0,0 );	
	RGetDevice()->SetIndices(NULL);

	EndState();

	Clear();

	return true;	
}

////////////////////////////////////////////////////////////////////////////////////

ZEffectBillboardList::ZEffectBillboardList()
{
	m_bUseRocketSmokeColor = false;
}

ZEFFECTBILLBOARDITEM * ZEffectBillboardList::Add(const rvector &pos, const rvector &velocity, const rvector &accel,float fStartSize,float fEndSize,float fLifeTime,DWORD color,bool bTrainSmoke)
{
	ZEFFECTBILLBOARDITEM *pNewItem = new ZEFFECTBILLBOARDITEM;

	push_back(pNewItem);

	pNewItem->fElapsedTime=0;
	pNewItem->velocity=velocity;
	pNewItem->normal=-RCameraDirection;
	pNewItem->up=rvector(0,0,1);
	pNewItem->accel=accel;
	pNewItem->position=pos;
	pNewItem->fStartSize=fStartSize;
	pNewItem->fEndSize=fEndSize;
	pNewItem->fCurScale = fStartSize;
	pNewItem->fOpacity=1.f;
	pNewItem->dwColor = color;
	pNewItem->bUseTrainSmoke = bTrainSmoke;
	pNewItem->bUseSteamSmoke = false;
	pNewItem->nDir = -1;
	
	if(fLifeTime==-1)
		pNewItem->fLifeTime = m_fLifeTime;
	else
		pNewItem->fLifeTime=fLifeTime;

	return pNewItem;
}

#define _D3DRGBA(r, g, b, a) \
	(   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
	|   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
	)

float ZEffectBillboardList::GetLifeRatio(ZEFFECTBILLBOARDITEM *p)// 0 - 1
{
	return min( (p->fElapsedTime / p->fLifeTime), 1.f );
}

void ZEffectBillboardList::Update(float fElapsed)
{
	for(iterator i=begin();i!=end();)
	{
		ZEFFECTBILLBOARDITEM *p = (ZEFFECTBILLBOARDITEM*)*i;
		p->fElapsedTime+=fElapsed;

		if( p->fElapsedTime > p->fLifeTime)
		{
			delete p;
			i=erase(i);
			continue;
		}

		p->normal=-RCameraDirection;
		p->fOpacity = min(1.f, max(0.f, (p->fLifeTime - p->fElapsedTime) / m_fVanishTime));

		if( p->bUseSteamSmoke ) {

			float fRatio = GetLifeRatio(p);
			
			float fLen = Magnitude(p->accel);

			if( fLen < 1.f ) {
				p->velocity+=fElapsed*p->accel2;
				p->position+=fElapsed*p->velocity;
			}
			else {
				p->velocity+=fElapsed * (p->accel2*(1.f-fRatio) + p->accel*fRatio);
				p->position+=fElapsed*p->velocity;
			}
		}
		else if( p->bUseTrainSmoke ) {
			
			float fRatio = GetLifeRatio(p);

			if(fRatio < 0.1f) {
				p->velocity += fElapsed * rvector(0, 0, 10);
				p->position += fElapsed * p->velocity ;
			}
			else if(fRatio > 0.6f) {

				if(p->nDir == -1) {// 퍼질 어느한 방향을 잡아주기
					p->nDir = rand()%4;
				}

				rvector rand_vec3 = rvector(0,0,0);

					 if(p->nDir==0) { rand_vec3.y =  100.f; }
				else if(p->nDir==1) { rand_vec3.x =  100.f; }
				else if(p->nDir==2) { rand_vec3.y = -100.f; }
				else if(p->nDir==3) { rand_vec3.x = -100.f; }

				p->velocity += fElapsed * p->accel ;
				p->position += fElapsed * (p->velocity + rand_vec3);
			}
			else {
				p->velocity+=fElapsed*p->accel;
				p->position+=fElapsed*p->velocity;
			}
		}
		else {
			p->velocity+=fElapsed*p->accel;
			p->position+=fElapsed*p->velocity;
		}

		if( m_bUseRocketSmokeColor ) {

			float as = (p->fElapsedTime / p->fLifeTime);
			float sas = 0.f;

			rvector color1;
			rvector color2;
			rvector col;

			if(as < 0.05f) {

				sas = as / 0.05f;

				color1 = rvector(0.7f, 0.5f, 0.4f);
				color2 = rvector(0.6f, 0.5f, 0.4f);
				col = Lerp(color1, color2, sas);

				p->dwColor = _D3DRGBA(col.x,col.y,col.z ,0.f);
			}
			else if(as < 0.1f)	{

				sas = (as-0.05f) / 0.05f;

				color1=rvector(0.6f,0.5f,0.4f);
				color2=rvector(0.55f,0.55f,0.45f);
				col = Lerp(color1, color2, sas);

				p->dwColor = _D3DRGBA(col.x,col.y,col.z ,0.f);
			}
			else if(as < 0.15f){
				sas = (as-0.1f) / 0.05f;

				color1=rvector(0.55f,0.55f,0.45f);
				color2=rvector(0.5f,0.5f,0.5f);
				col = Lerp(color1, color2, sas);

				p->dwColor = _D3DRGBA(col.x,col.y,col.z ,0.f);
			}
			else {
				col=rvector(0.5f,0.5f,0.5f);//회색
				p->dwColor = _D3DRGBA(col.x,col.y,col.z ,0.f);
			}
		}
		i++;
	}
}

void ZEffectBillboardList::BeginState()
{
	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	pDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE);

	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	RSetWBuffer(true);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	ZEffectBase::BeginState();
}

bool ZEffectBillboardList::Draw()
{
	if(!m_pVB) return false;

	if( size()==0 ) return true;

	BeginState();

	HRESULT		hr;

	DWORD		dwRemainNum = (DWORD)size();

	iterator itr=begin();

	while(dwRemainNum)
	{
		if(m_dwBase >= EFFECTBASE_DISCARD_COUNT)
			m_dwBase = 0;

		DWORD dwThisNum = min( dwRemainNum , static_cast<DWORD>(BILLBOARD_FLUSH_COUNT) );

		dwThisNum = min( dwThisNum , EFFECTBASE_DISCARD_COUNT - m_dwBase );	


		BYTE		*pVertices;
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
			ZEFFECTBILLBOARDITEM *p = (ZEFFECTBILLBOARDITEM*)*itr;

			// Transform
			rmatrix matTranslation;
			rmatrix matScaling;
			rmatrix matWorld;

			rvector dir = p->normal;

			rvector up=p->up;
			rvector right;

			if(IS_EQ(dir.z,1.f)) up=rvector(1,0,0);

			right = Normalized(CrossProduct(up, dir));
			up = Normalized(CrossProduct(right, dir));

			rmatrix mat;
			GetIdentityMatrix(mat);
			mat._11=right.x;mat._12=right.y;mat._13=right.z;
			mat._21=up.x;mat._22=up.y;mat._23=up.z;
			mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

			rvector pos=p->position;

			float fInt = min(1.f, max(0.f, (p->fLifeTime - p->fElapsedTime) / p->fLifeTime));

			float fScale=p->fStartSize * fInt + p->fEndSize * (1.f - fInt);

			if( p->bUseTrainSmoke ) {

				float fRatio = GetLifeRatio(p);
				float fAddScale = (p->fEndSize - p->fStartSize) / p->fLifeTime;

				if(fRatio < 0.1f ) {
					fAddScale *= 0.001f;
				}
				else if(fRatio < 0.4) {
					fAddScale *= 0.02f;
				}
				else {
					fAddScale *= 0.05f;
				}

				p->fCurScale += fAddScale;

				if(p->fCurScale > p->fEndSize)
					p->fCurScale = p->fEndSize;

				fScale = p->fCurScale;
			}

			matScaling = ScalingMatrix(fScale * m_Scale);
			matTranslation = TranslationMatrix(pos);

			matWorld = matScaling * mat;
			matWorld *= matTranslation;

			DWORD color = ((DWORD)(p->fOpacity * 255))<<24 | p->dwColor;

			static ZEFFECTCUSTOMVERTEX v[] = {
				{{-1, -1, 0}, 0xFFFFFFFF, 1, 0 },
				{{-1,  1, 0}, 0xFFFFFFFF, 1, 1 },
				{{ 1,  1, 0 }, 0xFFFFFFFF, 0, 1},
				{ { 1, -1, 0}, 0xFFFFFFFF, 0, 0 },
			};

			static const rvector sv[4] = { rvector(-1,-1,0) , rvector(-1,1,0) , rvector(1,1,0) , rvector(1,-1,0) };

			for (size_t i{}; i < 4; ++i)
				v[i].pos = TransformCoord(sv[i], matWorld);

			v[0].color=v[1].color=v[2].color=v[3].color=color;

			memcpy(pVertices,v,sizeof(ZEFFECTCUSTOMVERTEX)*4);
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
		dwRemainNum-=dwThisNum;

	}

	RGetDevice()->SetStreamSource( 0, NULL , 0,0 );	
	RGetDevice()->SetIndices(NULL);

	EndState();

	return true;
}

ZEffectBillboardTexAniList::ZEffectBillboardTexAniList()
{
	m_nMaxFrame = 16;// 128*128 texture 32*32 size 16
	m_fSpeed = 0.f;

	for(int i=0;i<8;i++)
		m_fUV[i] = 0.f;

	m_nXCnt		= 1;
	m_nYCnt		= 1;
	m_fXTileUV	= 1.f;
	m_fYTileUV	= 1.f;

	m_bFixFrame = false;

	m_nRenderMode = ZEDM_ADD;
}

void ZEffectBillboardTexAniList::Add(const rvector &pos, const rvector& vel,int frame,float fAddTime,float fStartSize,float fEndSize,float fLifeTime,
									 ZCharacter* pChar,RMeshPartsPosInfoType partstype)
{
	ZEFFECTBILLBOARDTEXANIITEM *pNewItem = new ZEFFECTBILLBOARDTEXANIITEM;

	push_back(pNewItem);

	pNewItem->fElapsedTime=0;
	pNewItem->velocity = vel;
	pNewItem->normal=-RCameraDirection;
	pNewItem->up=rvector(0,0,1);
	pNewItem->accel = rvector(0,0,0);
	pNewItem->position=pos;
	pNewItem->fStartSize=fStartSize;
	pNewItem->fEndSize=fEndSize;
	pNewItem->fOpacity=1.f;
	pNewItem->dwColor = 0xffffff;
	pNewItem->fAddTime = fAddTime;

	if(fLifeTime==-1)
		pNewItem->fLifeTime = m_fLifeTime;
	else
		pNewItem->fLifeTime = fLifeTime;

	pNewItem->frame = frame;

	if(pChar)	pNewItem->CharUID = pChar->GetUID();
	else 		pNewItem->CharUID = MUID(0,0);

	pNewItem->partstype = partstype;
}

void ZEffectBillboardTexAniList::Update(float fElapsed)
{
	ZCharacter* pChar = NULL;

	for(iterator i=begin();i!=end();)
	{
		ZEFFECTBILLBOARDTEXANIITEM *p = (ZEFFECTBILLBOARDTEXANIITEM*)*i;
		p->fElapsedTime+=fElapsed;

		if( p->fElapsedTime > p->fLifeTime + p->fAddTime ) {
			delete p;
			i=erase(i);
			continue;
		}

		p->normal=-RCameraDirection;
		p->velocity+=fElapsed*p->accel;
		p->position+=fElapsed*p->velocity;
		p->fOpacity = min(1.f, max(0.f, (p->fLifeTime - p->fElapsedTime) / m_fVanishTime));

		pChar = ZGetCharacterManager()->Find(p->CharUID);

		if( pChar ) {
			if( p->partstype != eq_parts_pos_info_etc) {
				if(pChar->m_pVMesh) {
					p->position = pChar->m_pVMesh->GetBipTypePosition( p->partstype );
				}
			}
		}

		// frame animation
		if( m_bFixFrame==false )
			p->frame = m_nMaxFrame * (p->fElapsedTime / p->fLifeTime);
		
		i++;
	}
}

void ZEffectBillboardTexAniList::BeginState()
{
	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	pDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE);

	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP   , D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	//	add
	if( m_nRenderMode == ZEDM_ADD)
	{
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	//alpha
	else {
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	RSetWBuffer(true);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	ZEffectBase::BeginState();
}

void ZEffectBillboardTexAniList::SetTile(int xCnt,int yCnt,float fXTileUV,float fYTileUV)
{
	m_nXCnt		= xCnt;
	m_nYCnt		= yCnt;
	m_fXTileUV	= fXTileUV;
	m_fYTileUV	= fYTileUV;
	m_nMaxFrame = xCnt * yCnt;
}

void ZEffectBillboardTexAniList::GetFrameUV(int frame)
{
//	기본적으로  128*128 크기의 texture 를 사용하는 걸로..

	int x = frame%m_nXCnt;
	int y = frame/m_nXCnt;

	float AddU = m_fXTileUV;

	float fU = x * m_fXTileUV;
	float fV = y * m_fYTileUV;

	m_fUV[0] = fU + m_fXTileUV;
	m_fUV[1] = fV;

	m_fUV[2] = fU + m_fXTileUV;
	m_fUV[3] = fV + m_fYTileUV;

	m_fUV[4] = fU;
	m_fUV[5] = fV + m_fYTileUV;

	m_fUV[6] = fU;
	m_fUV[7] = fV;
}

bool ZEffectBillboardTexAniList::Draw()
{
	if(!m_pVB) return false;

	if( size()==0 ) return true;

	BeginState();

	RSetFog(FALSE);

	HRESULT	hr;

	DWORD	dwRemainNum = (DWORD)size();

	iterator itr = begin();

	while(dwRemainNum)
	{
		if(m_dwBase >= EFFECTBASE_DISCARD_COUNT)
			m_dwBase = 0;

		DWORD dwThisNum = min( dwRemainNum , static_cast<DWORD>(BILLBOARD_FLUSH_COUNT) );

		dwThisNum = min( dwThisNum , EFFECTBASE_DISCARD_COUNT - m_dwBase );	

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

		int nRenderCnt = 0;

		ZCharacter* pChar = NULL;

		for(DWORD j=0;j<dwThisNum;j++)
		{
			ZEFFECTBILLBOARDTEXANIITEM *p = (ZEFFECTBILLBOARDTEXANIITEM*)*itr;

			if(p->fElapsedTime < p->fAddTime ) {
				itr++;
				continue;
			}

			pChar = ZGetCharacterManager()->Find(p->CharUID);

			if( pChar ) {
				if( pChar->m_pVMesh ) {
					if( pChar->m_pVMesh->m_bIsRender==false) {
						itr++;
						continue;
					}
				}
			}

			nRenderCnt++;

			// Transform
			rmatrix matTranslation;
			rmatrix matScaling;
			rmatrix matWorld;

			rvector dir = p->normal;

			rvector up = p->up;
			rvector right;

			if (IS_EQ(dir.z, 1.f)) up = rvector(1, 0, 0);

			right = Normalized(CrossProduct(up, dir));
			up = Normalized(CrossProduct(right, dir));

			rmatrix mat;
			GetIdentityMatrix(mat);
			mat._11 = right.x; mat._12 = right.y; mat._13 = right.z;
			mat._21 = up.x; mat._22 = up.y; mat._23 = up.z;
			mat._31 = dir.x; mat._32 = dir.y; mat._33 = dir.z;

			rvector pos = p->position;

			float fInt = min(1.f, max(0.f, (p->fLifeTime - p->fElapsedTime) / p->fLifeTime));
			float fScale=p->fStartSize * fInt + p->fEndSize * (1.f - fInt);

			matScaling = ScalingMatrix(fScale * m_Scale);
			matTranslation = TranslationMatrix(pos);

			matWorld = matScaling * mat;
			matWorld *= matTranslation;

			DWORD color = ((DWORD)(p->fOpacity * 255))<<24 | p->dwColor;

			static ZEFFECTCUSTOMVERTEX v[] = {
				{{-1, -1, 0}, 0xFFFFFFFF, 1, 0 },
				{{-1,  1, 0}, 0xFFFFFFFF, 1, 1},
				{{ 1,  1, 0}, 0xFFFFFFFF, 0, 1},
				{{ 1, -1, 0}, 0xFFFFFFFF, 0, 0},
			};

			static rvector sv[4] = { rvector(-1,-1,0) , rvector(-1,1,0) , rvector(1,1,0) , rvector(1,-1,0) };

			GetFrameUV( min( p->frame,m_nMaxFrame-1) );

			v[0].tu = m_fUV[0];
			v[0].tv = m_fUV[1];
			v[1].tu = m_fUV[2];
			v[1].tv = m_fUV[3];
			v[2].tu = m_fUV[4];
			v[2].tv = m_fUV[5];
			v[3].tu = m_fUV[6];
			v[3].tv = m_fUV[7];

			for (size_t i{}; i < 4; ++i)
				v[i].pos = TransformCoord(sv[i], matWorld);

			v[0].color=v[1].color=v[2].color=v[3].color=color;

			memcpy(pVertices,v,sizeof(ZEFFECTCUSTOMVERTEX)*4);
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

		if(FAILED( hr = RGetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_dwBase*4,nRenderCnt*4,m_dwBase*6,nRenderCnt*2) ))
			return false;

		m_dwBase+=dwThisNum;
		dwRemainNum-=dwThisNum;

	}

	RGetDevice()->SetStreamSource( 0, NULL , 0,0 );	
	RGetDevice()->SetIndices(NULL);

	if(ZGetWorld()) {
		ZGetWorld()->SetFog(true);
	}

	EndState();

	return true;
}