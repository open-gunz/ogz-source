#include "stdafx.h"
#include "RealSoundOgg.h"
#include <windows.h>
#include <process.h>
#include "MDebug.h"

#define OGG_BUF_SERVICE_INTERVAL	250
#define OGG_BUF_SIZE				65536         // buffer length
RealSoundOgg*	g_pRealSoundOgg = NULL;

RealSoundOgg::RealSoundOgg()
{
	m_pDSB = NULL;
	m_pRealSound = NULL;
	m_nBufSize = OGG_BUF_SIZE;
	m_bPlaying = false;
	m_nLength = 0;
	m_nOggOffset = 0;
	m_nOggMaxSize = 0;
	m_fp = NULL;
	g_pRealSoundOgg = this;
	m_bCreated = false;
	m_hThread = 0;
	m_nThreadID = 0;
	m_bMute = false;
}

RealSoundOgg::~RealSoundOgg()
{
	Destroy();
	g_pRealSoundOgg = NULL;
}

bool RealSoundOgg::Create(RealSound* pRealSound)
{
	if (m_bCreated == true) return false;

	m_pRealSound = pRealSound;

	m_bOpened = false;

	InitializeCriticalSection(&m_csWrite);
	InitializeCriticalSection(&m_csProcess);

	m_bCreated = true;
	return true;
}

void RealSoundOgg::Destroy()
{
	if (m_bCreated == false) return;

	Close();



	EnterCriticalSection(&m_csWrite);
	LeaveCriticalSection(&m_csWrite);
	DeleteCriticalSection(&m_csWrite);

	EnterCriticalSection(&m_csProcess);
	LeaveCriticalSection(&m_csProcess);
	DeleteCriticalSection(&m_csProcess);


	m_bCreated = false;
}

void RealSoundOgg::Play(bool bLoop)
{
	if ((!m_bOpened) || (m_pDSB == NULL)) return;
	if (m_bPlaying)
	{
		Stop();
	}

	m_pDSB->SetCurrentPosition(0);
	m_nCurSection = m_nLastSection = 0;
	WriteStream(m_nBufSize*2);

	if( !m_bMute )
		m_pDSB->Play(0,0,DSBPLAY_LOOPING);    
	
	m_bPlaying = true;
	m_bLoop = bLoop;
	m_bDone = false;
	m_bAlmostDone = false;

	//m_hThread = (HANDLE)_beginthread(NULL, 0, StreamThread, 0, (void*)this);
	m_hThread = CreateThread(NULL, 0, StreamThread, (void*)this, 0, &m_nThreadID);
}

void RealSoundOgg::Stop()
{
	if (!m_bOpened) return;

	EnterCriticalSection(&m_csProcess);

	if (m_bPlaying)
	{
		ov_pcm_seek(&m_vf, 0);
		m_pDSB->Stop();
		m_bPlaying = false;
	}
	LeaveCriticalSection(&m_csProcess);

	if (m_hThread) 
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

}


bool RealSoundOgg::CreateSoundBuffer()
{
	vorbis_info *vi = ov_info(&m_vf,-1);

	// set the wave format
	WAVEFORMATEX wfm;

	memset(&wfm, 0, sizeof(wfm));

	wfm.cbSize          = sizeof(wfm);
	wfm.nChannels       = vi->channels;
	wfm.wBitsPerSample  = 16;                    // ogg vorbis is always 16 bit
	wfm.nSamplesPerSec  = vi->rate;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec*wfm.nChannels*2;
	wfm.nBlockAlign     = 2*wfm.nChannels;
	wfm.wFormatTag      = 1;

	// set up the buffer
	HRESULT hr;
	memset(&m_dsbd, 0, sizeof(DSBUFFERDESC));
	m_dsbd.dwSize         = sizeof(DSBUFFERDESC);
	m_dsbd.dwFlags        = DSBCAPS_CTRLVOLUME;
	m_dsbd.dwBufferBytes  = m_nBufSize * 2;
	m_dsbd.lpwfxFormat    = &wfm;
	hr = m_pRealSound->GetDS()->CreateSoundBuffer(&m_dsbd, &m_pDSB, NULL );

	if( hr != DS_OK ) return false;
	return true;
}

bool RealSoundOgg::OpenStream(void* pData, int nMaxSize)
{
	if (m_bOpened) Close();

	m_nOggMaxSize = nMaxSize;
	ov_callbacks callbacks = {ReadCallback, SeekCallback, CloseCallback, TellCallback};

	if (ov_open_callbacks(pData, &m_vf, NULL, 0, callbacks) < 0) 
	{
		m_nOggMaxSize = 0;
		return false;
	}

	m_nLength = (int)ov_time_total(&m_vf, -1) * 1000;

	if (!CreateSoundBuffer())
	{
		m_nOggMaxSize = 0;
		return false;
	}

	m_bOpened = true;

	return true;
}

bool RealSoundOgg::Open(const char* szFileName)
{
	if (m_bOpened) Close();

	FILE* fp = nullptr;
	fopen_s(&fp, szFileName, "rb");
	if (!fp) return false;

	ov_open(fp, &m_vf, NULL, 0);

	if (!CreateSoundBuffer())
	{
		fclose(fp);
		return false;
	}

	m_bOpened = true;

	return true;
}

