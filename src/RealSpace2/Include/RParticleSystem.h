#pragma once

#include "RTypes.h"
#include "RBaseTexture.h"

#define DISCARD_COUNT	2048
#define FLUSH_COUNT		512

_NAMESPACE_REALSPACE2_BEGIN

struct RParticle {
	rvector position;
	rvector velocity;
	rvector accel;
	float	ftime;

	virtual bool Update(float fTimeElapsed);
};

struct POINTVERTEX
{
	rvector v;
	D3DCOLOR color;

	static const u32 FVF;
};

class RParticles : public std::list<RParticle*> {

protected:
	rvector mInitialPos;

public:
	RParticles();
	virtual ~RParticles();

	bool Create(const char *szTextureName,float fSize);
	void Destroy();
	void Clear();

	virtual bool Draw();
	virtual bool Update(float fTime);

protected:
	float	m_fSize;

	RBaseTexture* m_Texture;

};

class RParticleSystem : public std::list<RParticles*> {
public:
	RParticleSystem();
	virtual ~RParticleSystem();

	void Destroy();

	bool Draw();
	bool Update(float fTime);

	RParticles *AddParticles(const char *szTextureName,float fSize);

	static void BeginState();
	static void EndState();

	static bool Restore();
	static bool Invalidate();

	static LPDIRECT3DVERTEXBUFFER9 m_pVB;
	static DWORD m_dwBase;
};


_NAMESPACE_REALSPACE2_END
