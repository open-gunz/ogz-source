#include "stdafx.h"
#include "RealSoundEffect.h"
#include "RealSound.h"
#include "RealSoundWaveFile.h"
#include <dsound.h>
#include <crtdbg.h>
#include <math.h>
#include "MDebug.h"
#include "dxerr.h"

#define OutputDebugStr(a) __noop

struct RSEDSSET{
public:
	LPDIRECTSOUNDBUFFER		m_pDSB;
	LPDIRECTSOUND3DBUFFER	m_pDS3DB;
	//DS3DBUFFER				m_BufferParams;
public:
	RSEDSSET(){
		m_pDSB = NULL;
		m_pDS3DB = NULL;
	}
	virtual ~RSEDSSET(){
		_ASSERT(m_pDSB==NULL);
		_ASSERT(m_pDS3DB==NULL);
	}
};

RealSoundEffect::RealSoundEffect()
{
	m_pDSSet	= new RSEDSSET;
	m_pFX		= 0;
}

RealSoundEffect::~RealSoundEffect()
{
	if( m_pDSSet != 0 )
		delete m_pDSSet;
	m_pDSSet = NULL;
}

#define DEFAULT_MIN_DISTANCE	3.0f
#define DEFAULT_MAX_DISTANCE	5.0f

bool RealSoundEffect::Create(RealSound* pRealSound, WAVEFILECLASS* pWaveFile, bool b3D)
{
	unsigned long dwDataLen;
	//WAVEFORMATEX wfFormat;
	DSBUFFERDESC dsbd;
	u8* pDSBData;

	dwDataLen = pWaveFile->GetSize();
	//pWaveFile->GetFormat(&wfFormat);

	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize = sizeof(DSBUFFERDESC);

	//dsbd.dwFlags = DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_STATIC;
	// 믹싱할때 소리 왜곡이 생겨서 DSBCAPS_LOCSOFTWARE로 우선 처리
	if(b3D==true) 
	{
		dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE | DSBCAPS_MUTE3DATMAXDISTANCE;
		if( GetFxType() != FX_NONE )
		{
			dsbd.dwFlags	|= DSBCAPS_CTRLFX;
		}
	}
	else 
	{
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC;
	}
	dsbd.dwBufferBytes = dwDataLen;
	//dsbd.lpwfxFormat = &wfFormat;
	//dsbd.lpwfxFormat = NULL;
	dsbd.lpwfxFormat = pWaveFile->GetFormat();

	// 3D Algorithm
	if(b3D==true){
		dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
		//dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
		//dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
	}

	LPDIRECTSOUND pDS = pRealSound->GetDS();

	HRESULT hr = pDS->CreateSoundBuffer( &dsbd, &(m_pDSSet->m_pDSB), NULL );
	if( hr != DS_OK )
	{
		OutputDebugStr( DXGetErrorString8( hr ));
		OutputDebugStr( "\n" );
		dsbd.lpwfxFormat->cbSize			= 0;
		dsbd.lpwfxFormat->nAvgBytesPerSec	= 88200;
		dsbd.lpwfxFormat->nBlockAlign		= 2;
		dsbd.lpwfxFormat->nChannels			= 1;
		dsbd.lpwfxFormat->nSamplesPerSec	= 44100;
		dsbd.lpwfxFormat->wBitsPerSample	= 16;
		dsbd.lpwfxFormat->wFormatTag		= WAVE_FORMAT_PCM;

		if( FAILED ( pDS->CreateSoundBuffer( &dsbd, &(m_pDSSet->m_pDSB), NULL ) ) )
		{
			OutputDebugStr( DXGetErrorString( hr ));
			OutputDebugStr( "\n" );
			OutputDebugStr( "Fail to Create Sound Buffer :" );
			OutputDebugStr( DXGetErrorString8( hr ) );
			OutputDebugStr( "\n" );//*/
			m_pDSSet->m_pDSB = NULL;
			return false;
		}
	}

    // Get the 3D buffer from the secondary buffer
	if(b3D==true){
		if( FAILED( (m_pDSSet->m_pDSB)->QueryInterface( IID_IDirectSound3DBuffer, (VOID**)&(m_pDSSet->m_pDS3DB)) ) ){
			SAFE_RELEASE(m_pDSSet->m_pDSB);
			SAFE_RELEASE(m_pDSSet->m_pDS3DB);
			return false;
		}
		// Get the 3D buffer parameters
		/*
		m_pDSSet->m_BufferParams.dwSize = sizeof(DS3DBUFFER);
		m_pDSSet->m_pDS3DB->GetAllParameters( &(m_pDSSet->m_BufferParams) );
		// Set new 3D buffer parameters
		//m_pDSSet->m_BufferParams.dwMode = DS3DMODE_HEADRELATIVE;
		m_pDSSet->m_BufferParams.dwMode = DS3DMODE_NORMAL;
		//m_pDSSet->m_BufferParams.flMinDistance = DEFAULT_MIN_DISTANCE;
		//m_pDSSet->m_BufferParams.flMaxDistance = DEFAULT_MAX_DISTANCE;
		m_pDSSet->m_pDS3DB->SetAllParameters( &(m_pDSSet->m_BufferParams), DS3D_IMMEDIATE );
		*/
	}
	else{
		m_pDSSet->m_pDS3DB = NULL;
	}

	if( (m_pDSSet->m_pDSB)->Lock( 0, dwDataLen, (LPVOID *)&pDSBData, &dwDataLen, NULL, 0, 0 ) != DS_OK ){
		SAFE_RELEASE(m_pDSSet->m_pDSB);
		SAFE_RELEASE(m_pDSSet->m_pDS3DB);
		return false;
	}
	u32 dwDataLenRead;
	if( FAILED( pWaveFile->ResetFile() ) )
	{
		(m_pDSSet->m_pDSB)->Unlock( pDSBData, dwDataLen, NULL, 0 );
		SAFE_RELEASE((m_pDSSet->m_pDSB));
		SAFE_RELEASE((m_pDSSet->m_pDS3DB));
		return false;
	}

	if( FAILED( pWaveFile->Read(pDSBData, dwDataLen, &dwDataLenRead )) )
	{
		(m_pDSSet->m_pDSB)->Unlock( pDSBData, dwDataLen, NULL, 0 );
		SAFE_RELEASE((m_pDSSet->m_pDSB));
		SAFE_RELEASE((m_pDSSet->m_pDS3DB));
		return false;
	}
	//dwDataLen = pWaveFile->GetData( pDSBData, dwDataLen );

	if( (m_pDSSet->m_pDSB)->Unlock( pDSBData, dwDataLen, NULL, 0 ) != DS_OK ){
		SAFE_RELEASE(m_pDSSet->m_pDSB);
		SAFE_RELEASE(m_pDSSet->m_pDS3DB);
		return false;
	}

	if( GetFxType() != FX_NONE )
	{
		m_pFX	= new RealSoundEffectFx;
		if(m_pFX->Initialize( m_pDSSet->m_pDSB, GetFxType() ))
		{
			mlog("Success to set FX !\n");
		}
	}

	return true;
}

