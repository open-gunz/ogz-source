#pragma once

#include "GlobalTypes.h"
#include "RAnimationDef.h"

_NAMESPACE_REALSPACE2_BEGIN

class RMesh;

class RAnimationNode : public RBaseObject
{
public:
	RAnimationNode();
	~RAnimationNode();

	float GetVisValue(int frame);

	rquaternion GetRotValue(int frame);
	rvector		GetPosValue(int frame);
	rmatrix			GetTMValue(int frame);
	int 			GetVecValue(int frame, rvector* pVecTable);

	void ConnectToNameID();

public:

	rmatrix			m_mat_base;
	int				m_node_id;
	RMesh*			m_pConnectMesh;

	int				m_pos_cnt;
	int				m_rot_cnt;
	int				m_mat_cnt;
	int				m_vis_cnt;

	RPosKey*		m_pos;
	RQuatKey*		m_quat;
	RTMKey*			m_mat;
	RVisKey*		m_vis;

	int				m_vertex_cnt;
	int				m_vertex_vcnt;
	u32*			m_vertex_frame;
	rvector**		m_vertex;
};


_NAMESPACE_REALSPACE2_END