void RealSoundOgg::Close()
{
	if (!m_bOpened) return;

	Stop();
	ov_clear(&m_vf);

	if (m_fp != NULL) 
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	if (m_pDSB)
	{
		m_pDSB->Release();
		m_pDSB = NULL;
	}

	m_bOpened = false;
}

bool RealSoundOgg::WriteStream(DWORD nSize)
{
	HRESULT hr;
	char    *buf;

	EnterCriticalSection(&m_csWrite);

	hr = m_pDSB->Lock( m_nLastSection * m_nBufSize, nSize, (LPVOID*)&buf, &nSize, NULL, NULL, 0 );
	if (hr != DS_OK) return false;

	DWORD   pos = 0;
	int     sec = 0;
	int     ret = 1;

	while((ret) && (pos < nSize))
	{
		ret = ov_read(&m_vf, buf + pos, nSize - pos, 0, 2, 1, &sec);
		pos += ret;
	}

	if (!ret)
	{
		if (m_bLoop)
		{
			ret = 1;
			ov_pcm_seek(&m_vf, 0);
			while((ret) && (pos < nSize))
			{
				ret = ov_read(&m_vf, buf + pos, nSize - pos, 0, 2, 1, &sec);
				pos += ret;
			}
		}
		else
		{
			while(pos < nSize) 
			{
				*(buf+pos)=0; 
				pos++;
			}

			m_bAlmostDone = true;
		}
	}

	m_pDSB->Unlock( buf, nSize, NULL, NULL );

	m_nLastSection = m_nCurSection;

	LeaveCriticalSection(&m_csWrite);

	return true;
}

bool RealSoundOgg::ServiceBuffer()
{
	if( m_bMute ) return true;
	if (m_pDSB == NULL) return false;

	DWORD pos = 0;
	m_pDSB->GetCurrentPosition(&pos, NULL);
	m_nCurSection = ((int)pos < m_nBufSize) ? 0:1;

	if (m_nCurSection != m_nLastSection)
	{
		if (m_bDone && !m_bLoop) 
		{
			if (m_bOpened)
			{
				if (m_bPlaying)
				{
					ov_pcm_seek(&m_vf, 0);
					m_pDSB->Stop();
					m_bPlaying = false;
				}
			}

			return false;
		}
		if (m_bAlmostDone && !m_bLoop) m_bDone = true;

		WriteStream(m_nBufSize);
	}

	return true;
}

size_t RealSoundOgg::ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	char* temp = (char*)datasource;
	unsigned long bytesread;
	bytesread = (unsigned long)(size*nmemb);

	if ((g_pRealSoundOgg->m_nOggOffset + (int)bytesread) >= g_pRealSoundOgg->m_nOggMaxSize)
	{
		bytesread = (unsigned long)(g_pRealSoundOgg->m_nOggMaxSize - g_pRealSoundOgg->m_nOggOffset);
	}
	memcpy(ptr, &temp[g_pRealSoundOgg->m_nOggOffset], bytesread);
	g_pRealSoundOgg->m_nOggOffset += bytesread;
	return bytesread/size;

}
int RealSoundOgg::SeekCallback(void *datasource, ogg_int64_t offset, int whence)
{
	switch(whence)
	{
	case SEEK_SET:
		g_pRealSoundOgg->m_nOggOffset = (int)offset;
		break;
	case SEEK_CUR:
		g_pRealSoundOgg->m_nOggOffset = (int)(g_pRealSoundOgg->m_nOggOffset + offset);
		break;
	case SEEK_END:
		g_pRealSoundOgg->m_nOggOffset = (int)(g_pRealSoundOgg->m_nOggMaxSize - offset);
		break;
	}

	return 0;
}

int RealSoundOgg::CloseCallback(void *datasource)
{
	g_pRealSoundOgg->m_nOggOffset = 0;
	return 0;
}

long RealSoundOgg::TellCallback(void *datasource)
{
	return g_pRealSoundOgg->m_nOggOffset;
}

void RealSoundOgg::SetVolume(float t)
{
	float fVolumeConstant = t;
#define MIN	(DSBVOLUME_MIN/2)
	if(m_pDSB!=NULL) m_pDSB->SetVolume(LinearToLogVol((double)t));
}

DWORD WINAPI RealSoundOgg::StreamThread(void *pParam)
{

	RealSoundOgg* pRealSoundOgg = (RealSoundOgg*)pParam;
	BOOL ret;
	do 
	{
		ret = FALSE;
		Sleep(OGG_BUF_SERVICE_INTERVAL);

		EnterCriticalSection(&pRealSoundOgg->m_csProcess);

		if ((pRealSoundOgg != NULL) && (pRealSoundOgg->IsPlaying() == true))
		{
			ret = pRealSoundOgg->ServiceBuffer();
		}

		LeaveCriticalSection(&pRealSoundOgg->m_csProcess);

	} while(ret == TRUE);



	ExitThread(0);
	return 0;
}

void RealSoundOgg::SetMute( bool b )
{
	if( m_bMute == b ) return;
	
	m_bMute = b;

	if( !m_pDSB ) return;

	if( b ) // Mute
	{
		EnterCriticalSection(&m_csProcess);
		m_pDSB->Stop();
		LeaveCriticalSection(&m_csProcess);
	}
	else // play
	{
		EnterCriticalSection(&m_csProcess);
		m_pDSB->SetCurrentPosition(0L);
		m_pDSB->Play( 0,0,DSBPLAY_LOOPING );
		LeaveCriticalSection(&m_csProcess);
	}
}