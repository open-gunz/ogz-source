#include "MCplug2.h"
#include "PHYEXP.H"

//////////////////////////////////////////////////////////////
// object export

Modifier* FindPhysiqueModifier (INode* nodePtr)
{
	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr) return NULL;

	if (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID) {

		IDerivedObject* DerivedObjectPtr = static_cast<IDerivedObject*>(ObjectPtr);

		int ModStackIndex = 0;

		while (ModStackIndex < DerivedObjectPtr->NumModifiers()) {

			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)) {

				return ModifierPtr;
			}

			ModStackIndex++;
		}
	}

	return NULL;
}

void MCplug2::export_physique( ObjectState *os, INode *node,mesh_data* mesh_node)
{
    Modifier *phyMod = FindPhysiqueModifier(node);

    if (!phyMod) return;    

    IPhysiqueExport *phyExport = (IPhysiqueExport *)phyMod->GetInterface(I_PHYINTERFACE);

	// Remove the non uniform scale

//	IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);
//	BipIface->RemoveNonUniformScale(1);

    if (phyExport) {

		IPhyContextExport *mcExport = (IPhyContextExport *)phyExport->GetContextInterface(node);

		if (mcExport) {

			mcExport->ConvertToRigid(TRUE);
			mcExport->AllowBlending(TRUE);		//<--

			int p_num = os->obj->NumPoints();

			mesh_node->m_physique_num = p_num;
			mesh_node->m_physique = new RPhysiqueInfo [p_num];
			memset(mesh_node->m_physique,0,sizeof(RPhysiqueInfo)*p_num);

			phyMod->DisableMod();
			phyExport->SetInitialPose( true );
//			phyExport->GetInitNodeTM( this, MeshRefMatrix );

			for (int i = 0; i < p_num;  i++) {

				IPhyVertexExport *vtxExport = mcExport->GetVertexInterface(i);

				if (vtxExport) {

					if (vtxExport->GetVertexType() & BLENDED_TYPE) {

						IPhyBlendedRigidVertex *vtxBlend = (IPhyBlendedRigidVertex *)vtxExport;
			
						int no = vtxBlend->GetNumberNodes();

						mesh_node->m_physique[i].m_num = no;

						Point3 BlendP(0.0f, 0.0f, 0.0f);

						for (int n = 0; n < no; n++) {

							INode *Bone		= vtxBlend->GetNode(n);
							Point3 Offset	= vtxBlend->GetOffsetVector(n);
							float Weight	= vtxBlend->GetWeight(n);

							mesh_node->m_physique[i].m_offset[n].x = Offset.x;
							mesh_node->m_physique[i].m_offset[n].y = Offset.z;
							mesh_node->m_physique[i].m_offset[n].z = Offset.y;

							strcpy(mesh_node->m_physique[i].m_parent_name[n],Bone->GetName());
							mesh_node->m_physique[i].m_weight[n] = Weight;
						}
		
						mcExport->ReleaseVertexInterface(vtxExport);
						vtxExport = NULL;	

					} else {

						IPhyRigidVertex *vtxNoBlend = (IPhyRigidVertex *)vtxExport;
						INode *Bone = vtxNoBlend->GetNode();
						Point3 Offset = vtxNoBlend->GetOffsetVector();

						mesh_node->m_physique[i].m_offset[0].x = Offset.x;
						mesh_node->m_physique[i].m_offset[0].y = Offset.z;
						mesh_node->m_physique[i].m_offset[0].z = Offset.y;

						mesh_node->m_physique[i].m_num = 1;
						strcpy(mesh_node->m_physique[i].m_parent_name[0],Bone->GetName());
						mesh_node->m_physique[i].m_weight[0] = 1.0f;

						mcExport->ReleaseVertexInterface(vtxExport);
						vtxExport = NULL;
					}
				}
			}

			phyExport->SetInitialPose( false );
			phyMod->EnableMod();

			phyExport->ReleaseContextInterface(mcExport);
		}
        phyMod->ReleaseInterface(I_PHYINTERFACE, phyExport);
    }
}

