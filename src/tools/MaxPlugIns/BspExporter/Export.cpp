// realspace2 bsp exporter .. based on asciiexp from 3dsmax sdk.

#include "stdafx.h"
#include "rbsexp.h"
#include "BElog.h"

void RbsExp::ExportGlobalInfo()
{
	Color ambLight = ip->GetAmbient(GetStaticFrame(), FOREVER);
	rsm->m_AmbientLightColor=rvector(ambLight.r,ambLight.g,ambLight.b);
}

/****************************************************************************

  GeomObject output
  
****************************************************************************/

void RbsExp::ExportGeomObject(INode* node)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	
	ExportMesh(node, GetStaticFrame());
//	ExportMesh_Extrude(node, GetStaticFrame());
//	ExportMesh_CSG(node, GetStaticFrame());
}


/****************************************************************************

  Light output
  
****************************************************************************/

void RbsExp::ExportLightObject(INode* node)
{
	TimeValue t = GetStaticFrame();

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj) return;
	
	GenLight* light = (GenLight*)os.obj;
	struct LightState ls;
	Interval valid = FOREVER;
	Interval animRange = ip->GetAnimRange();

	light->EvalLightState(t, valid, &ls);

	if(ls.type==OMNI_LIGHT)
	{
		RLIGHT *plight=new RLIGHT;
		Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
		AffineParts ap;
		decomp_affine(pivot, &ap);
		plight->Name=node->GetName();	
		plight->Color=rvector(ls.color.r,ls.color.g,ls.color.b);
		plight->Position=rvector(-ap.t.x,ap.t.y,ap.t.z);
		plight->fIntensity=ls.intens;
		plight->fAttnStart=ls.attenStart;
		plight->fAttnEnd=ls.attenEnd;
		
		plight->dwFlags=0;
		if(light->GetShadow()!=FALSE)
			plight->dwFlags|=RM_FLAG_CASTSHADOW;

		rsm->m_StaticLightList.push_back(plight);
	}
}

#define P32RV(a) rvector(-(a).x,(a).y,(a).z)
#define P32DP(a) dpoint(-(a).x,(a).y,(a).z)

void RbsExp::ExportShapeObject(INode* node)
{
	TimeValue t = GetStaticFrame();
	Matrix3 tm = node->GetObjTMAfterWSM(t);

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=SHAPE_CLASS_ID) {
		return;
	}

	// 이름규칙
	char *part=strstr(node->GetName(),RTOK_MAX_PARTITION);
	if(part)
	{
		int nPartitionNumber=atoi(part+strlen(RTOK_MAX_PARTITION));

		ShapeObject* shape = (ShapeObject*)os.obj;
		PolyShape pShape;
		int numLines;

		shape->MakePolyShape(t, pShape);
		numLines = pShape.numLines;

		if(numLines!=1) return;

		PolyLine* line=&pShape.lines[0];

		if(!line->IsClosed()) return;

		int numVerts = line->numPts;
		if(numVerts<3) return;

		dpoint v[3];
		for (int i=0; i<3; i++) {
			PolyPt* pt = &line->pts[i];
			Point3 pos=tm*pt->p;
			v[i]=P32DP(pos);
		}
		dplane plane;
		DPlaneFromPoints(&plane,v[0],v[1],v[2]);
		rsm->m_PartitionPlanes.insert(map<int,dplane>::value_type(nPartitionNumber,plane));
	}

	if(strstr(node->GetName(),RTOK_MAX_OCCLUSION)!=NULL)
	{
		ShapeObject* shape = (ShapeObject*)os.obj;
		PolyShape pShape;
		int numLines;

		shape->MakePolyShape(t, pShape);
		numLines = pShape.numLines;

		if(numLines!=1) return;

		PolyLine* line=&pShape.lines[0];

		if(!line->IsClosed()) return;

		int numVerts = line->numPts;

		ROcclusion *poc=new ROcclusion;
		poc->nCount=numVerts;
		poc->pVertices=new rvector[numVerts];
		poc->Name=node->GetName();	

		for (int i=0; i<numVerts; i++) {
			PolyPt* pt = &line->pts[i];
			Point3 pos=tm*pt->p;
			poc->pVertices[i]=P32RV(pos);

			/*
			if (pt->flags & POLYPT_KNOT) {
			fprintf(pStream,"%s\t\t%s\t%d\t%s\n", indent.data(), ID_SHAPE_VERTEX_KNOT, i,
			Format(tm * pt->p));
			}
			else {
			fprintf(pStream,"%s\t\t%s\t%d\t%s\n", indent.data(), ID_SHAPE_VERTEX_INTERP,
			i, Format(tm * pt->p));
			}
			*/

		}

		rsm->m_OcclusionList.push_back(poc);
	}
}

/****************************************************************************

  Mesh output
  
****************************************************************************/

void TransformUV(float &u,float &v,MaxSubMaterial *m)
{
	float ou=float(u*m->tileu),ov=float(v*m->tilev);
	u=float(ou*m->cosa+ov*m->sina);
	v=float(-ou*m->sina+ov*m->cosa);
}

