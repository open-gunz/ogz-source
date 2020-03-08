#include "MCplug2.h"

/////////////////////////////////////////////////////////////
// ani out

#define ALMOST_ZERO 1.0e-3f

BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > ALMOST_ZERO)	return FALSE;
	if (fabs(p1.y - p2.y) > ALMOST_ZERO)	return FALSE;
	if (fabs(p1.z - p2.z) > ALMOST_ZERO)	return FALSE;

	return TRUE;
}

MCplug2::IsKnownController(Control* cont)
{
	ulong partA, partB;

	if (!cont)	return FALSE;

	partA = cont->ClassID().PartA();
	partB = cont->ClassID().PartB();

	if (partB != 0x00)	return FALSE;

	switch (partA) 
	{
		case TCBINTERP_POSITION_CLASS_ID:
		case TCBINTERP_ROTATION_CLASS_ID:
		case TCBINTERP_SCALE_CLASS_ID:
		case HYBRIDINTERP_POSITION_CLASS_ID:
		case HYBRIDINTERP_ROTATION_CLASS_ID:
		case HYBRIDINTERP_SCALE_CLASS_ID:
		case LININTERP_POSITION_CLASS_ID:
		case LININTERP_ROTATION_CLASS_ID:
		case LININTERP_SCALE_CLASS_ID:
			return TRUE;
	}

	return FALSE;
}

void MCplug2::export_anikey(INode* node, mesh_data* mesh_node) 
{
	BOOL bPosAnim;
	BOOL bRotAnim;
	BOOL bScaleAnim;
	BOOL bDoKeys = FALSE;

	if (CheckForAnimation(node, bPosAnim, bRotAnim, bScaleAnim)) 
	{
		if (bPosAnim) 		DumpPosSample  (node, mesh_node);
		if (bRotAnim) 		DumpRotSample  (node, mesh_node);
		if (bScaleAnim) 	DumpScaleSample(node, mesh_node);

	}

	//모든경우에 

	DumpTMSample(node, mesh_node);

	if(m_is_vertex_ani_out) {
		DumpVertexAni(node, mesh_node);
	}

	DumpVisSample(node, mesh_node);
}

void MCplug2::DumpVertexAni(INode* node, mesh_data* mesh_node)
{
	if( mesh_node->m_point_num==0 )//가진점이 없다면...
		return;

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end	= ip->GetAnimRange().End();
	TimeValue t;

	int delta = GetTicksPerFrame() * GetKeyFrameStep();

	Matrix3 tm(1);

	int cnt = 0;

	for (t=start; t<=end; t+=delta) {
		cnt++;
	}

	mesh_node->m_vertex_ani_num = cnt;
	mesh_node->m_vertex_ani_frame = new DWORD[cnt];
	mesh_node->m_vertex_ani_list = new D3DXVECTOR3*[cnt];

	//////////////////////////////////////////////////////

	BOOL needDel;
	Mesh* mesh;
	int vcnt;
	int frame_cnt = 0;
	int i;

	Point3 v;

	D3DXVECTOR3* pVert = NULL;
	TriObject* tri = NULL;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetObjTMAfterWSM(t);

		tri = GetTriObjectFromNode(node, t, needDel);
		
		if (!tri) {
			continue;
		}

		mesh = &tri->mesh;
		vcnt = mesh->getNumVerts();

		pVert = new D3DXVECTOR3[vcnt];

		mesh_node->m_vertex_ani_list[frame_cnt] = pVert;
		mesh_node->m_vertex_ani_frame[frame_cnt] = t-m_nBeginFrame;

		for (i=0; i<vcnt; i++) {

			v = mesh->verts[i] * tm;

			pVert[i].x = v.x;
			pVert[i].y = v.z;
			pVert[i].z = v.y;
		}

		if (needDel) {
			delete tri;
		}

		frame_cnt++;
	}
}

void MCplug2::DumpTMSample(INode* node, mesh_data* mesh_node)
{
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end	= ip->GetAnimRange().End();
	TimeValue t;

	int delta = GetTicksPerFrame() * GetKeyFrameStep();

	Matrix3 tm;

	int mat_key_cnt = 0;

	for (t=start; t<=end; t+=delta) {
		mat_key_cnt++;
	}

	mesh_node->m_tm_key_num = mat_key_cnt;

	mesh_node->m_tm_key = new RTMKey[mat_key_cnt];
	memset(mesh_node->m_tm_key,0,sizeof(RTMKey) * mat_key_cnt);

	//////////////////////////////////////////////////////

	int i=0;
	D3DXMATRIX dm;
	Point3 row;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t);
