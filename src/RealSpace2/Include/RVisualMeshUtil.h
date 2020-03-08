#pragma once

#include "RAnimationMgr.h"
#include <vector>

#include "SafeString.h"

_NAMESPACE_REALSPACE2_BEGIN

class RMesh;
class RMeshNode;
class RBspObject;


enum AnimationPlayState {
	APState_Stop = 0,
	APState_Play,
	APState_Pause
};

enum REnchantType {

	REnchantType_None = 0,
	REnchantType_Fire,
	REnchantType_Cold,
	REnchantType_Lightning,
	REnchantType_Poison,
	REnchantType_End,
};

class PartsInfo {
public:
	PartsInfo() {
		m_PartsType	= eq_parts_etc;
		m_PartsName[0] = NULL;
		m_PartsFileName[0] = NULL;
	}

	~PartsInfo() {

	}

public:

	char m_PartsName[MAX_NAME_LEN];
	char m_PartsFileName[MAX_NAME_LEN];
	RMeshPartsType m_PartsType;
};

enum RAniMode {
	ani_mode_lower = 0,
	ani_mode_upper ,
	ani_mode_blend_lower,
	ani_mode_blend_upper,
	ani_mode_end,
};

class RAniSoundInfo {
private:
	bool m_bRelatedToBsp;
public:
	RAniSoundInfo() {
		Clear();
	}

	void Clear() {
		m_bRelatedToBsp = false;
		isPlay = false;
		Name[0] = NULL;
		Pos.x = 0.f;
		Pos.y = 0.f;
		Pos.z = 0.f;
	}

	bool SetName(char* pName) {
		strcpy_safe(Name,pName);
		return true;
	}

	void SetPos(float x,float y,float z) {
		Pos.x = x;
		Pos.y = y;
		Pos.z = z;
	}

	bool isPlay;
	char Name[256];
	rvector Pos;

	bool IsRelatedToBsp() { return m_bRelatedToBsp; }
	void SetRelatedToBsp(bool bValue) { m_bRelatedToBsp = bValue; }
};

class RWeaponSNode
{
public:
	RWeaponSNode() {
		up = rvector(0.f,0.f,0.f);
		down = rvector(0.f,0.f,0.f);
		len = 0.0f;
		color[0] = 0xff888888;
		color[1] = 0xff888888;
	}

	DWORD	color[2];
	rvector up;
	rvector down;
	float	len;
};

class RWeaponTracks {
public:
	RWeaponTracks();
	~RWeaponTracks();

	void Create(int size);
	void Destroy();

	void MakeBuffer();
	void Render();

	void Update();
	void CheckShift();

	void AddVertex(RLVertex* pVert );
	void SetVertexSpline(rvector& p, DWORD c );

	void Clear();

	int  GetLastAddVertex(rvector* pOutVec);

public:

	rvector m_vSwordPos[2];

	RLVertex* m_pVert;
	RLVertex* m_pVertSpline;

	RWeaponSNode* m_pNode;

	bool m_bSpline;

	int	m_max_size;
	int m_current_node_size;
	int	m_vertex_size;
	int m_spline_vertex_size;
	int	m_current_vertex_size;
	int m_current_spline_vertex_size;
};

class RPartsInfo
{
public:
	void Init() {
		GetIdentityMatrix(mat);
		vis = 1.f;
		isUpdate = false;
	}

	rmatrix mat;
	float	vis;
	bool	isUpdate;
};

class RFireEffectTexture
{
public:
	RFireEffectTexture();
	~RFireEffectTexture();

	void Init();

	HRESULT Create(LPDIRECT3DDEVICE9 dev,int w,int h);
	void	Destroy();

	HRESULT UpdateTexture();
	void	ProcessFire(int coolamount);

	LPDIRECT3DTEXTURE9 GetTexture() { return m_pTexture; }

public:

	unsigned char*	m_pData;  
	unsigned char*	m_pData2; 
	unsigned char*	m_pFireActive;   
	unsigned char*	m_pFireScratch;  

	int					m_w;
	int					m_h;
	LPDIRECT3DTEXTURE9	m_pTexture;
	PALETTEENTRY		m_Palette[256];
};

_NAMESPACE_REALSPACE2_END
