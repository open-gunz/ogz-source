/*
	WaveFile.h
	----------

	Programming by Chojoongpil
	This Routine Handles Waveform File(s).
*/
#ifndef __WAVEFILE_HEADER__
#define __WAVEFILE_HEADER__

#include <Windows.h>
#include <mmsystem.h>

class Package;

using WAVEFORMATEX = struct tWAVEFORMATEX;

class RSMemWaveFile {
private:
	u8 *m_pImageData;	// 실제 Waveform Data
	u32 m_dwImageLen;	// 실제 Waveform Data Length

	bool m_bResource;	
public:
	RSMemWaveFile();
	virtual ~RSMemWaveFile();

	// Methods
	bool Open( char *szFileName );
	bool Open( u32 uID, HMODULE hMod );
	bool Open( Package *pPackage, int nIndex );
	bool Open( Package *pPackage, char* Name );

	void Close();
	
	// Inline Methods
	// IsValid : Whether valid or invalid?
	bool IsValid() const { return (m_pImageData?true:false); }
	
	// Play : Instant Playback of Waveform Data
	bool Play(bool bAsync = true, bool bLooped = false);
	bool GetFormat( WAVEFORMATEX& wfFormat );
	u32 GetData( u8*& pWaveData, u32 dwMaxLen );
	//u32 GetDataLen(){ return m_dwImageLen; }
	u32 GetSize(){ return m_dwImageLen; }
};

/**
	Streaming을 위한 Wave File Class
	--------------------------------
*/
class RSMWaveFile
{
public:
    RSMWaveFile();
    virtual ~RSMWaveFile();
    bool Open(char* pszFilename);
    bool Cue();
    u32 Read(u8 * pbDest, u32 cbSize);
	u8 GetSilenceData();

	// Inline Functions
    u32 GetNumBytesRemaining(){ return (m_nDataSize - m_nBytesPlayed); }
    u32 GetAvgDataRate(){ return (m_nAvgDataRate); }
    u32 GetDataSize(){ return (m_nDataSize); }
    u32 GetNumBytesPlayed(){ return (m_nBytesPlayed); }
    u32 GetDuration(){ return (m_nDuration); }
    
    WAVEFORMATEX * m_pwfmt;
protected:
    HMMIO m_hmmio;
    MMRESULT m_mmr;
    MMCKINFO m_mmckiRiff;
    MMCKINFO m_mmckiFmt;
    MMCKINFO m_mmckiData;
    u32 m_nDuration;           // duration of sound in msec
    u32 m_nBlockAlign;         // wave data block alignment spec
    u32 m_nAvgDataRate;        // average wave data rate
    u32 m_nDataSize;           // size of data chunk
    u32 m_nBytesPlayed;        // offset into data chunk
};



// (SDK root)\samples\C++\Common\Include\dsutil.h

//-----------------------------------------------------------------------------
// Typing macros 
//-----------------------------------------------------------------------------
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    u32         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    u32         m_dwFlags;
    //bool          m_bIsReadingFromMemory;
    u8*         m_pbData;
    u8*         m_pbDataCur;
    ULONG         m_ulDataSize;
    CHAR*         m_pResourceBuffer;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( LPTSTR strFileName, WAVEFORMATEX* pwfx=NULL, u32 dwFlags=WAVEFILE_READ );
	//HRESULT OpenFromMemory( u8* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, u32 dwFlags=WAVEFILE_READ );
    HRESULT Close();

	bool IsValid() const { return (m_hmmio?true:false); }

    HRESULT Read( u8* pBuffer, u32 dwSizeToRead, u32* pdwSizeRead );
    HRESULT Write( u32 nSizeToWrite, u8* pbData, u32* pnSizeWrote );

    u32   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };
};


#endif