bool RealSoundEffect::Create(RealSound* pRealSound, RealSoundEffect* pSE, bool b3D)
{
	LPDIRECTSOUND pDS = pRealSound->GetDS();

	if( pDS->DuplicateSoundBuffer( pSE->m_pDSSet->m_pDSB, &(m_pDSSet->m_pDSB) ) != DS_OK ){
		m_pDSSet->m_pDSB = NULL;
		return false;
	}

	if(b3D==true){
		if( FAILED( (m_pDSSet->m_pDSB)->QueryInterface( IID_IDirectSound3DBuffer, (VOID**)&(m_pDSSet->m_pDS3DB)) ) ){
			SAFE_RELEASE(m_pDSSet->m_pDSB);
			SAFE_RELEASE(m_pDSSet->m_pDS3DB);
			return false;
		}
	}
	else{
		m_pDSSet->m_pDS3DB = NULL;
	}

	return true;
}

void RealSoundEffect::Destory()
{
	if(m_pDSSet->m_pDSB!=NULL){
		m_pDSSet->m_pDSB->Release();
		m_pDSSet->m_pDSB = NULL;
	}
	if(m_pDSSet->m_pDS3DB!=NULL){
		m_pDSSet->m_pDS3DB->Release();
		m_pDSSet->m_pDS3DB = NULL;
	}
}

