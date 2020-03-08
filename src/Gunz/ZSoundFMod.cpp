#include "stdafx.h"
#include "ZApplication.h"
#include "ZSoundFMod.h"
#include "MDebug.h"
#include "fmod.h"
#include "fmod_errors.h"

#define DEFAULT_STREAM_BUFFER_LENGTH	1000

ZSoundFMod ZSoundFMod::ms_Instance;

ZSoundFMod* ZSoundFMod::GetInstance() 
{
	return &ms_Instance;
}

ZSoundFMod* ZGetSoundFMod()
{
	return ZSoundFMod::GetInstance();
}

bool ZSoundFMod::Create( HWND hwnd, FSOUND_OUTPUTTYPES type, int maxrate, int minchannels, int maxchannels, int maxsoftwarechannels, unsigned int flag )
{
	if (FSOUND_GetVersion() < FMOD_VERSION)
	{
		mlog("Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
		return false;
	}

	if( !FSOUND_SetHWND( hwnd ))
	{
		mlog("SetHWND: %s\n", FMOD_ErrorString(FSOUND_GetError()));;
	}	

	if( !FSOUND_SetOutput(-1)) // Auto-detect base on operating system
	{
		mlog("SetOutput: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return false;
	}

	if( !FSOUND_SetDriver(0) )	// set Default Sound Card
	{
		mlog("SetDriver: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return false;
	}

	int nDriver = GetNumDriver();
	unsigned int DriverCaps = 0;
	for( int i = 0; i < nDriver; ++i )
	{
		FSOUND_GetDriverCaps( i, &DriverCaps);
		if( DriverCaps & FSOUND_CAPS_HARDWARE )
		{
			mlog( "%d(%s): Hardware Mixing Supported\n", i, GetDriverName(i) );
		}
		else
		{
			mlog( "%d(%s): Hardware Mixing Not Supported\n", i, GetDriverName(i) );
			
			if( maxsoftwarechannels < 16 ) return false;
		}
	}

	if( !FSOUND_SetMinHardwareChannels( minchannels ))
	{
		mlog("SetMinHardwareChannels: %s\n", FMOD_ErrorString(FSOUND_GetError()));;
	}
	if( !FSOUND_SetMaxHardwareChannels( maxchannels ))
	{
		mlog("SetMaxHardwareChannels: %s\n", FMOD_ErrorString(FSOUND_GetError()));;
	}

	if (!FSOUND_Init(maxrate, maxsoftwarechannels, flag))
	{
		mlog("Init: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return false;
	}

	if (!FSOUND_Stream_SetBufferSize(DEFAULT_STREAM_BUFFER_LENGTH))
	{
		mlog("Stream_SetBufferSize: %s\n", FMOD_ErrorString(FSOUND_GetError()));
	}

	mlog("[[[getMaxChannel%d]]]]\n", FSOUND_GetMaxChannels() );

	return true;
}

FSOUND_SAMPLE* ZSoundFMod::LoadWave( char* szSoundFileName, int Flag )
{
	MZFile	mzf;
	int		SoundFileLength = 0;
	char	*Buffer;

	MZFileSystem* pFileSystem = ZApplication::GetFileSystem();

	if(!mzf.Open(szSoundFileName, pFileSystem))
		return nullptr;
    
	SoundFileLength = mzf.GetLength();

	Buffer = new char[SoundFileLength];
	mzf.Read( Buffer, SoundFileLength );
	mzf.Close();

	FSOUND_SAMPLE* r = FSOUND_Sample_Load(FSOUND_FREE, Buffer, (Flag | FSOUND_LOADMEMORY), 0, SoundFileLength);
	if( r == 0 )
	{
		mlog("LoadWave: %s\n", FMOD_ErrorString(FSOUND_GetError()));
	}
	delete Buffer;

	return r;
}

int ZSoundFMod::Play( FSOUND_SAMPLE* pFS, int vol, int priority, bool bLoop )
{
	return Play( pFS, 0, 0, vol, priority, true, bLoop );
}

int ZSoundFMod::Play( FSOUND_SAMPLE* pFS, const rvector* pos, const rvector* vel, int vol, int priority, bool bPlayer, bool bLoop )
{
	if(pFS == NULL) return -1;

	int r;

	if(bPlayer)
		priority = 200;

 	int flag = FSOUND_Sample_GetMode(pFS);
	
	r = FSOUND_PlaySoundEx( FSOUND_FREE, pFS, 0, true );

	if( r == -1 )
	{
		mlog("Play : %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return r;
	}
	else if(!bPlayer)
	{
		FSOUND_3D_SetAttributes( r, (const float*)pos, (const float*)vel );
	}	
	FSOUND_SetPriority( r, priority );
	FSOUND_SetVolume( r, bPlayer?vol*0.6f:vol );
	FSOUND_SetPaused( r, false );

	return r;
}

void ZSoundFMod::SetRollFactor( float f )
{
	FSOUND_3D_SetRolloffFactor( f );
}

void ZSoundFMod::SetDistanceFactor( float f )
{
	FSOUND_3D_SetDistanceFactor( f );
}

void ZSoundFMod::SetDopplerFactor( float f )
{
	FSOUND_3D_SetDopplerFactor( f );
}

void ZSoundFMod::SetVolume( int vol )
{
	if( m_iMusicChannel == -1 )
	{
		FSOUND_SetPaused(FSOUND_ALL, true );
		FSOUND_SetVolume(FSOUND_ALL, vol );
		FSOUND_SetPaused(FSOUND_ALL, false );
	}
	else
	{
		int music_vol = FSOUND_GetVolume( m_iMusicChannel );
		FSOUND_SetPaused(FSOUND_ALL, true );
		FSOUND_SetVolume( FSOUND_ALL, vol );
		FSOUND_SetPaused(FSOUND_ALL, false );
		FSOUND_SetPaused(m_iMusicChannel, true );
		SetVolume( m_iMusicChannel, music_vol );
		FSOUND_SetPaused(m_iMusicChannel, false );
	}
}

void ZSoundFMod::SetVolume( int iChannel, int vol )
{
	if(!FSOUND_SetPaused(iChannel, true ))
	{
		mlog("SetPaused:%s\n",FMOD_ErrorString(FSOUND_GetError()));
	}
	if(!FSOUND_SetVolume( iChannel, vol ))
	{
		mlog("SetVolume:%s\n",FMOD_ErrorString(FSOUND_GetError()));
	}
	FSOUND_SetPaused(iChannel, false );
}

void ZSoundFMod::SetMute( bool m )
{
	FSOUND_SetMute(FSOUND_ALL, m );
}

void ZSoundFMod::SetMute( int iChannel, bool m )
{
	FSOUND_SetMute(iChannel, m);
}

void ZSoundFMod::Free( FSOUND_SAMPLE* pFS )
{
	FSOUND_Sample_Free( pFS );
}

void ZSoundFMod::Close()
{
	CloseMusic();
	FSOUND_Close();
}

void ZSoundFMod::SetMinMaxDistance( FSOUND_SAMPLE* pFS, float min, float max )
{
	FSOUND_Sample_SetMinMaxDistance( pFS, min, max );
}

void ZSoundFMod::SetMinMaxDistance( int nChannel, float min, float max )
{
	FSOUND_3D_SetMinMaxDistance(nChannel, min, max);
}

void ZSoundFMod::Update()
{
	FSOUND_Update();
}

void ZSoundFMod::SetListener(rvector& pos, rvector& vel, float fx, float fy, float fz, float ux, float uy, float uz )
{
	FSOUND_3D_Listener_SetAttributes( (const float*)&pos, NULL, fx, fy, fz, ux, uy, uz );
}
void ZSoundFMod::SetListener(rvector* pos, rvector* vel, float fx, float fy, float fz, float ux, float uy, float uz )
{
	FSOUND_3D_Listener_SetAttributes( (const float*)pos, NULL, fx, fy, fz, ux, uy, uz );
}

void ZSoundFMod::StopSound( int iChannel )
{
	FSOUND_StopSound( iChannel );
}

void ZSoundFMod::StopSound()
{
}

unsigned int ZSoundFMod::GetNumDriver() const
{
	return FSOUND_GetNumDrivers();
}

const char* ZSoundFMod::GetDriverName( int id )
{
	return FSOUND_GetDriverName( id );
}

void ZSoundFMod::StopMusic()
{
	if( m_pStream != 0 )
		FSOUND_Stream_Stop( m_pStream );
}

void ZSoundFMod::CloseMusic()
{
	if( m_pStream != 0 )
		FSOUND_Stream_Close( m_pStream );
	
	m_pStream = 0;
	m_iMusicChannel = -1;
}

void ZSoundFMod::PlayMusic( bool bLoop )
{
	if (m_pStream == 0) return;
	
	if(bLoop) FSOUND_Stream_SetMode(m_pStream,FSOUND_LOOP_NORMAL|FSOUND_NORMAL|FSOUND_LOADMEMORY);
	else FSOUND_Stream_SetMode(m_pStream,FSOUND_LOOP_OFF|FSOUND_NORMAL|FSOUND_LOADMEMORY);
	
		
	m_iMusicChannel = FSOUND_Stream_PlayEx( FSOUND_FREE, m_pStream, 0, true );
	
	FSOUND_SetPriority( m_iMusicChannel, 255 );
	FSOUND_SetPaused( m_iMusicChannel, false );
}

void ZSoundFMod::SetMusicVolume( int vol )
{
	if( m_iMusicChannel != -1)
	{
		FSOUND_SetPaused(m_iMusicChannel, true);
		SetVolume( m_iMusicChannel, vol );
		FSOUND_SetPaused(m_iMusicChannel, false);
	}
}

void ZSoundFMod::SetMusicMute( bool m )
{
	if( m_pStream != 0 )
	{
		if( m ) StopMusic();
		else 
		{
			bool loop;
			unsigned int uiMod = FSOUND_Stream_GetMode(m_pStream);
			if(uiMod & FSOUND_LOOP_NORMAL) loop = true;
			else loop = false;
			PlayMusic( loop );
		}
	}
}

bool ZSoundFMod::OpenStream( void* pData, int Length )
{
	if( m_pStream != 0 )
		CloseMusic();

	m_pStream = FSOUND_Stream_Open( (const char*)pData, FSOUND_HW2D | FSOUND_LOOP_NORMAL | FSOUND_NORMAL | FSOUND_LOADMEMORY, 0, Length);
	FSOUND_Stream_SetEndCallback(m_pStream, (FSOUND_STREAMCALLBACK)STREAM_END_CALLBACK, 0);

	return !(m_pStream == 0);
}

const char* ZSoundFMod::GetStreamName()
{
	if (m_pStream == 0) return 0;
	
	FSOUND_SAMPLE* p = FSOUND_Stream_GetSample( m_pStream );
	return FSOUND_Sample_GetName( p );
}

void ZSoundFMod::SetSamplingBits( FSOUND_SAMPLE* pFS, bool b8Bits )
{
	unsigned int Flag = FSOUND_Sample_GetMode( pFS );

	Flag = Flag & ~(FSOUND_8BITS|FSOUND_16BITS);
	
	if(b8Bits) Flag |= FSOUND_8BITS;
	else Flag |= FSOUND_16BITS;
	
	if( !FSOUND_Sample_SetMode( pFS, Flag ))
	{
		mlog("SetSamplingBits: %s\n", FMOD_ErrorString(FSOUND_GetError()));
	}
}

ZSoundFMod::ZSoundFMod() : m_fnMusicEndCallback(NULL)
{
	m_pStream = 0;
	m_iMusicChannel = -1;
}

ZSoundFMod::~ZSoundFMod()
{
}

void ZSoundFMod::SetPan( int iChannel, float Pan )
{
	Pan = max((min(Pan,1.0f)),-1.0f);
	if(!FSOUND_SetPan( iChannel, (int)(255*((Pan+1)*0.5f))))
	{
		mlog("SetPan: %s\n", FMOD_ErrorString(FSOUND_GetError()));
	}
}

signed char F_CALLBACKAPI ZSoundFMod::STREAM_END_CALLBACK(FSOUND_STREAM *stream, void *buff, int len, int param)
{
	if (ZSoundFMod::GetInstance()->m_fnMusicEndCallback)
		ZSoundFMod::GetInstance()->m_fnMusicEndCallback(ZSoundFMod::GetInstance()->m_pContext);

    return TRUE;
}