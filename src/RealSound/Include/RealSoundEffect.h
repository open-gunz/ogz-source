#ifndef REALSOUNDEFFECT_H
#define REALSOUNDEFFECT_H

class RSMemWaveFile;
class RealSound;

struct RSEDSSET;
class CWaveFile;

#include "RealSoundDef.h"
//#include "mempool.h"
#include "dsound.h"

//#define WAVEFILECLASS	RSMemWaveFile
#define WAVEFILECLASS	CWaveFile

enum E_FX_SOUND_TYPE
{
	FX_NONE		= 0,
	FX_REVERB	,
	FX_TEMP,	// NEVER USE THIS
	FX_NUM		= FX_TEMP -1,
};

class RealSoundEffectFx;

class RealSoundEffect// : public CMemPool<RealSoundEffect>
{
protected:
	friend class RealSoundEffectSource;
	RSEDSSET*			m_pDSSet;
	RealSoundEffectFx*	m_pFX;
	E_FX_SOUND_TYPE		m_FxType;

protected:
	//bool Create(RealSound* pRealSound, WAVEFILECLASS* pWaveFile, bool b3D);
	bool Create(RealSound* pRealSound, CWaveFile* pWaveFile, bool b3D);
	bool Create(RealSound* pRealSound, RealSoundEffect* pSE, bool b3D);
	void Destory(void);
	bool IsLost(void);
	void Restore(void);

	// RealSoundEffectSource에 의해서만 생성 가능
	RealSoundEffect(void);
	virtual ~RealSoundEffect(void);

public:

	bool Play(float x, float y, float z, bool bLoop=false);
	void Stop(void);
	bool IsPlaying(void);

	void SetPosition(float x, float y, float z);		// 3D Positional Sound Only
	void SetVolume(float t);							// 2D & 3D Sound
	float GetVolume(void);

	void SetMinDistance(float fMin);
	void SetMaxDistance(float fMax);

	void SetMode(RealSoundEffectMode nMode);

	inline E_FX_SOUND_TYPE GetFxType() const { return m_FxType;	};
	inline void SetFxType( E_FX_SOUND_TYPE type_ ) { m_FxType = type_;	};
};

class Package;

class RealSoundEffectSource{
protected:
	RealSound*			m_pRealSound;
	WAVEFILECLASS*		m_pWaveFile;					// 메모리에 올려지는 웨이브 파일
	char				m_szFileName[256];				// 파일 이름
	bool				m_bDynamicLoadable;				// Ref. Count에 따라 메모리에 올렸다 내림
	int					m_nRefCount;					// Reference Count
	RealSoundEffect*	m_pOriginalRealSoundEffect;		// Duplicate가 아닌 실제 버퍼
	RealSoundEffect*	m_pPreserveRealSoundEffect;		// OriginalRealSoundEffect 가 없어지지 않게 하기 위해 임시 보존하는 RealSoundEffect

	float				m_fMinDistance;					// 최소 거리,  이 거리까지는 소리의 감쇠가 없다
	float				m_fMaxDistance;					// 최대 거리, 최소거리에서 최대거리 까지 감쇠가 일어남, 이 거리가 지나가면 소리가 들리지 않게 된다

	Package*			m_pPackage;
	bool				m_bIsPakFile;
	int					m_iPriority;

protected:
	bool LoadWaveFromFile(void);
	bool LoadWaveFromMemory( unsigned char* pBuffer_, int Length );
	bool LoadWaveFromMemory(void);	
	bool LoadWaveFromPak(void);
	void RemoveWaveFromMemory(void);

	RealSoundEffect* NewRealSoundEffect(bool b3D, E_FX_SOUND_TYPE eType_ = FX_NONE );
	RealSoundEffect* DupRealSoundEffect(RealSoundEffect* pRealSoundEffect, bool b3D);
	void DelRealSoundEffect(RealSoundEffect* pOriginalRealSoundEffect);

public:
	RealSoundEffectSource(void);
	virtual ~RealSoundEffectSource(void);