/*
void RbsExp::ExportMesh(INode* node, TimeValue t)
{
	int i;

	int mtlID = -1;
	Mtl* nodeMtl = node->GetMtl();
	if (nodeMtl) {
		mtlID = mtlList.GetMtlID(nodeMtl);
	}

	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);
	int vx1, vx2, vx3;

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}

	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}

	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri) {
		return;
	}

	Mesh* mesh = &tri->mesh;

	MaxMaterial *mat=(mtlID==-1)?NULL:rsm->MaxMaterialList.Get(mtlID);

	int nTwoSideFaces=0;
	for(i=0;i<mesh->getNumFaces();i++)
	{
		mesh->faces[i].getMatID();
	}

	DWORD dwFlags=0;

	// 이름규칙들
	if(strstr(node->GetName(),RTOK_MAX_NOPATH)!=NULL)
		dwFlags|=RM_FLAG_NOTWALKABLE;
	if(strstr(node->GetName(),RTOK_MAX_PASSTHROUGH)!=NULL) 
		dwFlags|=RM_FLAG_PASSTHROUGH;
	if(strstr(node->GetName(),RTOK_MAX_HIDE)!=NULL) 
		dwFlags|=RM_FLAG_HIDE;
	if(strstr(node->GetName(),RTOK_MAX_PASSBULLET)!=NULL) 
		dwFlags|=RM_FLAG_PASSBULLET;
	if(strstr(node->GetName(),RTOK_MAX_PASSROCKET)!=NULL) 
		dwFlags|=RM_FLAG_PASSROCKET;

	if(node->CastShadows())
		dwFlags|=RM_FLAG_CASTSHADOW;
	if(node->RcvShadows())
		dwFlags|=RM_FLAG_RECEIVESHADOW;

	mesh->buildNormals();

	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
	pivot.NoTrans();

	Point3 fn;  // Face normal
	Point3 vn;  // Vertex normal
	int  vert;
	Face* f;

	bool bExportTexCoord=false;

	if (!CheckForAndExportFaceMap(nodeMtl, mesh)) {
		// If not, export standard tverts
		int numTVx = mesh->getNumTVerts();

		if (numTVx) {
			bExportTexCoord=true;
		}
	}


	for (i=0; i<mesh->getNumFaces(); i++) {

		rpolygon *rf=new rpolygon;

		// 플래그 설정
		rf->dwFlags=dwFlags;

		// coordinate..

		rf->v=new rvertex[3];
		rf->nCount=3;

		Point3 p;
		p=tm * mesh->verts[mesh->faces[i].v[vx1]];
		rf->v[0].coord=P32RV(p);
		p=tm * mesh->verts[mesh->faces[i].v[vx2]];
		rf->v[1].coord=P32RV(p);
		p=tm * mesh->verts[mesh->faces[i].v[vx3]];
		rf->v[2].coord=P32RV(p);

		// vertices normal..

		f = &mesh->faces[i];
		fn = pivot*mesh->getFaceNormal(i);
		rf->normal=P32RV(fn);
		rf->d=-DotProduct(rf->normal,rf->v[0].coord);

		vert = f->getVert(vx1);
		vn = pivot*GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
		rf->v[0].normal=P32RV(vn);

		vert = f->getVert(vx2);
		vn = pivot*GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
		rf->v[1].normal=P32RV(vn);

		vert = f->getVert(vx3);
		vn = pivot*GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
		rf->v[2].normal=P32RV(vn);

		// texture coordinate..

		if(bExportTexCoord)
		{
			TVFace *tvf=&mesh->tvFace[i];

			rf->v[0].u=mesh->tVerts[tvf->t[vx1]].x;
			rf->v[0].v=1.0f-mesh->tVerts[tvf->t[vx1]].y;
			rf->v[1].u=mesh->tVerts[tvf->t[vx2]].x;
			rf->v[1].v=1.0f-mesh->tVerts[tvf->t[vx2]].y;
			rf->v[2].u=mesh->tVerts[tvf->t[vx3]].x;
			rf->v[2].v=1.0f-mesh->tVerts[tvf->t[vx3]].y;
		}

		// material index..

		int nMaterial=mesh->faces[i].getMatID();

		if(mat)
		{
			rf->nMaterial= (nMaterial<mat->nSubMaterial) ? mat->SubMaterials[nMaterial] : mat->SubMaterials[0];

			MaxSubMaterial *pmat=rsm->MaxSubMaterialList.Get(rf->nMaterial);

			TransformUV(rf->v[0].u,rf->v[0].v,pmat);
			TransformUV(rf->v[1].u,rf->v[1].v,pmat);
			TransformUV(rf->v[2].u,rf->v[2].v,pmat);
		}
		else
			rf->nMaterial= -1;

		rsm->face.Add(rf);

	}

	if (needDel) {
		delete tri;
	}
}
*/

Point3 RbsExp::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			vertexNormal = mesh->getFaceNormal(faceNo);
			for (int i = 0; i < numNormals; i++) {
				
				// 이부분이 처리가 안되있는듯. 추가한 코드.
				if(rv->ern[i].getSmGroup()==smGroup)
					vertexNormal = rv->ern[i].getNormal();
			}
		}
	}
	else {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}

/****************************************************************************

  Material and Texture Export
  
****************************************************************************/

void RbsExp::ExportMaterialList()
{
	int numMtls = mtlList.Count();

	for (int i=0; i<numMtls; i++) {
		DumpMaterial(mtlList.GetMtl(i), i, -1);
	}

}

void RbsExp::DumpMaterial(Mtl* mtl, int mtlID, int subNo)
{
	MaxSubMaterial *subm=NULL;
	MaxMaterial *maxm=NULL;

	int i;
	TimeValue t = GetStaticFrame();
	
	if (!mtl) return;
	
	TSTR className;
	mtl->GetClassName(className);
	
	
	if (subNo == -1) {
		// Top level material
		maxm=new MaxMaterial;
		rsm->MaxMaterialList.Add(maxm);
	}
	
	// We know the Standard material, so we can get some extra info
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
		StdMat* std = (StdMat*)mtl;

		subm=new MaxSubMaterial;
		subm->Name=string(mtl->GetName());
		subm->Ambient=rvector(std->GetAmbient(t).r,std->GetAmbient(t).g,std->GetAmbient(t).b);
		subm->Diffuse=rvector(std->GetDiffuse(t).r,std->GetDiffuse(t).g,std->GetDiffuse(t).b);
		subm->Specular=rvector(std->GetSpecular(t).r,std->GetSpecular(t).g,std->GetSpecular(t).b);
		
		subm->dwFlags=0;
		if(std->GetTransparencyType()==TRANSP_ADDITIVE) subm->dwFlags|=RM_FLAG_ADDITIVE;
		if(std->GetTwoSided()==TRUE) subm->dwFlags|=RM_FLAG_TWOSIDED;
		if(strnicmp(mtl->GetName(),"at_",3)==0) subm->dwFlags|=RM_FLAG_USEALPHATEST;

		rsm->thismat=subm;
	}

	for (i=0; i<mtl->NumSubTexmaps(); i++) {
		Texmap* subTex = mtl->GetSubTexmap(i);
		float amt = 1.0f;
		if (subTex) {
			// If it is a standard material we can see if the map is enabled.
			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
				if (!((StdMat*)mtl)->MapEnabled(i))
					continue;
				amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
				
			}
			DumpTexture(subTex, mtl->ClassID(), i, amt);
		}
	}
	
	if (mtl->NumSubMtls() > 0)  {
		maxm->nSubMaterial=mtl->NumSubMtls();
		maxm->SubMaterials=new int[maxm->nSubMaterial];

		for (i=0; i<mtl->NumSubMtls(); i++) {
			Mtl* subMtl = mtl->GetSubMtl(i);
			if (subMtl) {
				DumpMaterial(subMtl, 0, i);
				(maxm->SubMaterials)[i]=rsm->MaxSubMaterialList.GetByName(subMtl->GetName());
			}
			else
			{
				(maxm->SubMaterials)[i]=-1;
			}
		}
	}

	if(subm&&(rsm->MaxSubMaterialList.GetByName(subm->Name.c_str())==-1))
		rsm->MaxSubMaterialList.Add(subm);
	else
		delete subm;
	if(subm&&maxm) // top level & standard
	{
		maxm->nSubMaterial=1;
		maxm->SubMaterials=new int[1];
		maxm->SubMaterials[0]=rsm->MaxSubMaterialList.GetByName(mtl->GetName());
	}

}