//		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		D3DXMatrixIdentity(&dm);

		//////////////////////////////////////////////////

		AffineParts aps,ap;
//		Matrix3 tms = node->GetNodeTM(GetStaticFrame()) * Inverse(node->GetParentTM(GetStaticFrame()));
		decomp_affine(tm, &aps);
/*
		mesh_node->m_ap_scale.x = aps.k.x;
		mesh_node->m_ap_scale.y = aps.k.y;
		mesh_node->m_ap_scale.z = aps.k.z;
*/		
		float rotAngle;
		Point3 rotAxis;
//		decomp_affine(tm, &ap);
		AngAxisFromQ(aps.q, &rotAngle, rotAxis);

		row = tm.GetRow(0);

		dm._11 = row.x;
		dm._12 = row.z;
		dm._13 = row.y;

		row = tm.GetRow(1);	

		dm._31 = row.x;
		dm._32 = row.z;
		dm._33 = row.y;

		row = tm.GetRow(2);	

		dm._21 = row.x;
		dm._22 = row.z;
		dm._23 = row.y;

		row = tm.GetRow(3);	

		dm._41 = row.x;
		dm._42 = row.z;
		dm._43 = row.y;

		mesh_node->m_tm_key[i].frame = t-m_nBeginFrame;
 //		mesh_node->m_tm_key[i] = dm;
		memcpy(&mesh_node->m_tm_key[i],&dm,sizeof(D3DXMATRIX));

		i++;
	}
}

BOOL MCplug2::CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale)
{
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame();
	Matrix3 tm;
	AffineParts ap;
	Point3 firstPos;
	float rotAngle, firstRotAngle;
	Point3 rotAxis, firstRotAxis;
	Point3 firstScaleFactor;

	bPos = bRot = bScale = FALSE;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		AngAxisFromQ(ap.q, &rotAngle, rotAxis);

		if (t != start) {

			if (!bPos) {

				if (!EqualPoint3(ap.t, firstPos)) 						bPos = TRUE;
			}

			if (!bRot) {
				if (fabs(rotAngle - firstRotAngle) > ALMOST_ZERO) 		bRot = TRUE;
				else if (!EqualPoint3(rotAxis, firstRotAxis)) 			bRot = TRUE;
			}

			if (!bScale) {
				if (!EqualPoint3(ap.k, firstScaleFactor)) 				bScale = TRUE;
			}

		} else {

			firstPos = ap.t;
			firstRotAngle = rotAngle;
			firstRotAxis = rotAxis;
			firstScaleFactor = ap.k;
		}

		if (bPos && bRot && bScale)		break;
	}

	return bPos || bRot || bScale;
}

void MCplug2::DumpPosSample(INode* node,mesh_data* mesh_node) 
{	
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end	= ip->GetAnimRange().End();
	TimeValue t;

	int delta = GetTicksPerFrame() * GetKeyFrameStep();

	Matrix3 tm,ptm;

	AffineParts ap;
	Point3	prevPos;

	int pos_key_cnt = 0;

	for (t=start; t<=end; t+=delta) {
		pos_key_cnt++;
	}

	mesh_node->m_pos_key_num = pos_key_cnt;

	mesh_node->m_is_pos = true;
	mesh_node->m_pos_key = new RPosKey[pos_key_cnt];
	memset(mesh_node->m_pos_key,0,sizeof(RPosKey)*pos_key_cnt);

	//////////////////////////////////////////////////////
	//

	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());							//pivot mirror 변형 matrix
	Matrix3 satm = node->GetNodeTM(start) * Inverse(node->GetParentTM(start));	//에니의 시작 프레임

	Matrix3 stm = pivot * Inverse(satm);

	int i=0;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t) * Inverse( node->GetParentTM(t) );

		decomp_affine(tm, &ap);

		Point3 pos = ap.t;
//		Quat q;
//		SpectralDecomp(tm,pos,q);

		mesh_node->m_pos_key[i].frame = t-m_nBeginFrame;
		mesh_node->m_pos_key[i].x = pos.x;
		mesh_node->m_pos_key[i].y = pos.z;
		mesh_node->m_pos_key[i].z = pos.y;

		i++;
	}
}

