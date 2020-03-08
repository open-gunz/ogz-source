#include "stdafx.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "MDebug.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

RWeaponTracks::RWeaponTracks() 
{
	m_pVert = NULL;
	m_pNode = NULL;

	m_max_size				= 0;
	m_current_vertex_size	= 0;
	m_vertex_size			= 0;
	m_spline_vertex_size	= 0;
	m_current_node_size		= 0;
	m_current_spline_vertex_size = 0;

	m_bSpline = true;

	m_vSwordPos[0] = rvector(0,0,0);
	m_vSwordPos[1] = rvector(0,0,0);
}

RWeaponTracks::~RWeaponTracks()
{
	Destroy();
}

void RWeaponTracks::Create(int size)
{
	m_vertex_size = size * 2;
	m_spline_vertex_size = size * 50;
	m_max_size = size;

	m_pNode = new RWeaponSNode[size];
	m_pVert = new RLVertex[m_vertex_size];
	m_pVertSpline = new RLVertex[m_spline_vertex_size];

	memset( m_pVert,0,sizeof(RLVertex) * m_vertex_size );
	memset( m_pVertSpline,0,sizeof(RLVertex) * m_spline_vertex_size );
}

void RWeaponTracks::Destroy()
{
	if(m_pVert) {
		delete [] m_pVert;
		m_pVert = NULL;
	}

	if(m_pNode) {
		delete [] m_pNode;
		m_pNode = NULL;
	}

	if(m_pVertSpline) {
		delete [] m_pVertSpline;
		m_pVertSpline = NULL;
	}

	m_max_size = 0;
	m_current_vertex_size = 0;
	m_vertex_size = 0;
	m_spline_vertex_size = 0;
}

void RWeaponTracks::SetVertexSpline(rvector& p,DWORD c)
{
	m_pVertSpline[m_current_spline_vertex_size].p = p;
	m_pVertSpline[m_current_spline_vertex_size].color = c;

	m_current_spline_vertex_size++;

	if(m_current_spline_vertex_size > m_spline_vertex_size-1) {
		m_current_spline_vertex_size--;
	}
}

union _trcolor {
	u32 color;
	struct {
		u8 b;
		u8 g;
		u8 r;
		u8 a;
	};
};

DWORD GetColor(DWORD _color,float at,float ct)
{
	_trcolor color;

	color.color = _color;

	float r,g,b,a;

	a = color.a * at;
	r = color.r * ct;
	g = color.g * ct;
	b = color.b * ct;

	color.a = (u8)min(max(a, 0.f), (float)color.a);
	color.r = (u8)min(max(r, 0.f), (float)color.r);
	color.g = (u8)min(max(g, 0.f), (float)color.g);
	color.b = (u8)min(max(b, 0.f), (float)color.b);

	return color.color;
}


void RWeaponTracks::MakeBuffer()
{
	if(m_bSpline==false) return;

	m_current_spline_vertex_size = 0;

	if(m_current_node_size > 3)
	{
		for(int i=0;i<m_current_node_size;i++)
		{
			if( (i < 1) || (i > m_current_node_size-3) ) {
				SetVertexSpline(m_pNode[i].up , m_pNode[i].color[0]);
				SetVertexSpline(m_pNode[i].down , m_pNode[i].color[1]);
			}
			else
			{
				SetVertexSpline(m_pNode[i].up , m_pNode[i].color[0]);
				SetVertexSpline(m_pNode[i].down , m_pNode[i].color[1]);

				int cnt = 0;

				if( m_pNode[i].len > 10 )
					cnt = int(m_pNode[i].len / 10);

				rvector vOut;

				float s = 1.0f;

				rvector v1,v2,v3,v4,v5,v6,v7,v8;

				v1 = m_pNode[i-1].up;
				v2 = m_pNode[i  ].up;
				v3 = m_pNode[i+1].up;
				v4 = m_pNode[i+2].up;

				v5 = m_pNode[i-1].down;
				v6 = m_pNode[i  ].down;
				v7 = m_pNode[i+1].down;
				v8 = m_pNode[i+2].down;

				float fLen = m_pNode[i].len;

				for(int j=1;j<cnt+1;j++) {

					s = j * 10 / fLen;

					vOut = CatmullRomSpline(v1, v2, v3, v4, s);
					SetVertexSpline(vOut, m_pNode[i].color[0]);

					vOut = CatmullRomSpline(v5, v6, v7, v8, s);
					SetVertexSpline(vOut, m_pNode[i].color[1]);
				}
			}
		}
	}
	else
	{
		for(int i=0;i<m_current_node_size;i++)
		{
			SetVertexSpline(m_pNode[i].up   , m_pNode[i].color[0]);
			SetVertexSpline(m_pNode[i].down , m_pNode[i].color[1]);
		}
	}

	float at,ct;

	for(int i=0;i<m_current_spline_vertex_size;i++)
	{
		if(i) {
			at = (i/(float)m_current_vertex_size);
			ct = at;
		}
		else {
			at = 0.1f;
			ct = at;
		}

		m_pVertSpline[i].color = GetColor(m_pVertSpline[i].color,at,ct);
	}


}

