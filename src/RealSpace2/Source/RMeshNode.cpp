#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include "MXml.h"
#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"
#include "MDebug.h"
#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"
#include "MZFileSystem.h"
#include "fileinfo.h"
#include "RShaderMgr.h"
#include "LogMatrix.h"

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

#define RVERTEX_MAX		1024*100*32			// 34133 face

RMeshNodeInfo::RMeshNodeInfo()
{
	m_isAddMeshNode = false;
	m_isDummy		= false;
	m_isDummyMesh	= false;
	m_isWeaponMesh	= false;
	m_isAlphaMtrl	= false;
	m_isCollisionMesh = false;
	m_isLastModel	= false;
	m_isClothMeshNode	= false;

	m_vis_alpha = 1.f;
	m_bClothMeshNodeSkip  = true;
	
	m_CutPartsType	  = cut_parts_upper_body;
	m_LookAtParts	  = lookat_parts_etc;
	m_WeaponDummyType = weapon_dummy_etc;

	m_PartsPosInfoType = eq_parts_pos_info_etc;
	m_PartsType = eq_parts_etc;

	m_nAlign = 0;

	m_AlphaSortValue = 0.f;
	m_bNpcWeaponMeshNode = false;
}

///////////////////////////////////////////////////////////////////////////////////

RBatch::RBatch()
{
	for(int i=0;i<MAX_PRIMITIVE;i++) {

		m_ib[i] = NULL;
	}

	m_bIsMakeVertexBuffer = false;

	m_vsb = NULL;
	m_vb = NULL;

}

RBatch::~RBatch()
{
	for(int i=0;i<MAX_PRIMITIVE;i++) {
		DEL(m_ib[i]);
	}

	m_bIsMakeVertexBuffer = false;

	DEL(m_vb);
	DEL(m_vsb);
}

bool RBatch::CreateVertexBuffer(char* pVert,DWORD fvf,int vertexsize,int vert_num,DWORD flag)
{
	if(m_vb==NULL) {
		m_vb = new RVertexBuffer;
		m_vb->Create(pVert,fvf,vertexsize,vert_num,flag);
	}
	return true;
}

bool RBatch::UpdateVertexBuffer(char* pVert)
{
	if(m_vb==NULL) return false;

	m_vb->UpdateData(pVert);

	return true;
}

bool RBatch::UpdateVertexBufferSoft(char* pVert)
{
	if(m_vb==NULL) return false;

	m_vb->UpdateDataSW(pVert);

	return true;
}

