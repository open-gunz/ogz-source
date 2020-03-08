#include "el_mesh.h"

mesh_data::mesh_data()
{
//	memset(this,0,sizeof(mesh_data));

	m_id = -1;
	m_u_id = -1;

	List = NULL;

	m_Name[0] = 0;
	m_Parent[0] = 0;

	m_pParent = NULL;

	D3DXMatrixIdentity(&m_mat_base);
	D3DXMatrixIdentity(&m_mat_init);
	D3DXMatrixIdentity(&m_etc_mat);

	m_ap_scale = D3DXVECTOR3(0.f,0.f,0.f);

	m_axis_scale = D3DXVECTOR3(0.f,0.f,0.f);
	m_axis_scale_angle = 0.f;

	m_axis_rot = D3DXVECTOR3(0.f,0.f,0.f);
	m_axis_rot_angle = 0.f;

	m_is_rot = false;
	m_is_pos = false;

	m_is_vertex_ani = false;

	// face info

	m_point_num			= 0;
	m_face_num			= 0;
	m_point_color_num	= 0;

	m_point_list		= NULL;
	m_face_list			= NULL;

	m_point_color_list	= NULL;
	m_face_normal_list	= NULL;

	m_physique_num = 0;
	m_physique = NULL;

	m_vertex_ani_list = NULL;
	m_vertex_ani_frame = NULL;
	m_vertex_ani_num = 0;

	m_mtrl_id = 0;

	m_tex_face_num = 0;

	m_NormalFlag = 0;

	m_pos_key = NULL;
	m_quat_key = NULL;
	m_tm_key = NULL;
	m_vis_key = NULL;

	m_pos_key_num = 0;
	m_quat_key_num =0;
	m_tm_key_num = 0;
	m_vis_key_num = 0;

	m_frame = 0;
	m_max_frame = 0;
	m_max_frame_tick = 0;
}

mesh_data::~mesh_data()
{
	if(m_face_num)			DEL2(m_face_list);
	if(m_point_num)			DEL2(m_point_list);
	if(m_tex_point_num)		DEL2(m_tex_point_list);

	DEL2(m_face_normal_list);

	if(m_physique_num)		DEL2(m_physique);

	if(m_pos_key_num)		DEL2(m_pos_key);
	if(m_quat_key_num)		DEL2(m_quat_key);
	if(m_tm_key_num)		DEL2(m_tm_key);
	if(m_vis_key_num)		DEL2(m_vis_key);

	if(m_point_color_num)	DEL2(m_point_color_list);
//	if(m_point_normal_num)	DEL2(m_point_normal_list);

	if(m_vertex_ani_num) {
		for(int i=0;i<m_vertex_ani_num;i++)
			DEL2(m_vertex_ani_list[i]);
		DEL2(m_vertex_ani_list);
	}
}


el_mesh::el_mesh()
{
	m_data_num		= 0;
	m_max_frame		= 0;
	m_mtrl_num		= 0;
	
//	memset(m_data,0,MAX_NODE*4);
//	memset(m_mtrl_data,0,MAX_NODE*4);

	m_data.reserve(MAX_NODE);//기본

	for(int i=0;i<MAX_NODE;i++)
		m_data[i] = NULL;

	m_mtrl_data.reserve(MAX_NODE);//기본

	for(int i=0;i<MAX_NODE;i++)
		m_mtrl_data[i] = NULL;
}

el_mesh::~el_mesh()
{
	del_mesh_list();
}

void el_mesh::ClearUsedMtrlCheck()
{
	if(m_mtrl_list.empty())
		return;

	el_mtrl_node node;

	mtrl_data* pMtrl = NULL;

	for(node = m_mtrl_list.begin(); node != m_mtrl_list.end(); ++node) {

		pMtrl = (*node);

		if(pMtrl) {
			pMtrl->m_bUse = false;
		}
	}

	return ;
}

