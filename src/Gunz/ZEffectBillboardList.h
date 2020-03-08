#ifndef _ZEFFECTBILLBOARDLIST_H
#define _ZEFFECTBILLBOARDLIST_H

//#pragma once

#include "ZEffectBase.h"
#include "ZEffectBulletMarkList.h"

#include "mempool.h"
#include "RMeshUtil.h"

struct ZEFFECTBILLBOARDITEM : public ZEFFECTITEM , public CMemPoolSm<ZEFFECTBILLBOARDITEM>{
	rvector position;
	rvector normal;
	rvector	up;
	rvector	velocity;
	rvector accel;
	rvector accel2;
	float	fStartSize;
	float	fEndSize;
	float	fOpacity;
	float	fLifeTime;
	float	fCurScale;
	DWORD	dwColor;
	bool	bUseTrainSmoke;
	bool	bUseSteamSmoke;
	int		nDir;
};

namespace RealSpace2
{
	class RBaseTexture;
}

class ZEffectBillboardList : public ZEffectBase
{
public:
	ZEffectBillboardList();
	ZEFFECTBILLBOARDITEM * Add(const rvector &pos, const rvector &velocity, const rvector &accel,float fStartSize,float fEndSize,float fLifeTime=-1.f,DWORD color=0xffffff,bool bTrainSmoke=false);

	virtual void BeginState();
	virtual void Update(float fElapsed);
	virtual bool Draw();

	float GetLifeRatio(ZEFFECTBILLBOARDITEM *p);

	bool m_bUseRocketSmokeColor;
};

///////////////////////////////////////////////////////////////////////////////////

struct ZEFFECTSHADOWITEM : public ZEFFECTITEM , public CMemPoolSm<ZEFFECTSHADOWITEM>{
	DWORD	dwColor;
	rmatrix worldmat;
};

class ZEffectShadowList : public ZEffectBase
{
public:
	ZEffectShadowList();

	ZEFFECTSHADOWITEM * Add(rmatrix &m,DWORD _color);

	virtual void BeginState();
	virtual void EndState();
	virtual void Update(float fElapsed);
	virtual bool Draw();

};

/////////////////////////////////////////////////////////////////////////////////////////////////////

class ZCharacter;

struct ZEFFECTBILLBOARDTEXANIITEM : public ZEFFECTITEM , public CMemPoolSm<ZEFFECTBILLBOARDTEXANIITEM>{
	rvector position;
	rvector normal;
	rvector	up;
	rvector	velocity;
	rvector accel;
	float	fStartSize;
	float	fEndSize;
	float	fOpacity;
	float	fLifeTime;
	float	fAddTime;
	DWORD	dwColor;

	int		frame;

	MUID	CharUID;
	RMeshPartsPosInfoType partstype;
};

class ZEffectBillboardTexAniList : public ZEffectBase
{
public:
	ZEffectBillboardTexAniList();
	void Add(const rvector &pos, const rvector& vel,int frame,float fAddTime ,float fStartSize,float fEndSize,float fLifeTime=-1.f,ZCharacter* pChar=NULL,RMeshPartsPosInfoType partstype=eq_parts_pos_info_etc);

	virtual void BeginState();
	virtual void Update(float fElapsed);
	virtual bool Draw();

	void GetFrameUV(int frame);

	void SetTile(int xCnt,int YCnt,float fXTileUV,float fYTileUV);

public:
	
	bool	m_bFixFrame;

	int		m_nRenderMode;

	int		m_nXCnt;
	int		m_nYCnt;
	float	m_fXTileUV;
	float	m_fYTileUV;

	int		m_nMaxFrame;
	float	m_fSpeed;
	float	m_fUV[8];
};

#endif