bool RBatch::CreateIndexBuffer(int index,WORD* pIndex,int _size)
{
	if(m_ib[index]==NULL) {
		m_ib[index] = new RIndexBuffer;
		m_ib[index]->Create(_size,pIndex);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
// RMeshNode Class

RMeshNode::RMeshNode()
{
	m_pParentMesh = NULL;
	m_pBaseMesh = NULL;

	m_Next = NULL;
	m_ChildRoot = NULL;

	m_pAnimationNode = NULL;

	m_Parent[0] = NULL;
	m_nParentNodeID = -1;

	m_pParent = NULL;

	int i = 0;

	m_MatrixCount = 0;

	for(i=0;i<MAX_MATRIX;i++) {
		m_MatrixMap[i] = 0;
	}

	m_BoneBaseMatrix = NULL;
	m_nBoneBaseMatrixCnt = 0;

	m_bConnectPhysique = false;
}

RMeshNode::~RMeshNode()
{

	DEL2(m_BoneBaseMatrix);

	m_nBoneBaseMatrixCnt = 0;

}

///////////////////////////////////////////////////////////

bool RMeshNode::ConnectMtrl()
{
	if (m_face_num == 0)
		return false;

	RMtrlMgr* pMtrlList = NULL;

	if(m_pParentMesh) {
		pMtrlList = &m_pParentMesh->m_mtrl_list_ex;

		RMtrl* pMtrl = NULL;

		if(pMtrlList) {
			pMtrl = pMtrlList->Get_s(m_mtrl_id,-1);

			if(pMtrl) {

				int mtrl_cnt = 1;

				if( pMtrl->m_sub_mtrl_num ) {// submtrl
					mtrl_cnt = pMtrl->m_sub_mtrl_num;

					m_pMtrlTable = new RMtrl*[mtrl_cnt];

					for (int s = 0; s < mtrl_cnt ; s ++) {
						m_pMtrlTable[s] = pMtrlList->Get_s(m_mtrl_id,s);
					}
				}
				else 
				{
					m_pMtrlTable = new RMtrl*[1];
					m_pMtrlTable[0] = pMtrl;
				}

				m_nMtrlCnt = mtrl_cnt;

				return true;
			}
		}
	}

	mlog( "%s MeshNode mtrl 연결실패\n" , m_Name.c_str() );

	return false;
}

#define MAX_BONE			100
#define MAX_MATRIX_LIMIT	20
#define MAX_VERTEX			1024*8


#define SetBVert(vt,_pos,_normal,u,v,w1,w2)				\
	vt.p=_pos;											\
	vt.matIndex[0]=0;									\
	vt.matIndex[1]=0;									\
	vt.matIndex[2]=0;									\
	vt.normal = _normal;								\
	vt.tu = u;											\
	vt.tv = v;											\
	vt.weight1 = w1;									\
	vt.weight2 = w2;									\


bool RMeshNode::SetBVertData(RBlendVertex* pBVert,int i,int j,int pv_index,int* DifferenceMap,int& matrixIndex)
{
	int point_index = m_face_list[i].m_point_index[j];
	RPhysiqueInfo* pPhysique = &m_physique[point_index];

	int point_pos = 3*i+j;

	float w1,w2;

	if( point_index < 0 || point_index >= m_point_num ) {
		mlog("Index of Vertex(Pointer) is Out of Range.. Point Index : %d, Num Vertices : %d, Mesh Node : %s \n",
			point_index, m_point_num, m_Name.c_str() );
		return false;
	}

	if( pPhysique->m_num > 3 || pPhysique->m_num <= 0 ) {
		mlog("%s mesh %s node %d face %d point -> physique 3 개 이상\n",m_pParentMesh->GetFileName() ,m_Name.c_str(),i,j);
		return false;
	}

	if( 1 == pPhysique->m_num ) {
		w1 = 1;
		w2 = 0;
	}
	else {
		w1 = pPhysique->m_weight[0];
		w2 = pPhysique->m_weight[1];
	}

	SetBVert(pBVert[point_pos],
		m_point_list[point_index],
		m_face_normal_list[i].m_pointnormal[j],
		m_face_list[i].m_point_tex[j].x,
		m_face_list[i].m_point_tex[j].y, 
		w1,w2);

	int index;

	for( int k = 0 ; k < pPhysique->m_num; ++k )
	{
		index = pPhysique->m_parent_id[k];

		if( DifferenceMap[index] == -1 )
		{
			DifferenceMap[index] = 3 * matrixIndex;
#ifdef USE_OLD_SKIN_VSO
			DifferenceMap[index] += ANIMATION_MATRIX_BASE;
#endif
			m_MatrixMap[matrixIndex++] = index;
		}

		pBVert[pv_index].matIndex[k] = float(DifferenceMap[index]);
	}

	return true;
}

void RMeshNode::MakeVertexBuffer(int index,bool lvert,char* pBuf,int _vsize,DWORD flag)
{
	DWORD fvf = 0;
	int vertsize = 0;

	if(lvert) {
		fvf = RLVertexType;
		vertsize = sizeof(RLVertex);
	}
	else {
		fvf = RVertexType;
		vertsize = sizeof(RVertex);
	}

	CreateVertexBuffer(pBuf,fvf,vertsize,_vsize,flag);

	m_bIsMakeVertexBuffer = true;
}

bool RMeshNode::MakeVSVertexBuffer()
{
	_ASSERT( m_physique );
	_ASSERT( m_pParentMesh );
	_ASSERT( (m_face_num * 3 ) < MAX_VERTEX );
	_ASSERT( RIsSupportVS() );
	_ASSERT( m_physique_num == m_point_num );

	if( (m_face_num * 3 ) > MAX_VERTEX ) {
		mlog("Point Number is larger than defined max vertex .. \n");
		return false;
	}

	if( !RIsSupportVS() )	return false;

	LPDIRECT3DDEVICE9 dev =	RGetDevice();

	int numMatrices = 0;
	int matrixIndex = 0;

	int DifferenceMap[MAX_BONE];

	memset( DifferenceMap , -1, sizeof(int)*MAX_BONE );

	static RBlendVertex pBVert[MAX_VERTEX];

	for (int i = 0; i < m_face_num; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (!SetBVertData(pBVert, i, j, 3 * i + j, DifferenceMap, matrixIndex))
				return false;
		}
	}

	if(m_vsb==NULL) {
		m_vsb = new RVertexBuffer;
		m_vsb->Create((char*)pBVert,RBLENDVERTEXTYPE,sizeof(RBlendVertex),m_face_num * 3,USE_VERTEX_HW);
	}

	if( MAX_MATRIX_LIMIT < matrixIndex )
	{
		m_MatrixCount = -1;
		return false;
	}

	m_MatrixCount = matrixIndex;

	return true;
}

void RMeshNode::RenderNodeVS(RMesh* pMesh, const rmatrix& pWorldMat_,ESHADER shader_ )
{
	int i;

	__BP(5009,"RMesh::RenderNodeVS");

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	rmatrix matTemp;
	rmatrix world;
	rmatrix view;
	rmatrix proj;
	rmatrix transformation;
	rmatrix tworldmat;

	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->mpMtrl->m_diffuse.a = min(m_vis_alpha,pMesh->m_fVis);
	}

	__BP(5010,"RMesh::RenderNodeVS_SetVertexShaderConstant");

	//Register Matrix
	dev->SetVertexShaderConstantF( CAMERA_POSITION, static_cast<float*>(RCameraPosition), 1 );

	dev->GetTransform(D3DTS_WORLD, static_cast<D3DMATRIX*>(world));
	dev->GetTransform(D3DTS_VIEW, static_cast<D3DMATRIX*>(view));
	dev->GetTransform(D3DTS_PROJECTION, static_cast<D3DMATRIX*>(proj));

	GetIdentityMatrix(matTemp);

	dev->SetVertexShaderConstantF( 0, (float*)&matTemp, 3 );

	for( i = 0 ; i < m_MatrixCount; ++i )
	{
		matTemp =  m_mat_ref * pMesh->m_data[ m_MatrixMap[i]]->m_mat_ref_inv * pMesh->m_data[ m_MatrixMap[i]]->m_mat_result;

		matTemp = Transpose(matTemp),
		dev->SetVertexShaderConstantF( ANIMATION_MATRIX_BASE + (i)*3, (float*)&matTemp, 3);
	}

	if( pMesh->m_isScale ) {
		tworldmat = ScalingMatrix(pMesh->m_vScale) * pWorldMat_;
	}
	else 
		tworldmat = pWorldMat_;

	// set Transformation matrix
	matTemp = Transpose(tworldmat);
	dev->SetVertexShaderConstantF( WORLD_MATRIX, (float*)&matTemp, 4 );

	transformation = view * proj;
	matTemp = Transpose(transformation);
	dev->SetVertexShaderConstantF( VIEW_PROJECTION_MATRIX, (float*)&matTemp, 4 );

	RMtrl *pMtrl =  m_pMtrlTable[0];
	int num_mtrl =  m_nMtrlCnt;

	__EP(5010);

	__BP(5011,"RMesh::RenderNodeVS_SetCharacterMtrl_ON");

	if( shader_ == SHADER_SKIN_SPEC )
	{
		BOOL b;
		RGetDevice()->GetLightEnable(1, &b);
		if( b )
		{
			dev->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
			dev->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

			dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG0, D3DTA_CURRENT );
		}
		else
		{
			dev->SetTexture( 1, NULL );
			dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	}
	else
	{
		dev->SetTexture( 1, NULL );
		dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

	__EP(5011);

	RMtrl* psMtrl;

	D3DPtr<IDirect3DVertexShader9> PrevShader;
	dev->GetVertexShader(MakeWriteProxy(PrevShader));
	
	dev->SetVertexDeclaration( RGetShaderMgr()->getShaderDecl(0) );
	dev->SetVertexShader( RGetShaderMgr()->getShader(shader_) );

	m_vsb->SetVSVertexBuffer();

	for( i = 0 ; i <  m_nMtrlCnt  ; ++i )
	{
		__BP(5012,"RMesh::SetCharacterMtrl_ON");

		psMtrl =  GetMtrl(i);
		pMesh->SetCharacterMtrl_ON( psMtrl,this, min( m_vis_alpha,pMesh->m_fVis),  GetTColor());
			
		__EP(5012);

		__BP(5013,"RMesh::RenderNodeVS_Update");

		RGetShaderMgr()->Update();

		__EP(5013);

		__BP(5014,"RMesh::RenderNodeVS_DrawPrimitive");

		m_vsb->RenderIndexBuffer(m_ib[i]);

		__EP(5014);

		pMesh->SetCharacterMtrl_OFF( psMtrl, min( m_vis_alpha,pMesh->m_fVis) );
	}

	__EP(5009);

	dev->SetVertexShader(PrevShader.get());
}


void RMeshNode::ConnectToNameID()
{
	int id = RGetMeshNodeStringTable()->Get( m_Name );
	
	if(id==-1) {
		mlog("등록불가 파츠 %s \n",m_Name.c_str());
	}

	m_NameID = id;
}

RBoneBaseMatrix* RMeshNode::GetBaseMatrix(int pid)
{
	for(int i=0;i<m_nBoneBaseMatrixCnt;i++) 
	{
		if( m_BoneBaseMatrix[i].id == pid )
			return &m_BoneBaseMatrix[i];
	}
	return NULL;
}

inline void SetVertex(RVertex* v,rvector& p,rvector& n,rvector& uv) {

	v->p = p;	
	v->n = n;
	v->tu = uv.x;
	v->tv = uv.y;
}

inline void SetLVertex(RLVertex* v,rvector& p,DWORD c,rvector& uv) {

	v->p = p;	
	v->color = c;
	v->tu = uv.x;
	v->tv = uv.y;
}

void RMeshNode::UpdateNodeBuffer()
{
	if( !m_vb ) 
		_ASSERT(0);

	bool lvert	= m_pBaseMesh->m_LitVertexModel;

	RFaceInfo* pFace = NULL;

	rvector* pP = m_point_list;
	RFaceNormalInfo* pFNL = NULL;

	if(!m_vb->m_pVert)
		_ASSERT(0);

	RVertex*	 pV  = (RVertex*)m_vb->m_pVert;
	RLVertex*	 pLV = (RLVertex*)m_vb->m_pVert;

	int w,p0,p1,p2;

	for (int i = 0; i < m_face_num  ; i ++) {

		w = 3*i;

		pFace = &m_face_list[i];
		pFNL = &m_face_normal_list[i];

		p0 = pFace->m_point_index[0];
		p1 = pFace->m_point_index[1];
		p2 = pFace->m_point_index[2];

		////////////////////////////////////////////////////

		if(lvert) {

			(pLV+w  )->p = pP[p0];
			(pLV+w+1)->p = pP[p1];
			(pLV+w+2)->p = pP[p2];
		} 
		else if(!RMesh::m_bVertexNormalOnOff) {

			(pV+w  )->p = pP[p0];
			(pV+w+1)->p = pP[p1];
			(pV+w+2)->p = pP[p2];

		} else {

			(pV+w  )->p = pP[p0];
			(pV+w+1)->p = pP[p1];
			(pV+w+2)->p = pP[p2];
		}
	}
}

void RMeshNode::MakeNodeBuffer(DWORD flag)
{
	static char	_pVert[RVERTEX_MAX];
	static WORD _pIndex[RVERTEX_MAX*3];

	bool lvert	= m_pBaseMesh->m_LitVertexModel;

	RFaceInfo* pFace = NULL;

	rvector* pP = m_point_list;
	RFaceNormalInfo* pFNL = NULL;

	RVertex*	 pV  = (RVertex*)_pVert;
	RLVertex*	 pLV = (RLVertex*)_pVert;

	DWORD color = 0xffffffff;

	int w,w2,p0,p1,p2;

	if (m_face_num * 3 * sizeof(RVertex) > RVERTEX_MAX - 20)
		mlog("RMeshNode::MakeNodeBuffer - Too many vertices! Can handle at most %d, got %d\n",
			RVERTEX_MAX - 20, m_face_num * 3 * 32);

	int sub_mtrl;
	int face_cnt=0;		

	for (int i = 0; i < m_face_num  ; i ++) {

		w = 3*i;

		pFace = &m_face_list[i];
		pFNL = &m_face_normal_list[i];

		p0 = pFace->m_point_index[0];
		p1 = pFace->m_point_index[1];
		p2 = pFace->m_point_index[2];

		if(lvert) {

			SetLVertex((pLV+w  ),pP[p0],color,pFace->m_point_tex[0]);
			SetLVertex((pLV+w+1),pP[p1],color,pFace->m_point_tex[1]);
			SetLVertex((pLV+w+2),pP[p2],color,pFace->m_point_tex[2]);

		} 
		else if(!RMesh::m_bVertexNormalOnOff) {

			SetVertex((pV+w  ),pP[p0],pFNL->m_normal,pFace->m_point_tex[0]);
			SetVertex((pV+w+1),pP[p1],pFNL->m_normal,pFace->m_point_tex[1]);
			SetVertex((pV+w+2),pP[p2],pFNL->m_normal,pFace->m_point_tex[2]);

		} else {

			SetVertex((pV+w  ),pP[p0],pFNL->m_pointnormal[0],pFace->m_point_tex[0]);
			SetVertex((pV+w+1),pP[p1],pFNL->m_pointnormal[1],pFace->m_point_tex[1]);
			SetVertex((pV+w+2),pP[p2],pFNL->m_pointnormal[2],pFace->m_point_tex[2]);
		}
	}

	MakeVertexBuffer(0,lvert,_pVert,m_face_num*3,flag);

	for (int index = 0; index < m_nMtrlCnt ; index ++) 
	{
		face_cnt = 0;

		for (int i = 0; i < m_face_num  ; i ++) {

			if(m_nMtrlCnt != 1) {
				sub_mtrl = m_face_list[i].m_mtrl_id;
				if(sub_mtrl >= m_nMtrlCnt) 
					sub_mtrl -= m_nMtrlCnt;
				if(sub_mtrl != index) 
					continue;
			}

			w  = 3*face_cnt;
			w2 = 3*i;

			_pIndex[w  ] = w2;
			_pIndex[w+1] = w2+1;
			_pIndex[w+2] = w2+2;

			face_cnt++;
		}

		if(face_cnt)
			CreateIndexBuffer(index,_pIndex,face_cnt*3);
	}
}

bool RMeshNode::isSoftRender()
{
	bool bSoft = !RIsHardwareTNL();

	bool bVertexAnimation = m_pBaseMesh->isVertexAnimation(this);

	if( m_pBaseMesh->m_isCharacterMesh || 
		m_pBaseMesh->m_isNPCMesh || 
		bVertexAnimation ||	m_physique_num )
		bSoft = true;

	return bSoft;	
}

void RMeshNode::ToonRenderSettingOnOld(RMtrl* pMtrl)
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {
		
		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;

		DWORD color = 0xffffffff;
		color_r32 dx_color{ 1, 1, 1, 1 };

		int ColorMode = 0;

		if( pMtrl->GetTColor() != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 1;
			color = pMtrl->GetTColor() | 0xff000000;
		}
		else if( m_dwTFactorColor != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 2;
			color = m_dwTFactorColor | 0xff000000;
		}
		
		if(ColorMode) {

			BYTE a,r,g,b;

			a = 255;
			r = (color>>16)&0xff;
			g = (color>> 8)&0xff;
			b = (color    )&0xff;

			dx_color = color_r32(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
		}

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		if( toonLighting ) {

			dev->SetRenderState( D3DRS_LIGHTING , TRUE );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			SetMtrl(&dx_color,1.f);
		}
		else {

			dev->SetRenderState( D3DRS_LIGHTING , FALSE );
			dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_POINT);
			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_POINT);
			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		}

		dev->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
		dev->SetRenderState( D3DRS_ZENABLE , TRUE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );

		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		dev->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
		dev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );

		dev->SetTransform(D3DTS_TEXTURE0, static_cast<const D3DMATRIX*>(m_pParentMesh->m_pVisualMesh->m_ToonUVMat));
		dev->SetTexture( 0, m_pParentMesh->m_pVisualMesh->m_ToonTexture );

		if(toonTexture) {
    		dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );

			dev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			dev->SetTexture( 1, pMtrl->GetTexture() );
		}
		else {

			dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			dev->SetTexture(1, NULL);
		}
	}
}

