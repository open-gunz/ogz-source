/*
	RealSoundBgm.cpp
	-------------

	Backgound Music Waveform Object Handling
	
	WaGe는 4개의 트랙을 관리하며, 그중 한개를 그 길이가 긴 웨이브
	화일을 위해 할당한다. 
	이는 길이가 긴 음성 출력, 또는 음악 화일을 위해 사용하게 되며
	그 음질은 22kHz, 16bit Stereo Sampling을 기준으로 한다.
	BGM Object는 Pannig등의 효과를 줄수 없으며 오직 볼륨에 대한 조
	정을 할 수 있다.

	Programming by Chojoongpil
	All copyright 1996 (c), MAIET entertainment software
*/
#include "stdafx.h"
#include <crtdbg.h>
//#include "..\CoreLib\CMLog.h"
#include "RealSoundBgm.h"
#include <math.h>

#ifdef _DEBUG
	#define _D		::OutputDebugString
#else 
	#define _D		
#endif

/////////////////////////////////////////
// RealSoundBgm Class Implement

// The following constants are the defaults for our streaming buffer operation.
const UINT DefBufferLength          = 1000;	// default buffer length in msec
const UINT DefBufferServiceInterval = 250;	// default bSuffer service interval in msec

//////////////////////////////////////
// Constructor & Destructor

RealSoundBgm::RealSoundBgm()
{
    // Initialize data members
    m_pRealSound = NULL;					// WaGe (Waveform Generator) Object's Pointer
    m_pwavefile = NULL;				// Wavefile Object's Pointer
    m_pdsb = NULL;					// BGM 데이터를 저장할 DirectSound Buffer
    m_ptimer = NULL;				// Multimedia Timer

    m_bLooped = m_fPlaying = m_fCued = FALSE;	// Semaphores for Streaming
    m_lInService = FALSE;			// Semaphore

    m_cbBufOffset = 0;				// Write Position
    m_nBufLength = DefBufferLength;	// Sound Buffer의 길이 (in msec) ?????
    m_cbBufSize = 0;				// Waveform Object Wave Buffer Size
    m_nBufService = DefBufferServiceInterval;	// Service Interval
    m_nDuration = 0;				// duration of wave file ( in KB(s) )
	/*
	    m_nDataSize = m_mmckiData.cksize;
	    m_nDuration = (m_nDataSize * 1000) / m_nAvgDataRate;
	*/
    m_nTimeStarted = 0;				// Play가 시작된 시스템 시간.
    m_nTimeElapsed = 0;				// Play된 이후의 경과 시간.	
}

RealSoundBgm::~RealSoundBgm()
{
    Destroy ();
}

//////////////////////////////////////
// Member Functions Implementation

// Multimedia Timer객체를 위한 Callback함수.
bool RealSoundBgm::TimerCallback (uintptr_t dwUser)
{
    RealSoundBgm * pas = (RealSoundBgm *) dwUser;	// C++에서의 Callback함수를 위해 이와 같은 방법을 사용하였다.
	if( pas ) return pas->ServiceBuffer(); else return FALSE;
}

/*
	pszFilename으로 주어진 Waveform file을 기초로 BGM Object를 구성한다.

	pszFilename : Wave file name
	pRealSound       : WaGe object pointer
*/
bool RealSoundBgm::Create (LPSTR pszFilename, RealSound* pRealSound)
{
	Destroy();

#ifdef _DEBUG
    _ASSERT (pszFilename);
    _ASSERT (pRealSound);
#endif	   
    m_pRealSound = pRealSound;

	if( !(m_pwavefile = new RSMWaveFile) ){
		_D("RealSoundBgm::Create error : Cannot Create Wave Object\n");
		return FALSE;
	}
	
	if( !m_pwavefile->Open( pszFilename ) ){
		delete (m_pwavefile);
		m_pwavefile = NULL;
		return FALSE;
	}	
	
	m_cbBufSize = (m_pwavefile->GetAvgDataRate () * m_nBufLength) / 1000;	

	m_cbBufSize = (m_cbBufSize > m_pwavefile->GetDataSize ()) ? m_pwavefile->GetDataSize () : m_cbBufSize;

	// Get duration of sound
	m_nDuration = m_pwavefile->GetDuration();
	                
	// Create sound buffer
	HRESULT hr;
	memset (&m_dsbd, 0, sizeof (DSBUFFERDESC));
	m_dsbd.dwSize = sizeof (DSBUFFERDESC);
	m_dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
	m_dsbd.dwBufferBytes = m_cbBufSize;
	m_dsbd.lpwfxFormat = m_pwavefile->m_pwfmt;
	hr = (m_pRealSound->GetDS())->CreateSoundBuffer (&m_dsbd, &m_pdsb, NULL);
	
	if( hr != DS_OK ){
		_D("RealSoundBgm::Create error : Cannot create sound buffer.\n");
		return FALSE;
	}

	Cue();	// 준비신호를 준다.
        
    return TRUE;
}

