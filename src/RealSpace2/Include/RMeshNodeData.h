#pragma once

#include "RMtrl.h"
#include "RMeshUtil.h"
#include "RMeshNodeStringTable.h"

_NAMESPACE_REALSPACE2_BEGIN

class RMesh;
class RAnimationNode;
class RAnimation;

class RMeshNodeData :public RBaseObject
{
public:
	RMeshNodeData();
	~RMeshNodeData();

	// bbox

	void SubCalc(rvector* v);
	void BBoxClear();
	void CalcLocalBBox();

public:

	// id

	int				m_id;
	int				m_u_id;
	int				m_mtrl_id;

	// name

	char			m_Parent[MAX_NAME_LEN];
	int				m_nParentNodeID;

	// bbox

	rvector		m_max;
	rvector		m_min;

	// matrix

	// Base matrix -- transforms from mesh origin to the node's bone on an untransformed skeleton
	rmatrix		m_mat_base;
	// Parent inverse base matrix
	rmatrix		m_mat_parent_inv;
	// Local matrix -- base * inverse parent base
	rmatrix		m_mat_local;
	// Inverse base matrix
	rmatrix		m_mat_inv;
	rmatrix		m_mat_etc;
	rmatrix		m_mat_flip;
	rmatrix		m_mat_scale;
	// Fully transformed bone matrix -- transforms from mesh origin to the node's bone on a transformed skeleton
	rmatrix		m_mat_result;

	rmatrix		m_mat_add;

	rmatrix		m_mat_ref;
	rmatrix		m_mat_ref_inv;

	int				m_point_num;
	int				m_face_num;
	int				m_physique_num;
	int				m_point_normal_num;
	int				m_point_color_num;

	rvector*	m_point_list;
	rvector*	m_point_color_list;

	RFaceInfo*		m_face_list;
	RPhysiqueInfo*	m_physique;

	RFaceNormalInfo* m_face_normal_list;

	// temp

	rvector		m_spine_local_pos;
	rvector		m_ap_scale;

	rvector		m_axis_scale;
	float			m_axis_scale_angle;

	rvector		m_axis_rot;
	float			m_axis_rot_angle;

};

//////////////////////////////////////////////////////////////////
// Node Mtrl 

class RMeshNodeMtrl
{
public:
	RMeshNodeMtrl();
	~RMeshNodeMtrl();

	void	SetTColor(DWORD color);
	DWORD	GetTColor();

	int		GetMtrlCount();
	RMtrl*	GetMtrl(int i=0);

	void	SetMtrlDiffuse(RMtrl* pMtrl,float vis_alpha);
	void	SetMtrl(RMtrl* pMtrl,float vis_alpha,bool bNpc,D3DCOLORVALUE color );
	void	SetMtrl(color_r32* c,float vis_alpha);

public:

	DWORD			m_dwTFactorColor;

	int				m_nMtrlCnt;
	RMtrl**			m_pMtrlTable;
};

_NAMESPACE_REALSPACE2_END