void RMeshNode::ToonRenderSettingOn(RMtrl* pMtrl)
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {
		
		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;

		DWORD color = 0xffffffff;
		color_r32 dx_color{ 1, 1, 1, 1 };

		int ColorMode = 0;

		if( pMtrl->GetTColor() != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 1;
			color = pMtrl->GetTColor() | 0xff000000;
		}
		else if( m_dwTFactorColor != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 2;
			color = m_dwTFactorColor | 0xff000000;
		}
		
		if(ColorMode) {

			BYTE a,r,g,b;

			a = 255;
			r = (color>>16)&0xff;
			g = (color>> 8)&0xff;
			b = (color    )&0xff;

			dx_color = color_r32(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
		}

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		if( toonLighting ) {

			dev->SetRenderState( D3DRS_LIGHTING , TRUE );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			SetMtrl(&dx_color,1.f);
		}
		else {

			dev->SetRenderState( D3DRS_LIGHTING , FALSE );
			dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, pMtrl->m_TextureBlendMode);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		}

		dev->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
		dev->SetRenderState( D3DRS_ZENABLE , TRUE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );

		dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , pMtrl->m_FilterType );
		dev->SetSamplerState( 0, D3DSAMP_MINFILTER , pMtrl->m_FilterType );

		dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		dev->SetRenderState( D3DRS_ALPHAREF, pMtrl->m_AlphaRefValue );
		dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

		dev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		dev->SetTexture( 0, pMtrl->GetTexture() );

		if(toonTexture) {

			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   pMtrl->m_ToonTextureBlendMode );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );

			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

			dev->SetSamplerState( 1, D3DSAMP_MAGFILTER , pMtrl->m_ToonFilterType);
			dev->SetSamplerState( 1, D3DSAMP_MINFILTER , pMtrl->m_ToonFilterType);

			dev->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );

			dev->SetTransform(D3DTS_TEXTURE1, static_cast<const D3DMATRIX*>(m_pParentMesh->m_pVisualMesh->m_ToonUVMat));

			if(pMtrl->m_pToonTexture)
				dev->SetTexture( 1, pMtrl->m_pToonTexture->GetTexture() );
			else						
				dev->SetTexture( 1, m_pParentMesh->m_pVisualMesh->m_ToonTexture );

		}
		else {

			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			dev->SetTexture( 1, NULL );
		}
	}
}


