#pragma once

#include <list>
#include <vector>
#include <string>
#include <map>
#include "RBaseTexture.h"
#include "RTypes.h"
#include <d3d9.h>

_USING_NAMESPACE_REALSPACE2

class RMtrl
{
public:
	RMtrl();
	~RMtrl();
	RMtrl(const RMtrl& rhs) = delete;

	void CheckAniTexture();

	LPDIRECT3DTEXTURE9 GetTexture();

	void Restore(LPDIRECT3DDEVICE9 dev,char* path=NULL);

	void	SetTColor(u32 color);
	u32		GetTColor();

	RBaseTexture*  m_pTexture;

	RBaseTexture*  m_pToonTexture;

	D3DTEXTUREFILTERTYPE m_FilterType;
	D3DTEXTUREFILTERTYPE m_ToonFilterType;

	u32					m_AlphaRefValue;
	D3DTEXTUREOP		m_TextureBlendMode;
	D3DTEXTUREOP		m_ToonTextureBlendMode;

	color_r32 m_ambient;
	color_r32 m_diffuse;
	color_r32 m_specular;

	float	m_power;

	char	m_mtrl_name[255];

	char	m_name[255];
	char	m_opa_name[255];

	char	m_name_ani_tex[255];
	char	m_name_ani_tex_ext[20];

	int		m_id;
	int		m_u_id;
	int		m_mtrl_id;	  
	int		m_sub_mtrl_id;

	int		m_sub_mtrl_num;

	bool	m_bDiffuseMap;
	bool	m_bTwoSided;
	bool	m_bAlphaMap;
	bool	m_bAlphaTestMap;
	bool	m_bAdditive;

	int		m_nAlphaTestValue;

	bool	m_bUse;

	bool	m_bAniTex;
	int 	m_nAniTexCnt;
	int 	m_nAniTexSpeed;
	int 	m_nAniTexGap;
	u64		m_backup_time;

	bool	m_bObjectMtrl;// effect ,interface , object

	u32		m_dwTFactorColor;

	RBaseTexture** m_pAniTexture;
};

#define MAX_MTRL_NODE 100

class RMtrlMgr final : public std::list<RMtrl*>
{
public:
	RMtrlMgr();
	RMtrlMgr(const RMtrlMgr&) = delete;
	~RMtrlMgr();

	int		Add(char* name,int u_id = -1);
	int		Add(RMtrl* tex);

	void	Del(int id);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();
	void	Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL);

	void	CheckUsed(bool b);
	void	ClearUsedCheck();
	void	ClearUsedMtrl();

	void	SetObjectTexture(bool bObject) { m_bObjectMtrl = bObject; }

	RMtrl*	Get_s(int mtrl_id,int sub_id);

	LPDIRECT3DTEXTURE9 Get(int id);
	LPDIRECT3DTEXTURE9 Get(int id,int sub_id);
	LPDIRECT3DTEXTURE9 GetUser(int id);
	LPDIRECT3DTEXTURE9 Get(char* name);

	RMtrl*  GetMtrl(char* name);
	RMtrl*  GetToolMtrl(char* name);

	void	Del(RMtrl* tex);
	int		GetNum();

	std::vector<RMtrl*>	m_node_table;

	bool	m_bObjectMtrl{};// effect ,interface , object
	int		m_id_last{};
};