void MCplug2::export_object_list(INode* node)
{
	mesh_data* mesh_node = new mesh_data;
	memset(mesh_node,0,sizeof(mesh_data));

	ObjectState os = node->EvalWorldState(GetStaticFrame());

	if (!os.obj)	return;
	
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))		return;
	
	strcpy(mesh_node->m_Name,FixupName(node->GetName()));

	INode* parent = node->GetParentNode();

	if (parent && !parent->IsRootNode()) {

		strcpy(mesh_node->m_Parent,FixupName(parent->GetName()));
	}

	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());

//	Matrix3 tm = node->GetObjTMAfterWSM(t);
//	BOOL negScale = TMNegParity(tm);

	DWORD iFlags = node->GetTMController()->GetInheritanceFlags();
	
	D3DXMatrixIdentity(&mesh_node->m_mat_base);

	Point3 row;
	
	row = pivot.GetRow(0);

	mesh_node->m_mat_base._11 = row.x;
	mesh_node->m_mat_base._12 = row.z;
	mesh_node->m_mat_base._13 = row.y;

	row = pivot.GetRow(1);	

	mesh_node->m_mat_base._31 = row.x;
	mesh_node->m_mat_base._32 = row.z;
	mesh_node->m_mat_base._33 = row.y;

	row = pivot.GetRow(2);	

	mesh_node->m_mat_base._21 = row.x;
	mesh_node->m_mat_base._22 = row.z;
	mesh_node->m_mat_base._23 = row.y;

	row = pivot.GetRow(3);	

	mesh_node->m_mat_base._41 = row.x;
	mesh_node->m_mat_base._42 = row.z;
	mesh_node->m_mat_base._43 = row.y;

	// scale , rot
	AffineParts aps;
	float fAngle;
	Point3 vAxis;
	AngAxis value;

	Matrix3 tms = node->GetNodeTM(GetStaticFrame()) * Inverse(node->GetParentTM(GetStaticFrame()));
	decomp_affine(tms, &aps);

	mesh_node->m_ap_scale.x = aps.k.x;
	mesh_node->m_ap_scale.y = aps.k.z;
	mesh_node->m_ap_scale.z = aps.k.y;

	AngAxisFromQ(aps.q, &fAngle, vAxis);
	value = AngAxis(vAxis, fAngle);

	mesh_node->m_axis_rot.x		= value.axis.x;
	mesh_node->m_axis_rot.y		= value.axis.z;
	mesh_node->m_axis_rot.z		= value.axis.y;
	mesh_node->m_axis_rot_angle = value.angle;

	AngAxisFromQ(aps.u, &fAngle, vAxis);
	value = AngAxis(vAxis, fAngle);

	mesh_node->m_axis_scale.x	  = value.axis.x;
	mesh_node->m_axis_scale.y	  = value.axis.z;
	mesh_node->m_axis_scale.z	  = value.axis.y;
	mesh_node->m_axis_scale_angle = value.angle;

	D3DXMATRIX mm;
	D3DXMatrixIdentity(&mm);

//	AngAxisFromQ(q, &angle, axis);
//	AngAxis value = AngAxis(axis, angle);

	D3DXQUATERNION out;

	D3DXQuaternionRotationAxis( &out, &mesh_node->m_axis_rot, mesh_node->m_axis_rot_angle);

	D3DXMatrixRotationQuaternion(&mm,&out);

	mm._41 = aps.t.x;
	mm._42 = aps.t.z;
	mm._43 = aps.t.y;

	D3DXMATRIX mm_inv;

	D3DXMatrixInverse(&mm_inv,NULL,&mm);

	mesh_node->m_etc_mat = mesh_node->m_mat_base * mm_inv;

/*
	decomp_affine(pivot, &ap);
	AngAxisFromQ(ap.q, &rotAngle, rotAxis);
*/

//	export_mesh(node, 8000 , mesh_node);
	export_mesh(node, GetStaticFrame(), mesh_node);

	export_anikey(node, mesh_node);

//	if(m_is_vertex_ani_out)
//		export_meshani(node,mesh_node);

	Mtl* mtl = node->GetMtl();

	if (mtl) {
		int mtlID = mtlList.GetMtlID(mtl);
		if (mtlID >= 0) {
			mesh_node->m_mtrl_id = mtlID;
		}
	} else {
		DWORD c = node->GetWireColor();
	}

	export_physique(&os,node,mesh_node);

	m_mesh_list.add_mesh(mesh_node);
}