/*
	Destroy

	Destroy A BGM Object.
*/
void RealSoundBgm::Destroy (void)
{
    Stop ();	// if playing, stop it!
	m_bLooped = m_fPlaying = FALSE;
	        
    if (m_pdsb){		
        m_pdsb->Release ();		// release DirectSoundBuffer for BGM.
        m_pdsb = NULL;
    }
	    
    if (m_pwavefile){
        delete (m_pwavefile);	// Destroy Wavefile Object...
        m_pwavefile = NULL;
    }
}

/*
	Cue

	Play를 위해서 신호를 준다. (ready시킨다.)
*/
void RealSoundBgm::Cue (void)
{   
    if (!m_fCued)
    {	        
        m_cbBufOffset = 0;				// Write Position을 zero로 리셋시킨다.		        
        m_pwavefile->Cue ();			// File Pointer역시 리셋.        
        m_pdsb->SetCurrentPosition (0); // Buffer상태를 초기화. 
        WriteWaveData (m_cbBufSize);	// Fill buffer with wave data        
        m_fCued = TRUE;					// Semaphore True로 초기화.
    }
}

/*
	WriteWaveData

	size만큼 Sound Buffer에 Wavedata를 기록한다.
*/
bool RealSoundBgm::WriteWaveData (UINT size)
{
    HRESULT hr;
    LPBYTE lpbuf1 = NULL;
    LPBYTE lpbuf2 = NULL;
    unsigned long dwsize1 = 0;
    unsigned long dwsize2 = 0;
    u32 dwbyteswritten1 = 0;
    u32 dwbyteswritten2 = 0;
    
    // Lock the sound buffer
    hr = m_pdsb->Lock(m_cbBufOffset, size, (void **)&lpbuf1, &dwsize1, (void **)&lpbuf2, &dwsize2, 0);
	if( hr != DS_OK ) return FALSE;
   
	// Write data to sound buffer. Because the sound buffer is circular, we may have to
	// do two write operations if locked portion of buffer wraps around to start of buffer.

	_ASSERT (lpbuf1);	
	if ((dwbyteswritten1 = m_pwavefile->Read (lpbuf1, dwsize1)) != dwsize1) return FALSE;
		// Second write required?
	
	if ( lpbuf2 )	// 만일 2중 버퍼까지 써야 한다면 다음 동작을 행한다.
		if ((dwbyteswritten2 = m_pwavefile->Read (lpbuf2, dwsize2)) != dwsize2) 
			return FALSE;	

	// Update Write Cursor
    m_cbBufOffset = (m_cbBufOffset + dwbyteswritten1 + dwbyteswritten2) % m_cbBufSize;

    m_pdsb->Unlock (lpbuf1, dwbyteswritten1, lpbuf2, dwbyteswritten2);
	
    return TRUE;
}

