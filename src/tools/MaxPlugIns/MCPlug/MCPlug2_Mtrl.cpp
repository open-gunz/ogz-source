#include "MCplug2.h"

////////////////////////////////////////////
// mtrl list

BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
	if (!mtl) return FALSE;

	int numMtls = mtlTab.Count();

	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return FALSE;
		}
	}

	mtlTab.Append(1, &mtl, 25);

	return TRUE;
}

int MtlKeeper::GetMtlID(Mtl* mtl)
{
	int numMtls = mtlTab.Count();

	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) 
			return i;
	}

	return -1;
}

int MtlKeeper::Count()
{
	return mtlTab.Count();
}

Mtl* MtlKeeper::GetMtl(int id)
{
	return mtlTab[id];
}

///////////////////////////////////////////////////////////////////
// mtrl list export

void MCplug2::export_mtrl_list()
{
	int numMtls = mtlList.Count();

	for (int i=0; i<numMtls; i++) {
		DumpMaterial(mtlList.GetMtl(i), i, -1);
	}
}

/////////////////////////////////////////////
// object 의 머터리얼 id 표시

void MCplug2::DumpMaterial(Mtl* mtl, int mtlID, int subNo)
{
	mtrl_data* mtrl_node = new mtrl_data;
	memset(mtrl_node,0,sizeof(mtrl_data));

	mtrl_node->m_mtrl_id	 = mtlID;
	mtrl_node->m_sub_mtrl_id = subNo;
	mtrl_node->m_power		 = 0.f;

	int i;
	TimeValue t = GetStaticFrame();
	
	if (!mtl) return;
	
	TSTR className;
	mtl->GetClassName(className);
	
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {

		StdMat* std = (StdMat*)mtl;

		mtrl_node->m_ambient.r	= std->GetAmbient(t).r;
		mtrl_node->m_ambient.g	= std->GetAmbient(t).g;
		mtrl_node->m_ambient.b	= std->GetAmbient(t).b;

		mtrl_node->m_diffuse.r	= std->GetDiffuse(t).r;
		mtrl_node->m_diffuse.g	= std->GetDiffuse(t).g;
		mtrl_node->m_diffuse.b	= std->GetDiffuse(t).b;

		mtrl_node->m_specular.r = std->GetSpecular(t).r;
		mtrl_node->m_specular.g = std->GetSpecular(t).g;
		mtrl_node->m_specular.b = std->GetSpecular(t).b;

		mtrl_node->m_twosided	= std->GetTwoSided() ? 1 : 0;
		mtrl_node->m_power		= std->GetShinStr(t);
		
		mtrl_node->m_additive	= (std->GetTransparencyType()==TRANSP_ADDITIVE) ? 1 : 0;
		
		if(strnicmp(mtl->GetName(),"at_",3)==0)//alpha_test 식별..
			mtrl_node->m_alphatest_ref = 100;
/*
//		추가시

//		fprintf(m_p_txt,"%s\t%s %s\n",indent.data(), ID_SHINE			, Format(std->GetShininess(t)));
//		fprintf(m_p_txt,"%s\t%s %s\n",indent.data(), ID_SHINE_STRENGTH	, Format(std->GetShinStr(t)));
//		fprintf(m_p_txt,"%s\t%s %s\n",indent.data(), ID_TRANSPARENCY	, Format(std->GetXParency(t)));
//		fprintf(m_p_txt,"%s\t%s %s\n",indent.data(), ID_WIRESIZE		, Format(std->WireSize(t)));

		switch(std->GetShading()) 
		{
			case SHADE_CONST: fprintf(m_p_txt,"%s\n", ID_MAT_SHADE_CONST); break;
			case SHADE_PHONG: fprintf(m_p_txt,"%s\n", ID_MAT_SHADE_PHONG); break;
			case SHADE_METAL: fprintf(m_p_txt,"%s\n", ID_MAT_SHADE_METAL); break;
			case SHADE_BLINN: fprintf(m_p_txt,"%s\n", ID_MAT_SHADE_BLINN); break;
			default: fprintf(m_p_txt,"%s\n", ID_MAT_SHADE_OTHER); break;
		}
		
		if (std->GetTwoSided())		fprintf(m_p_txt,"%s\t%s\n", indent.data(), ID_TWOSIDED);
		if (std->GetWire()) 		fprintf(m_p_txt,"%s\t%s\n", indent.data(), ID_WIRE);
		if (std->GetWireUnits())	fprintf(m_p_txt,"%s\t%s\n", indent.data(), ID_WIREUNITS);
		if (std->GetFaceMap())		fprintf(m_p_txt,"%s\t%s\n", indent.data(), ID_FACEMAP);
		if (std->GetSoften()) 		fprintf(m_p_txt,"%s\t%s\n", indent.data(), ID_SOFTEN);
		
		switch (std->GetTransparencyType()) 
		{
			case TRANSP_FILTER:			fprintf(m_p_txt,"%s\n", ID_MAP_XPTYPE_FLT); break;
			case TRANSP_SUBTRACTIVE:	fprintf(m_p_txt,"%s\n", ID_MAP_XPTYPE_SUB); break;
			case TRANSP_ADDITIVE:		fprintf(m_p_txt,"%s\n", ID_MAP_XPTYPE_ADD); break;
			default:					fprintf(m_p_txt,"%s\n", ID_MAP_XPTYPE_OTH); break;
		}
*/
	}
	else {
		mtrl_node->m_ambient.r	= mtl->GetAmbient().r;
		mtrl_node->m_ambient.g	= mtl->GetAmbient().g;
		mtrl_node->m_ambient.b	= mtl->GetAmbient().b;

		mtrl_node->m_diffuse.r	= mtl->GetDiffuse().r;
		mtrl_node->m_diffuse.g	= mtl->GetDiffuse().g;
		mtrl_node->m_diffuse.b	= mtl->GetDiffuse().b;

		mtrl_node->m_specular.r = mtl->GetSpecular().r;
		mtrl_node->m_specular.g = mtl->GetSpecular().g;
		mtrl_node->m_specular.b = mtl->GetSpecular().b;

//		mtrl_node->m_twosided	= mtl->GetTwoSided() ? 1 : 0;
//		mtrl_node->m_power		= mtl->GetShinStr(t);
	}

	for (i=0; i<mtl->NumSubTexmaps(); i++) 	{
		Texmap* subTex = mtl->GetSubTexmap(i);
		float amt = 1.0f;

		if (subTex) {
			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) 	{
				if (!((StdMat*)mtl)->MapEnabled(i))	continue;
				amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
			}
			
			DumpTexture(subTex, mtl->ClassID(), i, amt,mtrl_node);
		}
	}
	
	if (mtl->NumSubMtls() > 0)  {
		mtrl_node->m_sub_mtrl_num = mtl->NumSubMtls();

		for (i=0; i<mtl->NumSubMtls(); i++) {
			Mtl* subMtl = mtl->GetSubMtl(i);

			if (subMtl) DumpMaterial(subMtl, mtlID, i);
		}
	}

	m_mesh_list.add_mtrl(mtrl_node);
}