void RMeshNode::ToonRenderSettingOff()
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;

		if(!toonLighting)
		{
			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
		}

		if(toonTexture) {
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			dev->SetTexture( 1, NULL );
		}

	}
}

void RMeshNode::ToonRenderSilhouetteSettingOn()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	DWORD color = 0xff111111;

	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);

	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	dev->SetRenderState(D3DRS_TEXTUREFACTOR, color);
	dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	dev->SetTexture(0, NULL);
	dev->SetTexture(1, NULL);

}

void RMeshNode::ToonRenderSilhouetteSettingOff()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
}

uint32_t BlendColor = 0;

void RMeshNode::Render(rmatrix* pWorldMatrix)
{
	RMtrl* pMtrl = NULL;
	RMesh* pMesh = m_pBaseMesh;

	if(pMesh==NULL) return;

	bool bSoft = isSoftRender();

	if (pWorldMatrix)	RSetTransform(D3DTS_WORLD, *pWorldMatrix);
	else				RSetTransform(D3DTS_WORLD, m_ModelWorldMatrix);

	if(bSoft==false)
		m_vb->SetVertexBuffer();

	for (int index = 0; index < m_nMtrlCnt ; index ++) {

		pMtrl = GetMtrl(index);
	
		if(pMtrl==NULL) return;
		if(m_ib[index]==NULL) continue;

		__BP(402,"RMeshNode::Render State b");

		pMesh->SetCharacterMtrl_ON(pMtrl,this,pMesh->GetMeshNodeVis(this),GetTColor());

		__EP(402);

#ifdef USE_TOON_RENDER

		/////////////////////////////////////////////////////////////////////////////////

		ToonRenderSettingOn(pMtrl);	

		__BP(403,"RMeshNode::RenderIndex");

		if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {
			bSoft = true;
		}

		/////////////////////////////////////////////////////////////////////////////////		
#endif

		if (BlendColor)
		{
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CONSTANT);
			RGetDevice()->SetTextureStageState(1, D3DTSS_CONSTANT, BlendColor);

			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

			RGetDevice()->SetTexture(1, nullptr);
		}

		if(bSoft)	m_vb->RenderIndexSoft(m_ib[index]);
		else 		m_vb->RenderIndexBuffer(m_ib[index]);

		if (BlendColor)
		{
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		}


#ifdef USE_TOON_RENDER

		if(RMesh::m_bSilhouette)
		{
			ToonRenderSilhouetteSettingOn();

			rvector pos,cpos;
			rmatrix wm = m_pParentMesh->m_pVisualMesh->m_WorldMat;

			pos.x  = wm._41;
			pos.y  = wm._42;
			pos.z  = wm._43;

			cpos.x = RView._41;
			cpos.y = RView._42;
			cpos.z = RView._43;

			float fLen = Magnitude(cpos - pos) / RMesh::m_fSilhouetteLength;

			m_vb->ConvertSilhouetteBuffer(fLen);

			if(bSoft)	m_vb->RenderIndexSoft(m_ib[index]);
			else 		m_vb->RenderIndexBuffer(m_ib[index]);

			m_vb->ReConvertSilhouetteBuffer(fLen);

			ToonRenderSilhouetteSettingOff();
		}

		ToonRenderSettingOff();	

#endif

		__BP(411,"RMeshNode::Render State e");

		pMesh->SetCharacterMtrl_OFF(pMtrl,pMesh->GetMeshNodeVis(this));

		__EP(411);

	}
}

