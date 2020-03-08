#include "stdafx.h"

#include "ZGame.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "ZSoundEngine.h"
#include "ZEffectFlashBang.h"
#include "ZConfiguration.h"
#include "ZApplication.h"
#include "RBspObject.h"

using namespace RealSpace2;

ZEffectFlashBang ZEffectFlashBang::msInstance;
bool ZEffectFlashBang::mbDrawCopyScreen	= false;

#define CLEAR_TIME_DURATION 7

ZEffectFlashBang*	ZGetFlashBangEffect()
{
	return ZEffectFlashBang::GetInstance();
}

void CreateFlashBangEffect( rvector& ExplosionPos_, rvector playerPos_, rvector playerDir_, float Duration_ )
{
	ZGetFlashBangEffect()->Init( ExplosionPos_, playerPos_, playerDir_, Duration_ );
}

bool IsActivatedFlashBangEffect()
{
	return ZGetFlashBangEffect()->IsActivated();
}

void ShowFlashBangEffect()
{
	ZGetFlashBangEffect()->Render();
}

void ReleaseFlashBangEffect()
{
	ZGetFlashBangEffect()->ReleaseBuffer();
}

#define SCREEN_BLUR_TEXTURE_SIZE_WIDTH	800
#define SCREEN_BLUR_TEXTURE_SIZE_HEIGHT	600
#define MAX_MUSIC_VOLUMN	Z_AUDIO_BGM_VOLUME
#define MAX_EFFECT_VOLUMN	Z_AUDIO_EFFECT_VOLUME
#define MIN_VOLUMN			0

extern ZGame* g_pGame;

void ZEffectFlashBang::SetBuffer()
{
	constexpr float Correction = 0.01f;

	// left top
	mBuffer[0].p		= { 0.0f - Correction, 0.0f - Correction, 0, 1.0f };
	mBuffer[0].color	= 0xffffffff;
	mBuffer[0].tu		= 0.0f;
	mBuffer[0].tv		= 0.0f;

	// right top
	mBuffer[1].p		= { RGetScreenWidth() + Correction, 0.0f - Correction, 0, 1.0f };
	mBuffer[1].color	= 0xffffffff;
	mBuffer[1].tu		= 1.0f;
	mBuffer[1].tv		= 0.0f;

	// right bottom
	mBuffer[2].p		= { RGetScreenWidth() + Correction, RGetScreenHeight() + Correction, 0, 1.0f };
	mBuffer[2].color	= 0xffffffff;
	mBuffer[2].tu		= 1.0f;
	mBuffer[2].tv		= 1.0f;

	// left bottom
	mBuffer[3].p		= { 0.0f - Correction, static_cast<float>(RGetScreenHeight()), 0, 1.0f };
	mBuffer[3].color	= 0xffffffff;
	mBuffer[3].tu		= 0.0f;
	mBuffer[3].tv		= 1.0f;

	D3DDISPLAYMODE mode;
	RGetDevice()->GetDisplayMode( 0,&mode );

	if( mpBlurTexture != 0 )
	{
		_ASSERT( mpBlurSurface == 0 );
		return;
	}

	if( !mbDrawCopyScreen )
	{
		return;
	}

	if( mpBlurTexture == 0 ) 
	{
		if(FAILED(RGetDevice()->CreateTexture(
			SCREEN_BLUR_TEXTURE_SIZE_WIDTH, SCREEN_BLUR_TEXTURE_SIZE_HEIGHT,
			1, D3DUSAGE_RENDERTARGET,
			mode.Format, D3DPOOL_DEFAULT,
			&mpBlurTexture, nullptr)))
		{
			if (FAILED(RGetDevice()->CreateTexture(
					SCREEN_BLUR_TEXTURE_SIZE_WIDTH, SCREEN_BLUR_TEXTURE_SIZE_HEIGHT,
					1, D3DUSAGE_DYNAMIC | D3DUSAGE_RENDERTARGET,
					mode.Format, D3DPOOL_MANAGED,
					&mpBlurTexture, nullptr)))
			{
				mlog("Fail to create BLUR texture\n");
				mbDrawCopyScreen = false;
				mpBlurTexture = 0;
				return;
			}
		}

		if( mpBlurTexture != 0 )
		{
			mpBlurTexture->GetSurfaceLevel( 0, &mpBlurSurface );
		}

		if( mpBlurSurface != 0 )
		{
			D3DSURFACE_DESC desc;
			mpBlurSurface->GetDesc( &desc );
			RGetDevice()->CreateDepthStencilSurface( desc.Width, desc.Height, D3DFMT_D16, D3DMULTISAMPLE_NONE ,0,TRUE, &mpDepthBuffer , NULL);
			RGetDevice()->GetDepthStencilSurface( &mpHoldDepthBuffer );
		}
	}

	mbActivated		= false;
	mbInitialSound	= true;
}