mtrl_data* el_mesh::GetMtrl(int mtrl_id,int sub_id)
{
	if(m_mtrl_list.empty())
		return NULL;

	el_mtrl_node node;

	mtrl_data* pMtrl = NULL;

	for(node = m_mtrl_list.begin(); node != m_mtrl_list.end(); ++node) {

		pMtrl = (*node);

		if(pMtrl) {
			if(pMtrl->m_mtrl_id == mtrl_id) {
				if(pMtrl->m_sub_mtrl_id == sub_id) {
					return pMtrl;
				}
			}
		}
	}

	return NULL;
}

void el_mesh::ClearUsedMtrl()
{
	if(m_mtrl_list.empty())
		return;

	el_mtrl_node node;

	mtrl_data* pMtrl = NULL;

	for(node = m_mtrl_list.begin(); node != m_mtrl_list.end(); ) {

		pMtrl = (*node);

		if(pMtrl) {
			if(pMtrl->m_bUse==false) {
				delete pMtrl;
				pMtrl = NULL;
				node = m_mtrl_list.erase(node);// ++node
			} else {
				++node;
			}
		}
		else 
			++node;
	}

	m_mtrl_num = 0;

	for(node = m_mtrl_list.begin(); node != m_mtrl_list.end(); ++node) {

		pMtrl = (*node);

		if(pMtrl) {
			m_mtrl_data[ m_mtrl_num] = pMtrl;
			m_mtrl_num++;
		}
	}

	return;
}

void el_mesh::ClearVoidMtrl()
{
	mesh_data*	pMeshNode = NULL;
	mtrl_data*	pMtrl = NULL;
	mtrl_data*	pSMtrl = NULL;

	int mtrl_size = m_mtrl_list.size();

	if(!mtrl_size)
		return;

	ClearUsedMtrlCheck();

	el_mesh_node it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		pMeshNode = (*it_obj);

		if(pMeshNode) {

			pMtrl = GetMtrl(pMeshNode->m_mtrl_id,-1);

			if(pMtrl) {

				pMtrl->m_bUse = true;

				if( pMtrl->m_sub_mtrl_num ) {

					for(int i=0;i<pMtrl->m_sub_mtrl_num;i++) {

						pSMtrl = GetMtrl(pMeshNode->m_mtrl_id,i);

						if(pSMtrl) {
							pSMtrl->m_bUse = true;
						}
					}
				}
			}
		}
		it_obj++;
	}

	ClearUsedMtrl();
}

int el_mesh::add_mesh(mesh_data* data)
{
//  일정양 이상 예약하는것도 속도증가
//	if(m_data_num > MAX_NODE-1) return -1;

	m_list.push_back(data);
	
	m_data[ m_data_num] = data;
	m_data_num++;
	return m_data_num-1;
}

int el_mesh::add_mtrl(mtrl_data* data)
{
//	if(m_mtrl_num > MAX_NODE-1) return -1;

	m_mtrl_list.push_back(data);
	
	m_mtrl_data[ m_mtrl_num] = data;
	m_mtrl_num++;
	return m_mtrl_num-1;
}

void el_mesh::del_mesh_list()
{
	el_mtrl_node node_ =  m_mtrl_list.begin();

	while (node_ !=  m_mtrl_list.end()) {
		delete (*node_);
		node_ =  m_mtrl_list.erase(node_);
	}
	
	el_mesh_node node =  m_list.begin();
	
	mesh_data* pMesh=NULL;

	while (node !=  m_list.end()) {
		delete (*node);
		node =  m_list.erase(node);
	}
}

int el_mesh::ani_node_cnt()
{
	int cnt = 0;

	el_mesh_node node =  m_list.begin();

	while (node !=  m_list.end()) {
		if( (*node)->m_is_pos || (*node)->m_is_rot )
			cnt++;
		node++;
	}
	return cnt;
}

extern BOOL g_bip_mesh_out;

