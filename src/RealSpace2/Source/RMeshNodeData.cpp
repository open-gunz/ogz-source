#include "stdafx.h"

#include "RMeshNodeData.h"
#include "RealSpace2.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RMeshNodeData::RMeshNodeData() 
{
	m_id = -1;
	m_u_id = -1;
	m_mtrl_id = -1;

	m_spine_local_pos.x = 0.f;
	m_spine_local_pos.y = 0.f;
	m_spine_local_pos.z = 0.f;

	m_ap_scale = rvector(1.f,1.f,1.f);

	m_axis_scale = rvector(0.f,0.f,0.f);
	m_axis_scale_angle = 0.f;

	m_axis_rot = rvector(0.f,0.f,1.f);
	m_axis_rot_angle = 0.f;

	GetIdentityMatrix(m_mat_base);
	GetIdentityMatrix(m_mat_parent_inv);
	GetIdentityMatrix(m_mat_local);
	GetIdentityMatrix(m_mat_inv);
	GetIdentityMatrix(m_mat_etc);
	GetIdentityMatrix(m_mat_flip);
	GetIdentityMatrix(m_mat_scale);
	GetIdentityMatrix(m_mat_result);

	GetIdentityMatrix(m_mat_add);

	GetIdentityMatrix(m_mat_ref);
	GetIdentityMatrix(m_mat_ref_inv);

	m_point_list		= NULL;
	m_face_normal_list	= NULL;
	m_point_color_list	= NULL;
	m_face_list			= NULL;
	m_physique			= NULL;

	m_point_num			= 0;
	m_face_num			= 0;
	m_physique_num		= 0;
	m_point_normal_num	= 0;
	m_point_color_num	= 0;

	m_min = rvector( 999.f, 999.f, 999.f);
	m_max = rvector(-999.f,-999.f,-999.f);

}

RMeshNodeData::~RMeshNodeData() 
{
	if(m_point_num) {
		DEL2(m_point_list);
	}

	if( m_point_color_num ) {
		DEL2(m_point_color_list);
	}

	if(m_face_num) {
		DEL2(m_face_list);
		DEL2(m_face_normal_list);
	}

	if(m_physique_num){
		DEL2(m_physique);
	}
}

void RMeshNodeData::BBoxClear()
{
	m_min = rvector( 999.f, 999.f, 999.f);
	m_max = rvector(-999.f,-999.f,-999.f);
}

void RMeshNodeData::SubCalc(rvector* v)
{
	if(v==NULL) return;

	if (v->x < m_min.x) m_min.x = v->x;
	if (v->y < m_min.y) m_min.y = v->y;
	if (v->z < m_min.z) m_min.z = v->z;

	if (v->x > m_max.x) m_max.x = v->x;
	if (v->y > m_max.y) m_max.y = v->y;
	if (v->z > m_max.z) m_max.z = v->z; 
}

void RMeshNodeData::CalcLocalBBox()
{
	BBoxClear();

	for(int i=0;i<m_point_num;i++)
	{
		SubCalc(&m_point_list[i]);
	}
}

RMeshNodeMtrl::RMeshNodeMtrl()
{
	m_dwTFactorColor = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f);
	m_pMtrlTable = NULL;
	m_nMtrlCnt = 0;
}

RMeshNodeMtrl::~RMeshNodeMtrl()
{
	if(m_pMtrlTable)
		DEL2(m_pMtrlTable);
}

void RMeshNodeMtrl::SetTColor(DWORD color)
{
	m_dwTFactorColor = color;
}

DWORD RMeshNodeMtrl::GetTColor()
{
	return m_dwTFactorColor;
}

int RMeshNodeMtrl::GetMtrlCount()
{
	return m_nMtrlCnt;
}

RMtrl* RMeshNodeMtrl::GetMtrl(int i)
{
	if(i<m_nMtrlCnt) {
		if(m_pMtrlTable) {
			return m_pMtrlTable[i];
		}
	}

	return NULL;
}

void RMeshNodeMtrl::SetMtrlDiffuse(RMtrl* pMtrl,float vis_alpha)
{
	if(!pMtrl) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	auto* c = &pMtrl->m_diffuse;

	mtrl.Diffuse.r = c->r*0.5f;
	mtrl.Diffuse.g = c->g*0.5f;
	mtrl.Diffuse.b = c->b*0.5f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = c->r*0.1f;
	mtrl.Ambient.g = c->g*0.1f;
	mtrl.Ambient.b = c->b*0.1f;
	mtrl.Ambient.a = 1.0f;

	mtrl.Specular.r = 0.5f;
	mtrl.Specular.g = 0.5f;
	mtrl.Specular.b = 0.5f;
	mtrl.Specular.a = 1.f;

	mtrl.Power = pMtrl->m_power;

	if( pMtrl->m_power ) {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	}

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( pMtrl, vis_alpha );
	}
}

void RMeshNodeMtrl::SetMtrl(RMtrl* pMtrl,float vis_alpha,bool bNpc,D3DCOLORVALUE color)
{
	if(!pMtrl) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	if(bNpc) {

		mtrl.Diffuse.r = color.r;
		mtrl.Diffuse.g = color.g;
		mtrl.Diffuse.b = color.b;
		mtrl.Diffuse.a = 1.0f;

		mtrl.Ambient.r = color.r/2.f;
		mtrl.Ambient.g = color.g/2.f;
		mtrl.Ambient.b = color.b/2.f;
		mtrl.Ambient.a = 1.0f;
	}
	else 
	{
		mtrl.Diffuse.r = color.r;
		mtrl.Diffuse.g = color.g;
		mtrl.Diffuse.b = color.b;
		mtrl.Diffuse.a = 1.0f;

		mtrl.Ambient.r = color.r/2.f;
		mtrl.Ambient.g = color.g/2.f;
		mtrl.Ambient.b = color.b/2.f;
		mtrl.Ambient.a = 1.0f;
	}

	mtrl.Specular.r = 0.5f;
	mtrl.Specular.g = 0.5f;
	mtrl.Specular.b = 0.5f;
	mtrl.Specular.a = 1.f;

	mtrl.Power = pMtrl->m_power;

	if( pMtrl->m_power ) {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	}

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( pMtrl, vis_alpha );
	}
}

void RMeshNodeMtrl::SetMtrl(color_r32* c,float vis_alpha) 
{
	if(!c) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	mtrl.Diffuse.r = c->r;
	mtrl.Diffuse.g = c->g;
	mtrl.Diffuse.b = c->b;
	mtrl.Diffuse.a = c->a;

	mtrl.Ambient.r = c->r*0.5f;
	mtrl.Ambient.g = c->g*0.5f;
	mtrl.Ambient.b = c->b*0.5f;
	mtrl.Ambient.a = 1.0f;

	mtrl.Specular.r = 1.0f;
	mtrl.Specular.g = 1.0f;
	mtrl.Specular.b = 1.0f;
	mtrl.Specular.a = 1.0f;

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( *c, vis_alpha );
	}
}

_NAMESPACE_REALSPACE2_END