auto MakeAlignScaleMatrix(const rmatrix& mat)
{
	v3 svec = {
		Magnitude(rvector(mat._11,mat._12,mat._13)),//right
		Magnitude(rvector(mat._21,mat._22,mat._23)),//up
		Magnitude(rvector(mat._31,mat._32,mat._33)),//dir
	};

	return ScalingMatrix(svec);
}

void RMeshNode::CheckAlignMapObject(rmatrix& hr_mat)
{
	int align = m_nAlign;

	if( m_pBaseMesh->m_is_map_object==false )	return;
	if( align == 0 )							return;

	rvector cam_dir = RCameraDirection;
	rmatrix ret_mat;

	ret_mat = hr_mat;

	auto scale_mat = MakeAlignScaleMatrix(ret_mat);

	if(align==1) {

		rmatrix mat;

		auto vUp		= rvector(ret_mat._21,ret_mat._22,ret_mat._23);
		auto vDir	= Normalized(cam_dir);
		auto vPos	= rvector(ret_mat._41,ret_mat._42,ret_mat._43);

		auto right = Normalized(CrossProduct(vUp, vDir));

		auto up = Normalized(CrossProduct(vDir, right));

		mat._14 = 0.f;
		mat._24 = 0.f;
		mat._34 = 0.f;
		mat._44 = 1.f;

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = up.x;
		mat._22 = up.y;
		mat._23 = up.z;

		mat._31 = vDir.x;
		mat._32 = vDir.y;
		mat._33 = vDir.z;

		mat._41 = vPos.x;
		mat._42 = vPos.y;
		mat._43 = vPos.z;

		hr_mat = scale_mat * mat;
	}
	else if(align==2) {

		rmatrix mat;

		mat = ret_mat;

		rvector right = rvector(mat._11,mat._12,mat._13);
		rvector dir	= rvector(mat._21,mat._22,mat._23);
		rvector up = rvector(mat._31,mat._32,mat._33);

		CrossProduct(&right,dir,cam_dir);
		Normalize(right);

		CrossProduct(&up,right,dir);
		Normalize(up);

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = dir.x;
		mat._22 = dir.y;
		mat._23 = dir.z;

		mat._31 = up.x;
		mat._32 = up.y;
		mat._33 = up.z;

		hr_mat = scale_mat * mat ;
	}
}

