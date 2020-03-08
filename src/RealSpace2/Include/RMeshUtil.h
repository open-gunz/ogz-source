#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include "RTypes.h"
#include "RMath.h"
#include "GlobalTypes.h"
#ifdef _WIN32
#include <d3d9.h>
#endif

#define DEL(p)  { if(p) { delete (p);   (p)=NULL; } }
#define DEL2(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define REL(p)	{ if(p != NULL) { (p)->Release(); (p)=NULL; }  }

#define		MAX_NAME_LEN		40
#define		MAX_PATH_NAME_LEN	256
#define		MAX_ANI_KEY			100
#define		MAX_MESH_NODE_TABLE	300
#define		MAX_PHYSIQUE_KEY	4

#ifndef USING_VERTEX_SHADER
#define USING_VERTEX_SHADER
#endif

#define EXPORTER_MESH_VER1	0x00000011
#define EXPORTER_MESH_VER2	0x00005001
#define EXPORTER_MESH_VER3	0x00005002
#define EXPORTER_MESH_VER4	0x00005003
#define EXPORTER_MESH_VER5	0x00005004
#define EXPORTER_MESH_VER6	0x00005005
#define EXPORTER_MESH_VER7	0x00005006
#define EXPORTER_MESH_VER8	0x00005007

#define EXPORTER_ANI_VER1	0x00000012
#define EXPORTER_ANI_VER2	0x00001001
#define EXPORTER_ANI_VER3	0x00001002
#define EXPORTER_ANI_VER4	0x00001003

#define EXPORTER_SIG		0x0107f060

typedef struct {
	u32	sig;
	u32	ver;
	int		mtrl_num;
	int		mesh_num;
} ex_hd_t;

typedef struct {
	u32	sig;
	u32	ver;
	int		maxframe;
	int		model_num;
	int		ani_type;
} ex_ani_t;

enum RWeaponMotionType {
	eq_weapon_etc = 0,

	eq_wd_katana,
	eq_ws_pistol,
	eq_wd_pistol,
	eq_wd_shotgun,
	eq_wd_rifle,
	eq_wd_grenade,
	eq_ws_dagger,
	eq_wd_item,
	eq_wd_rlauncher,// eq_wd_rocket_launcher
	eq_ws_smg,		// 10
	eq_wd_smg,		// 11
	eq_wd_sword,	// 12
	eq_wd_blade,	// 13
	eq_wd_dagger,	// 14

	eq_weapon_end,
};

/*

Bip01
Bip01 Head
Bip01 HeadNub
Bip01 L Calf
Bip01 L Clavicle
Bip01 L Finger0
Bip01 L Finger0Nub
Bip01 L Foot
Bip01 L ForeArm
Bip01 L Hand
Bip01 L Thigh
Bip01 L Toe0
Bip01 L Toe0Nub
Bip01 L UpperArm
Bip01 Neck
Bip01 Pelvis
Bip01 R Calf
Bip01 R Clavicle
Bip01 R Finger0
Bip01 R Finger0Nub
Bip01 R Foot
Bip01 R ForeArm
Bip01 R Hand
Bip01 R Thigh
Bip01 R Toe0
Bip01 R Toe0Nub
Bip01 R UpperArm
Bip01 Spine
Bip01 Spine1
Bip01 Spine2

*/

typedef enum _RMeshPartsPosInfoType {
	eq_parts_pos_info_etc = 0,

	eq_parts_pos_info_Root,
	eq_parts_pos_info_Head,
	eq_parts_pos_info_HeadNub,
	eq_parts_pos_info_Neck,
	eq_parts_pos_info_Pelvis,
	eq_parts_pos_info_Spine,
	eq_parts_pos_info_Spine1,
	eq_parts_pos_info_Spine2,

	eq_parts_pos_info_LCalf,
	eq_parts_pos_info_LClavicle,
	eq_parts_pos_info_LFinger0,
	eq_parts_pos_info_LFingerNub,
	eq_parts_pos_info_LFoot,
	eq_parts_pos_info_LForeArm,
	eq_parts_pos_info_LHand,
	eq_parts_pos_info_LThigh,
	eq_parts_pos_info_LToe0,
	eq_parts_pos_info_LToe0Nub,
	eq_parts_pos_info_LUpperArm,

	eq_parts_pos_info_RCalf,
	eq_parts_pos_info_RClavicle,
	eq_parts_pos_info_RFinger0,
	eq_parts_pos_info_RFingerNub,
	eq_parts_pos_info_RFoot,
	eq_parts_pos_info_RForeArm,
	eq_parts_pos_info_RHand,
	eq_parts_pos_info_RThigh,
	eq_parts_pos_info_RToe0,
	eq_parts_pos_info_RToe0Nub,
	eq_parts_pos_info_RUpperArm,

	eq_parts_pos_info_Effect,

	eq_parts_pos_info_end
} RMeshPartsPosInfoType;

