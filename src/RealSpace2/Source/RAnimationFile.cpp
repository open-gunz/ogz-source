#include "stdafx.h"
#include "RAnimationFile.h"

#include "RealSpace2.h"

#include "MZFileSystem.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RAnimationFile::RAnimationFile()
{
	m_ani_node		= NULL;
	m_ani_node_cnt	= 0;
	m_ani_type		= RAniType_Bone;

	m_pBipRootNode = NULL;

	m_max_frame		= 0;
	m_nRefCount		= 0;

	AddRef();
}

RAnimationFile::~RAnimationFile()
{
	if( m_ani_node ) {
		for(int i=0;i<m_ani_node_cnt;i++) {
			delete m_ani_node[i];
		}
		delete[] m_ani_node;
		m_ani_node = NULL;
	}

	DecRef();

	if(m_nRefCount != 0) {

	}
}

void RAnimationFile::AddRef()
{
	m_nRefCount++;
}

void RAnimationFile::DecRef()
{
	m_nRefCount--;
}

RAnimationNode* RAnimationFile::GetNode(const char* name)
{
	for(int i=0;i<m_ani_node_cnt;i++) {
		if(m_ani_node[i]->CheckName(name))
			return m_ani_node[i];
	}
	return NULL;
}

bool RAnimationFile::LoadAni(const char* filename)
{
	int node_cnt=0;

	char t_mesh_name[256];

	MZFile mzf;

	if (!mzf.Open(filename, g_pFileSystem)) {
		mlog("in zip : %s file not found!! \n ", filename);
		return false;
	}

	ex_ani_t t_hd;

	mzf.Read(&t_hd,sizeof(ex_ani_t));

	u32 ver = t_hd.ver;

	m_ani_node_cnt = t_hd.model_num;

	if( m_ani_node_cnt == 0 ) {
		mlog("키가 없는 %s 에니메이션 사용\n",filename);
		mzf.Close();
		return false;
	}

	m_ani_node = new RAnimationNode*[m_ani_node_cnt];

	RAnimationNode* pANode = NULL;

	int vis_max_frame = 0;
	int max_frame = 0;

	int mode = t_hd.ani_type;

	m_ani_type = (AnimationType)mode;

	if( mode == RAniType_Vertex) {

		int i,j,vcnt;

		for(i=0;i<m_ani_node_cnt;i++) {

			m_ani_node[i] = new RAnimationNode;

			pANode = m_ani_node[i];

			mzf.Read(t_mesh_name ,MAX_NAME_LEN );
			pANode->SetName(t_mesh_name);

			mzf.Read(&pANode->m_vertex_cnt,4 );

			if(pANode->m_vertex_cnt) {
				pANode->m_vertex = new rvector*[pANode->m_vertex_cnt];
			}

			mzf.Read(&vcnt,4 );

			pANode->m_vertex_vcnt = vcnt;

			if(pANode->m_vertex_cnt) {
				pANode->m_vertex_frame = new u32[pANode->m_vertex_cnt];
			}

			mzf.Read(pANode->m_vertex_frame,sizeof(u32)*pANode->m_vertex_cnt);

			for(j=0;j<pANode->m_vertex_cnt;j++) {

				pANode->m_vertex[j] = new rvector[vcnt];
				mzf.Read(pANode->m_vertex[j],sizeof(rvector)*vcnt);
			}

			if(ver > EXPORTER_ANI_VER1) {
				mzf.Read(&pANode->m_vis_cnt,sizeof(u32) );

				if(pANode->m_vis_cnt) {
					pANode->m_vis = new RVisKey[pANode->m_vis_cnt];
					mzf.Read(pANode->m_vis,sizeof(RVisKey)*pANode->m_vis_cnt);

					if(pANode->m_vis[pANode->m_vis_cnt-1].frame > vis_max_frame) {
						vis_max_frame = pANode->m_vis[ pANode->m_vis_cnt-1 ].frame;
					}
				}
			}
		}

		int cnt = 0;

		for(i=0;i<m_ani_node_cnt;i++) {

			cnt = m_ani_node[i]->m_vertex_cnt;

			if(cnt) {
				max_frame = m_ani_node[i]->m_vertex_frame[cnt-1];
				break;
			}
		}

	}
	else if( mode == RAniType_Tm ) {

		for(int i=0;i<m_ani_node_cnt;i++) {

			m_ani_node[i] = new RAnimationNode;

			pANode = m_ani_node[i];

			mzf.Read(t_mesh_name ,MAX_NAME_LEN );
			pANode->SetName(t_mesh_name);

			mzf.Read(&pANode->m_mat_cnt,4 );
			pANode->m_mat = new RTMKey[pANode->m_mat_cnt];

			mzf.Read(pANode->m_mat,sizeof(RTMKey)*pANode->m_mat_cnt);

			if(ver > EXPORTER_ANI_VER1) {
				mzf.Read(&pANode->m_vis_cnt,sizeof(u32) );

				if(pANode->m_vis_cnt) {
					pANode->m_vis = new RVisKey[pANode->m_vis_cnt];
					mzf.Read(pANode->m_vis,sizeof(RVisKey)*pANode->m_vis_cnt);

					if(pANode->m_vis[pANode->m_vis_cnt-1].frame > vis_max_frame) {
						vis_max_frame = pANode->m_vis[ pANode->m_vis_cnt-1 ].frame;
					}
				}
			}
			pANode->m_mat_base = pANode->m_mat[0];
		}

		int cnt = 0;

		for(int i=0;i<m_ani_node_cnt;i++) {

			cnt = m_ani_node[i]->m_mat_cnt;

			if(cnt) {
				max_frame = m_ani_node[i]->m_mat[cnt-1].frame;
				break;
			}
		}

	}
	else {

		for(int i=0;i<m_ani_node_cnt;i++) {

			m_ani_node[i] = new RAnimationNode;

			pANode = m_ani_node[i];

			mzf.Read(t_mesh_name  ,MAX_NAME_LEN );
			mzf.Read(&pANode->m_mat_base,sizeof(rmatrix) );
			pANode->SetName(t_mesh_name);

			if(strcmp(pANode->GetName(),"Bip01")==0) {
				m_pBipRootNode = pANode;
			}

			int pos_key_num = 0;
			int rot_key_num = 0;
			int vertex_num	= 0;

			mzf.Read(&pos_key_num,4 );

			pANode->m_pos_cnt = pos_key_num;

			if(pos_key_num) {

				pANode->m_pos = new RPosKey[pos_key_num+1];

				mzf.Read(pANode->m_pos,sizeof(RPosKey)*pos_key_num);

				pANode->m_pos[pos_key_num] = pANode->m_pos[pos_key_num-1];

				if(pANode->m_pos[pos_key_num].frame > max_frame) {
					max_frame = pANode->m_pos[pos_key_num].frame;
				}
			}

			mzf.Read(&rot_key_num,4 );

			pANode->m_rot_cnt = rot_key_num;

			if(rot_key_num) {

				RQuatKey q;

				rquaternion eq,q1,q2;

				pANode->m_quat = new RQuatKey[rot_key_num+1];

				RRotKey t_rk;

				for(int j=0;j<rot_key_num;j++) {

					if(ver > EXPORTER_ANI_VER3) {

						mzf.Read(&pANode->m_quat[j],sizeof(RQuatKey) );
						static_assert(sizeof(RQuatKey) == 20, "Wrong RQuatKey size");

					} else {// old

						mzf.Read(&t_rk,sizeof(RRotKey) );

						RRot2Quat(q,t_rk);

						memcpy(&pANode->m_quat[j] , &q , sizeof(RQuatKey));
						pANode->m_quat[j].frame  = t_rk.frame;
					}
				}

				pANode->m_quat[rot_key_num] = pANode->m_quat[rot_key_num-1];

				if(pANode->m_quat[rot_key_num].frame > max_frame) {
					max_frame = pANode->m_quat[rot_key_num].frame;
				}
			}

			if(ver > EXPORTER_ANI_VER1) {
				mzf.Read(&pANode->m_vis_cnt,sizeof(u32) );

				if(pANode->m_vis_cnt) {
					pANode->m_vis = new RVisKey[pANode->m_vis_cnt];
					mzf.Read(pANode->m_vis,sizeof(RVisKey)*pANode->m_vis_cnt);

					if(pANode->m_vis[pANode->m_vis_cnt-1].frame > vis_max_frame) {
						vis_max_frame = pANode->m_vis[ pANode->m_vis_cnt-1 ].frame;
					}
				}
			}
		}
	}

	m_max_frame = max_frame;

	if(m_max_frame < vis_max_frame) {
		m_max_frame = vis_max_frame;
	}

	mzf.Close();

	return true;
}

_NAMESPACE_REALSPACE2_END