void RMeshNode::CheckAlign(const rmatrix& world_mat)
{
	if (m_nAlign == 0) return;

	if (m_pBaseMesh->m_is_map_object) return;

	rvector cam_dir;

	cam_dir.x = RCameraDirection.x * world_mat._11 + RCameraDirection.y * world_mat._12 + RCameraDirection.z * world_mat._13;
	cam_dir.y = RCameraDirection.x * world_mat._21 + RCameraDirection.y * world_mat._22 + RCameraDirection.z * world_mat._23;
	cam_dir.z = RCameraDirection.x * world_mat._31 + RCameraDirection.y * world_mat._32 + RCameraDirection.z * world_mat._33;

	int align = m_nAlign;

	rmatrix ret_mat;

	ret_mat = m_mat_result;

	auto scale_mat = MakeAlignScaleMatrix(ret_mat);

	if(align==1) {

		rmatrix mat;

		auto vUp		= rvector(ret_mat._21,ret_mat._22,ret_mat._23);
		auto vDir	= Normalized(cam_dir);
		auto vPos	= rvector(ret_mat._41,ret_mat._42,ret_mat._43);

		auto right = Normalized(CrossProduct(vUp, vDir));

		auto up = Normalized(CrossProduct(vDir, right));

		mat._14 = 0.f;
		mat._24 = 0.f;
		mat._34 = 0.f;
		mat._44 = 1.f;

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = up.x;
		mat._22 = up.y;
		mat._23 = up.z;

		mat._31 = vDir.x;
		mat._32 = vDir.y;
		mat._33 = vDir.z;

		mat._41 = vPos.x;
		mat._42 = vPos.y;
		mat._43 = vPos.z;

		m_mat_result = scale_mat * mat;
	}
	else if(align==2) {

		rmatrix mat;

		mat = ret_mat;

		rvector right = rvector(mat._11,mat._12,mat._13);
		rvector dir	= rvector(mat._21,mat._22,mat._23);
		rvector up = rvector(mat._31,mat._32,mat._33);

		Normalize(dir);
		Normalize(cam_dir);

		CrossProduct(&right,dir,cam_dir);
		Normalize(right);

		CrossProduct(&up,right,dir);
		Normalize(up);

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = dir.x;
		mat._22 = dir.y;
		mat._23 = dir.z;

		mat._31 = up.x;
		mat._32 = up.y;
		mat._33 = up.z;

		m_mat_result = scale_mat * mat ;
	}
}