typedef enum _RMeshPartsType {

	eq_parts_etc = 0,
	eq_parts_head,
	eq_parts_face,
	eq_parts_chest,
	eq_parts_hands,
	eq_parts_legs,
	eq_parts_feet,
	eq_parts_sunglass,

	// left weapon

	eq_parts_left_pistol,
	eq_parts_left_smg,
	eq_parts_left_blade,
	eq_parts_left_dagger,

	// right weapon

	eq_parts_right_katana,
	eq_parts_right_pistol,
	eq_parts_right_smg,
	eq_parts_right_shotgun,		
	eq_parts_right_rifle,
	eq_parts_right_grenade,
	eq_parts_right_item,	
	eq_parts_right_dagger,
	eq_parts_right_rlauncher,
	eq_parts_right_sword,
	eq_parts_right_blade,

	eq_parts_end,

} RMeshPartsType;

enum CutParts {
	cut_parts_upper_body = 0,
	cut_parts_lower_body,
	cut_parts_etc_body,
	cut_parts_end,
};

enum LookAtParts {
	lookat_parts_etc = 0,
	lookat_parts_spine,
	lookat_parts_spine1,
	lookat_parts_spine2,
	lookat_parts_head,
	lookat_parts_end,
};

enum WeaponDummyType {
	weapon_dummy_etc = 0,
	weapon_dummy_muzzle_flash,
	weapon_dummy_cartridge01,
	weapon_dummy_cartridge02,
	weapon_dummy_end,
};

enum RPickType {
	pick_bbox = 0,
	pick_collision_mesh,
	pick_real_mesh,
	pick_end
};

enum RShaderConst {
	IDENTITY_MATRIX = 0,
	WORLD_MATRIX = 3,
	VIEW_PROJECTION_MATRIX = 6,
	CONSTANTS = 10,
	CAMERA_POSITION = 11,
	MATERIAL_AMBIENT = 12,
	MATERIAL_DIFFUSE,
	MATERIAL_SPECULAR,
	MATERIAL_POWER,
	GLOBAL_AMBIENT,
	LIGHT0_POSITION,
	LIGHT0_AMBIENT,
	LIGHT0_DIFFUSE,
	LIGHT0_SPECULAR,
	LIGHT0_RANGE,
	LIGHT1_POSITION,
	LIGHT1_AMBIENT,
	LIGHT1_DIFFUSE,
	LIGHT1_SPECULAR,
	LIGHT1_RANGE,
	LIGHT0_ATTENUATION,
	LIGHT1_ATTENUATION,
	ANIMATION_MATRIX_BASE
};

enum RShaderBlendInput {
    VPOSITION,
	WEIGHT2,
	MATRIX_INDEX,
	NORMAL,
	TEXTURE_UV
};

struct RTLVertex { 
	v4 p;   
	u32 color;     
	float tu, tv; 
};

struct RLVertex { 
	rvector p;   
	u32 color;     
	float tu, tv; 
};

#ifndef _MAX_EXPORT

struct RVertex { 
	rvector p;
	rvector n; 
	float tu, tv;
};

#endif

struct RBlendVertex
{
	rvector p;
	float weight1, weight2;
	float matIndex[3];
	rvector normal;
	float tu, tv;	
};

#define RTLVertexType		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define RLVertexType		(D3DFVF_XYZ   |D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define RVertexType			(D3DFVF_XYZ   |D3DFVF_NORMAL |D3DFVF_TEX1)
#define RBLENDVERTEXTYPE	( D3DFVF_XYZB2 | D3DFVF_XYZ | D3DFVF_DIFFUSE  | D3DFVF_TEX1 | D3DFVF_NORMAL )

class RPosKey : public rvector {
public:
	int	frame;
};

class RQuatKey : public rquaternion {
public:
	using rquaternion::rquaternion;
	int	frame;
};

using RRotKey = RQuatKey;

class RVisKey {
public:
	float v;
	int frame;
};

class RTMKey : public rmatrix {
public:
	int frame;
};

class RVertexAniKey : public rvector {
public:
	int frame;
};

