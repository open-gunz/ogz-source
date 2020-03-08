#include "stdafx.h"

#include "ZInitialLoading.h"
#include "Mint4R2.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "ZTips.h"
#include "ZGameInterface.h"
#include "ZGlobal.h"
#include "ZMyInfo.h"
#include "VertexTypes.h"

using namespace RealSpace2;

#define NUM_BAR_BITMAP	1
#define IL_TEXT_BUFFER_SIZE 512
#define ADJUST_SIZE				0.5f
#define ADJUST_SIZE2			0.0f
#define FADEIN_NUM_DRAWING		120
#define FADEOUT_NUM_DRAWING		120
#define BLENDING_NUM_DRAWING	240

//////////////////////////////////////////////////////////////////////////
//	Global
//////////////////////////////////////////////////////////////////////////
ZInitialLoading	ZInitialLoading::msInstance;
ZInitialLoading*	ZGetInitialLoading()
{
	return ZInitialLoading::GetInstance();
}

//////////////////////////////////////////////////////////////////////////
//	Initialize
//////////////////////////////////////////////////////////////////////////
bool ZInitialLoading::Initialize( int numScene_, 
								 float x_, float y_, float w_, float h_, 
								 float sx_, float sy_, float sw_, float sh_, bool bTipsVisible)
{
	SAFE_DELETE_ARRAY( mBitmaps );
	SAFE_DELETE_ARRAY( mBitmap_Bar );

	mBitmaps			= new MBitmapR2[numScene_];
	mBitmap_Bar	= new MBitmapR2[NUM_BAR_BITMAP];
	miNumScene	= numScene_;
	miCurrScene	= 0;

	mx	= x_;		my	= y_;		mw	= w_;	mh	= h_;
	msx	= sx_;	msy	= sy_;	msw	= sw_;	msh	= sh_;

	mbUseEnable	= true;
	mbText		= FALSE;

	mbBitmap			= false;
	mbBitmapBar	= false;

	m_bTipsVisible = bTipsVisible;
	m_szTips[0] = 0;

	if (m_bTipsVisible)
	{
		SetRandomTipsText();
	}

	// 청소년 자율 규제 적용안(쓰벌쓰벌쓰벌...)...
#ifdef LOCALE_KOREA
	if ( (rand() % 2) == 0)
		sprintf_safe( m_szNotice, "장시간의 게임 이용은 건강을 해칠 수 있습니다.");
	else
		sprintf_safe( m_szNotice, "공부와 일에 휴식이 필요하듯이, 게임 이용에도 적절한 휴식이 필요 합니다.");
#endif

	m_pLoadingStr = NULL;

	return true;
}