void RbsExp::DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt)
{
	if (!tex) return;
	
	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {

		switch(subNo) {
			case ID_DI:
			{
				TSTR mapName = ((BitmapTex *)tex)->GetMapName();
				rsm->thismat->DiffuseMap=string(mapName);

				StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
				if (uvGen) {
					DumpUVGen(uvGen);
				}
			}break;
			case ID_OP:
			{
				TSTR mapName = ((BitmapTex *)tex)->GetMapName();
//				if(stricmp(rsm->thismat->MapFileName,mapName)==0)
				{
//					rsm->thismat->bOpacitymap=TRUE;
					rsm->thismat->dwFlags|=RM_FLAG_USEOPACITY;
				}
			}break;
		}
	}
	
	for (int i=0; i<tex->NumSubTexmaps(); i++) {
		DumpTexture(tex->GetSubTexmap(i), tex->ClassID(), i, 1.0f);
	}
}

void RbsExp::DumpUVGen(StdUVGen* uvGen)
{
	TimeValue t = GetStaticFrame();
	rsm->thismat->tileu=(uvGen->GetUScl(t));
	rsm->thismat->tilev=(uvGen->GetVScl(t));
	rsm->thismat->sina=sin(uvGen->GetAng(t));
	rsm->thismat->cosa=cos(uvGen->GetAng(t));
}

/****************************************************************************

  Face Mapped Material functions
  
****************************************************************************/


BOOL RbsExp::CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh)
{
	if (!mtl || !mesh) {
		return FALSE;
	}
	
	ULONG matreq = mtl->Requirements(-1);
	
	// Are we using face mapping?
	if (!(matreq & MTLREQ_FACEMAP)) {
		return FALSE;
	}
	
	return TRUE;
}


// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale factor
// so when calculating the normal we should take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'.
BOOL RbsExp::TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* RbsExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

// spawn positions
#include <stdio.h>
void RbsExp::ExportHelperObject(INode* node)
{
	TimeValue t = GetStaticFrame();

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj) return;

	Object* helperObj = node->EvalWorldState(0).obj;
	if (helperObj == NULL) return;

	TSTR className;
	helperObj->GetClassName(className);

	int nPropValue = 0;
	float fPropValue = 0.f;
	TSTR cstrBuffer;

	// 더미만 익스포트한다.
	if (!strnicmp(className, "dummy", 5))
	{
		RDUMMY *pdm=new RDUMMY;
		pdm->name=node->GetName();

		// 필요한 값이 있을때마다 추가해주도록 하자.
		int nPropValue=0;
		if (node->GetUserPropInt("time", nPropValue))
		{
			pdm->SetUserPropValue("time", nPropValue);
		}

		TSTR strValue;
		if(node->GetUserPropString("type", strValue))
		{
			pdm->SetUserPropValue("type", strValue );
		}
		if( node->GetUserPropString("min_point", strValue))
		{
			pdm->SetUserPropValue("min_point", strValue );
		}
		if( node->GetUserPropString("max_point", strValue))
		{
			pdm->SetUserPropValue("max_point", strValue );
		}
		if( node->GetUserPropString("r", strValue))
		{
			pdm->SetUserPropValue("r", strValue );
		}
		if( node->GetUserPropString("file", strValue))
		{
			pdm->SetUserPropValue("file", strValue );
		}

		Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
		AffineParts ap;
		decomp_affine(pivot, &ap);
		pdm->position=rvector(-ap.t.x,ap.t.y,ap.t.z);

		Matrix3 matRot;

		ap.q.MakeMatrix(matRot);
		Point3 dir = matRot.PointTransform(Point3(0.0f, 1.0f, 0.0f));


		pdm->direction= rvector(-dir.x, dir.y, dir.z);
		Normalize(pdm->direction);

		// Smoke 일경우
		const char* cstrSmoke = "smk_";

		if ( strnicmp( pdm->name.c_str(), cstrSmoke, sizeof(cstrSmoke) ) == 0 )
		{
//			pdm->SetUserPropValue(RTOK_SMOKE_NAME, filename );

			if( node->GetUserPropInt("direction", nPropValue) )
				pdm->SetUserPropValue(RTOK_SMOKE_DIRECTION,nPropValue);

			if( node->GetUserPropFloat("power", fPropValue) )
				pdm->SetUserPropValue(RTOK_SMOKE_POWER, fPropValue);

			if( node->GetUserPropFloat("size", fPropValue) ) 
				pdm->SetUserPropValue(RTOK_SMOKE_SIZE,fPropValue);

			if( node->GetUserPropString("color", cstrBuffer) ) 
				pdm->SetUserPropValue(RTOK_SMOKE_COLOR,cstrBuffer);

			if( node->GetUserPropInt("delay", nPropValue) )
				pdm->SetUserPropValue(RTOK_SMOKE_DELAY,nPropValue);

			if( node->GetUserPropFloat("life",fPropValue) )
				pdm->SetUserPropValue(RTOK_SMOKE_LIFE,fPropValue);

			if( node->GetUserPropFloat("togglemintime",fPropValue) )
				pdm->SetUserPropValue(RTOK_SMOKE_TOGMINTIME,fPropValue);
		}

		rsm->m_DummyList.push_back(pdm);
	}

}