void MCplug2::DumpRotSample(INode* node,mesh_data* mesh_node) 
{	
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end   = ip->GetAnimRange().End();
	TimeValue t;

	int delta = GetTicksPerFrame() * GetKeyFrameStep();

	Matrix3 tm,ptm;
	AffineParts ap;

	int rot_key_cnt = 0;

	for (t=start; t<=end; t+=delta) {

		rot_key_cnt++;
	}

	mesh_node->m_quat_key_num = rot_key_cnt;

	mesh_node->m_is_rot = true;
	mesh_node->m_quat_key = new RQuatKey[rot_key_cnt];
	memset(mesh_node->m_quat_key,0,sizeof(RQuatKey)*rot_key_cnt);

	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());							//pivot mirror 변형 matrix
	Matrix3 satm = node->GetNodeTM(start) * Inverse(node->GetParentTM(start));	//에니의 시작 프레임

	Matrix3 stm = pivot * Inverse(satm);

	int i=0;
	
//	Quat sq;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		Quat q = ap.q;
//		Point3 pos;
//		SpectralDecomp(tm,pos,q);

//		if(t == start)
//			sq= ap.q;

//		q = q + sq;

		Point3 axis;
		float angle;

		AngAxisFromQ(q, &angle, axis);
		AngAxis value = AngAxis(axis, angle);

		mesh_node->m_quat_key[i].frame = t-m_nBeginFrame;
		mesh_node->m_quat_key[i].x = value.axis.x;
		mesh_node->m_quat_key[i].y = value.axis.z;
		mesh_node->m_quat_key[i].z = value.axis.y;
		mesh_node->m_quat_key[i].w = value.angle;

		i++;
	}
}

void MCplug2::DumpVisSample(INode* node, mesh_data* mesh_node)
{
	Control* cont = node->GetVisController();

	RVisKey* pVis = NULL;

//	float fVis = node->GetVisibility();
	//시작과 끝에 하나씩 꼭 넣어준다
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end	= ip->GetAnimRange().End();

	if(cont) {

		int i;
		IKeyControl *ikc = NULL;
		ikc = GetKeyControlInterface(cont);

		if(ikc)	{

			mesh_node->m_vis_key = new RVisKey[ikc->GetNumKeys() + 2];
			mesh_node->m_vis_key_num = ikc->GetNumKeys() + 2;
			//시작
			pVis = &mesh_node->m_vis_key[0];
			pVis->frame = start;
			pVis->v = node->GetVisibility(start);

			for (i=0; i<ikc->GetNumKeys(); i++) {
				pVis = &mesh_node->m_vis_key[ i+1 ];

				if(cont->ClassID() == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0)) {
					ITCBFloatKey key;
					ikc->GetKey(i, &key);
//					pVis->frame = key.time / GetTicksPerFrame();
					pVis->frame = key.time;
					pVis->v		= key.val;
				}
				else if(cont->ClassID() == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID, 0))
				{
					IBezFloatKey key;
					ikc->GetKey(i, &key);
					pVis->frame = key.time;
					pVis->v		= key.val;
				}
				else if(cont->ClassID() == Class_ID(LININTERP_FLOAT_CLASS_ID, 0)) {
					ILinFloatKey key;
					ikc->GetKey(i, &key);
					pVis->frame = key.time;
					pVis->v		= key.val;
				}
			}
			//끝
			pVis = &mesh_node->m_vis_key[mesh_node->m_vis_key_num-1];
			pVis->frame = end;
			pVis->v = node->GetVisibility(end);
		}
	}
}

// 안쓴다~
void MCplug2::DumpScaleSample(INode* node,mesh_data* mesh_node) 
{
/*
//	TSTR indent = GetIndent(indentLevel);
//	fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_SCALE_TRACK);

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end	= ip->GetAnimRange().End();
	TimeValue t;

	int delta = GetTicksPerFrame() * GetKeyFrameStep();

	Matrix3 tm;
	AffineParts ap;
	Point3	prevFac;

	int scale_key_cnt = 0;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		tm.NoTrans();

		decomp_affine(tm, &ap);
		if (t!= start && EqualPoint3(ap.k, prevFac)) {
			continue;
		}
		prevFac = ap.k;
		scale_key_cnt++;
	}

	int i=0;

	for (t=start; t<=end; t+=delta) {

		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		if (t!= start && EqualPoint3(ap.k, prevFac)) {
			continue;
		}

		prevFac = ap.k;
		i++;
		
//		fprintf(pStream, "%s\t\t\t%s %d\t%s %s\n",indent.data(),ID_SCALE_SAMPLE,t,Format(ap.k),	Format(ap.u));
	}
*/
//	fprintf(pStream,"%s\t\t}\n", indent.data());
}