//////////////////////////////////////////////////////////////////////////
//	AddBitmap
//////////////////////////////////////////////////////////////////////////
bool ZInitialLoading::AddBitmap( int index_, const char* bitmapName_ )
{
	if( index_ >= miNumScene || index_ < 0 )
	{
		return false;
	}
	char Buff[64];
	sprintf_safe( Buff, "Scene%d", index_ );
	if( !mBitmaps[index_].Create( Buff, RGetDevice(), bitmapName_ ) )
	{
		mlog("Fail to create Bitmap for Initial loading\n" );
		return false;
	}
	mbBitmap		= true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//	AddBitmapBar
//////////////////////////////////////////////////////////////////////////
bool	ZInitialLoading::AddBitmapBar( const char* bitmapName_ )
{
	if( !mBitmap_Bar[0].Create( "Progress_Bar", RGetDevice(), bitmapName_ ) )
	{
		mlog("Fail to create Bitmap for Initial loading\n" );
		return false;
	}
	mbBitmapBar	= true;
	return true;
}

void ZInitialLoading::Draw(LOADING_SCENE_MODE mode_, int destIndex_, bool bFlip_, bool fromThread_)
{
	RRESULT isOK=RIsReadyToRender();
	if(isOK ==R_NOTREADY)
		return;

	if( !mbUseEnable ) return;
	if( !mbBitmap ) return;

	if( mBitmaps[miCurrScene].m_pTexture  == NULL ) 
	{		return;		}

	int	numDrawing = 1;

	float ftw = (float)mBitmaps[miCurrScene].m_pTexture->GetWidth();
	float fth = (float)mBitmaps[miCurrScene].m_pTexture->GetHeight();
	float msw2 = msw; 
	float msh2 = msh;

	D3DFORMAT d3dformat = mBitmaps[miCurrScene].m_pTexture->GetFormat();
	
	if( d3dformat==D3DFMT_DXT1 ||
		d3dformat==D3DFMT_DXT2 ||
		d3dformat==D3DFMT_DXT3 ||
		d3dformat==D3DFMT_DXT4 ||
		d3dformat==D3DFMT_DXT5 )
	{
		msw2 = (float)NextPowerOfTwo((int)msw);
		msh2 = (float)NextPowerOfTwo((int)msh);
	}

	ScreenSpaceTexVertex Sprite[4] = {
		{mx      - ADJUST_SIZE , my      - ADJUST_SIZE , 0, 1.0f, (msx)/ftw       , (msy)/fth },
		{mx + mw - ADJUST_SIZE2, my      - ADJUST_SIZE , 0, 1.0f, (msx + msw2)/ftw, (msy)/fth },
		{mx + mw - ADJUST_SIZE2, my + mh - ADJUST_SIZE2, 0, 1.0f, (msx + msw2)/ftw, (msy + msh2)/fth },
		{mx      - ADJUST_SIZE , my + mh - ADJUST_SIZE2, 0, 1.0f, (msx)/ftw       , (msy + msh2)/fth},
	};

	float blendFactor = 0;
	switch( mode_ )
	{
	
	case MODE_DEFAULT:
		numDrawing	= 1;
		RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		break;
	
	case MODE_FADEOUT:
		blendFactor		= 1;
		numDrawing		= FADEIN_NUM_DRAWING;
		RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		RGetDevice()->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );	
		RGetDevice()->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		break;

	case MODE_FADEIN:
		blendFactor		= 0;
		numDrawing		= FADEOUT_NUM_DRAWING;
		RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		RGetDevice()->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );	
		RGetDevice()->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		break;
	
	default:
		numDrawing	= 1;
		RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		break;
	}

	for( int i = 0 ; i < numDrawing; ++i )
	{
		RGetDevice()->Clear( 0 , NULL, D3DCLEAR_TARGET, 0x00000000, 0, 0 );
		if( mode_ == MODE_FADEIN )
		{
			blendFactor += 1.0f / FADEIN_NUM_DRAWING;
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF*blendFactor))<<24);
		}
		else if( mode_ == MODE_FADEOUT )
		{
			blendFactor -= 1.0f / FADEOUT_NUM_DRAWING;
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF*blendFactor))<<24);
		}

		RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
		RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
	
		RGetDevice()->SetFVF(ScreenSpaceTexFVF);
		RGetDevice()->SetTexture(0, mBitmaps[miCurrScene].m_pTexture->GetTexture());
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(ScreenSpaceTexVertex));

	//	if( mode_ == MODE_FADEOUT )
		{
			DrawBar( (float)(RGetScreenWidth() * 0.276), (float)(RGetScreenHeight() * 0.925f), 
				(float)(RGetScreenWidth() * 0.4482), (float)(RGetScreenHeight() * 0.018f),  mPercentage* 0.01f ); 
		}

		// 로딩 이름
		if(m_pLoadingStr && mpDC ) {
			char buffer[256];
			sprintf_safe(buffer,"Loading %s ...",m_pLoadingStr);
			int nWidth = mpDC->m_Font.GetTextWidth(buffer);
			int x = (int)(RGetScreenWidth() * 0.5) - nWidth/2;
			int y = (int)(RGetScreenHeight() * 0.9f);
			mpDC->m_Font.DrawText( x, y, buffer, 0xFFCCCCCC );
		}

	// 청소년 자율 규제 적용안(쓰벌쓰벌쓰벌...)...
#ifdef LOCALE_KOREA
		if (mpDC)
		{
			int nWidth = mpDC->m_Font.GetTextWidth( m_szNotice);
			int x = (int)(RGetScreenWidth() * 0.5) - nWidth/2;
			int y = (int)(RGetScreenHeight() * 0.86f);
			mpDC->m_Font.DrawText( x, y, m_szNotice, 0xFFFFFFFF );
		}
#endif

		if( mode_ == MODE_DEFAULT && mbText )
			DrawText();

		if (m_bTipsVisible) DrawTips();

		if( bFlip_ )		{			RFlip();			}
	}
}

