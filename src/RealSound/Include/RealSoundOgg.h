#ifndef __REALSOUNDOGG_HEADER__
#define __REALSOUNDOGG_HEADER__

#include <time.h>
#include <math.h>
#include <io.h>

#include "RealSound.h"
#include "MMTimer.h"
#include <dsound.h>
#include <vorbis/vorbisfile.h>


#ifdef _DEBUG
#pragma comment(lib, "ogg_static_d.lib")
#pragma comment(lib, "vorbis_static_d.lib")
#pragma comment(lib, "vorbisfile_static_d.lib")
#else
#pragma comment(lib, "ogg_static.lib")
#pragma comment(lib, "vorbis_static.lib")
#pragma comment(lib, "vorbisfile_static.lib")
#endif
#pragma comment( lib, "dsound.lib" )


class RealSoundOgg
{
private:
protected:
    RealSound* m_pRealSound;

	FILE*				m_fp;
	OggVorbis_File		m_vf;
	LPDIRECTSOUNDBUFFER	m_pDSB;
	DSBUFFERDESC		m_dsbd;

	DWORD		m_nThreadID;
	bool		m_bOpened;
	int			m_nLastSection;				// which half of the buffer are/were
	int			m_nCurSection;				// we playing?
	bool        m_bAlmostDone;            // only one half of the buffer to play
	bool        m_bDone;                  // done playing
	bool        m_bLoop;                  // loop?
	bool		m_bPlaying;
	int			m_nBufSize;
	int			m_nLength;
	bool		m_bCreated;
	int			m_nOggOffset;
	int			m_nOggMaxSize;
	bool		m_bMute;

	HANDLE				m_hThread;
	CRITICAL_SECTION	m_csWrite;
	CRITICAL_SECTION	m_csProcess;
	//
	bool WriteStream(DWORD nSize);

	// ov_callbacks
	static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource);
	static int SeekCallback(void *datasource, ogg_int64_t offset, int whence);
	static int CloseCallback(void *datasource);
	static long TellCallback(void *datasource);

	bool ServiceBuffer();
	bool CreateSoundBuffer();

	static DWORD WINAPI StreamThread(void *);
public:

	RealSoundOgg();
	virtual ~RealSoundOgg();
	bool Create(RealSound* pRealSound);
	void Destroy();
	void Stop();
	void Play(bool bLoop = true);
	bool Open(const char* szFileName);
	bool OpenStream(void* pData, int nMaxSize);
	void Close();
	void SetVolume(float t);
	bool IsPlaying() { return m_bPlaying; }
	void SetMute( bool b );
};
#endif	
