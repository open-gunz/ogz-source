#include "StdAfx.h"
#include "ZProfiler.h"

#include "mmsystem.h"

ZProfiler::ZProfiler(void) : m_nRingHead(0)
{
	m_dwLastTime = GetGlobalTimeMS();
	for(int i=0;i<FRAME_RING_BUFFER_SIZE;i++)
	{
		m_dwRingBuffer[i].dwElapsed = 0;
		m_dwRingBuffer[i].dwTime = m_dwLastTime;
	}
}

ZProfiler::~ZProfiler(void)
{
}

void ZProfiler::Update()
{
	DWORD dwCurrent = GetGlobalTimeMS();
	DWORD dwElapsed = dwCurrent - m_dwLastTime;

	m_dwRingBuffer[m_nRingHead].dwTime = dwCurrent;
	m_dwRingBuffer[m_nRingHead].dwElapsed = dwElapsed;
	m_nRingHead = (m_nRingHead +1) % FRAME_RING_BUFFER_SIZE;

	m_dwLastTime = dwCurrent;
}

void ZProfiler::Render()
{
	static D3DXVECTOR4 v[FRAME_RING_BUFFER_SIZE];
	int nRingIndex = m_nRingHead;
	for(int i=0;i<FRAME_RING_BUFFER_SIZE;i++)
	{
		v[i].x = i;
		v[i].y = RGetScreenHeight()-m_dwRingBuffer[nRingIndex].dwElapsed;
		v[i].z = 0;
		v[i].w = 1;
		nRingIndex = (nRingIndex +1) % FRAME_RING_BUFFER_SIZE;
	}

	RGetDevice()->SetFVF( D3DFVF_XYZRHW );
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE );
	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR ,   0xff00ffff);
	RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,FRAME_RING_BUFFER_SIZE-1,&v,sizeof(D3DXVECTOR4));
}