//////////////////////////////////////////////////////////////////////////////

void MCplug2::export_meshani(INode* node,mesh_data* mesh_node)
{
}

void MCplug2::export_meshani_sub(INode* node,TimeValue t,mesh_data* mesh_node,int cnt)
{
}

void MCplug2::export_mesh(INode* node, TimeValue t, mesh_data* mesh_node)
{
	int i;
	Mtl* nodeMtl	= node->GetMtl();
	Matrix3 tm		= node->GetObjTMAfterWSM(t);
	BOOL negScale	= TMNegParity(tm);

	Matrix3 tmoff = node->GetNodeTM(0);
	tmoff = tm * Inverse(tmoff);

	int vx1, vx2, vx3;

	ObjectState os = node->EvalWorldState(t);

	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; 
	}
	
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	} else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}
	
	BOOL needDel;

	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
//	PatchObject* patch = GetPatchObjectFromNode(node,t,needDel);

	if (!tri) {
		return;
	}
	
	Mesh* mesh = &tri->GetMesh();

	mesh->buildNormals();

	int point_num = mesh->getNumVerts();
	int face_num  = mesh->getNumFaces();

	mesh_node->m_point_num = point_num;
	mesh_node->m_face_num  = face_num;

	mesh_node->m_point_list = new D3DXVECTOR3[point_num];
	mesh_node->m_face_list = new RFaceInfo[face_num];

	memset(mesh_node->m_point_list,0,sizeof(D3DXVECTOR3)*point_num);
	memset(mesh_node->m_face_list,0,sizeof(RFaceInfo)*face_num);

	Matrix3 piv(1);

	piv.SetTrans(node->GetObjOffsetPos());
	PreRotateMatrix(piv,node->GetObjOffsetRot());
	ApplyScaling(piv,node->GetObjOffsetScale());

	for (i=0; i<point_num; i++) {

		Point3 v = mesh->verts[i] * piv;

		mesh_node->m_point_list[i].x = v.x;// y-z
		mesh_node->m_point_list[i].y = v.z;
		mesh_node->m_point_list[i].z = v.y;
	}

	for (i=0; i<face_num; i++) {

		mesh_node->m_face_list[i].m_point_index[0] = mesh->faces[i].v[vx3];//012 -> 210
		mesh_node->m_face_list[i].m_point_index[1] = mesh->faces[i].v[vx2];
		mesh_node->m_face_list[i].m_point_index[2] = mesh->faces[i].v[vx1];

		mesh_node->m_face_list[i].m_mtrl_id = mesh->faces[i].getMatID();

		for (int j=0; j<32; j++) {
			if (mesh->faces[i].smGroup & (1<<j)) {
				if (mesh->faces[i].smGroup>>(j+1)) {
					mesh_node->m_face_list[i].m_sg_id = j+1;
				}
				else {
					mesh_node->m_face_list[i].m_sg_id = j+1;
				}

			}
		}
	}

	if (!CheckForAndExportFaceMap(nodeMtl, mesh)) {

		int numTVx = mesh->getNumTVerts();

		mesh_node->m_tex_point_num = numTVx;
		mesh_node->m_tex_point_list = new D3DXVECTOR3[numTVx];
		memset(mesh_node->m_tex_point_list,0,sizeof(D3DXVECTOR3) * numTVx);

		if (numTVx) {

			for (i=0; i<numTVx; i++) {

				UVVert tv = mesh->tVerts[i];

				mesh_node->m_tex_point_list[i].x = tv.x;
				mesh_node->m_tex_point_list[i].y = tv.y;
				mesh_node->m_tex_point_list[i].z = tv.z;
			}

			int a,b,c;

			for (i=0; i<mesh->getNumFaces(); i++) {

				a = mesh->tvFace[i].t[vx3];//012 -> 210
				b = mesh->tvFace[i].t[vx2];
				c = mesh->tvFace[i].t[vx1];

				mesh_node->m_face_list[i].m_point_tex[0].x =		mesh_node->m_tex_point_list[ a ].x;
				mesh_node->m_face_list[i].m_point_tex[0].y = 1.0f -	mesh_node->m_tex_point_list[ a ].y; 
				mesh_node->m_face_list[i].m_point_tex[1].x =		mesh_node->m_tex_point_list[ b ].x; 
				mesh_node->m_face_list[i].m_point_tex[1].y = 1.0f -	mesh_node->m_tex_point_list[ b ].y; 
				mesh_node->m_face_list[i].m_point_tex[2].x =		mesh_node->m_tex_point_list[ c ].x; 
				mesh_node->m_face_list[i].m_point_tex[2].y = 1.0f -	mesh_node->m_tex_point_list[ c ].y; 
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	int numCVx = mesh->numCVerts;

//	mesh_node->m_point_color_num = numCVx;

	if ( numCVx ) {
		mesh_node->m_point_color_num = point_num;

		D3DXVECTOR3* pCVec = new D3DXVECTOR3 [ numCVx ];
		 
		mesh_node->m_point_color_list = new D3DXVECTOR3 [ point_num ];// 정점과 순서 보장

		Point3 vc;

		for (i=0; i<numCVx; i++) {

			vc = mesh->vertCol[i];

			pCVec[i].x = vc.x;
			pCVec[i].y = vc.y;
			pCVec[i].z = vc.z;
		}

		int in1,in2,in3;
		int out1,out2,out3;

		int nFaceNum = mesh->getNumFaces();

		for (i=0; i<nFaceNum; i++) {

			in1 = mesh->vcFace[i].t[vx3];
			in2 = mesh->vcFace[i].t[vx2];
			in3 = mesh->vcFace[i].t[vx1];

			out1 = mesh->faces[i].v[vx3];
			out2 = mesh->faces[i].v[vx2];
			out3 = mesh->faces[i].v[vx1];

			mesh_node->m_point_color_list[out1] = pCVec[in1];
			mesh_node->m_point_color_list[out2] = pCVec[in2];
			mesh_node->m_point_color_list[out3] = pCVec[in3];
		}

		delete [] pCVec;

		// check 모두 1인 컬러값이면 출력 안한다..
		// 디자이너가 컬러값을 모두 지운다고 흰색으로 만들어버린 경우도 
		// max 에서는 지워지지 않아서 다시 작업해야 할수도 있다..
		
		D3DXVECTOR3* pColor = NULL;

		bool bCheck = true;

		for(i=0;i<point_num;i++)
		{
			pColor = &mesh_node->m_point_color_list[i];
			if( (pColor->x != 1.f) || (pColor->y != 1.f) ||	(pColor->z != 1.f)) {
				bCheck = false;
				break;
			}
		}

		if(bCheck) {//모두 흰색인 경우는 출력할 필요없다..
			mesh_node->m_point_color_num = 0;
			delete [] mesh_node->m_point_color_list;
			mesh_node->m_point_color_list = NULL;
		}

	}

	///////////////////////////////////////////////////////////////////////////////

	Point3 fn;  // Face normal
	Point3 vn;  // Vertex normal
	int  vert;
	Face* f;

	int face_cnt = mesh->getNumFaces();

	if(face_cnt) {
		mesh_node->m_face_normal_list = new RFaceNormalInfo[face_cnt];
	}

	RFaceNormalInfo* pTN = NULL;

	for (i=0; i<face_cnt; i++) {

		f = &mesh->faces[i];
		fn = mesh->getFaceNormal(i);

		pTN = &mesh_node->m_face_normal_list[i];

		pTN->m_normal.x = fn.x;
		pTN->m_normal.y = fn.z;
		pTN->m_normal.z = fn.y;

		vert = f->getVert(vx3);
		vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));

		pTN->m_pointnormal[0].x = vn.x;
		pTN->m_pointnormal[0].y = vn.z;
		pTN->m_pointnormal[0].z = vn.y;

		vert = f->getVert(vx2);
		vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));

		pTN->m_pointnormal[1].x = vn.x;
		pTN->m_pointnormal[1].y = vn.z;
		pTN->m_pointnormal[1].z = vn.y;

		vert = f->getVert(vx1);
		vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));

		pTN->m_pointnormal[2].x = vn.x;
		pTN->m_pointnormal[2].y = vn.z;
		pTN->m_pointnormal[2].z = vn.y;
	}

	if (needDel) {

		delete tri;
	}
}

Point3 MCplug2::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			for (int i = 0; i < numNormals; i++) {
				if (rv->ern[i].getSmGroup() & smGroup) {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else {
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}