TCHAR* MCplug2::GetMapID(Class_ID cid, int subNo)
{
	static TCHAR buf[50];
	
	if (cid == Class_ID(0,0)) {
		strcpy(buf, ID_ENVMAP);
	}
	else if (cid == Class_ID(DMTL_CLASS_ID, 0)) {
		switch (subNo) 	{
			case ID_AM: strcpy(buf, ID_MAP_AMBIENT);		break;
			case ID_DI: strcpy(buf, ID_MAP_DIFFUSE);		break;
			case ID_SP: strcpy(buf, ID_MAP_SPECULAR);		break;
			case ID_SH: strcpy(buf, ID_MAP_SHINE);			break;
			case ID_SS: strcpy(buf, ID_MAP_SHINESTRENGTH);	break;
			case ID_SI: strcpy(buf, ID_MAP_SELFILLUM);		break;
			case ID_OP: strcpy(buf, ID_MAP_OPACITY);		break;
			case ID_FI: strcpy(buf, ID_MAP_FILTERCOLOR);	break;
			case ID_BU: strcpy(buf, ID_MAP_BUMP);			break;
			case ID_RL: strcpy(buf, ID_MAP_REFLECT);		break;
			case ID_RR: strcpy(buf, ID_MAP_REFRACT);		break;
		}
	}
	else 
		strcpy(buf, ID_MAP_GENERIC);
	
	return buf;
}

void el_cut_path(char *name)
{
	char r_name[256];

	for(int i=strlen(name)-1; i>=0; i--) {
		if(name[i]=='\\') {
			break;
		}
	}

	int st=i+1;
	
	for(i=st; i<(int)strlen(name); i++)	{
		r_name[i-st]=name[i];
	}

	r_name[i-st]=NULL;
	strcpy(name,r_name);
}

void MCplug2::DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt,mtrl_data* mtrl_node)
{
	if (tex==NULL) return;
	
	TSTR className;
	tex->GetClassName(className);

//	맵종류 MAP_DIFFUSE : MAP_OPACITY 두종류 사용 ++

	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
//		경로는 잘라버리고 이름만 저장
		TSTR mapName = ((BitmapTex *)tex)->GetMapName();
		TSTR str = FixupName(mapName);

		el_cut_path((char*)str);

		if(strcmp(GetMapID(cid, subNo),ID_MAP_DIFFUSE)==0) {
			strcpy(mtrl_node->m_tex_name,str);
		}

		if(strcmp(GetMapID(cid, subNo),ID_MAP_OPACITY)==0) {
			strcpy(mtrl_node->m_opa_name,str);
		}
	}
}

BOOL MCplug2::CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh)
{
	if (!mtl || !mesh) {
		return FALSE;
	}
	
	ULONG matreq = mtl->Requirements(-1);
	
	if (!(matreq & MTLREQ_FACEMAP)) {
		return FALSE;
	}
	
	for (int i=0; i<mesh->getNumFaces(); i++) {
		Point3 tv[3];
		Face* f = &mesh->faces[i];
		make_face_uv(f, tv);
	}

	return TRUE;
}