void ZEffectFlashBang::ReleaseBuffer()
{
	End();
	SAFE_RELEASE( mpBlurSurface );
	SAFE_RELEASE( mpBlurTexture );
	SAFE_RELEASE( mpDepthBuffer );
	SAFE_RELEASE( mpHoldDepthBuffer );
}

void ZEffectFlashBang::OnInvalidate()
{
	ReleaseBuffer();
}

void ZEffectFlashBang::OnRestore()
{
	SetBuffer();
}

void ZEffectFlashBang::Init( rvector& ExplosionPos_, rvector playerPos_, rvector playerDir_, float Duration_  )
{
	if( !isInViewFrustum( ExplosionPos_, RGetViewFrustum()) )
	{
		return;
	}

	mbActivated	= true;
	
	mfDuration	= Duration_;

	rvector flashDir = Normalized(ExplosionPos_ - playerPos_);

	mfPower = DotProduct(playerDir_, flashDir) * mfDuration;
	mfStartTime = g_pGame->GetTime();

	if( !mbDrawCopyScreen )
	{
		return;
	}

	// Blur
	if( mpBlurSurface != 0 )
	{
		RGetDevice()->GetRenderTarget( 0,&mpHoldBackBuffer );
		RGetDevice()->SetRenderTarget( 0,mpBlurSurface ); 
		RGetDevice()->SetDepthStencilSurface(mpDepthBuffer);

		RGetDevice()->Clear( 0 , NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0.0f );

		auto view = ViewMatrix(RCameraPosition, RCameraDirection, RCameraUp);
		RSetTransform(D3DTS_VIEW, view);

		ZGetGame()->GetWorld()->GetBsp()->Draw();
		g_pGame->m_ObjectManager.Draw();
		ZGetEffectManager()->Draw(GetGlobalTimeMS());

		RGetDevice()->SetRenderTarget( 0,mpHoldBackBuffer );
		RGetDevice()->SetDepthStencilSurface(mpHoldDepthBuffer);

		SAFE_RELEASE( mpHoldBackBuffer );
	}

}

void ZEffectFlashBang::Render()
{
	if( !mbActivated )
	{
		return;
	}

	float	elapsedTime	= g_pGame->GetTime() - mfStartTime;
	if( elapsedTime < mfDuration )
	{
		RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		if( mpBlurTexture != 0 && mbDrawCopyScreen )
		{
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			RGetDevice()->SetTexture( 0, mpBlurTexture );
		}
		else
		{
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
			RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		}
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );

		float alpha		= max(min( ((mfPower - elapsedTime)/CLEAR_TIME_DURATION), 1.0f ), 0.0f);

		RGetDevice()->SetRenderState( D3DRS_TEXTUREFACTOR, 
			(DWORD)((BYTE)( 0xFF * alpha ))<<24 );

		RGetDevice()->SetFVF( RTLVertexType );
		DWORD dw;
		RGetDevice()->GetRenderState( D3DRS_ZENABLE, &dw );
		RGetDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
		RGetDevice()->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, mBuffer, sizeof(RTLVertex) );
		RGetDevice()->SetRenderState( D3DRS_ZENABLE, dw );

		PlaySound();
	}
	else
		End();
}