struct RFaceInfoOld {
	int				m_point_index[3];
	rvector		m_point_tex[3];
	int				m_mtrl_id;
};

struct RFaceInfo {
	int				m_point_index[3];
	rvector		m_point_tex[3];
	int				m_mtrl_id;
	int				m_sg_id;
};

struct RFaceNormalInfo {
	rvector m_normal;
	rvector m_pointnormal[3];
};

struct RPhysiqueInfo {

	RPhysiqueInfo()	{

		for(int i=0;i<MAX_PHYSIQUE_KEY;i++)
			m_parent_name[i][0] = 0;

		m_num = 0;
	};

	char	m_parent_name[MAX_PHYSIQUE_KEY][MAX_NAME_LEN];
	float	m_weight[MAX_PHYSIQUE_KEY];
	int		m_parent_id[MAX_PHYSIQUE_KEY];
	int		m_num;

	rvector m_offset[MAX_PHYSIQUE_KEY];
};

#define USE_VERTEX_SW 1
#define USE_VERTEX_HW 1<<1

#ifdef _WIN32
class RIndexBuffer final {
public:
	RIndexBuffer();
	~RIndexBuffer();

	void Lock();
	void Unlock();

	void Update(int size,u16* pData);
	bool Create(int size,u16* pData,u32 flag=USE_VERTEX_HW|USE_VERTEX_SW,u32 Usage=D3DUSAGE_WRITEONLY,D3DPOOL Pool=D3DPOOL_MANAGED);


	int GetFaceCnt();

	void SetIndices();

public:
	
	bool	m_bUseSWVertex;
	bool	m_bUseHWVertex;

	u32	m_dwUsage;
	D3DPOOL	m_dwPool;
	u32	m_dwLockFlag;

	u16*	m_pIndex;
	u16*	m_i;

	int m_size;
	LPDIRECT3DINDEXBUFFER9 m_ib;
};

class RVertexBuffer final {
public:
	RVertexBuffer();
	~RVertexBuffer();

	void Init();
	void Clear();

	bool Create(char* pVertex, u32 fvf, int VertexSize, int VertexCnt,
		u32 flag, u32 Usage = D3DUSAGE_WRITEONLY, D3DPOOL Pool = D3DPOOL_MANAGED);

	bool Update(char* pVertex,u32 fvf,int VertexSize,int VertexCnt);
	bool UpdateData(char* pVertex);		
	bool UpdateDataSW(char* pVertex);
	bool UpdateDataHW(char* pVertex);

	bool UpdateData(rvector* pVec);

#ifndef _MAX_EXPORT
	void UpdateDataLVert(RLVertex* pVert,rvector* pVec,int nCnt);
	void UpdateDataVert(RVertex* pVert,rvector* pVec,int nCnt);
#endif

	void Lock();
	void Unlock();

	void SetStreamSource();

	void Render();
	void RenderFVF();
	void Render(RIndexBuffer* ib );
	void RenderSoft();
	void RenderIndexSoft(RIndexBuffer* ib);
	void SetVertexBuffer();
	void SetVSVertexBuffer();
	void RenderIndexBuffer(RIndexBuffer* ib);

	void ConvertSilhouetteBuffer(float fLineWidth);
	void ReConvertSilhouetteBuffer(float fLineWidth);

public:

	bool	m_is_init;
	bool	m_bUseSWVertex;
	bool	m_bUseHWVertex;
	char*	m_pVert;
	char*	m_v;

	u32	m_dwFVF;
	u32	m_dwUsage;
	D3DPOOL	m_dwPool;
	u32	m_dwLockFlag;
	int		m_nVertexSize;
	int		m_nVertexCnt;
	int		m_nBufferSize;
	int		m_nRealBufferSize;

	int		m_nRenderCnt;

	D3DPRIMITIVETYPE m_PrimitiveType;

	LPDIRECT3DVERTEXBUFFER9	m_vb;
};
#endif

rquaternion* QuaternionUnitAxisToUnitAxis2(rquaternion *pOut, const rvector *pvFrom, const rvector *pvTo);
rquaternion* QuaternionAxisToAxis(rquaternion *pOut, const rvector *pvFrom, const rvector *pvTo);

#ifdef _WIN32
class CD3DArcBall
{
	int            m_iWidth;				
	int            m_iHeight;				
	float          m_fRadius;				
	float          m_fRadiusTranslation;	

	rquaternion m_qDown;					
	rquaternion m_qNow;
	rmatrix     m_matRotation;			
	rmatrix     m_matRotationDelta;		
	rmatrix     m_matTranslation;		
	rmatrix     m_matTranslationDelta;	
	bool           m_bDrag;					
	bool           m_bRightHanded;			