bool RealSoundEffect::IsLost()
{
	if(m_pDSSet->m_pDSB!=NULL){
		unsigned long dwStatus;
		m_pDSSet->m_pDSB->GetStatus(&dwStatus);
		return ((dwStatus&DSBSTATUS_BUFFERLOST)?true:false);
	}

	return false;
}

void RealSoundEffect::Restore()
{
	if(m_pDSSet->m_pDSB!=NULL) m_pDSSet->m_pDSB->Restore();
}

bool RealSoundEffect::Play(float x, float y, float z, bool bLoop)
{
	if(IsLost()==true) Restore();

	//SetPosition(x, y, z);

    u32 dwLooped = bLoop ? DSBPLAY_LOOPING : 0L;
    if( FAILED( m_pDSSet->m_pDSB->Play( 0, 0, dwLooped ) ) )
        return false;	

	return true;
}

void RealSoundEffect::Stop()
{
	if(m_pDSSet->m_pDSB!=NULL){
		m_pDSSet->m_pDSB->Stop();
		m_pDSSet->m_pDSB->SetCurrentPosition( 0L );
	}
}

bool RealSoundEffect::IsPlaying()
{
	if(m_pDSSet->m_pDSB!=NULL){
		unsigned long dwStatus;
		MBeginProfile( 33, "RealSoundEffect :: IsPlaying : GetStatus" );
		m_pDSSet->m_pDSB->GetStatus(&dwStatus);
		MEndProfile( 33 );
		return ((dwStatus&DSBSTATUS_PLAYING)?true:false);
	}
	return false;
}

void RealSoundEffect::SetPosition(float x, float y, float z)
{
	if(m_pDSSet->m_pDS3DB!=NULL)
		m_pDSSet->m_pDS3DB->SetPosition(x, y, z, DS3D_IMMEDIATE );
}

void RealSoundEffect::SetVolume(float t)
{
	float fVolumeConstant = t;
#define MINVOLUME	(DSBVOLUME_MIN/2)
	//fVolumeConstant = (float)cos(3.141592*(1.0f-t));
	/*if(m_pDSSet->m_pDSB!=NULL)
		m_pDSSet->m_pDSB->SetVolume(long(MINVOLUME + (DSBVOLUME_MAX-MINVOLUME)*fVolumeConstant));
		*/
	//HRESULT hr = m_pDSSet->m_pDSB->SetVolume((DSBVOLUME_MIN)*t);
	if( m_pDSSet->m_pDSB != NULL )
	{
		//m_pDSSet->m_pDSB->SetVolume( long( DSBVOLUME_MIN + abs(DSBVOLUME_MAX - DSBVOLUME_MIN) * t ));
		HRESULT hr = m_pDSSet->m_pDSB->SetVolume( LinearToLogVol(t) );
		//OutputDebugStr( DXGetErrorString8(hr) );
	}
}

float RealSoundEffect::GetVolume()
{
	long lVolume = 0;
	if(m_pDSSet->m_pDSB!=NULL){
		m_pDSSet->m_pDSB->GetVolume(&lVolume);
		float fVolume = (lVolume-MINVOLUME)/float(DSBVOLUME_MAX-MINVOLUME);
		return fVolume;
	}
	return 0;
}

void RealSoundEffect::SetMinDistance(float fMin)
{
	if(m_pDSSet->m_pDS3DB!=NULL)
		m_pDSSet->m_pDS3DB->SetMinDistance(fMin, DS3D_IMMEDIATE);
}

void RealSoundEffect::SetMaxDistance(float fMax)
{
	if(m_pDSSet->m_pDS3DB!=NULL)
		m_pDSSet->m_pDS3DB->SetMaxDistance(fMax, DS3D_IMMEDIATE);
}