void RWeaponTracks::Render()
{
	Update();

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if(m_current_node_size > 1) {

		MakeBuffer();

		static rmatrix _init_mat = GetIdentityMatrix();
		dev->SetTransform(D3DTS_WORLD, static_cast<D3DMATRIX*>(_init_mat));

		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		dev->SetTexture(0, NULL);
		dev->SetFVF( RLVertexType );

		if(m_bSpline)
			dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_current_spline_vertex_size-2 ,(LPVOID)m_pVertSpline, sizeof(RLVertex));
		else
			dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_current_vertex_size-2 ,(LPVOID)m_pVert, sizeof(RLVertex));

		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
		dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	}
}

void RWeaponTracks::Update()
{
	float at,ct;

	for(int i=0;i < m_current_vertex_size;i++) {

		if(i) {
			at = (i/(float)m_current_vertex_size)+0.3f;
			ct = at*1.2f;
		}
		else {
			at = 0.1f;
			ct = at*1.2f;
		}

		m_pVert[i].color = GetColor(m_pVert[i].color,at,ct);
	}

}

void RWeaponTracks::CheckShift()
{
	if( m_current_vertex_size >= m_vertex_size ) {

		int i=0;

		for(i=0;i < m_current_vertex_size-2;i++) {

			m_pVert[i].p	 = m_pVert[i+2].p;
			m_pVert[i].color = m_pVert[i+2].color;
		}

		for(i=0;i < m_current_node_size-1;i++) {
			m_pNode[i] = m_pNode[i+1];
		}

		m_current_vertex_size -= 2;
		m_current_node_size--;
	}
}

void RWeaponTracks::Clear() 
{
	m_current_vertex_size = 0;
	m_current_node_size = 0;
}

int RWeaponTracks::GetLastAddVertex(rvector* pOutVec)
{
	int cnt = 0;

	if( pOutVec && m_current_node_size )
	{
		pOutVec[0] = m_vSwordPos[0];
		pOutVec[1] = m_vSwordPos[1];

		cnt = 2;
	}

	return cnt;
}

void RWeaponTracks::AddVertex(RLVertex* pVert)
{
	CheckShift();

	RLVertex* pV = &m_pVert[m_current_vertex_size];
	RWeaponSNode* pN = &m_pNode[m_current_node_size];

	pV->p = pVert->p;
	pV->color = pVert->color;
	pN->up = pVert->p;
	pN->color[0] = pVert->color;

	pV++;
	pVert++;

	pV->p = pVert->p;
	pV->color = pVert->color;
	pN->down = pVert->p;
	pN->color[1] = pVert->color;

	// calc_length
	if( m_current_node_size )
		pN->len =  Magnitude( m_pNode[m_current_node_size].up - m_pNode[m_current_node_size-1].up);
	else 
		pN->len = 0.f;

	m_current_vertex_size += 2;
	m_current_node_size++;
}

unsigned char g_FireRed[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,4,4,4,4,4,4,4,8,8,8,8,8,12,12,12,12,16,16,16,16,20,20,20,24,24,
24,28,28,32,32,32,36,36,40,40,44,44,48,48,52,52,56,56,60,60,64,68,68,72,72,
76,80,80,84,88,88,92,92,96,100,100,104,108,108,112,116,120,120,124,128,128,
132,136,136,140,144,144,148,152,152,156,160,160,164,164,172,172,172,176,176,
180,184,184,188,188,192,192,196,196,200,200,204,204,208,208,208,212,212,216,
216,220,220,220,220,224,224,224,228,228,228,228,232,232,232,232,236,236,236,
236,236,240,240,240,240,240,240,240,244,244,244,244,244,244,244,244,244,244,
248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,
248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,
248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,
248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,
248,248,248,248,248};

unsigned char g_FireGreen[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,8,8,8,8,
8,8,8,8,8,12,12,12,12,12,12,16,16,16,16,16,16,20,20,20,20,20,24,24,24,24,28,
28,28,28,32,32,32,32,36,36,36,36,40,40,40,44,44,44,48,48,48,52,52,52,56,56,
56,60,60,60,64,64,64,68,68,72,72,72,76,76,80,80,80,84,84,84,88,88,92,92,92,
96,96,100,100,104,104,104,108,108,112,112,116,116,116,120,120,124,124,128,
128,128,132,132,136,136,140,140,140,144,144,148,148,148,152,152,156,156,160,
160,163,164,164,168,168,168,172,172,172,176,176,180,180,180,184,184,184,188,
188,188,192,192,192,196,196,200,200,200,200,200,204,204,204,208,208,208,208,
212,212,212,212,216,216,216,216,220,220,220,220,220,224,224,224,224,224,228,
228,228,228,228,228,232,232,232,232,232,232,236,236,236,236,236,236,236,236,
240,240,240,240,240,240};

