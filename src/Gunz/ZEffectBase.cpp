#include "stdafx.h"

#include "ZEffectBase.h"

LPDIRECT3DVERTEXBUFFER9	ZEffectBase::m_pVB = NULL;
LPDIRECT3DINDEXBUFFER9	ZEffectBase::m_pIB = NULL;
DWORD	ZEffectBase::m_dwBase = EFFECTBASE_DISCARD_COUNT;


ZEffectBase::ZEffectBase(void)
{
	m_fLifeTime=2.f;
	m_fVanishTime=1.f;
	m_pBaseTexture=NULL;
	m_Scale=rvector(1.f,1.f,1.f);
}

ZEffectBase::~ZEffectBase(void)
{
	Clear();
	Destroy();
}

void ZEffectBase::Clear()
{
	while(size())
	{
		delete *begin();
		erase(begin());
	}
}

void ZEffectBase::Destroy()
{
	if(m_pBaseTexture)
	{
		RDestroyBaseTexture(m_pBaseTexture);
		m_pBaseTexture=NULL;
	}
}

void ZEffectBase::CreateBuffers()
{
	OnRestore();
}

void ZEffectBase::ReleaseBuffers()
{
	OnInvalidate();
}


bool ZEffectBase::Create(const char *szTextureName)
{
	m_pBaseTexture = RCreateBaseTexture(szTextureName);

	if(!m_pVB || !m_pIB) return false;
	return true;
}


void ZEffectBase::OnRestore()
{
	SAFE_RELEASE(m_pVB);
	if(FAILED(RGetDevice()->CreateVertexBuffer(
		sizeof(ZEFFECTCUSTOMVERTEX) * EFFECTBASE_DISCARD_COUNT * 4 , D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY , ZEFFECTBASE_D3DFVF, D3DPOOL_DEFAULT, &m_pVB,NULL)))
		return ;

	SAFE_RELEASE(m_pIB);
	if(FAILED(RGetDevice()->CreateIndexBuffer(
		sizeof(WORD) * EFFECTBASE_DISCARD_COUNT * 6 , D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY , D3DFMT_INDEX16 , D3DPOOL_DEFAULT, &m_pIB,NULL)))
		return ;

	m_dwBase = EFFECTBASE_DISCARD_COUNT; 
}

void ZEffectBase::OnInvalidate()
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
}


void ZEffectBase::BeginState()
{
	rmatrix id;
	GetIdentityMatrix(id);

	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	RSetTransform(D3DTS_WORLD, id);
	pDevice->SetStreamSource(0,m_pVB,0,sizeof(ZEFFECTCUSTOMVERTEX));
	pDevice->SetIndices(m_pIB);
	pDevice->SetFVF(ZEFFECTBASE_D3DFVF);
	pDevice->SetTexture(0,m_pBaseTexture ? m_pBaseTexture->GetTexture() : NULL );
}


void ZEffectBase::EndState()
{
}

void ZEffectBase::Update(float fElapsed)
{
}

bool ZEffectBase::Draw()
{
	return true;
}