void RealSoundEffect::SetMode(RealSoundEffectMode nMode)
{
	if(m_pDSSet->m_pDS3DB!=NULL){
		u32 dwMode = DS3DMODE_DISABLE;
		if(nMode==RSEM_DISABLE) dwMode = DS3DMODE_DISABLE;
		else if(nMode==RSEM_HEADRELATIVE) dwMode = DS3DMODE_HEADRELATIVE;
		else if(nMode==RSEM_NORMAL) dwMode = DS3DMODE_NORMAL;
		else _ASSERT(FALSE);
		(m_pDSSet->m_pDS3DB)->SetMode(dwMode, DS3D_IMMEDIATE);
	}
}

bool RealSoundEffectSource::LoadWaveFromFile()
{
	if(m_pWaveFile->IsValid()==TRUE) return true;

	if( FAILED(m_pWaveFile->Open(m_szFileName)) ) 
	{
		return false;
	}

	return true;
}
/*
bool RealSoundEffectSource::LoadWaveFromMemory( byte* pBuffer_, int Length )
{
	if( m_pWaveFile->IsValid() == TRUE )
	{
		return true;
	}

	if( FAILED(m_pWaveFile->OpenFromMemory( pBuffer_, (ULONG)Length , 0, WAVEFILE_READ ) ) )
	{
		return false;
	}

	return true;
}
//*/
bool RealSoundEffectSource::LoadWaveFromMemory()
{
	//if( m_pWaveFile->IsValid() == true )
	//{
	//	return true;
	//}

	//if( FAILED(m_pWaveFile->OpenFromMemory

	return true;
}
bool RealSoundEffectSource::LoadWaveFromPak()
{
	if(m_pWaveFile->IsValid()==TRUE) return true;

	/*
	if(m_pWaveFile->Open(m_pPackage ,m_szFileName)==false)
		return false;
	*/
	_ASSERT(FALSE);

	return true;
}


void RealSoundEffectSource::RemoveWaveFromMemory()
{
//	if( !m_pWaveFile->m_bIsReadingFromMemory )
		m_pWaveFile->Close();
}

RealSoundEffect* RealSoundEffectSource::NewRealSoundEffect(bool b3D, E_FX_SOUND_TYPE eType_ )
{
	RealSoundEffect* pRealSoundEffect = new RealSoundEffect();
	pRealSoundEffect->SetFxType( eType_ );

	if(pRealSoundEffect->Create(m_pRealSound, m_pWaveFile, b3D)==false){
		delete pRealSoundEffect;
		return NULL;
	}

	return pRealSoundEffect;
}

RealSoundEffect* RealSoundEffectSource::DupRealSoundEffect(RealSoundEffect* pOriginalRealSoundEffect, bool b3D)
{
	RealSoundEffect* pRealSoundEffect = new RealSoundEffect();

	if(pRealSoundEffect->Create(m_pRealSound, pOriginalRealSoundEffect, b3D)==false){
		delete pRealSoundEffect;
		return NULL;
	}

	return pRealSoundEffect;
}

void RealSoundEffectSource::DelRealSoundEffect(RealSoundEffect* pRealSoundEffect)
{
	pRealSoundEffect->Destory();
	delete pRealSoundEffect;
}

RealSoundEffectSource::RealSoundEffectSource()
{
	m_pWaveFile = new WAVEFILECLASS;
	m_pOriginalRealSoundEffect = NULL;
	m_pPreserveRealSoundEffect = NULL;
	m_nRefCount = 0;
	m_pRealSound = NULL;
	m_bDynamicLoadable = false;
	m_szFileName[0] = NULL;
	m_bIsPakFile = false;
	m_pPackage = NULL;
	m_iPriority	= 0;
}

RealSoundEffectSource::~RealSoundEffectSource()
{
	Destroy();
	delete m_pWaveFile;
}

bool RealSoundEffectSource::Create(RealSound* pRealSound, const char* szFileName, bool bDynamicLoadable, byte* pBuffer, int Length )
{
	Destroy();

	m_pRealSound = pRealSound;

	strcpy_safe(m_szFileName, szFileName);

	m_bDynamicLoadable = bDynamicLoadable;
/*
	if( pBuffer != NULL )
	{
		if( LoadWaveFromMemory( pBuffer, Length ))
		{
			return true;
		}
	}
//*/
	////if( bDynamicLoadable == false )
	//{
	//	if( LoadWaveFromFile())
	//	{
	//		return true;
	//	}
	//}

	//return false;
	return LoadWaveFromFile();
}

