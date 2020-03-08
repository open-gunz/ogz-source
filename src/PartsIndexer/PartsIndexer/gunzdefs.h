#pragma once
#include <Windows.h>

#define		MAX_NAME_LEN		40
#define		MAX_PATH_NAME_LEN	256
#define		MAX_ANI_KEY			100
#define		MAX_MESH_NODE_TABLE	300
#define		MAX_PHYSIQUE_KEY	4

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
	DWORD	sig;
	DWORD	ver;
	int		mtrl_num;
	int		mesh_num;
} ex_hd_t;

struct RFaceInfoOld {
	int				m_point_index[3];
	D3DXVECTOR3		m_point_tex[3];
	int				m_mtrl_id;
};

struct RFaceInfo {
	int				m_point_index[3];
	D3DXVECTOR3		m_point_tex[3];
	int				m_mtrl_id;
	int				m_sg_id;
};

struct RFaceNormalInfo {
	D3DXVECTOR3 m_normal;
	D3DXVECTOR3 m_pointnormal[3];
};

struct RPhysiqueInfo {
	RPhysiqueInfo() {

		for (int i = 0; i<MAX_PHYSIQUE_KEY; i++)
			m_parent_name[i][0] = 0;

		m_num = 0;
	};

	char	m_parent_name[MAX_PHYSIQUE_KEY][MAX_NAME_LEN];
	float	m_weight[MAX_PHYSIQUE_KEY];
	int		m_parent_id[MAX_PHYSIQUE_KEY];
	int		m_num;

	D3DXVECTOR3 m_offset[MAX_PHYSIQUE_KEY];
};