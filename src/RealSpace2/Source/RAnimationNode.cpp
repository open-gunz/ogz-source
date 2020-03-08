#include "stdafx.h"
#include "RAnimationNode.h"
#include "RealSpace2.h"
#include "RMeshNodeStringTable.h"

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

RAnimationNode::RAnimationNode() 
{
	m_node_id = -1;

	m_pos_cnt = 0;
	m_rot_cnt = 0;
	m_mat_cnt = 0;
	m_vis_cnt = 0;

	m_vertex_cnt = 0;
	m_vertex_vcnt = 0;

	m_pos = NULL;
	m_quat = NULL;
	m_mat = NULL;
	m_vis = NULL;

	m_vertex = NULL;
	m_vertex_frame = NULL;

	m_pConnectMesh = NULL;

	GetIdentityMatrix(m_mat_base);
}

RAnimationNode::~RAnimationNode() 
{
	DEL2(m_pos);
	DEL2(m_quat);
	DEL2(m_mat);
	DEL2(m_vis);

	if(m_vertex_cnt) {
		for(int i=0;i<m_vertex_cnt;i++){
			DEL2(m_vertex[i]);
		}
		DEL2(m_vertex);
	}

	DEL2(m_vertex_frame);
}

void RAnimationNode::ConnectToNameID()
{
	m_NameID = RGetMeshNodeStringTable()->Get(m_Name);;
}

float GetVisKey(RVisKey* pKey,int pos,int key_max,int frame)
{
	if(!pKey) return 1.f;

	if( pos < 0 || pos >= key_max )
		return 1.0f;

	float d = 1.f;;

	int f2 = pKey[pos+1].frame;
	int f1 = pKey[pos].frame;

	int s = ( f2 - f1 );

	if(s != 0 )	d = (float)(frame - f1) /(float)s;

	d = pKey[pos].v + (pKey[pos+1].v - pKey[pos].v) * d;

	return d;
}

int GetVisKeyPos(RVisKey* pKey,int key_max,int frame)
{
	if(!pKey)		return 0;
	if(key_max==0)	return 0;

	int p;
	for (p=0;p<key_max;p++) {
		if ( pKey[p].frame > frame) {
			break;
		}
	}

	if(p) p--;

	return p;
}

float RAnimationNode::GetVisValue(int frame)
{
	if(m_vis_cnt == 0)	return 1.0f;
	if(m_vis==NULL) 	return 1.0f;

	int key_pos = GetVisKeyPos(m_vis,m_vis_cnt,frame);

	if( key_pos+2 > m_vis_cnt )
		return m_vis[m_vis_cnt-1].v;

	return GetVisKey(m_vis,key_pos,m_vis_cnt,frame);
}

inline v3 operator *(const v3& vec, const rquaternion& quat) {
	v3 quat_vec{ EXPAND_VECTOR(quat) };
	auto uv = CrossProduct(quat_vec, vec);
	auto uuv = CrossProduct(quat_vec, uv);
	return vec + ((uv * quat.w) + uuv) * 2;
}

rquaternion RAnimationNode::GetRotValue(int frame)
{
	if (m_rot_cnt == 0 || m_quat == NULL) {
		return MatrixToQuaternion(m_mat_base);
	}

	int p;
	for (p = 0; p < m_rot_cnt; p++) {
		if (m_quat[p].frame > frame) {
			break;
		}
	}

	if(p>=m_rot_cnt) {
		return m_quat[m_rot_cnt-1];
	}

	if(p)
		p--;

	int s;
	float d = 1.f;

	s = (m_quat[p+1].frame - m_quat[p].frame );

	if (s != 0)	d = (float)(frame - m_quat[p].frame) / (float)s;

	return Slerp(m_quat[p], m_quat[p + 1], d);
}

rvector RAnimationNode::GetPosValue(int frame)
{
	rvector v;

	if (m_pos_cnt == 0 || m_pos == NULL) {
		return GetTransPos(m_mat_base);
	}

	int p;
	for (p = 0; p < m_pos_cnt; p++) {
		if (m_pos[p].frame > frame) {
			break;
		}
	}

	if (p >= m_pos_cnt) {
		return m_pos[m_pos_cnt - 1];
	}

	if (p) p--;

	int s = (m_pos[p + 1].frame - m_pos[p].frame);

	float d = 1.f;

	if (s != 0)	d = (float)(frame%s) / (float)s;
	v = m_pos[p] - m_pos[p + 1];

	if (v.x == 0.f && v.y == 0.f && v.z == 0.f) d = 0;
	v = m_pos[p] - v * d;

	return v;
}

int	RAnimationNode::GetVecValue(int frame,rvector* pVecTable)
{
	u32 dwFrame = frame;

	int j;
	for (j=0;j<m_vertex_cnt;j++) {
		if ( m_vertex_frame[j] > dwFrame) 
			break;
	}

	if( j>= m_vertex_cnt) {

		int vcnt = m_vertex_vcnt;

		rvector* v1 = m_vertex[vcnt-1];
		memcpy(pVecTable,v1,sizeof(rvector)*vcnt);

		return vcnt;
	}

	if(j)	j--;

	int   s = m_vertex_frame[j+1] - m_vertex_frame[j];

	float d = 1;

	if (s != 0)	d = (frame - m_vertex_frame[j] )/(float)s;

	rvector* v1 = m_vertex[j];
	rvector* v2 = m_vertex[j+1];

	rvector v;

	int vcnt = m_vertex_vcnt;

	for(int k=0;k<vcnt;k++) {
		v = Lerp(v1[k], v2[k], d);
		pVecTable[k] = v;
	}

	return vcnt;
}

rmatrix RAnimationNode::GetTMValue(int frame)
{
	int j = 0;

	for (j=0;j<m_mat_cnt;j++) {
		if ( m_mat[j].frame > frame) break;
	}

	if(j >= m_mat_cnt) {
		return m_mat[m_mat_cnt-1];
	}

	if(j)
		j--;

	return m_mat[j];
}

_NAMESPACE_REALSPACE2_END