bool RealSoundEffectSource::CreatePackage(RealSound* pRealSound, const char* szFileName,Package* pak, bool bDynamicLoadable)
{
	Destroy();

	m_pRealSound = pRealSound;

	strcpy_safe(m_szFileName, szFileName);

	m_bDynamicLoadable = bDynamicLoadable;

	m_pPackage = pak;
	m_bIsPakFile = true;

	if(bDynamicLoadable==false) LoadWaveFromPak();

	return true;
}

void RealSoundEffectSource::Destroy()
{
	_ASSERT(m_nRefCount==0);
	_ASSERT(m_pOriginalRealSoundEffect==NULL);
	_ASSERT(m_pPreserveRealSoundEffect==NULL);

	RemoveWaveFromMemory();
}

RealSoundEffect* RealSoundEffectSource::CreateRealSoundEffect(bool b3D, E_FX_SOUND_TYPE eType_ )
{
	if( m_nRefCount==0 && m_bDynamicLoadable==true){

		//if( m_pWaveFile->m_bIsReadingFromMemory == true )
		//{
		//	if( LoadWaveFromMemory() )
		//	{

		//	}
		//}
		if(m_bIsPakFile) {
			if(LoadWaveFromPak()==false){
				return NULL;
			}
		}
		else {
			if(LoadWaveFromFile()==false){
				return NULL;
			}

		}
	}

	//m_nRefCount++;

	if( eType_ != FX_NONE )
	{
		m_pOriginalRealSoundEffect = NewRealSoundEffect(b3D, eType_ );
		if( m_pOriginalRealSoundEffect != NULL )
		{
			++m_nRefCount;
		}
		return m_pOriginalRealSoundEffect;
	}

	if(m_pPreserveRealSoundEffect!=NULL){
		m_pOriginalRealSoundEffect = m_pPreserveRealSoundEffect;
		m_pPreserveRealSoundEffect = NULL;
		if( m_pOriginalRealSoundEffect != NULL )
		{
			++m_nRefCount;
		}
		return m_pOriginalRealSoundEffect;
	}
	else if(m_pOriginalRealSoundEffect!=NULL){
		//	return DupRealSoundEffect(m_pOriginalRealSoundEffect, b3D);
		RealSoundEffect* pSE = DupRealSoundEffect( m_pOriginalRealSoundEffect, b3D );
		if( pSE != NULL )
		{
			++m_nRefCount;
		}
		return pSE;
	}
	else{
		m_pOriginalRealSoundEffect = NewRealSoundEffect(b3D);
		if( m_pOriginalRealSoundEffect != NULL )
		{
			++m_nRefCount;
		}
		return m_pOriginalRealSoundEffect;
	}
}