/*
	WriteSilence

	size만큼의 Silence데이터를 써넣는다.
*/
bool RealSoundBgm::WriteSilence (UINT size)
{	
    HRESULT hr;
    LPBYTE lpbuf1 = NULL;
    LPBYTE lpbuf2 = NULL;
	unsigned long dwsize1 = 0;
	unsigned long dwsize2 = 0;
    u32 dwbyteswritten1 = 0;
    u32 dwbyteswritten2 = 0;
        
    hr = m_pdsb->Lock(m_cbBufOffset, size, (void **)&lpbuf1, &dwsize1, (void **)&lpbuf2, &dwsize2, 0);
	if( hr != DS_OK ){
		_D("RealSoundBgm::WriteSilence error : Unable to lock.\n");
		return FALSE;
	}

	BYTE bSilence = m_pwavefile->GetSilenceData ();
        
    memset(lpbuf1, bSilence, dwsize1);	// 1차 버퍼에 쓰기 동작을 행한다. (Silence Data)
    
	dwbyteswritten1 = dwsize1;
        
    if (lpbuf2){	// 2nd Buffer가 필요하다면 2차 버퍼도 쓰기 동작을 행한다. (Silence Data)
        memset (lpbuf1, bSilence, dwsize1);
        dwbyteswritten2 = dwsize2;
    }
	
    // Write Cursor Position을 Update한다.
    m_cbBufOffset = (m_cbBufOffset + dwbyteswritten1 + dwbyteswritten2) % m_cbBufSize;
    m_pdsb->Unlock (lpbuf1, dwbyteswritten1, lpbuf2, dwbyteswritten2);

    return TRUE;
}


/*
	GetMaxWriteSize
	
	쓰기 동작을 위한 Sound Buffer의 최대 크기를 구한다.
*/
u32 RealSoundBgm::GetMaxWriteSize (void)
{
	unsigned long dwWriteCursor, dwPlayCursor, dwMaxSize;
	
	if (m_pdsb->GetCurrentPosition (&dwPlayCursor, &dwWriteCursor) != DS_OK){
		_D("Fatal ! : Unable To Get Maximum Writable Size\n");
		dwMaxSize = 0;
	}

    if (m_cbBufOffset <= dwPlayCursor)	// 쓰기 커서의 위치가 플레이 커서의 위치보다 작으면
    {
        // Our write position trails play cursor
        dwMaxSize = dwPlayCursor - m_cbBufOffset;
    }

    else // (m_cbBufOffset > dwPlayCursor)
    {	// Play Cursor를 Wrapping시킨다. (Sound Buffer는 환형이다.)
        dwMaxSize = m_cbBufSize - m_cbBufOffset + dwPlayCursor;
    }
	       
    return (dwMaxSize);
}

/*
	Play()

	Create된 객체를 실행시킨다.
	Play는 그 객체의 처음부터 재생하게 된다.
*/
void RealSoundBgm::Play( bool bLoop )
{
    if (m_pdsb)						// Buffer가 존재해야 한다.
    {
        if (m_fPlaying) Stop ();	// Play중일때엔 연주 중지.
        if (!m_fCued) Cue ();		// 만일 초기화 되어 있지 않다면 Buffer에 대한 초기화를 한다.
        HRESULT hr = m_pdsb->Play (0, 0, DSBPLAY_LOOPING);	// Streaming 기법에서는 버퍼를 루핑하여
															// 연주하는 기법을 사용한다.
															// Sound Buffer는 환형으로 존재하고 우리는
															// 멀티미디어 타이머를 이용하여 이를 갱신하여
															// 작은 버퍼를 사용하여 큰 데이터를 연주할 수
															// 있게 한다.
        if( hr != DS_OK ) _D("RealSoundBgm::Play : Fail to play\n");
		
		m_nTimeStarted = GetGlobalTimeMS ();
		m_ptimer = new MMTimer ();	// 버퍼 관리를 위하여 Multimedia Timer를 구동시킨다.
		if (m_ptimer){
			if( !m_ptimer->Create (m_nBufService, m_nBufService, u32 (this), TimerCallback) ){
				m_pdsb->Stop ();		
		        delete (m_ptimer);
				m_ptimer = NULL;
				m_fPlaying = FALSE;

				Cue();
				//LOG("RealSoundBgm::Play Error : Unable to create multimedia timer. GetLastError() = %d.\n", GetLastError());
				//_D ("RealSoundBgm::Play Error : Unable to create multimedia timer.\n");
				return;
			}
		} else {
			m_pdsb->Stop ();		
	        delete (m_ptimer);
			m_ptimer = NULL;
			m_fPlaying = FALSE;

			Cue();
			_D ("RealSoundBgm::Play Error : Unable to create multimedia timer.\n");
			return;
		}
		
		m_fPlaying = TRUE;	// Playing Flag를 셋트시킨다.
		m_fCued = FALSE;	// Cue flag는 꺼버린다. (이미 플레이는 시작되었다.)
		m_bLooped = bLoop;
    }
}