	rvector ScreenToVector( int sx, int sy );

public:

	LRESULT     HandleMouseMessages( HWND, UINT, WPARAM, LPARAM );

	rmatrix* GetRotationMatrix()         { return &m_matRotation; }
	rmatrix* GetRotationDeltaMatrix()    { return &m_matRotationDelta; }
	rmatrix* GetTranslationMatrix()      { return &m_matTranslation; }
	rmatrix* GetTranslationDeltaMatrix() { return &m_matTranslationDelta; }
	bool        IsBeingDragged()            { return m_bDrag; }

	VOID        SetRadius( float fRadius );
	VOID        SetWindow( int w, int h, float r=0.9 );
	VOID        SetRightHanded( bool bRightHanded ) { m_bRightHanded = bRightHanded; }

	CD3DArcBall();
};
#endif

void RRot2Quat(RQuatKey& q, const RRotKey& v);
void RQuat2Mat(rmatrix& mat, const RQuatKey& q);

struct IDirect3DDevice9;
using LPDIRECT3DDEVICE9 = IDirect3DDevice9*;
void draw_line(LPDIRECT3DDEVICE9 dev, rvector* vec, int size, u32 color);
void draw_box(rmatrix* wmat, const rvector& max, const rvector& min, u32 color);
void draw_query_fill_box(rmatrix* wmat , rvector& max,rvector& min,u32 color);

void _GetModelTry(RLVertex* pVert,int size,u32 color,int* face_num);
void _draw_try(LPDIRECT3DDEVICE9 dev,rmatrix& mat,float size,u32 color);
void _draw_matrix(LPDIRECT3DDEVICE9 dev,rmatrix& mat,float size);

class RDebugStr
{
public:
	RDebugStr();
	~RDebugStr();

	void Clear();

	void Add(const char* str, bool line = true);
	void Add(bool b, bool line = true);
	void Add(char c, bool line = true);
	void Add(short s, bool line = true);
	void Add(u16 w, bool line = true);
	void Add(int i, bool line = true);
	void Add(unsigned long d, bool line = true);
	void Add(unsigned int u, bool line = true)
	{
		Add(static_cast<unsigned long>(u), line);
	}
	void Add(float f, bool line = true);
	void Add(rvector& v, bool line = true);
	void AddLine(int cnt = 1);
	void AddTab(int cnt = 1);

	void PrintLog();

public:

	char m_temp[256];

	std::string m_str;
};

void GetPath(const char* str, char* path, size_t path_len);
template <size_t size>
void GetPath(const char* str, char(&path)[size]) {
	return GetPath(str, path, size);
}

class RBaseObject
{
public:
	RBaseObject() {
		m_NameID = -1;
	}
	virtual ~RBaseObject() {

	}

public:

	const char* GetName() const;
	void  SetName(const char* name);

	bool  CheckName(const char* name);
	bool  CheckName(const std::string& name);

public:

	int			m_NameID;
	std::string	m_Name;
};

template <class T>
class RHashList : public std::list<T>
{
protected:
	std::unordered_map<std::string, T>	m_HashMap;
	std::unordered_map<int, T>			m_HashMapID;
public:
	void PushBack(T pNode) {
		this->push_back(pNode);
		m_HashMap.insert({std::string(pNode->GetName()), pNode});
		if (pNode->m_NameID != -1)
			m_HashMapID.insert({pNode->m_NameID, pNode});

	}

	void Clear() {
		m_HashMap.clear();
		m_HashMapID.clear();
		this->clear();
	}

	auto Erase(typename RHashList<T>::iterator where) {

		auto itor = this->erase(where);

		if (itor != this->end()) {

			auto it = m_HashMap.find(std::string((*itor)->GetName()));

			if (it != m_HashMap.end()) {
				m_HashMap.erase(it);
			}

			auto it_id = m_HashMapID.find((*itor)->m_NameID);

			if (it_id != m_HashMapID.end()) {
				m_HashMapID.erase(it_id);
			}
		}
		return itor;
	}

	T Find(const char *name) {

		auto itor = m_HashMap.find(name);

		if (itor != m_HashMap.end()) {
			return (*itor).second;
		}
		return NULL;
	}

	T Find(int id) {
		auto itor = m_HashMapID.find(id);
		if (itor != m_HashMapID.end()) {
			return itor->second;
		}
		return nullptr;
	}

};