bool el_mesh::export_text(char* filename)
{
	FILE* fp;
	fp  = fopen(filename,"wt");
	if(fp==NULL)  return false;

	int i,j,k;
	mtrl_data* t_mtrl;

	fprintf(fp,"mtrl_num %d\n",m_mtrl_num);

	for(i=0;i<m_mtrl_num;i++) {

		t_mtrl = m_mtrl_data[i];

		if(t_mtrl == NULL) continue;

		fprintf(fp,"mtrl_id %d %d\n",t_mtrl->m_mtrl_id,t_mtrl->m_sub_mtrl_id);

		fprintf(fp,"ambient  %f %f %f \n",t_mtrl->m_ambient.r ,t_mtrl->m_ambient.g ,t_mtrl->m_ambient.b);
		fprintf(fp,"diffuse  %f %f %f \n",t_mtrl->m_diffuse.r ,t_mtrl->m_diffuse.g ,t_mtrl->m_diffuse.b);
		fprintf(fp,"specular %f %f %f \n",t_mtrl->m_specular.r,t_mtrl->m_specular.g,t_mtrl->m_specular.b);

		fprintf(fp,"power %f \n",t_mtrl->m_power);

		fprintf(fp,"additive %d \n",t_mtrl->m_additive);

		fprintf(fp,"tex_name %s\n",		t_mtrl->m_tex_name);
		fprintf(fp,"opa_name %s\n",		t_mtrl->m_opa_name);
		fprintf(fp,"sub_mtrl_num %d\n",	t_mtrl->m_sub_mtrl_num);
		fprintf(fp,"twoside %d\n",		t_mtrl->m_twosided);
		fprintf(fp,"alpha_test_ref %d\n",t_mtrl->m_alphatest_ref);


	}

	mesh_data* t_mesh;

	bool t_is_bip = false;

	fprintf(fp,"mesh_num %d\n",m_data_num);

	for(i=0;i<m_data_num;i++) {

		t_is_bip = false;

		t_mesh = m_data[i];
		if(t_mesh == NULL) continue;

		fprintf(fp,"name %s\n",t_mesh->m_Name);
		fprintf(fp,"pname %s\n",t_mesh->m_Parent);
		fprintf(fp,"mat\n");//,t_mesh->m_mat_base);

		fprintf(fp,"%f %f %f\n",	t_mesh->m_mat_base._11,	t_mesh->m_mat_base._12,	t_mesh->m_mat_base._13 );
		fprintf(fp,"%f %f %f\n",	t_mesh->m_mat_base._21,	t_mesh->m_mat_base._22,	t_mesh->m_mat_base._23 );
		fprintf(fp,"%f %f %f\n",	t_mesh->m_mat_base._31,	t_mesh->m_mat_base._32,	t_mesh->m_mat_base._33 );
		fprintf(fp,"%f %f %f\n",	t_mesh->m_mat_base._41,	t_mesh->m_mat_base._42,	t_mesh->m_mat_base._43 );

		fprintf(fp,"ap_scale (%f %f %f)\n",
			t_mesh->m_ap_scale.x,	
			t_mesh->m_ap_scale.y,	
			t_mesh->m_ap_scale.z );

		fprintf(fp,"axis_scale (%f %f %f) : %f\n",	
			t_mesh->m_axis_scale.x,	
			t_mesh->m_axis_scale.y,	
			t_mesh->m_axis_scale.z,	
			t_mesh->m_axis_scale_angle);

		fprintf(fp,"axis_rot (%f %f %f) : %f\n",
			t_mesh->m_axis_rot.x,	
			t_mesh->m_axis_rot.y,	
			t_mesh->m_axis_rot.z,	
			t_mesh->m_axis_rot_angle);
/*
		if( t_mesh->m_Name[0]=='B' && t_mesh->m_Name[1]=='i' && t_mesh->m_Name[2]=='p')
			t_is_bip = true;

		if(!g_bip_mesh_out && t_is_bip) continue; //bip 제거에 bip 라면
*/
		/////////////////////////////////////////////////////////////////////

		fprintf(fp,"point_num %d\n",t_mesh->m_point_num);

		for(j=0;j<t_mesh->m_point_num;j++) {

			fprintf(fp,"point %f %f %f\n",t_mesh->m_point_list[j].x,t_mesh->m_point_list[j].y,t_mesh->m_point_list[j].z);
		}

		fprintf(fp,"face_num %d\n",t_mesh->m_face_num);

		for(j=0;j<t_mesh->m_face_num;j++) {

			fprintf(fp,"face_index : %d %d %d ",
				t_mesh->m_face_list[j].m_point_index[0],
				t_mesh->m_face_list[j].m_point_index[1],
				t_mesh->m_face_list[j].m_point_index[2]);

			fprintf(fp,"face_tex : %f %f %f %f %f %f ",
				t_mesh->m_face_list[j].m_point_tex[0].x,
				t_mesh->m_face_list[j].m_point_tex[0].y,
				t_mesh->m_face_list[j].m_point_tex[1].x,
				t_mesh->m_face_list[j].m_point_tex[1].y,
				t_mesh->m_face_list[j].m_point_tex[2].x,
				t_mesh->m_face_list[j].m_point_tex[2].y);

			fprintf(fp,"face_mtrl_id : %d",t_mesh->m_face_list[j].m_mtrl_id);
			fprintf(fp,"face_smooth_id : %d\n",t_mesh->m_face_list[j].m_sg_id);
		}

		fprintf(fp,"point_color_num %d\n",t_mesh->m_point_color_num);

		for(j=0;j<t_mesh->m_point_color_num;j++) {

			fprintf(fp,"point color : %d %f %f %f\n",j,t_mesh->m_point_color_list[j].x,t_mesh->m_point_color_list[j].y,t_mesh->m_point_color_list[j].z);
		}

		//////////////////////////////////////////////////////////////////////

		fprintf(fp,"pos_key_num %d\n",t_mesh->m_pos_key_num);

		for(j=0;j<t_mesh->m_pos_key_num;j++) {

			fprintf(fp,"pos_key %d %f %f %f\n",	t_mesh->m_pos_key[j].frame,	t_mesh->m_pos_key[j].x,	t_mesh->m_pos_key[j].y,	t_mesh->m_pos_key[j].z);
		}

		fprintf(fp,"quat_key_num %d\n",t_mesh->m_quat_key_num);

		for(j=0;j<t_mesh->m_quat_key_num;j++) {

			fprintf(fp,"quat_key %d %f %f %f %f\n",	t_mesh->m_quat_key[j].frame,t_mesh->m_quat_key[j].x,t_mesh->m_quat_key[j].y,t_mesh->m_quat_key[j].z,t_mesh->m_quat_key[j].w );
		}

		fprintf(fp,"mtrl_id %d\n",t_mesh->m_mtrl_id);

		fprintf(fp,"physique_num %d\n",t_mesh->m_physique_num);

		for(j=0;j<t_mesh->m_physique_num;j++) {

			fprintf(fp,"num %d\n",t_mesh->m_physique[j].m_num);

			for(k=0;k<t_mesh->m_physique[j].m_num;k++) {

				fprintf(fp,"%s %f offset (%f %f %f)\n",
					t_mesh->m_physique[j].m_parent_name[k],
					t_mesh->m_physique[j].m_weight[k],
					t_mesh->m_physique[j].m_offset[k].x,
					t_mesh->m_physique[j].m_offset[k].y,
					t_mesh->m_physique[j].m_offset[k].z);
			}
		}
	}

	fclose(fp);
	return true;
}