	bool Create(RealSound* pRealSound, const char* szFileName, bool bDynamicLoadable=true, unsigned char* pBuffer=0, int Length=0 );
	bool CreatePackage(RealSound* pRealSound, const char* szFileName,Package* pak, bool bDynamicLoadable=true);

	void Destroy(void);

	RealSoundEffect* CreateRealSoundEffect(bool b3D=true, E_FX_SOUND_TYPE type_ = FX_NONE );
	void DestroyRealSoundEffect(RealSoundEffect* pRealSoundEffect);

	char* getFileName() const 	{ 		return (char*) m_szFileName; 	}

	inline void	SetMinDistance( float min_ ) { m_fMinDistance	= min_;	}
	inline float GetMinDistance( ) const { return m_fMinDistance;	}
	inline void	SetMaxDistance( float max_ ) { m_fMaxDistance	= max_;	}
	inline float GetMaxDistance( ) const { return m_fMaxDistance;	}
	inline void SetPriority( const int priority ) { m_iPriority = priority;	}
	inline int	GetPriority() const { return m_iPriority;	}
};


// RealSoundEffectSource의 인스턴스를 내부에 가지고 있어 독립적으로 삭제될 수 있는 클래스
class RealSoundEffectPlay// : public CMemPool<RealSoundEffectPlay>
{
public:
	RealSoundEffectSource*	const m_pSES;
protected:
	RealSoundEffect*		m_pRSE;
	bool					m_bLoop;
	float					m_fX, m_fY, m_fZ;

public:
	RealSoundEffectPlay(RealSoundEffectSource* pSES, bool b3D, float fMinDistance, float fMaxDistance, RealSoundEffectMode nMode, E_FX_SOUND_TYPE type_ = FX_NONE );
	virtual ~RealSoundEffectPlay(void);

	void Play(float x, float y, float z, bool bLoop);	// 플레이
	void Play();
	void Stop(void);				// 스탑
	bool IsPlaying(void);			// 플레이 하는가?
	bool IsPlayingLoop(void);		// Loop로 플레이 하는가?

	void SetPos(float x, float y, float z);
	void SetLoop(bool b) { m_bLoop = b; }
	void SetVolume(float t);
	float GetVolume(void);
};

class RealSoundEffectFx// : public CMemPool<RealSoundEffectFx>
{
protected:
	LPDIRECTSOUNDBUFFER8		m_pDSB8;
	
	LPDIRECTSOUNDFXWAVESREVERB8	m_pReverb8;
	DSFXWavesReverb				m_paramReverb;

	int							m_iNumEffect;
	int							m_EffectIndex[FX_NUM]; // 이펙트 인터페이스 얻어올때 쓰임

protected:
	bool	SetFX( E_FX_SOUND_TYPE type_ = FX_NONE );
	
	//////////////////////////////////////////////////////////////////////////
	//
	//	fInGain : (dB) DSFX_WAVESREVERB_INGAIN_MIN(-96.f) ~ DSFX_WAVESREVERB_INGAIN_MAX(0,0f) DSFX_WAVESREVERB_INGAIN_DEFAULT (Defulat,0.0f)
	//	fReverbMix : (dB) 리버브의 믹싱량
	//	fReverbTime : (milisecond) 0.001 ~ 3000 , default : 1000
	//  fHighFreqRTRatio : 고주파수 리버브 시간비 , 0.001~0.999, default : 0.001
	//
	bool	SetReverbParam( float fInGain_	= DSFX_WAVESREVERB_INGAIN_DEFAULT, 
							float fReverbMix_	= DSFX_WAVESREVERB_REVERBMIX_DEFAULT, 
							float fReverbTime_	= DSFX_WAVESREVERB_REVERBTIME_DEFAULT, 
							float fHighFreqRTRatio_		= DSFX_WAVESREVERB_HIGHFREQRTRATIO_DEFAULT );
	//
	//////////////////////////////////////////////////////////////////////////
	

public:
	bool	Initialize( LPDIRECTSOUNDBUFFER pDSB_, E_FX_SOUND_TYPE type_ = FX_NONE );
	void	Release();

public:
	RealSoundEffectFx();
	~RealSoundEffectFx();
};


#endif