int RMeshNode::CalcVertexBuffer_VertexAni(int frame)
{
	RAnimationNode* pANode = m_pAnimationNode;

	if(pANode) {

		if( m_point_num == pANode->m_vertex_vcnt) {

			int nCnt = pANode->GetVecValue(frame,m_point_list);
		}
		else {
			mlog("vertex ani 에서 버텍스 갯수가 틀림\n");
		}
	}

	return 1;
}


void RMeshNode::CalcVertexBuffer_Physique(const rmatrix& world_mat, int frame)
{
	int p_num,i,j,p_id;
	rvector _vec_all,_vec;
	float weight;
	rmatrix t_mat;

	RMeshNode* pTMP = NULL;

	RMesh* pMesh = m_pBaseMesh;

	for(i=0;i<m_physique_num;i++) { // point_num

		_vec_all = rvector(0,0,0);

		p_num = m_physique[i].m_num;

		if(p_num > MAX_PHYSIQUE_KEY) p_num = MAX_PHYSIQUE_KEY;

		for(j=0;j<p_num;j++) {// 4

			p_id	= m_physique[i].m_parent_id[j];
			weight	= m_physique[i].m_weight[j];

			pTMP = pMesh->m_data[p_id];

			if( pTMP) 	t_mat = pTMP->m_mat_result;
			else 		mlog("RMesh::CalcVertexBuffer() %s node : %d physique :num %d :not found !!! \n", GetName(), i, j);

			_vec = m_physique[i].m_offset[j];

			_vec = TransformCoord(_vec, t_mat);
			_vec_all += _vec * weight;
		}

		m_point_list[i] = _vec_all;
	}
}

void RMeshNode::CalcVertexBuffer_Tm(const rmatrix& world_mat, int frame) {}

void RMeshNode::CalcVertexBuffer_Bbox(CalcVertexBufferBboxMode nBboxMode,rmatrix& mat)
{
	int nCnt = m_point_num;
	rvector v;

	RMesh* pMesh = m_pBaseMesh;

	if(	nBboxMode==CalcVertexBufferBboxMode_VertexAni || 
		nBboxMode==CalcVertexBufferBboxMode_Physique ) {

			for(int i=0;i<nCnt;i++) {
				pMesh->SubCalcBBox( &pMesh->m_vBBMax, &pMesh->m_vBBMin, &m_point_list[i]);
				
			}
		}
	else if(nBboxMode==CalcVertexBufferBboxMode_TM_MapObject||
		nBboxMode==CalcVertexBufferBboxMode_TM_Object) {

			for(int i=0;i<nCnt;i++) { 
				v = m_point_list[i] * mat;
				pMesh->SubCalcBBox( &pMesh->m_vBBMax, &pMesh->m_vBBMin, &v);
			}
		}
}

