#pragma once

#include "ZEffectManager.h"
#include "RBaseTexture.h"

class ZEffectBillboardSource {
protected:
	RBaseTexture*	m_pTex;
public:
	ZEffectBillboardSource(const char* szTextureFileName);
	virtual ~ZEffectBillboardSource(void);

	bool Draw(rvector &Pos, rvector &Dir, rvector &Up, rvector &Scale, float fOpacity);
};

class ZEffectBillboard : public ZEffect{
protected:
	ZEffectBillboardSource*	m_pEffectBillboardSource;
public:
	rvector			m_Pos;
	rvector			m_Normal;
	rvector			m_Up;
	rvector			m_Scale;
	float			m_fOpacity;


public:
	ZEffectBillboard(ZEffectBillboardSource* pEffectBillboardSource);
	virtual ~ZEffectBillboard(void);

	void SetSource(ZEffectBillboardSource*	pSource) {
		m_pEffectBillboardSource = pSource;
	}

	virtual bool Draw(u64 nTime) override;

	virtual rvector GetSortPos() {
		return m_Pos;
	}

};

class ZEffectBillboardDrawer{
protected:
	bool			m_bCreate;
public:
	ZEffectBillboardDrawer(void);
	virtual ~ZEffectBillboardDrawer(void);

	void Create(void);
	bool IsCreated(void){ return m_bCreate; }

	bool Draw(LPDIRECT3DTEXTURE9 pEffectBillboardTexture, rvector &Pos, rvector &Dir, rvector &Up, rvector &Scale, float fOpacity);
};

class ZEffectBillboard2 : public ZEffect{
protected:
	LPDIRECT3DTEXTURE9	m_pEffectBillboardTexture;
	static ZEffectBillboardDrawer	m_EffectBillboardDrawer;
public:
	rvector			m_Pos;
	rvector			m_Normal;
	rvector			m_Up;
	rvector			m_Scale;
	float			m_fOpacity;
public:
	ZEffectBillboard2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture);
	virtual ~ZEffectBillboard2(void);

	virtual bool Draw(u64 nTime) override;

	virtual rvector GetSortPos() {
		return m_Pos;
	}

};