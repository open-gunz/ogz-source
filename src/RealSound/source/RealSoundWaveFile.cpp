/*
	WaveFile.cpp
	------------

	Programming by Chojoongpil
	
	This Routine Handles Waveform File(s).	
*/
#include "stdafx.h"
#include <stdio.h>
#include "crtdbg.h"
#include "LibPak.h"
#include "RealSoundWaveFile.h"
#include "MZFileSystem.h"
#include "RealSpace2.h"
#include <mmsystem.h>

using namespace RealSpace2;

//////////////////////////////////////////////////
// Constructor & Destructor 

RSMWaveFile::RSMWaveFile (void)
{
    // Init data members
    m_pwfmt = NULL;
    m_hmmio = NULL;
    m_nBlockAlign= 0;
    m_nAvgDataRate = 0;
    m_nDataSize = 0;
    m_nBytesPlayed = 0;
    m_nDuration = 0;

	ZeroMemory( &m_mmckiRiff, sizeof(MMCKINFO) );
    ZeroMemory( &m_mmckiFmt, sizeof(MMCKINFO) );
    ZeroMemory( &m_mmckiFmt, sizeof(MMCKINFO) );
}

RSMWaveFile::~RSMWaveFile (void)
{
    // Free memory
    if (m_pwfmt)
    {
        GlobalFree (m_pwfmt);
    }
    
    // Close file
    if (m_hmmio)
    {
        mmioClose (m_hmmio, 0);
    }

}


//////////////////////////////////////////////////
// Member Function Implementation

