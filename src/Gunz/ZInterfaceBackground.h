#pragma once

#include "RBspObject.h"
#include "ZMapDesc.h"

#define LOGIN_SCENE_FIXEDSKY	0
#define LOGIN_SCENE_FALLDOWN	1
#define LOGIN_SCENE_FIXEDCHAR	2
#define LOGIN_SCENE_SELECTCHAR	3

class ZInterfaceBackground {
private:
	RealSpace2::RBspObject*	m_pLogin;
	ZMapDesc* m_pMapDesc;

	rmatrix		m_matWorld;

	int			m_nSceneNumber;

	rvector		m_vCamPosSt;
	rvector		m_vCamPosEd;
	rvector		m_vCamDirSt;
	rvector		m_vCamDirEd;
	rvector		m_vCharPos;
	rvector		m_vCharDir;

	u32			m_dwClock;


protected:
	void SetFogState(float fStart, float fEnd, u32 color);

public:
	ZInterfaceBackground();
	virtual ~ZInterfaceBackground();

	RealSpace2::RBspObject* GetChurchEnd() { return m_pLogin; }
	int GetScene() { return m_nSceneNumber; }
	void SetScene(int nSceneNumber);

	void LoadMesh();
	void Free();
	void Draw();

	void OnUpdate(float fElapsed);
	void OnInvalidate();
	void OnRestore();

	rvector& GetCharPos() { return m_vCharPos; }
	rvector& GetCharDir() { return m_vCharDir; }
};