/*
float GetAngle(rvector &a)
{
	if(a.x>=1.0f) return 0.0f;
	if(a.x<=-1.0f) return -pi;
	if(a.y>0)
		return (float)acos(a.x);
	else
		return (float)-acos(a.x);
}

// 충돌체크를 위한 맵을 생성한다
void RbsExp::ExportMesh_Extrude(INode* node, TimeValue t)
{
	int i,j,k,l;

	int mtlID = -1;
	Mtl* nodeMtl = node->GetMtl();
	if (nodeMtl) {
		mtlID = mtlList.GetMtlID(nodeMtl);
	}
	MaxMaterial *mat=(mtlID==-1)?NULL:rsm->MaxMaterialList.Get(mtlID);

	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}

	// 이름규칙들 통과하는 폴리곤은 만들필요도 없다
	if(strstr(node->GetName(),RTOK_MAX_PASSTHROUGH)!=NULL) 
		return;

	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
	pivot.NoTrans();

	Point3 fn;  // Face normal
	Point3 vn;  // Vertex normal

	BOOL deleteIt = FALSE;

	Object *obj = node->EvalWorldState(t).obj;
	PolyObject *poly;
	if (obj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) { 
		poly = (PolyObject *) obj->ConvertToType(t, 
			Class_ID(POLYOBJ_CLASS_ID, 0));

		if (obj != poly) deleteIt = TRUE;
	}

	log("test %d ver / %d face \n",poly->mm.VNum(),poly->mm.FNum());

	Point3 *vertices=new Point3[poly->mm.VNum()];

	for(i=0;i<poly->mm.VNum(); i++){
		vertices[i] = tm * poly->mm.P(i);
	}

	for (i=0; i<poly->mm.FNum(); i++) {
		MNFace *f=poly->mm.F(i);

		if(mat)
		{
			int nMaterial=f->material;
			nMaterial= (nMaterial<mat->nSubMaterial) ? mat->SubMaterials[nMaterial] : mat->SubMaterials[0];
			MaxSubMaterial *pmat=rsm->MaxSubMaterialList.Get(nMaterial);
			if((pmat->dwFlags & RM_FLAG_ADDITIVE) != 0)
			{
//				g_nElimFace++;
				continue;
			}
		}

		Point3 fnormal;
		poly->mm.ComputeNormal(i,fnormal);
		fnormal=pivot*fnormal;
		fnormal=fnormal.Normalize();

		// fnormal을 법선으로 하고 vertex 0을 지나는 평면의 방정식을 만든다
		float d= - DotProd(fnormal ,vertices[f->vtx[0]]);

		// 폴리곤들이 모두 한 평면 위에 있고, 볼록 다각형이면 쪼갤 필요가없다
		bool bCoplanar=true;
		bool bConvex=true;

		int nSign=0;

		for(j=0;j<f->deg;j++)
		{
			rvector normal;
			rplane testplane;
			Point3 p;
			rvector edgea,edgeb;
			p=vertices[f->vtx[j]];
			edgea=P32RV(p);

#define COPLA_TOLER		1.f
			float fCop=DotProd(vertices[f->vtx[j]],fnormal) + d;
			if(fCop<-COPLA_TOLER || fCop>COPLA_TOLER)		// 동일 평면위에 있는지 확인
			{
				bCoplanar=false;
				break;
			}

			p=vertices[f->vtx[(j+1)%f->deg]];
			edgeb=P32RV(p);
			CrossProduct(&normal,edgeb-edgea,P32RV(fnormal));
			D3DXPlaneFromPointNormal(&testplane,&edgea,&normal);
			D3DXPlaneNormalize(&testplane,&testplane);

			// 각각의 edge 에 대해서 평면을 세우고 그 평면의 + 쪽에 다음 버텍스가 있는지 확인한다
#define CONVEX_TOLER	1.f

			p=vertices[f->vtx[(j+2)%f->deg]];
			rvector testv=P32RV(p);
			float ftestv=D3DXPlaneDotCoord(&testplane,&testv);
#define CONVEX_SIGN(x) ((x)<-CONVEX_TOLER ? -1 : (x)>CONVEX_TOLER ? 1 : 0)
			
			if(nSign==0)
				nSign=CONVEX_SIGN(ftestv);
			else
			{
				int sign=CONVEX_SIGN(ftestv);
				if(nSign!=sign && 0!=sign)
				{
					bConvex=false;
					break;
				}
			}
		}

		if(bConvex && bCoplanar)
		{
			rpolygon *rf=new rpolygon;

			// coordinate..

			rf->v=new rvertex[f->deg];
			rf->nCount=f->deg;

			Point3 p;

			for(j=0;j<f->deg;j++)
			{
				if(negScale)
					p=vertices[f->vtx[f->deg-1-j]];
				else
					p=vertices[f->vtx[j]];
				p+=50.f*fnormal;
				rf->v[j].coord=P32RV(p);
			}

			rplane plane;
			D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

			rf->normal=rvector(plane.a,plane.b,plane.c);
			rf->d=plane.d;

			rsm->faceCol.Add(rf);
		}
		else // 그렇지 않으면 쪼개야한다
		{
			Tab<int> tris;
			f->GetTriangles(tris);

			for(int j=0;j<f->TriNum();j++)
			{
				rpolygon *rf=new rpolygon;

				// coordinate..

				rf->v=new rvertex[3];
				rf->nCount=3;

				Point3 p;

				for(k=0;k<3;k++)
				{
					if(negScale)
						p=vertices[f->vtx[tris[j*3+(2-k)]]];
					else
						p=vertices[f->vtx[tris[j*3+k]]];
					p+=50.f*fnormal;
					rf->v[k].coord=P32RV(p);
				}

				rplane plane;
				D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

				rf->normal=rvector(plane.a,plane.b,plane.c);
				rf->d=plane.d;

				rsm->faceCol.Add(rf);
			}
		}
	}

	// 공유하는 edge 를 가진넘들은 
	for (i=0; i<poly->mm.FNum(); i++) {
		MNFace *f=poly->mm.F(i);

		Point3 fnormal;
		poly->mm.ComputeNormal(i,fnormal);
		fnormal=pivot*fnormal;
		fnormal=fnormal.Normalize();

		for (j=i+1; j<poly->mm.FNum(); j++) {

			MNFace *f2=poly->mm.F(j);

			Point3 fnormal2;
			poly->mm.ComputeNormal(j,fnormal2);
			fnormal2=pivot*fnormal2;
			fnormal2=fnormal2.Normalize();

			for(k=0;k<f->deg;k++) {
				for(l=0;l<f2->deg;l++) {

					if((f->vtx[k]==f2->vtx[(l+1)%f2->deg] && f->vtx[(k+1)%f->deg]==f2->vtx[l] ) ||
						(f->vtx[(k+1)%f->deg]==f2->vtx[(l+1)%f2->deg] && f->vtx[k]==f2->vtx[l] ) )

					{
						// edge 가 튀어나와있는형태인지, 들어가 있는형태인지.. 판단해서
						// 들어가 있는 형태일때는 edge 가 벌어지는게 아니므로 메꿀필요가 없다
						{
							Point3 p;
							rplane testplane;
							rvector normal;
							p=vertices[f->vtx[(k+1)%f->deg]]-vertices[f->vtx[k]];
							rvector edge=P32RV(p);
							CrossProduct(&normal,edge,P32RV(fnormal));
							Normalize(normal);

							rvector apoint=P32RV(vertices[f->vtx[k]]);
							D3DXPlaneFromPointNormal(&testplane,&apoint,&normal);
							if(negScale)
								testplane=-testplane;

							p=vertices[f2->vtx[l]];
							rvector testpoint=P32RV(p+50.f*fnormal2);
							
							if(D3DXPlaneDotCoord(&testplane,&testpoint)>-COPLA_TOLER)
								continue;
						}

						rpolygon *rf = new rpolygon;
						rf->nCount=4;
						rf->v=new rvertex[4];

						Point3 p[4];
						p[0] = tm * poly->mm.P(f2->vtx[(l+1)%f2->deg]) + 50.f*fnormal2;
						p[1] = tm * poly->mm.P(f2->vtx[l]) + 50.f*fnormal2;
						p[2] = tm * poly->mm.P(f->vtx[(k+1)%f->deg]) + 50.f*fnormal;
						p[3] = tm * poly->mm.P(f->vtx[k]) + 50.f*fnormal;

						for(int m=0;m<4;m++)
						{
							if(negScale)
								rf->v[m].coord= P32RV(p[3-m]);
							else
								rf->v[m].coord= P32RV(p[m]);
						}

						rplane plane;
						D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

						rf->normal=rvector(plane.a,plane.b,plane.c);
						rf->d=plane.d;

						rsm->faceCol.Add(rf);
					}								
				}
			}
		}
	}


	int *nRefCount=new int[poly->mm.VNum()];
	for (i=0; i<poly->mm.VNum(); i++) {
		nRefCount[i]=0;
	}
	
	for (i=0; i<poly->mm.FNum(); i++) {
		MNFace *f=poly->mm.F(i);
		for(j=0; j<f->deg; j++) {
			nRefCount[f->vtx[j]]++;
		}
	}

	for(i=0;i<poly->mm.VNum(); i++)
	{
		if(nRefCount[i]>=3)
		{
			rpolygon *rf = new rpolygon;
			rf->nCount=nRefCount[i];
			rf->v=new rvertex[nRefCount[i]];
			rf->normal=rvector(0,0,0);

			rvertex *pv=rf->v;

			rvector center=rvector(0,0,0);

			for (j=0; j<poly->mm.FNum(); j++) {
				MNFace *f=poly->mm.F(j);

				for(k=0; k<f->deg; k++) {
					if(f->vtx[k]==i)
					{
						Point3 fnormal;
						poly->mm.ComputeNormal(j,fnormal);
						fnormal=pivot*fnormal;
						fnormal=fnormal.Normalize();

						Point3 p;
						p = tm * poly->mm.P(i) + 50.f*fnormal;
						pv->coord= P32RV(p);

						bool bElim=false;
						for(rvertex *l=rf->v;l<pv;l++)
						{
							if(IS_EQ3(pv->coord,l->coord))
							{
								rf->nCount--;
								bElim=true;
								break;
							}
						}

						if(!bElim)
						{
							rf->normal+=fnormal;
							center+=pv->coord;
							pv++;
						}
					}
				}
			}

			if(rf->nCount<3){
				delete rf;
				continue;
			}

			Normalize(rf->normal);
			center *= 1.f/float(rf->nCount);

			rvector up=center-rf->v[0].coord;
			rvector at=center-rf->normal;
			
			rmatrix tm;
			D3DXMatrixLookAtLH(&tm,&center,&at,&up);

			map<float,rvector> vertices;

			bool bElim=false;
			for(j=0;j<rf->nCount;j++)
			{
				rvector tv;
				D3DXVec3TransformCoord(&tv,&rf->v[j].coord,&tm);
				if(fabs(tv.z)>1.f) {
					bElim=true;
					break;
				}

				Normalize(tv);
				float angle=GetAngle(tv);
				vertices.insert(map<float,rvector>::value_type(angle,rf->v[j].coord));
			}

			if(bElim)
			{
				delete rf;
				continue;
			}


			map<float,rvector>::iterator it=vertices.begin();
			for(j=0;j<rf->nCount;j++)
			{
				rf->v[j].coord=it->second;
				it++;
			}

			rplane plane;
			D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

			rf->normal=rvector(plane.a,plane.b,plane.c);
			rf->d=plane.d;

			rsm->faceCol.Add(rf);
		}
	}

	if (deleteIt) {
		delete poly;
	}

	return;
}

void RbsExp::ExportMesh_CSG(INode* node, TimeValue t)
{
	int i,j,k,l;

	int mtlID = -1;
	Mtl* nodeMtl = node->GetMtl();
	if (nodeMtl) {
		mtlID = mtlList.GetMtlID(nodeMtl);
	}
	MaxMaterial *mat=(mtlID==-1)?NULL:rsm->MaxMaterialList.Get(mtlID);

	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}

	// 이름규칙들 통과하는 폴리곤은 만들필요도 없다
	if(strstr(node->GetName(),RTOK_MAX_PASSTHROUGH)!=NULL) 
		return;


	RCSGObject *pCObj=new RCSGObject;
	pCObj->m_Name=string(node->GetName());
	pCObj->m_pPolygons=new RSPolygonList;
    
	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
	pivot.NoTrans();

	Point3 fn;  // Face normal
	Point3 vn;  // Vertex normal

	BOOL deleteIt = FALSE;

	Object *obj = node->EvalWorldState(t).obj;
	PolyObject *poly = NULL;
	if (obj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) { 
		poly = (PolyObject *) obj->ConvertToType(t, 
			Class_ID(POLYOBJ_CLASS_ID, 0));

		if (obj != poly) deleteIt = TRUE;
	}

	log("test %d ver / %d face \n",poly->mm.VNum(),poly->mm.FNum());

	Point3 *vertices=new Point3[poly->mm.VNum()];

	for(i=0;i<poly->mm.VNum(); i++){
		vertices[i] = tm * poly->mm.P(i);
	}

	for (i=0; i<poly->mm.FNum(); i++) {
		MNFace *f=poly->mm.F(i);

		if(mat)
		{
			int nMaterial=f->material;
			nMaterial= (nMaterial<mat->nSubMaterial) ? mat->SubMaterials[nMaterial] : mat->SubMaterials[0];
			MaxSubMaterial *pmat=rsm->MaxSubMaterialList.Get(nMaterial);
			if((pmat->dwFlags & RM_FLAG_ADDITIVE) != 0)
			{
				//				g_nElimFace++;
				continue;
			}
		}

		Point3 fnormal;
		poly->mm.ComputeNormal(i,fnormal);
		fnormal=pivot*fnormal;
		fnormal=fnormal.Normalize();

		// fnormal을 법선으로 하고 vertex 0을 지나는 평면의 방정식을 만든다
		float d= - DotProd(fnormal ,vertices[f->vtx[0]]);

		// 폴리곤들이 모두 한 평면 위에 있고, 볼록 다각형이면 쪼갤 필요가없다
		bool bCoplanar=true;
		bool bConvex=true;

		int nSign=0;

		for(j=0;j<f->deg;j++)
		{
			rvector normal;
			rplane testplane;
			Point3 p;
			rvector edgea,edgeb;
			p=vertices[f->vtx[j]];
			edgea=P32RV(p);

#define COPLA_TOLER		1.f
			float fCop=DotProd(vertices[f->vtx[j]],fnormal) + d;
			if(fCop<-COPLA_TOLER || fCop>COPLA_TOLER)		// 동일 평면위에 있는지 확인
			{
				bCoplanar=false;
				break;
			}

			p=vertices[f->vtx[(j+1)%f->deg]];
			edgeb=P32RV(p);
			CrossProduct(&normal,edgeb-edgea,P32RV(fnormal));
			D3DXPlaneFromPointNormal(&testplane,&edgea,&normal);
			D3DXPlaneNormalize(&testplane,&testplane);

			// 각각의 edge 에 대해서 평면을 세우고 그 평면의 + 쪽에 다음 버텍스가 있는지 확인한다
#define CONVEX_TOLER	1.f

			p=vertices[f->vtx[(j+2)%f->deg]];
			rvector testv=P32RV(p);
			float ftestv=D3DXPlaneDotCoord(&testplane,&testv);
#define CONVEX_SIGN(x) ((x)<-CONVEX_TOLER ? -1 : (x)>CONVEX_TOLER ? 1 : 0)

			if(nSign==0)
				nSign=CONVEX_SIGN(ftestv);
			else
			{
				int sign=CONVEX_SIGN(ftestv);
				if(nSign!=sign && 0!=sign)
				{
					bConvex=false;
					break;
				}
			}
		}

		if(bConvex && bCoplanar)
		{
			rpolygon *rf=new rpolygon;

			// coordinate..

			rf->v=new rvertex[f->deg];
			rf->nCount=f->deg;

			Point3 p;

			for(j=0;j<f->deg;j++)
			{
				if(negScale)
					p=vertices[f->vtx[f->deg-1-j]];
				else
					p=vertices[f->vtx[j]];
				rf->v[j].coord=P32RV(p);
			}

			rplane plane;
			D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

			rf->normal=rvector(plane.a,plane.b,plane.c);
			rf->d=plane.d;

			pCObj->m_pPolygons->Add(rf);
		}
		else // 그렇지 않으면 쪼개야한다
		{
			Tab<int> tris;
			f->GetTriangles(tris);

			for(int j=0;j<f->TriNum();j++)
			{
				rpolygon *rf=new rpolygon;

				// coordinate..

				rf->v=new rvertex[3];
				rf->nCount=3;

				Point3 p;

				for(k=0;k<3;k++)
				{
					if(negScale)
						p=vertices[f->vtx[tris[j*3+(2-k)]]];
					else
						p=vertices[f->vtx[tris[j*3+k]]];
					rf->v[k].coord=P32RV(p);
				}

				rplane plane;
				D3DXPlaneFromPoints(&plane,&rf->v[0].coord,&rf->v[2].coord,&rf->v[1].coord);

				rf->normal=rvector(plane.a,plane.b,plane.c);
				rf->d=plane.d;

				pCObj->m_pPolygons->Add(rf);
			}
		}
	}

	if (deleteIt) {
		delete poly;
	}

	if(pCObj->m_pPolygons->GetCount()==0)
	{
		delete pCObj;
	}else
		rsm->m_CSGObjects.push_back(pCObj);
	return;
}
*/