void RealSoundEffectSource::DestroyRealSoundEffect(RealSoundEffect* pRealSoundEffect)
{
	_ASSERT(m_pPreserveRealSoundEffect!=pRealSoundEffect);
	_ASSERT(m_pOriginalRealSoundEffect!=NULL);
	if(m_pOriginalRealSoundEffect==pRealSoundEffect && m_nRefCount>1){	// Original RealSoundEffect를 보존해야 하는 경우
		_ASSERT(m_pPreserveRealSoundEffect==NULL);
		m_pPreserveRealSoundEffect = pRealSoundEffect;
	}
	else if(m_pOriginalRealSoundEffect!=pRealSoundEffect && m_nRefCount>1){
		DelRealSoundEffect(pRealSoundEffect);
	}
	else if(m_pOriginalRealSoundEffect!=pRealSoundEffect && m_nRefCount==1){
		DelRealSoundEffect(pRealSoundEffect);

		_ASSERT(m_pOriginalRealSoundEffect==m_pPreserveRealSoundEffect);	// 보존되어 있는 것과 같아야 한다.
		DelRealSoundEffect(m_pOriginalRealSoundEffect);
		m_pOriginalRealSoundEffect = NULL;
		m_pPreserveRealSoundEffect = NULL;
	}
	else{
		_ASSERT(m_pOriginalRealSoundEffect==pRealSoundEffect && m_nRefCount==1);
		DelRealSoundEffect(pRealSoundEffect);
		m_pOriginalRealSoundEffect = NULL;
		m_pPreserveRealSoundEffect = NULL;
	}
	/*
	else{
		DelRealSoundEffect(pRealSoundEffect);
		if(m_pOriginalRealSoundEffect==pRealSoundEffect) m_pOriginalRealSoundEffect = NULL;
		if(m_nRefCount==1){							// 마지막 RealSoundEffect인 경우
			if(m_pOriginalRealSoundEffect!=pRealSoundEffect){				// 마지막이지만 Original이 아닌 경우
				_ASSERT(m_pOriginalRealSoundEffect==m_pPreserveRealSoundEffect);	// 보존되어 있는 것과 같아야 한다.
				DelRealSoundEffect(m_pOriginalRealSoundEffect);
			}
			m_pOriginalRealSoundEffect = NULL;
			m_pPreserveRealSoundEffect = NULL;
		}
	}
	*/

	m_nRefCount --;
	_ASSERT( m_nRefCount >= 0 );

	if(m_nRefCount==0 && m_bDynamicLoadable==true) RemoveWaveFromMemory();
}


RealSoundEffectPlay::RealSoundEffectPlay(RealSoundEffectSource* pSES, bool b3D, float fMinDistance, float fMaxDistance, RealSoundEffectMode nMode, E_FX_SOUND_TYPE eType_ )
: m_pSES(pSES)
{
	_ASSERT(m_pSES!=NULL);
	m_pRSE = m_pSES->CreateRealSoundEffect(b3D, eType_ );

	if(m_pRSE==NULL) return;

	m_pRSE->SetVolume(1.0f);

	m_pRSE->SetMinDistance(fMinDistance);		// Default Minimum Distance
	m_pRSE->SetMaxDistance(fMaxDistance);		// Default Maximum Distance
	m_pRSE->SetMode(nMode);

	m_bLoop = false;
}

RealSoundEffectPlay::~RealSoundEffectPlay()
{
	if(m_pSES!=NULL && m_pRSE!=NULL){
		m_pSES->DestroyRealSoundEffect(m_pRSE);
		m_pRSE = NULL;
	}
}

void RealSoundEffectPlay::Play(float x, float y, float z, bool bLoop)
{
	if(m_pRSE!=NULL) m_pRSE->Play(x, y, z, bLoop);
	else mlog("Real Sound Effect Object is NULL\n");
	m_bLoop = bLoop;
}

void RealSoundEffectPlay::Play()
{
	if( m_pRSE != NULL ) m_pRSE->Play( m_fX, m_fY, m_fZ, m_bLoop );
}

void RealSoundEffectPlay::Stop()
{
	if(m_pRSE!=NULL) m_pRSE->Stop();
}

bool RealSoundEffectPlay::IsPlaying()
{
	if(m_pRSE==NULL) return false;
	return (m_pRSE->IsPlaying()==TRUE)?true:false;
}

bool RealSoundEffectPlay::IsPlayingLoop()
{
	return (IsPlaying()==true && m_bLoop==true);
}

void RealSoundEffectPlay::SetPos(float x, float y, float z)
{
	m_fX = x; m_fY = y; m_fZ = z;
	if(m_pRSE!=NULL) m_pRSE->SetPosition(x, y, z);
}

void RealSoundEffectPlay::SetVolume(float t)
{
	if(m_pRSE==NULL) return;
	m_pRSE->SetVolume(t);
}

float RealSoundEffectPlay::GetVolume()
{
	if(m_pRSE==NULL) return 0;
	return m_pRSE->GetVolume();
}