bool RSMWaveFile::Open (char* pszFilename)
{
    WORD cbExtra = 0;    
    bool fRtn = true;
    
    /////////////////////////////////////////////////
	// pszFilename으로 지정된 wave화일을 연다.

    if ((m_hmmio = mmioOpen (pszFilename, NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL)
    {
        m_mmr = MMIOERR_CANNOTOPEN;
        goto OPEN_ERROR;
    }
    
    // Descend into initial chunk ('RIFF')
    if (m_mmr = mmioDescend (m_hmmio, &m_mmckiRiff, NULL, 0))
    {
        goto OPEN_ERROR;
    }
    
    // Validate that it's a WAVE file
    if ((m_mmckiRiff.ckid != FOURCC_RIFF) || (m_mmckiRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
    {
        m_mmr = MMIOERR_INVALIDFILE;
        goto OPEN_ERROR;
    }
    
    // Find format chunk ('fmt '), allocate and fill WAVEFORMATEX structure
    m_mmckiFmt.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (m_mmr = mmioDescend (m_hmmio, &m_mmckiFmt, &m_mmckiRiff, MMIO_FINDCHUNK))
    {
        goto OPEN_ERROR;
    }
    
    // Read the format chunk into temporary structure
    PCMWAVEFORMAT pcmwf;
    if (mmioRead (m_hmmio, (CHAR *) &pcmwf, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
    {
        m_mmr = MMIOERR_CANNOTREAD;
        goto OPEN_ERROR;
    }

    // If format is not PCM, then there are extra bytes appended to WAVEFORMATEX
    if (pcmwf.wf.wFormatTag != WAVE_FORMAT_PCM)
    {
        // Read WORD specifying number of extra bytes
        if (mmioRead (m_hmmio, (LPSTR) &cbExtra, sizeof (cbExtra)) != sizeof(cbExtra))
        {
            m_mmr = MMIOERR_CANNOTREAD;
            goto OPEN_ERROR;
        }
    }

    // Allocate memory for WAVEFORMATEX structure + extra bytes
    // UNDONE: GMEM_FIXED???? use malloc?
    if (m_pwfmt = (WAVEFORMATEX *) GlobalAlloc (GMEM_FIXED, sizeof(WAVEFORMATEX)+cbExtra))
    {
        // Copy bytes from temporary format structure
        memcpy (m_pwfmt, &pcmwf, sizeof(pcmwf));
        m_pwfmt->cbSize = cbExtra;
        
        // Read those extra bytes, append to WAVEFORMATEX structure
        if (cbExtra != 0)
        {
            if ((m_mmr = mmioRead (m_hmmio, (LPSTR) ((u8 *)(m_pwfmt) + sizeof (WAVEFORMATEX)), cbExtra)) != cbExtra)
            {
                // Error reading extra bytes
                m_mmr = MMIOERR_CANNOTREAD;
                goto OPEN_ERROR;
            }
        }
    }
    else
    {
        // Error allocating memory
        m_mmr = MMIOERR_OUTOFMEMORY;
        goto OPEN_ERROR;
    }
    
            
    // Init some member data from format chunk
    m_nBlockAlign = m_pwfmt->nBlockAlign;
    m_nAvgDataRate = m_pwfmt->nAvgBytesPerSec;

    // Ascend out of format chunk
    if (m_mmr = mmioAscend (m_hmmio, &m_mmckiFmt, 0))
    {
        goto OPEN_ERROR;
    }

    // Cue for streaming
    Cue ();
    
    // Init some member data from data chunk
    m_nDataSize = m_mmckiData.cksize;
    //m_nDuration = (m_nDataSize * 1000) / m_nAvgDataRate;
	m_nDuration = (int)((float)((float)m_nDataSize/(float)m_nAvgDataRate))*1000;
	    
    // Successful open!
    goto OPEN_DONE;
    
OPEN_ERROR:
    // Handle all errors here
    fRtn = false;
    if (m_hmmio)
    {
        // Close file
        mmioClose (m_hmmio, 0);
        m_hmmio = NULL;
    }
    if (m_pwfmt)
    {
        // UNDONE: Change here if using malloc
        // Free memory
        GlobalFree (m_pwfmt);
        m_pwfmt = NULL;
    }

OPEN_DONE:
    return (fRtn);
}


// Cue
//
bool RSMWaveFile::Cue (void)
{
    bool fRtn = true;    // assume success

    // Seek to 'data' chunk from beginning of file
    if (mmioSeek (m_hmmio, m_mmckiRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET) != -1)
    {
        // Descend into 'data' chunk
        m_mmckiData.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if ((m_mmr = mmioDescend (m_hmmio, &m_mmckiData, &m_mmckiRiff, MMIO_FINDCHUNK)) == MMSYSERR_NOERROR)
        {
            // Reset byte counter
            m_nBytesPlayed = 0;
        }
        else
        {
            // UNDONE: set m_mmr
            fRtn = false;
        }
    }
    else
    {
        // mmioSeek error
        m_mmr = MMIOERR_CANNOTSEEK;
        fRtn = false;
    }

    return fRtn;
}

// Read
//
// Returns number of bytes actually read.
// On error, returns 0, MMIO error code in m_mmr.
//
UINT RSMWaveFile::Read (u8 * pbDest, UINT cbSize)
{
    MMIOINFO mmioinfo;
    UINT cb;


	memset(&mmioinfo, 0, sizeof(MMIOINFO));

    // Use direct buffer access for reads to maximize performance
    if (m_mmr = mmioGetInfo (m_hmmio, &mmioinfo, 0))
    {
        goto READ_ERROR;
    }
                
    // Limit read size to chunk size
    cbSize = (cbSize > m_mmckiData.cksize) ? m_mmckiData.cksize : cbSize;

    // Adjust chunk size
    m_mmckiData.cksize -= cbSize;

    // Copy bytes from MMIO buffer
    for (cb = 0; cb < cbSize; cb++)
    {
        // Advance buffer if necessary
        if (mmioinfo.pchNext == mmioinfo.pchEndRead)
        {
            if (m_mmr = mmioAdvance (m_hmmio, &mmioinfo, MMIO_READ))
            {
                goto READ_ERROR;
            }
            
            if (mmioinfo.pchNext == mmioinfo.pchEndRead)
            {
                m_mmr = MMIOERR_CANNOTREAD;
                goto READ_ERROR;
            }
        }
            
        // Actual copy
        *(pbDest+cb) = *(mmioinfo.pchNext)++;
    }

    // End direct buffer access
    if (m_mmr = mmioSetInfo (m_hmmio, &mmioinfo, 0))
    {
        goto READ_ERROR;
    }

    // Successful read, keep running total of number of data bytes read
    m_nBytesPlayed += cbSize;
    goto READ_DONE;
    
READ_ERROR:
    cbSize = 0;

READ_DONE:
    return (cbSize);
}


// GetSilenceData
//
// Returns 8 bits of data representing silence for the Wave file format.
//
// Since we are dealing only with PCM format, we can fudge a bit and take
// advantage of the fact that for all PCM formats, silence can be represented
// by a single byte, repeated to make up the proper word size. The actual size
// of a word of wave data depends on the format:
//
// PCM Format       Word Size       Silence Data
// 8-bit mono       1 byte          0x80
// 8-bit stereo     2 bytes         0x8080
// 16-bit mono      2 bytes         0x0000
// 16-bit stereo    4 bytes         0x00000000
//
u8 RSMWaveFile::GetSilenceData (void)
{
    u8 bSilenceData = 0;

    // Silence data depends on format of Wave file
    if (m_pwfmt)
    {
        if (m_pwfmt->wBitsPerSample == 8)
        {
            // For 8-bit formats (unsigned, 0 to 255)
            // Packed u32 = 0x80808080;
            bSilenceData = 0x80;
        }
        else if (m_pwfmt->wBitsPerSample == 16)
        {
            // For 16-bit formats (signed, -32768 to 32767)
            // Packed u32 = 0x00000000;
            bSilenceData = 0x00;
        }
        else
        {
            _ASSERT(0);
        }
    }
    else
    {
		_ASSERT(0);
    }

    return (bSilenceData);
}

//////////////////////////////////////////////////
// RSMemWaveFile

RSMemWaveFile::RSMemWaveFile()
{
	m_pImageData = NULL;
	m_bResource = false;
	m_dwImageLen = 0;
}

RSMemWaveFile::~RSMemWaveFile()
{
	Close();
}

///////////////////////////////////
// Methods

bool RSMemWaveFile::Open( char *szFileName )
{
	FILE *fp;

	Close();
	m_bResource = false;
	
	fopen_s( &fp, szFileName, "rb" );
	if( !fp ){
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot open file. - " );
		OutputDebugString( szFileName );
		OutputDebugString( "\n");
#endif
		return false;
	}
	fseek( fp, 0, SEEK_END );
	m_dwImageLen = ftell( fp );

	m_pImageData = (u8 *)::GlobalLock(::GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, m_dwImageLen));
	if( !m_pImageData ){
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot allocate memory for reading.\n" );
#endif
		return false;
	}
	fseek( fp, 0, SEEK_SET );
	fread( m_pImageData, m_dwImageLen, 1, fp );	// loading from waveform file.
	fclose( fp );

	return true;
}

bool RSMemWaveFile::Open( UINT uID, HMODULE hMod )
{
	Close();

	m_bResource = true;
	
	HRSRC hresInfo;
	hresInfo = ::FindResource( hMod, MAKEINTRESOURCE(uID), "WAVE" );
	if( !hresInfo ){
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot find resource.\n" );
#endif
		return false;
	}
	HGLOBAL hgmemWave = ::LoadResource( hMod, hresInfo );
	
	if( hgmemWave ){
		m_pImageData = (u8 *)::LockResource( hgmemWave );
		m_dwImageLen = ::SizeofResource( hMod, hresInfo );
	} else {
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot load resource.\n" );
#endif
		m_pImageData = NULL;
		m_dwImageLen = 0;

		return false;
	}

	return true;
}

bool RSMemWaveFile::Open( Package *pPackage, int nIndex )
{
	PakData *pPakData;

	if( !pPackage || nIndex < 0 ) return false;
	pPakData = pPackage->GetPakData( nIndex );

	if( !pPakData ){
#ifdef _DEBUG
		OutputDebugString("RSMemWaveFile : (Open) Cannot find pakdata, GetPakData\n");
#endif
		return false;
	}
	m_dwImageLen = pPakData->GetFileSize();	
	m_pImageData = (u8 *)::GlobalLock(::GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, m_dwImageLen));
	if( !m_pImageData ){
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot allocate memory for reading package.\n" );
#endif
		return false;
	}
	pPakData->ReadFile( m_pImageData, m_dwImageLen );

	delete pPakData;	
	
	return true;
}

bool RSMemWaveFile::Open( Package *pPackage, char* Name )
{
	PakData *pPakData = NULL;

	if(!pPackage) return false;

//	if( !pPackage || nIndex < 0 ) return false;
//	pPakData = pPackage->GetPakData( nIndex );
	pPakData = pPackage->GetPakData( Name );

	if( !pPakData ){
#ifdef _DEBUG
		OutputDebugString("RSMemWaveFile : (Open) Cannot find pakdata, GetPakData\n");
#endif
		return false;
	}

	m_dwImageLen = pPakData->GetFileSize();	
	m_pImageData = (u8 *)::GlobalLock(::GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, m_dwImageLen));

	if( !m_pImageData ) {
#ifdef _DEBUG
		OutputDebugString( "RSMemWaveFile : Open, Cannot allocate memory for reading package.\n" );
#endif
		return false;
	}

	pPakData->ReadFile( m_pImageData, m_dwImageLen );

	if(pPakData) {
		delete pPakData;
		pPakData = NULL;
	}
	
	return true;
}

void RSMemWaveFile::Close()
{
	if( m_pImageData ){
		HGLOBAL hgmemWave = ::GlobalHandle( m_pImageData );
		if( hgmemWave ){
			if( m_bResource ){
				::FreeResource( hgmemWave );
			} else {
				::GlobalUnlock( hgmemWave );
				::GlobalFree( hgmemWave );
			}
		}
	}
	m_pImageData = NULL;
	m_dwImageLen = 0;
}

bool RSMemWaveFile::Play(bool bAsync, bool bLooped) {
	if (!IsValid()) {
#ifdef _DEBUG
		OutputDebugString("DsWave : Play, Data is not ready.\n");
#endif
		return false;
	}
	return ::PlaySound((const char*)m_pImageData,
		NULL,
		SND_MEMORY |
		SND_NODEFAULT |
		(bAsync ? SND_ASYNC : SND_SYNC) |
		(bLooped ? (SND_LOOP | SND_ASYNC) : 0)) != FALSE;
}

bool RSMemWaveFile::GetFormat( WAVEFORMATEX& wfFormat )
{
	HMMIO           hmmioIn;
	MMIOINFO		mmioInfo;
	MMCKINFO		ckInRIFF;
	MMCKINFO        ckIn;           // chunk info. for general use.

	//if( !IsValid() ) return false;

	ZeroMemory( &wfFormat, sizeof(WAVEFORMATEX) );
	ZeroMemory( &mmioInfo, sizeof(MMIOINFO) );
	
	// Access memory multimedia file
	mmioInfo.pIOProc = NULL;
	mmioInfo.fccIOProc = FOURCC_MEM;
	mmioInfo.cchBuffer = m_dwImageLen;
	mmioInfo.pchBuffer = (HPSTR) m_pImageData;
	
	if( (hmmioIn = mmioOpen( NULL, &mmioInfo, MMIO_READWRITE )) == NULL ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Memory waveform open failure\n" );
		return false;
	}

	if( mmioDescend( hmmioIn, &ckInRIFF, NULL, 0 ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Error reading.\n" );
		return false;
	}

	if( (ckInRIFF.ckid != FOURCC_RIFF) || (ckInRIFF.fccType != mmioFOURCC('W', 'A', 'V', 'E')) ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, This file is not wave file.\n" );
		return false;
	}

	/* Search the input file for for the 'fmt ' chunk.     */
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( mmioDescend( hmmioIn, &ckIn, &ckInRIFF, MMIO_FINDCHUNK ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Cannot find FMT chunk.\n" );
		return false;	
	}

    if( mmioRead( hmmioIn, (HPSTR)&wfFormat, sizeof(WAVEFORMATEX) ) != sizeof(WAVEFORMATEX) ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Cannot read WAVEFORMATEX data.\n" );
		return false;	
	}
	
	if( mmioAscend( hmmioIn, &ckIn, 0 ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Ascend failure.\n" );
		return false;
	}

	if( mmioClose( hmmioIn, 0 ) != 0 ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Fail to close multimedia file.\n" );
		return false;
	}

	return true;
}

u32 RSMemWaveFile::GetData( u8*& pWaveData, u32 dwMaxLen )
{
	HMMIO           hmmioIn;
	MMIOINFO		mmioInfo;
	MMCKINFO		ckInRIFF;
	MMCKINFO        ckIn;           // chunk info. for general use.
	u32			dwLenToCopy;

	if( !IsValid() ) return false;
	
	ZeroMemory( &mmioInfo, sizeof(MMIOINFO) );
	
	// Access memory multimedia file
	mmioInfo.pIOProc = NULL;
	mmioInfo.fccIOProc = FOURCC_MEM;
	mmioInfo.cchBuffer = m_dwImageLen;
	mmioInfo.pchBuffer = (HPSTR) m_pImageData;
	
	if( (hmmioIn = mmioOpen( NULL, &mmioInfo, MMIO_READWRITE )) == NULL ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, Memory waveform open failure\n" );
		return false;
	}

	if( mmioDescend( hmmioIn, &ckInRIFF, NULL, 0 ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, Error reading.\n" );
		return false;
	}

	if( (ckInRIFF.ckid != FOURCC_RIFF) || (ckInRIFF.fccType != mmioFOURCC('W', 'A', 'V', 'E')) ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, This file is not wave file.\n" );
		return false;
	}

	/* Search the input file for for the 'data' chunk.     */
    ckIn.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if( mmioDescend( hmmioIn, &ckIn, &ckInRIFF, MMIO_FINDCHUNK ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, Cannot find FMT chunk.\n" );
		return false;
	}
	dwLenToCopy = ckIn.cksize;

	if( pWaveData == NULL ){
		pWaveData = (u8 *)::GlobalLock(::GlobalAlloc(GMEM_MOVEABLE, dwLenToCopy));
	} else {
		if( dwMaxLen < dwLenToCopy ) dwLenToCopy = dwMaxLen;
	}

	if( pWaveData == NULL ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, Cannot allocate memory for waveform data.\n" );
		return false;
	}

    if( mmioRead( hmmioIn, (HPSTR) pWaveData, dwLenToCopy ) == -1 ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetData, Cannot read waveform data.\n" );
		return false;	
	}

	if( mmioAscend( hmmioIn, &ckIn, 0 ) != MMSYSERR_NOERROR ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Ascend failure.\n" );
		return false;
	}

	if( mmioClose( hmmioIn, 0 ) != 0 ){
		_RPT0( _CRT_ERROR, "RSMemWaveFile : GetFormat, Fail to close multimedia file.\n" );
		return false;
	}

	return dwLenToCopy;
}





// (SDK root)\samples\C++\Common\Src\dsutil.cpp

//#include "dxerr.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif


//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_pResourceBuffer = NULL;
    m_dwSize  = 0;
//    m_bIsReadingFromMemory = false;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

//    if( !m_bIsReadingFromMemory )
    SAFE_DELETE_ARRAY( m_pwfx );
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open( LPTSTR strFileName, WAVEFORMATEX* pwfx, u32 dwFlags )
{
    HRESULT hr;

    m_dwFlags = dwFlags;
//    m_bIsReadingFromMemory = false;

    if( m_dwFlags == WAVEFILE_READ )
    {
        if( strFileName == NULL )
            return E_INVALIDARG;
        SAFE_DELETE_ARRAY( m_pwfx );

        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ );

        if( NULL == m_hmmio )
        {
//          HRSRC   hResInfo;
//          HGLOBAL hResData;
//          u32   dwSize;
//          VOID*   pvRes;

			MZFile mzf;

			if( !mzf.Open( strFileName, g_pFileSystem ))
			{
				return E_FAIL;
			}

			int iLength	= mzf.GetLength();
			if( iLength <= 0 )
			{
				mzf.Close();
				return E_FAIL;
			}

			m_pResourceBuffer = new CHAR[iLength];
			if( !mzf.Read( m_pResourceBuffer, iLength ))
			{
				SAFE_DELETE_ARRAY( m_pResourceBuffer );
				mzf.Close();
				return E_FAIL;
			}

			mzf.Close();
//*/
/*
            // Loading it as a file failed, so try it as a resource
            if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAVE") ) ) )
            {
                if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAV") ) ) )
                    return E_FAIL;
            }

            if( NULL == ( hResData = LoadResource( NULL, hResInfo ) ) )
				return E_FAIL;

            if( 0 == ( dwSize = SizeofResource( NULL, hResInfo ) ) ) 
				return E_FAIL;

            if( NULL == ( pvRes = LockResource( hResData ) ) )
				return E_FAIL;

            m_pResourceBuffer = new CHAR[ dwSize ];
            memcpy( m_pResourceBuffer, pvRes, dwSize );
//*/
            MMIOINFO mmioInfo;
            ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
            mmioInfo.fccIOProc = FOURCC_MEM;
            //mmioInfo.cchBuffer = dwSize;
			mmioInfo.cchBuffer	= iLength;
            mmioInfo.pchBuffer = (CHAR*) m_pResourceBuffer;

            m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
        }

        if( FAILED( hr = ReadMMIO() ) )
        {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
			return hr;
        }

        if( FAILED( hr = ResetFile() ) )
			return hr;

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;
    }
    else
    {
        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF  | 
                                                  MMIO_READWRITE | 
                                                  MMIO_CREATE );
        if( NULL == m_hmmio )
			return E_FAIL;

        if( FAILED( hr = WriteMMIO( pwfx ) ) )
        {
            mmioClose( m_hmmio, 0 );
			return hr;
        }
                        
        if( FAILED( hr = ResetFile() ) )
			return hr;
    }

    return hr;
}



/*
//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory( u8* pbData, ULONG ulDataSize, 
                                   WAVEFORMATEX* pwfx, u32 dwFlags )
{
    m_pwfx       = pwfx;
    m_ulDataSize = ulDataSize;
    m_pbData     = pbData;
    m_pbDataCur  = m_pbData;
    m_bIsReadingFromMemory = true;
	m_dwFlags	= dwFlags;
    
    if( dwFlags != WAVEFILE_READ )
        return E_NOTIMPL;     

	if( m_hmmio == NULL )
	{
		MMIOINFO mmioInfo;
		ZeroMemory(&mmioInfo, sizeof(mmioInfo));
		mmioInfo.fccIOProc	= FOURCC_MEM;
		mmioInfo.cchBuffer	= ulDataSize;
		mmioInfo.pchBuffer	= (char*)pbData;

		m_hmmio		= mmioOpen( NULL, &mmioInfo, MMIO_READ );

		if( FAILED(ReadMMIO()) )
		{
			mmioClose( m_hmmio, 0 );
			return E_FAIL;
		}

		if( FAILED(ResetFile()) )
		{
			mmioClose( m_hmmio, 0 );
			return E_FAIL;
		}

		m_dwSize	= m_ck.cksize;
	}

	return S_OK;
}
//*/



//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx. 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

    //m_pwfx = NULL;
	SAFE_DELETE_ARRAY( m_pwfx );

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
		return E_FAIL;

    // Check to make sure this is a valid wave file
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
		return E_FAIL;

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
		return E_FAIL;

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
       if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
			return E_FAIL;

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, 
                  sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
		return E_FAIL;

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
        if( NULL == m_pwfx )
			return E_FAIL;

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }
    else
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
			return E_FAIL;

        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx )
			return E_FAIL;

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((u8*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            SAFE_DELETE( m_pwfx );
			return E_FAIL;
        }
    }

    // Ascend the input file out of the 'fmt ' chunk.
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        SAFE_DELETE( m_pwfx );
		return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file 
//-----------------------------------------------------------------------------
u32 CWaveFile::GetSize()
{
    return m_dwSize;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile()
{
   /* if( m_bIsReadingFromMemory )
    {
        m_pbDataCur = m_pbData;

		// Seek to the data
		if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
			SEEK_SET ) )
			return E_FAIL;

		// Search the input file for the 'data' chunk.
		m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
			return E_FAIL;

		m_pbDataCur	+= m_ck.dwDataOffset;
		m_dwSize	= m_ck.cksize;
    }
    else //*/
    {
        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( m_dwFlags == WAVEFILE_READ )
        {
            // Seek to the data
            if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
                            SEEK_SET ) )
				return E_FAIL;

            // Search the input file for the 'data' chunk.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
				return E_FAIL;
        }
        else
        {
            // Create the 'data' chunk that holds the waveform samples.  
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            m_ck.cksize = 0;

            if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) 
				return E_FAIL;

            if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
				return E_FAIL;
        }
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read( u8* pBuffer, u32 dwSizeToRead, u32* pdwSizeRead )
{
	MMIOINFO mmioinfoIn; // current status of m_hmmio

	if( m_hmmio == NULL )
		return CO_E_NOTINITIALIZED;
	if( pBuffer == NULL || pdwSizeRead == NULL )
		return E_INVALIDARG;

	if( pdwSizeRead != NULL )
		*pdwSizeRead = 0;

	if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
		return E_FAIL;

   /* if( m_bIsReadingFromMemory )
    {
        if( m_pbDataCur == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( (u8*)(m_pbDataCur + dwSizeToRead) > 
            (u8*)(m_pbData + m_ulDataSize) )
        {
            dwSizeToRead = m_ulDataSize - (u32)(m_pbDataCur - m_pbData);
        }
        
        CopyMemory( pBuffer, m_pbDataCur, dwSizeToRead );
        
        if( pdwSizeRead != NULL )
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else //*/
    {                
        UINT cbDataIn = dwSizeToRead;
        if( cbDataIn > m_ck.cksize ) 
            cbDataIn = m_ck.cksize;       

        m_ck.cksize -= cbDataIn;
    
        for( u32 cT = 0; cT < cbDataIn; cT++ )
        {
            // Copy the bytes from the io to the buffer.
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
					return E_FAIL;

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
					return E_FAIL;
            }

            // Actual copy.
            *((u8*)pBuffer+cT) = *((u8*)mmioinfoIn.pchNext);
            mmioinfoIn.pchNext++;
        }

        if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
			return E_FAIL;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if( m_dwFlags == WAVEFILE_READ )
    {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
        SAFE_DELETE_ARRAY( m_pResourceBuffer );
    }
    else
    {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
			return E_FAIL;
    
        // Ascend the output file out of the 'data' chunk -- this will cause
        // the chunk size of the 'data' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
			return E_FAIL;
    
        // Do this here instead...
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
			return E_FAIL;
        
        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
			return E_FAIL;
    
        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) 
        {
            u32 dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(u32) );
            mmioAscend( m_hmmio, &m_ck, 0 ); 
        }
    
        // Ascend the output file out of the 'RIFF' chunk -- this will cause
        // the chunk size of the 'RIFF' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
			return E_FAIL;
    
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.  
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.  
//-----------------------------------------------------------------------------
HRESULT CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
    u32    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO ckOut1;
    
    dwFactChunk = (u32)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
		return E_FAIL;
    
    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);   

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
		return E_FAIL;
    
    // Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. 
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
    {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                       sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
			return E_FAIL;
    }   
    else 
    {
        // Write the variable length size.
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) != 
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
			return E_FAIL;
    }  
    
    // Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
		return E_FAIL;
    
    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
		return E_FAIL;
    
    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != 
                    sizeof(dwFactChunk) )
		return E_FAIL;
    
    // Now ascend out of the fact chunk...
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
		return E_FAIL;
       
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Write( UINT nSizeToWrite, u8* pbSrcData, UINT* pnSizeWrote )
{
    UINT cT;

//    if( m_bIsReadingFromMemory )
//        return E_NOTIMPL;
    if( m_hmmio == NULL )
        return CO_E_NOTINITIALIZED;
    if( pnSizeWrote == NULL || pbSrcData == NULL )
        return E_INVALIDARG;

    *pnSizeWrote = 0;
    
    for( cT = 0; cT < nSizeToWrite; cT++ )
    {       
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
        {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
				return E_FAIL;
        }

        *((u8*)m_mmioinfoOut.pchNext) = *((u8*)pbSrcData+cT);
        (u8*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return S_OK;
}

