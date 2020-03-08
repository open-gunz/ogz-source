#pragma once

#include "RMeshUtil.h"

//	sort가 필요 없고 
//	effectmanager에서 같이 그려줄 경우 무한 loop가 발생할 수 있기 때문에
//	별도로 update / draw 해준다

class RealSoundEffectPlay;

class ZEffectFlashBang
{
protected:
	bool	mbActivated;			// 플래시뱅의 영향을 받는지의 여부
	bool	mbInitialSound;			// 사운드 초기화가 필요한지 여부
	static bool	mbDrawCopyScreen;
	
	float	mfStartTime;
	float	mfPower;				// myCharacter와 폭발 위치와의 거리 & 시선에 따라 결정됨 (0~1)
	float	mfDuration;				// 효과 지속 시간..
	float	mfVolumn;				// Def Sound 의 볼륨fElapsedTime

	RTLVertex	mBuffer[4];
	//RealSoundEffectPlay* mpRSEffectPlay;
	int m_iChannel;
	LPDIRECT3DTEXTURE9	mpBlurTexture;
	LPDIRECT3DSURFACE9	mpBlurSurface;
	LPDIRECT3DSURFACE9	mpDepthBuffer;
	LPDIRECT3DSURFACE9	mpHoldDepthBuffer;
	LPDIRECT3DSURFACE9	mpHoldBackBuffer;

	static ZEffectFlashBang		msInstance;

public:
	void	SetBuffer();			// 게임 생성시 호출
	void	ReleaseBuffer();		// 게임 파괴시 호출
	void	OnInvalidate();			
	void	OnRestore();			

	void	Init( rvector& ExplosionPos_, rvector playerPos_, rvector playerDir_, float Duration_ );
	void	End();	// 이펙트 강제 종료한다

	void	Render();
	void	PlaySound();

	bool	IsActivated() const 
	{
		return mbActivated;	
	}

	static	ZEffectFlashBang* GetInstance() { return &msInstance; }
	static	void SetDrawCopyScreen( bool b_ ) { mbDrawCopyScreen = b_; }

public:
	ZEffectFlashBang();
	~ZEffectFlashBang();
};

//////////////////////////////////////////////////////////////////////////
//	Interface
//////////////////////////////////////////////////////////////////////////
ZEffectFlashBang* ZGetFlashBangEffect();
//	Duration_ : 효과 지속 시간(무기의 속성), ExplosionPos_ : 폭파 장소
//	playerPos_ : myCharacter의 위치, playerDir_ : 캐릭터가 바라보고 있는 방향
void	CreateFlashBangEffect( rvector& ExplosionPos_, rvector playerPos_, rvector playerDir_, float Duration_ );
bool	IsActivatedFlashBangEffect();				// 게임에서 현재 flashbang효과를 받는지 여부를 체크
void	ShowFlashBangEffect();
void	ReleaseFlashBangEffect();