bool el_mesh::export_bin(char* filename)
{
	ex_hd_t t_hd;

	FILE* fp;
	fp = fopen(filename,"wb");
	if(fp==NULL) return false;

	t_hd.ver = EXPORTER_MESH_VER8;
	t_hd.sig = EXPORTER_SIG;
	t_hd.mtrl_num = m_mtrl_num;
	t_hd.mesh_num = m_data_num;

	fwrite(&t_hd,sizeof(ex_hd_t),1,fp);

	int i;

	mtrl_data* t_mtrl;

	for(i=0;i<m_mtrl_num;i++) {

		t_mtrl = m_mtrl_data[i];

		if(t_mtrl == NULL) continue;

		// 구조 정해질때 까지 하나씩 출력

		fwrite(&t_mtrl->m_mtrl_id    ,4,1,fp);
		fwrite(&t_mtrl->m_sub_mtrl_id,4,1,fp);

		fwrite(&t_mtrl->m_ambient ,sizeof(D3DXCOLOR),1,fp);
		fwrite(&t_mtrl->m_diffuse ,sizeof(D3DXCOLOR),1,fp);
		fwrite(&t_mtrl->m_specular,sizeof(D3DXCOLOR),1,fp);

		fwrite(&t_mtrl->m_power,4,1,fp);

		fwrite(&t_mtrl->m_sub_mtrl_num,4,1,fp);
		// ver EXPORTER_MESH_VER6
//		fwrite(t_mtrl->m_tex_name,MAX_NAME_LEN,1,fp);
//		fwrite(t_mtrl->m_opa_name,MAX_NAME_LEN,1,fp);
		// ver EXPORTER_MESH_VER7
		fwrite(t_mtrl->m_tex_name,MAX_PATH_NAME_LEN,1,fp);
		fwrite(t_mtrl->m_opa_name,MAX_PATH_NAME_LEN,1,fp);

		fwrite(&t_mtrl->m_twosided,sizeof(int),1,fp);
		fwrite(&t_mtrl->m_additive,sizeof(int),1,fp);
		fwrite(&t_mtrl->m_alphatest_ref,sizeof(int),1,fp);
	}

	mesh_data* t_mesh;

	bool t_is_bip = false;
	int  t_is_vertex_color = 0;

	for(i=0;i<m_data_num;i++) {

		t_is_bip = false;
		t_is_vertex_color = 0;

		t_mesh = m_data[i];

		if(t_mesh == NULL) continue;
        
		fwrite(t_mesh->m_Name  ,MAX_NAME_LEN,1,fp);
		fwrite(t_mesh->m_Parent,MAX_NAME_LEN,1,fp);

		////////////////////////////////////////////////////

//		if( strncmp(t_mesh->m_Name,"flag",4)==0 ) {
			t_is_vertex_color = t_mesh->m_point_color_num;
//		}

		fwrite(&t_mesh->m_mat_base,sizeof(D3DXMATRIX),1,fp);	// mat

		fwrite(&t_mesh->m_ap_scale,sizeof(D3DXVECTOR3),1,fp);	// ver2+

		// ver4
		fwrite(&t_mesh->m_axis_rot,sizeof(D3DXVECTOR3),1,fp);
		fwrite(&t_mesh->m_axis_rot_angle,sizeof(float),1,fp);

		fwrite(&t_mesh->m_axis_scale,sizeof(D3DXVECTOR3),1,fp);
		fwrite(&t_mesh->m_axis_scale_angle,sizeof(float),1,fp);

		fwrite(&t_mesh->m_etc_mat,sizeof(D3DXMATRIX),1,fp);	// mat

/*
		if( t_mesh->m_Name[0]=='B' && t_mesh->m_Name[1]=='i' && t_mesh->m_Name[2]=='p')
			t_is_bip = true;

		if(!g_bip_mesh_out && t_is_bip)//bip 제거에 bip 라면
		{
			DWORD v = 0;
			fwrite(&v,4,1,fp);
			fwrite(&v,4,1,fp); v = -1;
			fwrite(&v,4,1,fp); v = 0;
			fwrite(&v,4,1,fp);
		}
		else
		{
*/
			fwrite(&t_mesh->m_point_num,4,1,fp);

			if(t_mesh->m_point_num) {
				fwrite(t_mesh->m_point_list,sizeof(D3DXVECTOR3),t_mesh->m_point_num,fp);
				// ver 6
//				fwrite(t_mesh->m_point_normal_list,sizeof(D3DXVECTOR3),t_mesh->m_point_num,fp);
			}

			fwrite(&t_mesh->m_face_num,4,1,fp);

			if(t_mesh->m_face_num) {
				fwrite(t_mesh->m_face_list,sizeof(RFaceInfo),t_mesh->m_face_num,fp);
				// ver 6
				fwrite(t_mesh->m_face_normal_list,sizeof(RFaceNormalInfo),t_mesh->m_face_num,fp);
			}

			// ver 6

			fwrite(&t_is_vertex_color,sizeof(int),1,fp);

			if(t_is_vertex_color) {
				fwrite( t_mesh->m_point_color_list,sizeof(D3DXVECTOR3),t_mesh->m_point_color_num,fp );
			}

			// ani out remove

			fwrite(&t_mesh->m_mtrl_id,4,1,fp);

			fwrite(&t_mesh->m_physique_num,4,1,fp);

			if(t_mesh->m_physique_num) {

				for(int j=0;j<t_mesh->m_physique_num;j++) {

					fwrite(&t_mesh->m_physique[j],sizeof(RPhysiqueInfo),1,fp);
				}
			}
//		}
	}

	fclose(fp);

	return true;
}