bool RMeshNode::CalcPickVertexBuffer(const rmatrix& world_mat, std::vector<rvector>& OutVecs)
{
	RMesh* pMesh = m_pBaseMesh;

	CalcVertexBufferBboxMode nBboxMode = CalcVertexBufferBboxMode_None;
	rmatrix BBoxMatrix;

	LPDIRECT3DDEVICE9  dev = RGetDevice();

	rmatrix result_mat = m_mat_result;
	rmatrix scale_mat = GetIdentityMatrix();

	if( pMesh->m_isScale ) {
		scale_mat = ScalingMatrix(pMesh->m_vScale);
		result_mat *= scale_mat;
	}

	RAnimation* pAniSet = pMesh->GetNodeAniSet(this);

	int frame = GetNodeAniSetFrame();

	static rmatrix map_rot_mat = RGetRotY(180) * RGetRotX(90);

	rmatrix	ModelWorldMatrix = GetIdentityMatrix();

	if( pAniSet && pAniSet->GetAnimationType()  == RAniType_Vertex) {

		CalcVertexBuffer_VertexAni( frame );

		if(pMesh->m_is_map_object)	
			ModelWorldMatrix = scale_mat * map_rot_mat * world_mat;
		else						
			ModelWorldMatrix = scale_mat * world_mat;

		nBboxMode = CalcVertexBufferBboxMode_VertexAni;

	}
	else if( pAniSet && m_physique_num) {

		CalcVertexBuffer_Physique(world_mat,frame);

		ModelWorldMatrix = scale_mat * world_mat;

		nBboxMode = CalcVertexBufferBboxMode_Physique;

	}
	else {

		if(pMesh->m_is_map_object) { 

			ModelWorldMatrix = result_mat * map_rot_mat;
			CheckAlignMapObject(ModelWorldMatrix);

			nBboxMode = CalcVertexBufferBboxMode_TM_MapObject;
			BBoxMatrix = ModelWorldMatrix;

			ModelWorldMatrix = ModelWorldMatrix * world_mat;
		}
		else {

			nBboxMode = CalcVertexBufferBboxMode_TM_Object;

			BBoxMatrix = result_mat;

			ModelWorldMatrix = result_mat * world_mat;
		}

	}

	OutVecs.resize(m_point_num);
	for (int i = 0; i < m_point_num; i++) {
		OutVecs[i] = m_point_list[i] * ModelWorldMatrix;
	}

	return true;
}

void RMeshNode::CalcVertexBuffer(const rmatrix& world_mat, bool box)
{
	__BP(207,"RMesh::CalcVertexBuffer");

	int nNeedUpdate = 0;

	RMesh* pMesh = m_pBaseMesh;

	CalcVertexBufferBboxMode nBboxMode = CalcVertexBufferBboxMode_None;
	rmatrix BBoxMatrix;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	rmatrix result_mat = m_mat_result;
	rmatrix scale_mat;
	bool Scale = false;

	if (pMesh->m_isScale) {
		scale_mat = ScalingMatrix(pMesh->m_vScale);
		result_mat *= scale_mat;
		Scale = true;
	}

	RAnimation* pAniSet = pMesh->GetNodeAniSet(this);

	int frame = GetNodeAniSetFrame();

	static rmatrix map_rot_mat = RGetRotY(180) * RGetRotX(90);

	rmatrix	ModelWorldMatrix;

	auto SetModelWorldMatrix = [&](auto& mat)
	{
		if (Scale)
			ModelWorldMatrix = scale_mat * mat;
		else
			ModelWorldMatrix = mat;
	};

	if (pAniSet && pAniSet->GetAnimationType() == RAniType_Vertex) {
		nNeedUpdate = CalcVertexBuffer_VertexAni(frame);

		if (pMesh->m_is_map_object)
			SetModelWorldMatrix(map_rot_mat * world_mat);
		else
			SetModelWorldMatrix(world_mat);

		nBboxMode = CalcVertexBufferBboxMode_VertexAni;
	}
	else if (pAniSet && m_physique_num) {
		CalcVertexBuffer_Physique(world_mat, frame);
		SetModelWorldMatrix(world_mat);
		nBboxMode = CalcVertexBufferBboxMode_Physique;
		nNeedUpdate = 1;
	}
	else if (pMesh->m_is_map_object) {
		ModelWorldMatrix = result_mat * map_rot_mat;
		CheckAlignMapObject(ModelWorldMatrix);

		nBboxMode = CalcVertexBufferBboxMode_TM_MapObject;
		BBoxMatrix = ModelWorldMatrix;

		ModelWorldMatrix = ModelWorldMatrix * world_mat;
	}
	else {
		nBboxMode = CalcVertexBufferBboxMode_TM_Object;
		BBoxMatrix = result_mat;
		ModelWorldMatrix = result_mat * world_mat;
	}

	if(box)
		CalcVertexBuffer_Bbox(nBboxMode,BBoxMatrix);

	m_ModelWorldMatrix = ModelWorldMatrix;

	__EP(207);

	if(nNeedUpdate)
		UpdateNodeBuffer();
}

void RMeshNode::CalcVertexNormal(rmatrix* world_mat)
{
}

int RMeshNode::GetNodeAniSetFrame()
{
	if( m_pBaseMesh->m_pAniSet[1] ) {
		if(m_CutPartsType == cut_parts_upper_body) {
			return m_pBaseMesh->m_frame[1];
		}
	}
	return m_pBaseMesh->m_frame[0];
}

float RMeshNode::GetNodeVisValue()
{
	if(m_pAnimationNode) {

		if(m_pAnimationNode->m_vis_cnt) {

			int _nframe = GetNodeAniSetFrame();

			m_vis_alpha = m_pAnimationNode->GetVisValue(_nframe);

			return m_vis_alpha;
		}
	}

	m_vis_alpha = 1.f;

	return m_vis_alpha;
}

bool RMeshNode::isAlphaMtrlNode()
{
	if(m_isAlphaMtrl || m_vis_alpha != 1.f )
		return true;
	return false;
}

#undef __BP
#undef __EP

_NAMESPACE_REALSPACE2_END