bool	RealSoundEffectFx::Initialize( LPDIRECTSOUNDBUFFER pDSB_, E_FX_SOUND_TYPE type_ )
{
	CoInitialize(NULL);

	if( pDSB_ == NULL || type_ == FX_NONE )
	{
		return false;
	}

	if( FAILED( pDSB_->QueryInterface( IID_IDirectSoundBuffer8, (VOID**)&(m_pDSB8) )) )
	{
		return false;
	}

	bool brResult = SetFX( type_ );

	HRESULT hr;
	if( m_pReverb8 == 0 )
	{
		hr =  m_pDSB8->GetObjectInPath( GUID_DSFX_WAVES_REVERB, m_EffectIndex[FX_REVERB], IID_IDirectSoundFXWavesReverb8, (LPVOID*)&m_pReverb8 );
		if( FAILED( hr ))
		{
			brResult = false;
		}
	}

	return brResult;
}

void	RealSoundEffectFx::Release()
{
	CoUninitialize();

	if( m_pDSB8 != 0 )
	{
		m_pDSB8->Stop();
		m_pDSB8->SetFX( 0, NULL, NULL );
	}
	SAFE_RELEASE( m_pDSB8 );
	SAFE_RELEASE( m_pReverb8 );
}

bool	RealSoundEffectFx::SetFX( E_FX_SOUND_TYPE type_ )
{
	if( m_pDSB8 == NULL || type_ == FX_NONE )
	{
		return false;
	}

	m_iNumEffect	= 0;

	DSEFFECTDESC EffectDesc;
	memset( &EffectDesc, 0, sizeof(DSEFFECTDESC) );
	EffectDesc.dwSize	= sizeof(DSEFFECTDESC);
	//EffectDesc.dwFlags	= DSFX_LOCSOFTWARE;

	DWORD dwResults;
	HRESULT	hr;

	if( type_ & FX_REVERB )
	{
		EffectDesc.guidDSFXClass	= GUID_DSFX_WAVES_REVERB;

		DWORD status;
		m_pDSB8->GetStatus( &status );
		if( status & DSBSTATUS_BUFFERLOST )
		{
			OutputDebugStr( "DSBSTATUS_BUFFERLOST\n" );
		}
		if( status & DSBSTATUS_LOOPING )
		{
			OutputDebugStr( "DSBSTATUS_LOOPING\n" );
		}
		if( status & DSBSTATUS_PLAYING )
		{
			OutputDebugStr( "DSBSTATUS_PLAYING\n" );
		}
		if( status & DSBSTATUS_LOCSOFTWARE )
		{
			OutputDebugStr( "DSBSTATUS_LOCSOFTWARE\n" );
		}
		if( status & DSBSTATUS_LOCHARDWARE	)
		{
			OutputDebugStr( "DSBSTATUS_LOCHARDWARE\n" );
		}
		if( status & DSBSTATUS_TERMINATED )
		{
			OutputDebugStr( "DSBSTATUS_TERMINATED\n" );
		}

		m_EffectIndex[FX_REVERB]	= m_iNumEffect++;

		hr	= m_pDSB8->SetFX( m_iNumEffect, &EffectDesc, &dwResults );
		if( FAILED( hr ) )
		{
			OutputDebugStr( "Fail to Set Fx : " );
			OutputDebugStr( DXGetErrorString8( hr ) );
			OutputDebugStr( "\n" );
			return false;
		}
	}

	return true;
}

bool RealSoundEffectFx::SetReverbParam( float fInGain_, float fReverbMix_, float fReverbTime_, float fHighFreqRTRatio_ )
{
	if( m_pReverb8	== 0 )
	{
		return false;
	}

	m_paramReverb.fInGain			= fInGain_;
	m_paramReverb.fReverbMix		= fReverbMix_;
	m_paramReverb.fReverbTime		= fReverbTime_;
	m_paramReverb.fHighFreqRTRatio	= fHighFreqRTRatio_;

	if( FAILED( m_pReverb8->SetAllParameters( &m_paramReverb )) )
	{
		return false;
	}

	return true;
}

RealSoundEffectFx::RealSoundEffectFx()
{
	m_iNumEffect	= 0;
	m_pDSB8			= 0;
	m_pReverb8		= 0;
}

RealSoundEffectFx::~RealSoundEffectFx()
{
	Release();
}