bool el_mesh::export_etc(char* filename)
{
	FILE* fp;
	fp = fopen(filename,"wb");

	if(fp==NULL) return false;

	int i,j,k;

	mesh_data* t_mesh;

	WORD face_num = 0;
	WORD vertex_num = 3;
	WORD tex_index = 0;

	DWORD info = 1;

	D3DCOLOR color = 0xffffffff;
	D3DCOLOR specular = 0xffffffff;


	for(i=0;i<m_data_num;i++) {
		t_mesh = m_data[i];
		face_num += t_mesh->m_face_num;
	}

	fwrite(&face_num,sizeof(WORD),1,fp);
	
	RFaceInfo* pF = NULL;

	for(i=0;i<m_data_num;i++) {

		t_mesh = m_data[i];

		if(t_mesh == NULL) continue;

		for(j=0;j<t_mesh->m_face_num;j++) {

			pF = &t_mesh->m_face_list[j];

			fwrite(&vertex_num,sizeof(WORD),1,fp);

			for(k=0;k<3;k++) {
			
				fwrite(&t_mesh->m_point_list[pF->m_point_index[k]].x,sizeof(float),1,fp);
				fwrite(&t_mesh->m_point_list[pF->m_point_index[k]].y,sizeof(float),1,fp);
				fwrite(&t_mesh->m_point_list[pF->m_point_index[k]].z,sizeof(float),1,fp);

				fwrite(&info,sizeof(DWORD),1,fp);

				fwrite(&color,sizeof(D3DCOLOR),1,fp);
				fwrite(&specular,sizeof(D3DCOLOR),1,fp);

				fwrite(&pF->m_point_tex[k].x,sizeof(float),1,fp);
				fwrite(&pF->m_point_tex[k].y,sizeof(float),1,fp);
			}

			fwrite(&tex_index,sizeof(WORD),1,fp);
		}
	}

	fclose(fp);

	return true;
}