void RbsExp::ExportMesh(INode* node, TimeValue t)
{
	int i,j,k;

	int mtlID = -1;
	Mtl* nodeMtl = node->GetMtl();
	if (nodeMtl) {
		mtlID = mtlList.GetMtlID(nodeMtl);
	}
	MaxMaterial *mat=(mtlID==-1)?NULL:rsm->MaxMaterialList.Get(mtlID);

	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}

	log("object %s : ",node->GetName());

	BOOL deleteIt = FALSE;

	Object *obj = node->EvalWorldState(t).obj;
	PolyObject *poly = NULL;
	if (obj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0))) { 
		poly = (PolyObject *) obj->ConvertToType(t, 
			Class_ID(POLYOBJ_CLASS_ID, 0));

		if (obj != poly) deleteIt = TRUE;
	}

	log("%s: %d ver / %d face \n",node->GetName(), poly->mm.VNum(),poly->mm.FNum());

	DWORD dwFlags=0;

	
	// 이름규칙들
	if(strstr(node->GetName(),RTOK_MAX_NOPATH)!=NULL)
		dwFlags|=RM_FLAG_NOTWALKABLE;
	if(strstr(node->GetName(),RTOK_MAX_PASSTHROUGH)!=NULL) 
		dwFlags|=RM_FLAG_PASSTHROUGH;
	if(strstr(node->GetName(),RTOK_MAX_HIDE)!=NULL) 
		dwFlags|=RM_FLAG_HIDE;
	if(strstr(node->GetName(),RTOK_MAX_PASSBULLET)!=NULL) 
		dwFlags|=RM_FLAG_PASSBULLET;
	if(strstr(node->GetName(),RTOK_MAX_PASSROCKET)!=NULL) 
		dwFlags|=RM_FLAG_PASSROCKET;
	if(strstr(node->GetName(),RTOK_MAX_NAVIGATION)!=NULL)			// 퀘스트 AI용
	{
		dwFlags|=RM_FLAG_HIDE;
		dwFlags|=RM_FLAG_AI_NAVIGATION;
	}


	if(node->CastShadows())
		dwFlags|=RM_FLAG_CASTSHADOW;
	if(node->RcvShadows())
		dwFlags|=RM_FLAG_RECEIVESHADOW;


	// Navigation Mesh이면 따로 익스포트.. - bird
	if (dwFlags & RM_FLAG_AI_NAVIGATION)
	{
		ExportNavigationMesh(node, t);
		return;
	}

	RCSGObject *pCObj=new RCSGObject;
	pCObj->m_Name=string(node->GetName());
	pCObj->m_pPolygons=new RSPolygonList;

	Matrix3 pivot = tm;
	pivot.NoTrans();

	Point3 *vertices=new Point3[poly->mm.VNum()];

	for(i=0;i<poly->mm.VNum(); i++){
		vertices[i] = tm * poly->mm.P(i);
	}

	poly->mm.SpecifyNormals();

	MNNormalSpec *ns=poly->mm.GetSpecifiedNormals();
	ns->BuildNormals();
	ns->ComputeNormals();
	_ASSERT(poly->mm.FNum()==ns->GetNumFaces());

	int nmap=poly->mm.MNum();
	MNMap *mnmap=poly->mm.M(1);

	if(mnmap)
	{
		if(poly->mm.FNum()!=mnmap->FNum())
			mnmap=NULL;
	}
 
	MNNormalFace *nf = ns->GetFaceArray();

	for (i=0; i<poly->mm.FNum(); i++,nf++) {
		MNFace *f=poly->mm.F(i);

		/*
		MNNormalFace nf=ns->Face(i);
		_ASSERT(f->deg==nf.GetDegree());
		*/

		MNMapFace *mf=NULL;
		if(mnmap) {
			mf=mnmap->F(i);
			_ASSERT(f->deg==mf->deg);
		}

		

		int nMaterial=-1;
		if(mat)
		{
			nMaterial=f->material;
			nMaterial= (nMaterial<mat->nSubMaterial) ? mat->SubMaterials[nMaterial] : mat->SubMaterials[0];
			MaxSubMaterial *pmat=rsm->MaxSubMaterialList.Get(nMaterial);
			if((pmat->dwFlags & RM_FLAG_ADDITIVE) != 0)
			{
				// additive 는 충돌체크 하지 않는다
				//				g_nElimFace++;
				dwFlags|=RM_FLAG_PASSTHROUGH;
//				continue;
			}
		}

		// fnormal을 법선으로 하고 vertex 0을 지나는 평면의 방정식을 만든다
		Point3 center;
		Point3 fnormal;
		poly->mm.ComputeNormal(i,fnormal,&center);
		fnormal=pivot*fnormal;
		fnormal.Normalize();

		dplane plane;
		dpoint nor=P32DP(fnormal);
		nor.Normalize();
		dpoint apoint=P32DP(vertices[f->vtx[0]]);
		DPlaneFromPointNormal(&plane,apoint,nor);

		// 폴리곤들이 모두 한 평면 위에 있고, 볼록 다각형이면 쪼갤 필요가없다
		bool bCoplanar=true;
		bool bConvex=true;

		int nSign=0;

		for(j=0;j<f->deg;j++)
		{
			dpoint normal;
			dplane testplane;
			Point3 p;
			dpoint edgea,edgeb;
			p=vertices[f->vtx[j]];
			edgea=P32DP(p);

			dpoint testpoint=P32DP(vertices[f->vtx[j]]);
			double fCop=DPlaneDotCoord(plane,testpoint);
			if(fCop<-BSPTOLER || fCop>BSPTOLER)		// 동일 평면위에 있는지 확인
			{
				bCoplanar=false;
				break;
			}

			p=vertices[f->vtx[(j+1)%f->deg]];
			edgeb=P32DP(p);
			normal=CrossProduct(P32DP(fnormal),edgeb-edgea);
			normal.Normalize();
			DPlaneFromPointNormal(&testplane,edgea,normal);

			// 각각의 edge 에 대해서 평면을 세우고 그 평면의 + 쪽에 다음 버텍스가 있는지 확인한다
#define CONVEX_TOLER	1.f

			p=vertices[f->vtx[(j+2)%f->deg]];
			dpoint testv=P32DP(p);
			double ftestv=DPlaneDotCoord(testplane,testv);
#define CONVEX_SIGN(x) ((x)<-CONVEX_TOLER ? -1 : (x)>CONVEX_TOLER ? 1 : 0)

			if(nSign==0)
				nSign=CONVEX_SIGN(ftestv);
			else
			{
				int sign=CONVEX_SIGN(ftestv);
				if(nSign!=sign && 0!=sign)
				{
					bConvex=false;
					break;
				}
			}
		}

		if(bConvex && bCoplanar)
		{
			rpolygon *rf=new rpolygon;

			// coordinate..

			rf->v=new rvertex[f->deg];
			rf->nCount=f->deg;
			rf->dwFlags=dwFlags;

			Point3 p;

			for(j=0;j<f->deg;j++)
			{
				int ncorner;
				if(negScale)
					ncorner=f->deg-1-j;
				else
					ncorner=j;

				p=vertices[f->vtx[ncorner]];
				rf->v[j].coord=P32DP(p);
//				p=pivot*ns->GetNormal(i,ncorner);
				
				int normalid=nf->GetNormalID(ncorner);
				Point3 ptnormal=ns->Normal(normalid);
				p=pivot*ptnormal;
				rf->v[j].normal=P32DP(p);
				
				if(mf)
				{
					rf->v[j].u=mnmap->V(mf->tv[ncorner]).x;
					rf->v[j].v=1.0f-mnmap->V(mf->tv[ncorner]).y;
				}

			}

			rf->normal=dpoint(plane.a,plane.b,plane.c);
			rf->d=plane.d;
			rf->nMaterial=nMaterial;

			/*
			float fArea=0;
			for(j=0;j<f->deg-2;j++)
				fArea+=GetArea(rf->v[0].coord,rf->v[j+1].coord,rf->v[j+2].coord);
			_ASSERT(fArea>BSPTOLER);
			*/

			if((dwFlags & RM_FLAG_HIDE) == NULL && f->material!=444 )
			{
				rsm->face.Add(rf);
			}

//			if (dwFlags & RM_FLAG_AI_NAVIGATION)
//			{
//				rsm->faceNavigation.Add(rf);
//			}

			if((dwFlags & RM_FLAG_PASSTHROUGH) == NULL )
			{
				rpolygon *pp=new rpolygon(rf);
				pp->nID=rsm->face.GetCount()-1;
				pCObj->m_pPolygons->Add(pp);
			}

			double fDot=rf->normal.Length();
			_ASSERT(fDot>0.999);
		}
		else // 그렇지 않으면 쪼개야한다
		{
			Tab<int> tris;
			f->GetTriangles(tris);

			for(int j=0;j<f->TriNum();j++)
			{
				rpolygon *rf=new rpolygon;

				// coordinate..

				rf->v=new rvertex[3];
				rf->nCount=3;
				rf->dwFlags=dwFlags;

				Point3 p;

				for(k=0;k<3;k++)
				{
					int ncorner;
					if(negScale)
						ncorner=tris[j*3+(2-k)];
					else
						ncorner=tris[j*3+k];
					p=vertices[f->vtx[ncorner]];
					rf->v[k].coord=P32DP(p);

					int normalid=nf->GetNormalID(ncorner);
					Point3 ptnormal=ns->Normal(normalid);
					p=pivot*ptnormal;
					rf->v[k].normal=P32DP(p);

					if(mf)
					{
						rf->v[k].u=mnmap->V(mf->tv[ncorner]).x;
						rf->v[k].v=1.0f-mnmap->V(mf->tv[ncorner]).y;
					}

				}

				dplane plane;
				DPlaneFromPoints(&plane,rf->v[0].coord,rf->v[2].coord,rf->v[1].coord);

				bool bDelete=false;
				if(rf->v[0].coord==rf->v[1].coord || 
					rf->v[2].coord==rf->v[1].coord || 
					rf->v[2].coord==rf->v[0].coord)
				{
					// 같은점이 있으면 지운다. 의미없는 폴리곤
					bDelete=true;
				}

				rf->normal=dpoint(plane.a,plane.b,plane.c);
				rf->d=plane.d;
				rf->nMaterial=nMaterial;

				/*
				float fArea=GetArea(rf->v[0].coord,rf->v[1].coord,rf->v[2].coord);
				_ASSERT(fArea>BSPTOLER);
				*/

				if(bDelete) delete rf;
				else
				{
					if((dwFlags & RM_FLAG_HIDE) == NULL && f->material!=444 )
					{
						rsm->face.Add(rf);
					}

//					if (dwFlags & RM_FLAG_AI_NAVIGATION)
//					{
//						rsm->faceNavigation.Add(rf);
//					}

					if((dwFlags & RM_FLAG_PASSTHROUGH) == NULL )
					{
						rpolygon *pp=new rpolygon(rf);
						pp->nID=rsm->face.GetCount()-1;
						pCObj->m_pPolygons->Add(pp);
					}

					double fDot=rf->normal.Length();
					_ASSERT(fDot>0.999);

				}
			}
		}
	}

	if (deleteIt) {
		// 이렇게 지우면 뻗는다 -_- why?
		delete poly;
	}

	if(pCObj->m_pPolygons->GetCount()==0)
	{
		delete pCObj;
	}else
	{
		if ((dwFlags & RM_FLAG_AI_NAVIGATION) == NULL)
		{
			rsm->m_CSGObjects.push_back(pCObj);
		}
		else
		{
			delete pCObj;
			//rsm->m_NavObjects.push_back(pCObj);
		}
		
	}
}