void ZEffectFlashBang::End()
{
	mbActivated		= false;
	mbInitialSound	= true;
#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->SetMusicVolume( MAX_MUSIC_VOLUMN );
	ZGetSoundEngine()->SetEffectVolume( MAX_EFFECT_VOLUMN );
#endif
	if( m_iChannel != -1 )
	{
		ZGetSoundEngine()->StopSound( m_iChannel );
		m_iChannel = -1;
	}
}

void ZEffectFlashBang::PlaySound()
{
#ifdef _BIRDSOUND
	return;
#else
	FSOUND_SAMPLE* pFS = ZGetSoundEngine()->GetFS("fx_flashbang", true);

	if( pFS == NULL )
	{
		return;
	}

	float restDuration	= mfDuration - ( g_pGame->GetTime() - mfStartTime );

#ifdef _DEBUG
 	mlog("Rest Duration : %f(%f,%f)\n", restDuration, mfStartTime, mfDuration );
#endif

	static int stage = 0;

	switch(stage) 
	{
	case 0:
		if( restDuration >= 5 )
		{
			ZGetSoundEngine()->SetEffectVolume( 0.2 );
			ZGetSoundEngine()->SetMusicVolume( 0.2 );
			stage = 1;
		}
		break;
	case 1:
		if( restDuration < 5 )
		{
			ZGetSoundEngine()->SetEffectVolume( 0.4 * Z_AUDIO_EFFECT_VOLUME );
			ZGetSoundEngine()->SetMusicVolume( 0.4 * Z_AUDIO_BGM_VOLUME );
			stage = 2;
		}
		break;
	case 2:
		if( restDuration < 4 )
		{
			ZGetSoundEngine()->SetEffectVolume( 0.6 * Z_AUDIO_EFFECT_VOLUME );
			ZGetSoundEngine()->SetMusicVolume( 0.6 * Z_AUDIO_BGM_VOLUME );
			stage = 3;
		}
		break;
	case 3:
		if( restDuration < 3 )
		{
			ZGetSoundEngine()->SetEffectVolume( 0.8 * Z_AUDIO_EFFECT_VOLUME );
			ZGetSoundEngine()->SetMusicVolume( 0.8 * Z_AUDIO_BGM_VOLUME );
			stage = 4;
		}
		break;
	case 4:
		if( restDuration < 2 )
		{
			ZGetSoundEngine()->SetEffectVolume( 0.9 * Z_AUDIO_EFFECT_VOLUME );
			ZGetSoundEngine()->SetMusicVolume( 0.9 * Z_AUDIO_BGM_VOLUME );
			stage = 5;
		}
		break;
	case 5:
		if( restDuration < 1 )
		{
			ZGetSoundEngine()->SetEffectVolume( Z_AUDIO_EFFECT_VOLUME );
			ZGetSoundEngine()->SetMusicVolume( Z_AUDIO_BGM_VOLUME );
			stage = 0;
		}
		break;
	}
	
	if( mbInitialSound && pFS != NULL )
	{		
		m_iChannel = ZGetSoundEngine()->PlaySE( pFS, rvector(0,0,0), 200, false, true );
		mbInitialSound	= false;
		if( m_iChannel == -1 ) return;
		ZGetSoundEngine()->SetEffectVolume( m_iChannel, MAX_EFFECT_VOLUMN );
	}
	else if( m_iChannel != -1 )
	{
		mfVolumn	= MAX_EFFECT_VOLUMN *1.5* restDuration / mfDuration;
		mfVolumn	= max( min( 1.0f, mfVolumn ), 0.f );
		ZGetSoundEngine()->SetEffectVolume( m_iChannel, mfVolumn );
	}
#endif
}

ZEffectFlashBang::ZEffectFlashBang()
{
	mbActivated		= false;
	mbInitialSound	= true;
	m_iChannel = -1;
	mfStartTime = 0;
	mfPower = 0;
	mfDuration = 0;
	mfVolumn = 0;
	mpBlurSurface = 0;
	mpDepthBuffer = 0;
	mpHoldDepthBuffer = 0;
	mpHoldBackBuffer = 0;

}

ZEffectFlashBang::~ZEffectFlashBang()
{
}