unsigned char g_FireBlue[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
4,4,8,8,8,2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,12,12,12,12,12,12,12,12,12,12,
12,12,12,12,12,12,12,16,16,16,16,16,16,16,16,16,16,16,16,16,16,20,20,20,20,
20,20,20,20,20,20,20,20,24,24,24,24,24,24,24,24,24,24,24,24,28,28,28,28,28,
28,28,28,28,28,32,32,32,32,32,32,32,32,32,32,36,36,36,36,36,36,36,36,36,40,
40,40,40,40,40,40,40,44,44,44,44,44};


RFireEffectTexture::RFireEffectTexture()
{
	Init();
}

RFireEffectTexture::~RFireEffectTexture()
{
	Destroy();
}

HRESULT RFireEffectTexture::Create(LPDIRECT3DDEVICE9 dev, int w,int h)
{
	m_w = w;
	m_h = h;

	m_pData = new BYTE[w*h];
	m_pData2 = new BYTE[w*h];

	::ZeroMemory(m_pData, w*h);
	::ZeroMemory(m_pData2, w*h);

	m_pFireActive = m_pData;
	m_pFireScratch = m_pData2;

	return RGetDevice()->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture, nullptr);
}

void RFireEffectTexture::Destroy()
{
	if(m_pData) {
		delete[] m_pData;
		m_pData = NULL;
	}

	if(m_pData2) {
		delete[] m_pData2;
		m_pData2 = NULL;
	}

	if(m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

void RFireEffectTexture::Init()
{
	for (int q=0; q < 255; q++)  {

		m_Palette[q].peRed   = g_FireRed[q];
		m_Palette[q].peGreen = g_FireGreen[q];
		m_Palette[q].peBlue  = g_FireBlue[q];

		m_Palette[q].peFlags = 180;
	}

	m_w = 0;
	m_h = 0;

	m_pData = NULL;  
	m_pData2 = NULL; 
	m_pFireActive = NULL;   
	m_pFireScratch = NULL;  

}

HRESULT RFireEffectTexture::UpdateTexture()
{
	HRESULT hr;

	D3DLOCKED_RECT lockedrect;
	::ZeroMemory(&lockedrect, sizeof(lockedrect));

	if (FAILED(hr = m_pTexture->LockRect(0, &lockedrect, NULL, 0))) 
		return hr;

	unsigned char *pSurfBits = static_cast<unsigned char *>(lockedrect.pBits);

	int index = 0;
	int pos = 0;
	int y,x;

	for ( y=0; y < m_h; y++) {
		for ( x=0; x < m_w; x++) {

			pos = (y*m_w)+x;

			pSurfBits[index++] = m_Palette[m_pFireActive[pos]].peBlue;	// blue
			pSurfBits[index++] = m_Palette[m_pFireActive[pos]].peGreen;	// green
			pSurfBits[index++] = m_Palette[m_pFireActive[pos]].peRed;	// red
			pSurfBits[index++] = m_Palette[m_pFireActive[pos]].peFlags;	// alpha
		}
		index += lockedrect.Pitch - (m_w*4);
	}

	if (FAILED(hr = m_pTexture->UnlockRect(0))) 
		return hr;

	return S_OK;
}

void RFireEffectTexture::ProcessFire(int coolamount)
{
	int x,y;

	unsigned char *pInTex = m_pFireActive;
	unsigned char *pOutTex = m_pFireScratch;

	int w = m_w;
	int h = m_h;

	for ( y=0; y < h; y++) {

		for ( x=0; x < w-0; x++) {

			unsigned char firevalue_left, firevalue_right, firevalue_bottom, firevalue_top;

			int finalfirevalue;

			int xplus1, xminus1, yplus1, yminus1;

			xplus1 = x+1; if (xplus1 >= w)		xplus1=0;
			xminus1= x-1; if (xminus1 < 0)		xminus1 = w-1;
			yplus1 = y+1; if (yplus1 >= h)		yplus1=h-1;
			yminus1= y-1; if (yminus1 < 0)		yminus1 = 0;

			firevalue_right  = pInTex[(y*w)+xplus1];
			firevalue_left   = pInTex[(y*w)+xminus1];
			firevalue_bottom = pInTex[((yplus1)*w)+x];
			firevalue_top    = pInTex[((yminus1)*w)+x];

			finalfirevalue = (firevalue_left+firevalue_right+firevalue_top+firevalue_bottom)/4;

			finalfirevalue -= coolamount;

			if (finalfirevalue < 0) 
				finalfirevalue = 0;

			pOutTex[((yminus1)*w)+x] = finalfirevalue;

		}
	}

	for (int x=10; x < w-10; x+=2) {

		int y=h-1;

		int fuel = pInTex[(y*w)+x] + (rand() % 64) - 29;

		if (fuel > 255)	fuel = 255;
		if (fuel < 0)	fuel = 0;

		pOutTex[(y*w)+x]  = (unsigned char)fuel;
		pOutTex[(y*w)+x+1] = (unsigned char)fuel;
	}

	unsigned char *temp	= m_pFireActive;

	m_pFireActive	= m_pFireScratch;
	m_pFireScratch	= temp;
}

_NAMESPACE_REALSPACE2_END