void RbsExp::ExportNavigationMesh(INode* node, TimeValue t)
{
	int i;
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);
	int vx1, vx2, vx3;
	
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return;	// Safety net. This shouldn't happen.
	}
	
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}

	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri) {
		return;
	}

	Mesh* mesh = &tri->GetMesh();
	mesh->buildNormals();

	RSNavigationMesh* pNav = &rsm->m_Navigation;
	log("NaviMesh: %d ver / %d face \n",mesh->getNumVerts(), mesh->getNumFaces());

	pNav->Init(mesh->getNumVerts(), mesh->getNumFaces());

	// Export the vertices
	for (i = 0; i < mesh->getNumVerts(); i++)
	{
		Point3 v = tm*mesh->verts[i];
		pNav->vertices[i] = P32RV(v);
	}

	for (i=0; i < mesh->getNumFaces(); i++)
	{
		pNav->faces[i].v1 = mesh->faces[i].v[vx1];
		pNav->faces[i].v2 = mesh->faces[i].v[vx2];
		pNav->faces[i].v3 = mesh->faces[i].v[vx3];
	}

/*
	// 로그를 남겨본다
	for (int i = 0; i < pNav->nVertCount; i++)
	{
		log("Nav Vert(%d) - %.5f %.5f %.5f\n", i, pNav->vertices[i].x, pNav->vertices[i].y, pNav->vertices[i].z);
	}

	for (int i = 0; i < pNav->nFaceCount; i++)
	{
		log("Nav Face(%d) - %d %d %d\n", i, pNav->faces[i].v1, pNav->faces[i].v2, pNav->faces[i].v3);
	}
*/
}