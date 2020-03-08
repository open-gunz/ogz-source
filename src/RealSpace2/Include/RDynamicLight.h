#pragma once

#include "d3d9.h"
#include "RMeshUtil.h"
#include "vector"

using namespace std;

//////////////////////////////////////////////////////////////////////////
//	Enum
//////////////////////////////////////////////////////////////////////////
enum MAP_LIGHT_TYPE				// PRE DEFINED 라이트 속성
{
	GUNFIRE,			// 총 라이트
	EXPLOSION,		// 폭발 라이트
	MAP_LIGHT_NUM,
};

//////////////////////////////////////////////////////////////////////////
//	Define
//////////////////////////////////////////////////////////////////////////
#define MAX_EXPLOSION_LIGHT	10

//////////////////////////////////////////////////////////////////////////
//	Struct
//////////////////////////////////////////////////////////////////////////
struct sMapLightObj
{
	rvector		vLightColor;
	float		fRange;
	float		fLife;
	rvector		vPos;
	bool		bUsing;
	sMapLightObj();
};


typedef struct 
{
	int			iType;
	rvector		vLightColor;
	float		fRange;
	float		fLife;				// 수명
} sMapLight;


//////////////////////////////////////////////////////////////////////////
//	RDynamicLightManager			
//	다이나믹 라이트 관리자 - 싱글톤
//	실시간으로 생기는 라이트에 대한 관리
//////////////////////////////////////////////////////////////////////////
class RDynamicLightManager			
{
protected:
	static	RDynamicLightManager	msInstance;
	static	sMapLight 				msMapLightList[MAP_LIGHT_NUM];

	bool										mbGunLight;							
	sMapLightObj						mGunLight;			// 총의 라이트는 내 캐릭터의 것만 영향을 받음
	vector<sMapLightObj>			mExplosionLightList;	
    
	float								mTime;
	rvector							mvPosition;

	// 옵션
	int									miNumEnableLight;	//	가능한 라이트의 개수
	
public:
	bool	AddLight( MAP_LIGHT_TYPE light_type_, const rvector& pos_ );
	void	Update();									// 수명이 다한 놈 골로 보내기
	void	Initialize();									// 초기화
	int		SetLight(const rvector& pos_ );					// pos_ : 캐릭터의 현재 위치, return 셋팅된 라이트 개수
	void	ReleaseLight();
	void	SetPosition(const rvector& pos_ );				// 캐릭터 위치 /// 맵에서만 사용

	bool	SetLight(const rvector& pos_, int lightIndex_, float maxDistance_ );	//
	
	static	RDynamicLightManager* GetInstance()
	{
		return &msInstance;
	}
	static	sMapLight*	GetLightMapList()
	{
		return msMapLightList;
	}
	bool	IsThereLight( )
	{
		if( mbGunLight || mExplosionLightList.size() )
		{
			return true;
		}
		return false;
	}

public:
	RDynamicLightManager();
	~RDynamicLightManager();
};

// singleTon 얻어오기
RDynamicLightManager*	RGetDynamicLightManager();
sMapLight*				RGetMapLightList();