void rot2quat( RQuatKey& q,RQuatKey& v )
{
	D3DXQUATERNION out;
	D3DXVECTOR3 vec;

	vec.x = v.x;	
	vec.y = v.y;	
	vec.z = v.z;

	D3DXQuaternionRotationAxis(&out,&vec,v.w);

	q.x = out.x;	
	q.y = out.y;	
	q.z = out.z;	
	q.w = out.w;
	q.frame = v.frame;
}

bool el_mesh::export_ani(char* filename,int mode)
{
	char ani_file_name[256];
	strcpy(ani_file_name, filename);

//	if(mode==RAniType_Vertex)
//		strcat(ani_file_name,".vani");
//	else
		strcat(ani_file_name,".ani");

	FILE* fp;
	fp  = fopen(ani_file_name,"wb");

	if(fp==NULL) return false;

	ex_ani_t t_hd;

	t_hd.ver		= EXPORTER_ANI_VER4;
	t_hd.sig		= EXPORTER_SIG;
	t_hd.maxframe	= m_max_frame;
	t_hd.model_num	= m_data_num;
	t_hd.ani_type	= mode;

	fwrite(&t_hd,sizeof(ex_ani_t),1,fp);

	mesh_data* t_mesh;

	if( mode == RAniType_Vertex) {

		int i=0,j=0;

		for(i=0;i<m_data_num;i++) {//node 수만큼

			t_mesh = m_data[i];

			if(t_mesh == NULL) continue;

			fwrite( t_mesh->m_Name ,MAX_NAME_LEN,1,fp );

			fwrite(&t_mesh->m_vertex_ani_num,4,1,fp);	
			fwrite(&t_mesh->m_point_num,4,1,fp);

			fwrite(t_mesh->m_vertex_ani_frame,sizeof(DWORD),t_mesh->m_vertex_ani_num,fp);

			for(j=0;j<t_mesh->m_vertex_ani_num;j++) {
				fwrite( t_mesh->m_vertex_ani_list[j], sizeof(D3DXVECTOR3) * t_mesh->m_point_num,1,fp);
			}

			/////// ver2 /////////

			fwrite(&t_mesh->m_vis_key_num,4,1,fp);

			if(t_mesh->m_vis_key_num) {
				fwrite( t_mesh->m_vis_key, sizeof(RVisKey), t_mesh->m_vis_key_num, fp);
			}
		}
	} 
	else if( mode == RAniType_Tm ) {

		int i=0,j=0;

		for(i=0;i<m_data_num;i++) {

			t_mesh = m_data[i];

			if(t_mesh == NULL) continue;

			fwrite( t_mesh->m_Name ,MAX_NAME_LEN,1,fp );

			fwrite(&t_mesh->m_tm_key_num,4,1,fp);

			fwrite(t_mesh->m_tm_key,sizeof(RTMKey),t_mesh->m_tm_key_num,fp);

			/////// ver2 /////////

			fwrite(&t_mesh->m_vis_key_num,4,1,fp);

			if(t_mesh->m_vis_key_num) {
				fwrite( t_mesh->m_vis_key, sizeof(RVisKey), t_mesh->m_vis_key_num, fp);
			}
		}
	}
	else {// trans + bone
	
		for(int i=0;i<m_data_num;i++) {

			t_mesh = m_data[i];
			if(t_mesh == NULL) continue;

			fwrite(t_mesh->m_Name  ,MAX_NAME_LEN,1,fp);
			fwrite(&t_mesh->m_mat_base,sizeof(D3DXMATRIX),1,fp);//mat

			fwrite(&t_mesh->m_pos_key_num,4,1,fp);

			if(t_mesh->m_pos_key_num)
				fwrite(t_mesh->m_pos_key,sizeof(RPosKey),t_mesh->m_pos_key_num,fp);

			/////// ver4 /////////

			RQuatKey q;

			for(int j=0;j<t_mesh->m_quat_key_num;j++) {

				q = t_mesh->m_quat_key[j];
				rot2quat( t_mesh->m_quat_key[j] , q );
//				memcpy(&pANode->m_quat[j],&q,sizeof(RQuatKey));
//				pANode->m_quat[j].frame  = t_rk.frame;
			}

			fwrite(&t_mesh->m_quat_key_num,4,1,fp);

			if(t_mesh->m_quat_key_num)
				fwrite(t_mesh->m_quat_key,sizeof(RQuatKey),t_mesh->m_quat_key_num,fp);

			/////// ver2 /////////

			fwrite(&t_mesh->m_vis_key_num,4,1,fp);

			if(t_mesh->m_vis_key_num) {
				fwrite( t_mesh->m_vis_key, sizeof(RVisKey), t_mesh->m_vis_key_num, fp);
			}
		}
	}

	fclose(fp);
	return true;
}

