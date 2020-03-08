#ifndef _ZEFFECTBASE_H
#define _ZEFFECTBASE_H

//#pragma once

#include <list>
using namespace std;

#include "RBaseTexture.h"
#include "RealSpace2.h"
_USING_NAMESPACE_REALSPACE2

#define EFFECTBASE_DISCARD_COUNT	2048
#define EFFECTBASE_FLUSH_COUNT		128


#define ZEFFECTBASE_D3DFVF	 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct ZEFFECTCUSTOMVERTEX {
	v3 pos;
	DWORD	color;
	float	tu, tv;
};

struct ZEFFECTITEM {
	float fElapsedTime;
};


class ZEffectBase : public list<ZEFFECTITEM*>
{
protected:

	float	m_fLifeTime;
	float	m_fVanishTime;

	RBaseTexture	*m_pBaseTexture;

	static LPDIRECT3DVERTEXBUFFER9	m_pVB;
	static LPDIRECT3DINDEXBUFFER9	m_pIB;

	static DWORD	m_dwBase;

	rvector m_Scale;

public:
	ZEffectBase(void);
	~ZEffectBase(void);

	bool Create(const char *szTextureName);
	void Destroy();
	void Clear();

	void SetTexture(RBaseTexture *pTex)			{ m_pBaseTexture=pTex; }
	void SetLifeTime(float fLifeTime)			{ m_fLifeTime=fLifeTime; }
	void SetVanishTime(float fVanishTime)		{ m_fVanishTime=fVanishTime; }
	void SetScale(rvector scale)				{ m_Scale=scale; }

	static void CreateBuffers();
	static void ReleaseBuffers();
	static void OnInvalidate();
	static void OnRestore();

	virtual void BeginState();
	virtual void EndState();
	virtual void Update(float fElapsed);
	virtual bool Draw();
};


#endif