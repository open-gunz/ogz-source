#if !defined(AFX_EL_MESH_H__A5FA3593_78B0_4304_B36D_635328A82A04__INCLUDED_)
#define AFX_EL_MESH_H__A5FA3593_78B0_4304_B36D_635328A82A04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEL2(p) { if(p) { delete[] (p);   (p)=NULL; } }

#pragma warning (disable : 4530)

#define MAX_NAME_LEN		40
#define MAX_PATH_NAME_LEN	256
#define MAX_NODE			300

#include <list>
#include <vector>

#include <D3DX9.h>

#include "RMeshUtil.h"

using namespace std;

enum AnimationType{
	RAniType_TransForm = 0,
	RAniType_Vertex,
	RAniType_Bone,
	RAniType_Tm,
};

// 이전버젼과의 호환성 때문.. 나중에 하나로 정리

class el_mesh;

class mesh_data
{
public:
	mesh_data();
	~mesh_data();

	int				m_id;
	int				m_u_id;

	el_mesh*		List;

	char			m_Name[MAX_NAME_LEN];
	char			m_Parent[MAX_NAME_LEN];
	
	mesh_data*		m_pParent;

	D3DXMATRIX		m_mat_base;			
	D3DXMATRIX		m_mat_init;

	D3DXVECTOR3		m_ap_scale;

	D3DXVECTOR3		m_axis_scale;
	float			m_axis_scale_angle;

	D3DXVECTOR3		m_axis_rot;
	float			m_axis_rot_angle;
	
	D3DXMATRIX		m_etc_mat;
	
	bool			m_is_rot;
	bool			m_is_pos;
	
	bool			m_is_vertex_ani;

	int				m_point_num;
	int				m_face_num;

	D3DXVECTOR3*	m_point_list;
	RFaceInfo*		m_face_list;

	int				m_tex_point_num;
	D3DXVECTOR3*	m_tex_point_list;

	int				m_physique_num;
	RPhysiqueInfo*	m_physique;

	////////////////////////////////////////////////////

	RFaceNormalInfo* m_face_normal_list;

	int				m_point_color_num;	// m_point_num 과 같다~
	D3DXVECTOR3*	m_point_color_list;

//	int				m_point_normal_num;	// m_point_num 과 같다~
//	D3DXVECTOR3*	m_point_normal_list;

	////////////////////////////////////////////////////

	D3DXVECTOR3**	m_vertex_ani_list;
	DWORD*			m_vertex_ani_frame;
	int				m_vertex_ani_num;

	int				m_mtrl_id;

	int				m_tex_face_num;

	bool			m_NormalFlag;

	RPosKey*		m_pos_key;
	RQuatKey*		m_quat_key;
	RTMKey*			m_tm_key;
	RVisKey*		m_vis_key;

	int				m_pos_key_num;
	int				m_quat_key_num;
	int				m_tm_key_num;
	int				m_vis_key_num;

	int				m_frame;
	int				m_max_frame;
	int				m_max_frame_tick;
};

typedef list<mesh_data*>		el_mesh_list;
typedef el_mesh_list::iterator	el_mesh_node;

class mtrl_data {

public:

	int					m_id;
	int					m_mtrl_id;
	int					m_sub_mtrl_id;

	char				m_tex_name[MAX_PATH_NAME_LEN];
	char				m_opa_name[MAX_PATH_NAME_LEN];

	D3DXCOLOR			m_ambient;
	D3DXCOLOR			m_diffuse;
	D3DXCOLOR			m_specular;

	float				m_power;
	int					m_twosided;
	int					m_sub_mtrl_num;
	int					m_additive;
	int					m_alphatest_ref;
	bool				m_bUse;
};

typedef list<mtrl_data*>		el_mtrl_list;
typedef el_mtrl_list::iterator	el_mtrl_node;

class el_mesh  
{
public:
	el_mesh();
	virtual ~el_mesh();

	int	 add_mesh(mesh_data* data);
	int	 add_mtrl(mtrl_data* data);

	void del_mesh_list();

	bool export_text(char* filename);
	bool export_bin(char* filename);
	bool export_ani(char* filename,int mode);
	bool export_etc(char* filename);

	int  ani_node_cnt();

	void ClearVoidMtrl();
	void ClearUsedMtrlCheck();
	void ClearUsedMtrl();

	mtrl_data* GetMtrl(int id,int sid);

public:
	int				m_max_frame;

	el_mesh_list	m_list;
//	mesh_data*		m_data[MAX_NODE];
	vector<mesh_data*> m_data;
	int				m_data_num;

	el_mtrl_list	m_mtrl_list;
//	mtrl_data*		m_mtrl_data[MAX_NODE];
	vector<mtrl_data*> m_mtrl_data;
	int				m_mtrl_num;
};

#endif // !defined(AFX_EL_MESH_H__A5FA3593_78B0_4304_B36D_635328A82A04__INCLUDED_)