/*
	Stop

	BGM의 연주를 중지시킨다. 초기상태로 모두 전환시킨다.
*/
void RealSoundBgm::Stop()
{
    if (m_fPlaying)
    {		
        m_pdsb->Stop ();		
        delete (m_ptimer);
        m_ptimer = NULL;
        m_fPlaying = FALSE;
    }
}

/*
	ServiceBuffer

	타이머에 의해 실행되는 버퍼 관리 루틴, RealSoundBgm의 핵심 파트이다.
	Boolean형의 결과값을 반환한다. 정상인 경우 -> TRUE, 비정상 실행인 경우 -> FALSE
*/
bool RealSoundBgm::ServiceBuffer (void)
{
    bool fRtn = TRUE;

	//LOG("RealSoundBgm::ServiceBuffer...\n");

    // Check for reentrance
    if (InterlockedExchange (&m_lInService, TRUE) == FALSE)
    {		
        // Not reentered, proceed normally
		// Maintain elapsed time count
        m_nTimeElapsed = GetGlobalTimeMS () - m_nTimeStarted;

        // Stop if all of sound has played
        if (m_nTimeElapsed < m_nDuration)
        {
            // All of sound not played yet, send more data to buffer
            u32 dwFreeSpace = GetMaxWriteSize ();

            // Determine free space in sound buffer
            if (dwFreeSpace)
            {
                // See how much wave data remains to be sent to buffer
                u32 dwDataRemaining = m_pwavefile->GetNumBytesRemaining ();
                if (dwDataRemaining == 0)
                {
                    // All wave data has been sent to buffer
                    // Fill free space with silence
                    if (WriteSilence (dwFreeSpace) == FALSE)
                    {
                        // Error writing silence data
                        fRtn = FALSE;
                        _ASSERT (0);
                        _D("RealSoundBgm::ServiceBuffer Error : WriteSilence failed\n");
                    }
                }
                else if (dwDataRemaining >= dwFreeSpace)
                {
                    // Enough wave data remains to fill free space in buffer
                    // Fill free space in buffer with wave data
                    if (WriteWaveData (dwFreeSpace) == FALSE)
                    {
                        // Error writing wave data
                        fRtn = FALSE;
                        _ASSERT (0);
                        _D("RealSoundBgm::ServiceBuffer Error : WriteWaveData failed\n");
                    }
                }
                else
                {
                    // Some wave data remains, but not enough to fill free space
                    // Send wave data to buffer, fill remainder of free space with silence
                    if (WriteWaveData (dwDataRemaining) == TRUE)
                    {
                        if (WriteSilence (dwFreeSpace - dwDataRemaining) == FALSE)
                        {
                            // Error writing silence data
                            fRtn = FALSE;
                            _ASSERT (0);
                            _D("RealSoundBgm::ServiceBuffer Error : WriteSilence failed\n");
                        }
                    }
                    else
                    {
                        // Error writing wave data
                        fRtn = FALSE;
                        _ASSERT (0);
                        _D("RealSoundBgm::ServiceBuffer Error : WriteWaveData failed\n");
                    }
                }
            }
            else
            {
                // No free space in buffer for some reason
                fRtn = FALSE;
            }
        }
        else
        {
			//LOG("RealSoundBgm::ServiceBuffer, Wrapping Around.\n");
            // All of sound has played, stop playback            
			if( m_bLooped ){
				Play(TRUE);
			} else Stop();
        }

        // Reset reentrancy semaphore
        InterlockedExchange (&m_lInService, FALSE);
    }
    else
    {
		//LOG("RealSoundBgm::ServiceBuffer, Cannot InterlockedExchange.\n");
        // Service routine reentered. Do nothing, just return
        fRtn = FALSE;
    }
    return (fRtn);
}

bool RealSoundBgm::IsPlaying(void)
{
	return m_fPlaying;
}

void RealSoundBgm::SetVolume(float t)
{
	float fVolumeConstant = t;
	//fVolumeConstant = (float)cos(3.141592*(1.0f-t));
#define MIN	(DSBVOLUME_MIN/2)
	if(m_pdsb!=NULL) m_pdsb->SetVolume(long(MIN + (DSBVOLUME_MAX-MIN)*fVolumeConstant));
}