//////////////////////////////////////////////////////////////////////////
//	DrawBar
//////////////////////////////////////////////////////////////////////////
void ZInitialLoading::DrawBar( float x_, float y_, float w_, float h_, float percent_ )
{
	if( !mbBitmapBar ) return;
	if( percent_ > 1.0f ) return;

	ScreenSpaceTexVertex Sprite[4] = {
		{x_ - ADJUST_SIZE, y_ - ADJUST_SIZE, 0, 1.0f, 0, 0 },
		{x_ + ( w_ * percent_ ) - ADJUST_SIZE2, y_ - ADJUST_SIZE, 0, 1.0f, w_/h_ * percent_, 0 },
		{x_ + ( w_ * percent_ ) - ADJUST_SIZE2, y_ + h_ - ADJUST_SIZE2, 0, 1.0f, w_/h_ * percent_, 1 },
		{x_ - ADJUST_SIZE, y_ + h_ - ADJUST_SIZE2, 0, 1.0f, 0, 1 },
	};

	RGetDevice()->SetRenderState( D3DRS_ZENABLE , D3DZB_FALSE );

//	RGetDevice()->SetTextureStageState( 0, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	RGetDevice()->SetFVF(ScreenSpaceTexFVF);
	RGetDevice()->SetTexture(0, mBitmap_Bar[0].m_pTexture->GetTexture());
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(ScreenSpaceTexVertex));
	//Sleep( 0 );
}

//////////////////////////////////////////////////////////////////////////
//	생성자
//////////////////////////////////////////////////////////////////////////
ZInitialLoading::ZInitialLoading()
{
	mbUseEnable = false;
}

//////////////////////////////////////////////////////////////////////////
//	소멸자
//////////////////////////////////////////////////////////////////////////
ZInitialLoading::~ZInitialLoading()
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
//	Release
//////////////////////////////////////////////////////////////////////////
void	ZInitialLoading::Release()
{
    if( !mbUseEnable ) return;	// 활성화 되어 있지 않으면 skip

	// Boolean값 OFF
	mbUseEnable	= false;			
	mbBitmapBar	= false;
	mbBitmap			= false;

	// Bitmap 해제
	SAFE_DELETE_ARRAY( mBitmaps );
	SAFE_DELETE_ARRAY( mBitmap_Bar );
	
	mpDC				= NULL;
	mpStr				= NULL;
}

void	ZInitialLoading::SetText( MFontR2* pDc, float x, float y, char* str )
{
	mpDC	= pDc;
	mtx		= x;
	mty		= y;
	mpStr	= str;

	if( pDc == NULL || str == NULL )
	{		mbText		= false;		}
	else
	{		mbText		= true;		}
}

void	ZInitialLoading::DrawText()
{
	if(mpDC != 0)
	{
		mpDC->m_Font.DrawText( mtx, mty, mpStr, 0xFFCCCCCC );
	}
}

