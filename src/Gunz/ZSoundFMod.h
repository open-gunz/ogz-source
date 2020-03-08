#pragma once

#include "FMod.h"
#include "rtypes.h"

struct FSOUND_SAMPLE;

#define SOUND_DRIVER_NAME_MAX_LENGTH 32

typedef void(ZFMOD_CALLBACK)(void* pCallbackContext);

typedef FSOUND_STREAM* MicStream;

class ZSoundFMod
{
protected:
	static ZSoundFMod	ms_Instance;

protected:
	FSOUND_STREAM*	m_pStream;
	int				m_iMusicChannel;
	bool			m_b16Bit;

	// callback
	static signed char F_CALLBACKAPI STREAM_END_CALLBACK(FSOUND_STREAM *stream, void *buff, int len, int param);
	ZFMOD_CALLBACK*		m_fnMusicEndCallback;
	void*				m_pContext;
public:
	// Initialize & UnInitialize
	bool Create( HWND hwnd, FSOUND_OUTPUTTYPES type = FSOUND_OUTPUT_DSOUND, int maxrate = 44100, int minchannels = 16, int maxchannels = 16, int maxsoftwarechannels = 20, unsigned int flag = 0);	
	void Close();

	// Effect
	FSOUND_SAMPLE* LoadWave( char* szSoundFileName, int Flag );
	void Free( FSOUND_SAMPLE* pFS );
	int	Play(FSOUND_SAMPLE* pFS, int nVol, int nPriority, bool bLoop);
	int Play( FSOUND_SAMPLE* pFS, const rvector* pos, const rvector* vel, int vol, int priority, bool bPlayer, bool bLoop = false );
	void StopSound( int iChannel );
	void StopSound();
	void SetVolume( int vol );
	void SetVolume( int iChannel, int vol );
	void SetMute( bool m );
	void SetMute( int iChannel, bool m );
	void SetSamplingBits( FSOUND_SAMPLE* pFS, bool b8Bits );
	
	// 3D
	void SetRollFactor( float f = 1.0f );
	void SetDistanceFactor( float f = 1.0f );
	void SetDopplerFactor( float f = 1.0f );
	void SetListener(rvector& pos, rvector& vel, float fx, float fy, float fz, float ux, float uy, float uz );
	void SetListener(rvector* pos, rvector* vel, float fx, float fy, float fz, float ux, float uy, float uz );
	void Update();
	void SetMinMaxDistance( FSOUND_SAMPLE* pFS, float min, float max );
	void SetMinMaxDistance( int nChannel, float min, float max );
	//void Set3DAttribute(rvector* pos, rvector* vel);	

	//2D
	void SetPan( int iChannel, float Pan /*-1~1*/ );
	
	// Info
	const char* GetDriverName( int id );
	unsigned int GetNumDriver() const;
	const char* GetStreamName();

	// Music
	void StopMusic();
	void CloseMusic();
	void PlayMusic( bool bLoop );
	void SetMusicVolume( int vol );
	void SetMusicMute( bool m );
	bool OpenStream( void* pData, int Length );

	// ETC
	static ZSoundFMod* GetInstance();

	// callback
	void SetMusicEndCallback(ZFMOD_CALLBACK* func, void* pContext) { m_fnMusicEndCallback = func; m_pContext=pContext; }
	
public:
	ZSoundFMod();
	~ZSoundFMod();

};

// Global Method
ZSoundFMod* ZGetSoundFMod();