extern MFontR2*		g_pDefFont;
extern MDrawContextR2* g_pDC;
void	ZInitialLoading::DrawTips()
{
//	return;			// 현재 적용안됨

	const float fTipYRatio = 0.83f;

	MFont *pFont= MFontManager::Get("FONTa10_O2Wht");
	if (pFont == NULL) pFont = g_pDefFont;
	if ((g_pDC != 0) && (pFont != 0))
	{
		g_pDC->SetFont(pFont);
		g_pDC->SetColor(MCOLOR(0xFFFFFFFF));

		int x, y;
		int nStrWidth = pFont->GetWidth(m_szTips);

		if (nStrWidth >= RGetScreenWidth())
		{
			x = 10;
			y = (int)((float)RGetScreenHeight() * fTipYRatio) - 10;

			int nStripIndex = 0;
			char temp[512] = "";
			while ((RGetScreenWidth()-70) > (pFont->GetWidth(temp)))
			{
				strcat_safe(temp, "남");
			}

			nStripIndex = (int)strlen(temp);

			char szLine1[1024] = "", szLine2[1024] = "";
			if ((int)strlen(m_szTips) > nStripIndex+1)
			{
				while((m_szTips[nStripIndex]) != '\0')
				{
					if ((m_szTips[nStripIndex] == ' ') || (m_szTips[nStripIndex] == ',') ||
						(m_szTips[nStripIndex] == '.') ) break;
					nStripIndex++;
				}

                strncpy_safe(szLine1, m_szTips, nStripIndex);
				strcpy_safe(szLine2, &m_szTips[nStripIndex]);
			}
			else
			{
				strcpy_safe(szLine1, m_szTips);
				szLine2[0] = 0;
			}

			g_pDC->Text(x, y, szLine1);
			g_pDC->Text(x, y+18, szLine2);
		}
		else
		{
			g_pDC->SetColor( MCOLOR(0xFFDADADA));
			x = (RGetScreenWidth() - nStrWidth) / 2;
			if (x < 1) x = 1;
			y = (int)((float)RGetScreenHeight() * fTipYRatio);
			g_pDC->Text(x, y, m_szTips);

			int nPosx, nPosy;
			char szStr1[ 256], szStr2[ 256], szStr3[ 256];
			switch ( m_nTipNum)
			{
				case ( 0) :			// loading_dash.tga
					nPosx = 20;
					nPosy = 20;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE11));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE12));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE13));
					break;
				case ( 1) :			// loading_gaurd.tga
					nPosx = 20;
					nPosy = 20;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE21));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE22));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE23));
					break;
				case ( 2) :			// loading_ksa.tga
					nPosx = 20;
					nPosy = 390;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE31));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE32));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE33));
					break;
				case ( 3) :			// loading_safefall.tga
					nPosx = 20;
					nPosy = 390;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE41));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE42));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE43));
					break;
				case ( 4) :			// loading_tumbling.tga
					nPosx = 20;
					nPosy = 390;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE51));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE52));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE53));
					break;
				case ( 5) :			// loading_wallhang.tga
					nPosx = 20;
					nPosy = 20;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE61));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE62));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE63));
					break;
				case ( 6) :			// loading_walljump.tga
					nPosx = 20;
					nPosy = 20;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE71));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE72));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE73));
					break;
				case ( 7) :			// loading_wallrun01.tga
					nPosx = 20;
					nPosy = 390;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE81));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE82));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE83));
					break;
				case ( 8) :			// loading_wallrun02.tga
					nPosx = 20;
					nPosy = 390;
					sprintf_safe( szStr1, ZMsg( MSG_LOADING_MESSAGE91));
					sprintf_safe( szStr2, ZMsg( MSG_LOADING_MESSAGE92));
					sprintf_safe( szStr3, ZMsg( MSG_LOADING_MESSAGE93));
					break;
				default :
					nPosx = 20;
					nPosy = 20;
					break;
			}

			nPosx = (int)( nPosx * MGetWorkspaceWidth()  / 800.f);
			nPosy = (int)( nPosy * MGetWorkspaceHeight() / 600.f);
			g_pDC->Text( nPosx,   nPosy,    szStr1);
			g_pDC->Text( nPosx+5, nPosy+24, szStr2);
			g_pDC->Text( nPosx+5, nPosy+39, szStr3);
		}
	}
}

void	ZInitialLoading::SetRandomTipsText()
{
	const char* szTips;
	if (ZGetMyInfo()->IsNewbie())
	{
		if (!ZGetGameInterface()->GetTips()->IsShowedNewbieTips())
		{
			szTips = ZGetGameInterface()->GetTips()->GetTips(ZTIPS_CATEGORY_NEWBIE, ZTIPMSG_ID_NEWBIE_SHOW_HELP);
			ZGetGameInterface()->GetTips()->SetShowedNewbieTips(true);
		}
		else
		{
			szTips = ZGetGameInterface()->GetTips()->GetRandomTips();
		}
	}
	else
	{
		szTips = ZGetGameInterface()->GetTips()->GetRandomTips();
	}
	
	if (strlen(szTips) < 255)
	{
		strcpy_safe(m_szTips, szTips);
	}
}

#ifdef _DEBUG
float ZLoadingProgress::m_fLastProgress = 0.f;
#endif

ZLoadingProgress::ZLoadingProgress(const char *szName,ZLoadingProgress *pParent,float fRatio)
{
	strcpy_safe(m_szName,szName);
	m_pParent = pParent;
	m_fThisAmount = pParent ? pParent->m_fThisAmount * fRatio : fRatio;
	m_fTotalProgressStart = pParent ? pParent->m_fTotalProgressStart + pParent->m_fCurrentProgress : 0;
	m_fProgressStart = pParent ? pParent->m_fCurrentProgress : 0;
	m_fCurrentProgress = 0;
	
#ifdef _DEBUG
	if(pParent==NULL)
		m_fLastProgress=0;
#endif
}

void ZLoadingProgress::Update(float fProgress) 
{
	m_fCurrentProgress = fProgress; 
	if(m_pParent) 
		m_pParent->Update(m_fProgressStart + m_fCurrentProgress*m_fThisAmount);
}

void ZLoadingProgress::Draw() 
{
    float fTotalProgress = m_fTotalProgressStart + m_fThisAmount * m_fCurrentProgress;
#ifdef _DEBUG // 혹시 되돌아가는경우인지 체크
	_ASSERT(m_fLastProgress<=fTotalProgress);
	m_fLastProgress=fTotalProgress;
#endif

	ZGetInitialLoading()->SetLoadingStr( m_szName );
	ZGetInitialLoading()->SetPercentage( fTotalProgress * 100.f );
	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );
}
