#include <Windows.h>
#include <cstdio>
#include <fstream>
#include "../../sdk/dx9/Include/d3dx9.h"
#include "../../sdk/rapidxml/include/rapidxml.hpp"
#include "../../sdk/rapidxml/include/rapidxml_print.hpp"
#include "gunzdefs.h"

template <typename T>
bool Read(FILE* File, T &obj, size_t size = sizeof(T))
{
	return fread(&obj, size, 1, File) == 1;
}

bool ReadElu(const char* fname, rapidxml::xml_document<> &doc, rapidxml::xml_node<> &node)
{
#define MZF_READ(x,y) fseek(pFile, y, SEEK_CUR)

	char Path[256];
	char Name[256];

	Path[0] = NULL;
	Name[0] = NULL;

	int len = strlen(Path);

	FILE *pFile;
	fopen_s(&pFile, fname, "rb");

	if (!pFile)
		return false;

	auto Read = [&](auto& obj, int size = -1) {
		return ::Read(pFile, obj, size == -1 ? sizeof(obj) : size);
	};

	ex_hd_t t_hd;

	Read(t_hd);

	printf_s("Version: %08X, mesh_num: %d, mtrl_num: %d\n", t_hd.ver, t_hd.mesh_num, t_hd.mtrl_num);

	if (t_hd.sig != EXPORTER_SIG) {
		printf_s("%s elu file 파일 식별 실패.\n", fname);
		return false;
	}

	int i;
	for (i = 0; i<t_hd.mtrl_num; i++) {

		MZF_READ(&node->m_mtrl_id, 4);
		MZF_READ(&node->m_sub_mtrl_id, 4);

		MZF_READ(&node->m_ambient, sizeof(D3DXCOLOR));
		MZF_READ(&node->m_diffuse, sizeof(D3DXCOLOR));
		MZF_READ(&node->m_specular, sizeof(D3DXCOLOR));

		MZF_READ(&node->m_power, 4);

		MZF_READ(&node->m_sub_mtrl_num, 4);

		char Name[MAX_PATH_NAME_LEN];
		if (t_hd.ver< EXPORTER_MESH_VER7) {
			Read(Name, MAX_NAME_LEN);
			MZF_READ(&node->m_opa_name, MAX_NAME_LEN);
		}
		else {
			Read(Name, MAX_PATH_NAME_LEN);
			MZF_READ(&node->m_opa_name, MAX_PATH_NAME_LEN);
		}

		//printf_s("Mtrl name %s\n", Name);

		if (t_hd.ver > EXPORTER_MESH_VER2) {
			int twoside = 0;
			MZF_READ(&twoside, sizeof(int));
		}

		if (t_hd.ver > EXPORTER_MESH_VER4) {
			int additive = 0;
			MZF_READ(&additive, sizeof(int));
		}

		if (t_hd.ver > EXPORTER_MESH_VER7)
		{
			int alpha_test = 0;
			MZF_READ(&alpha_test, sizeof(int));
		}
	}

	bool bNeedScaleMat = false;

	for (i = 0; i<t_hd.mesh_num; i++) {

		char Name[MAX_NAME_LEN];
		Read(Name);
		printf_s("Node name: %s\n", Name);

		if (!strncmp(Name, "eq_", 3))
		{
			static const char parts[6][8] = { "face", "head", "chest", "hands", "legs", "feet" };
			for (int i = 0; i < sizeof(parts) / sizeof(parts[0]); i++)
			{
				char buf[32];
				sprintf_s(buf, "eq_%s", parts[i]);
				if (!strncmp(Name, buf, strlen(buf)))
					node.append_attribute(doc.allocate_attribute("part", doc.allocate_string(Name)));
			}
		}

		MZF_READ(pMeshNode->m_Parent, MAX_NAME_LEN);
		MZF_READ(&pMeshNode->m_mat_base, sizeof(D3DXMATRIX));//mat

		if (t_hd.ver >= EXPORTER_MESH_VER2) {
			MZF_READ(&pMeshNode->m_ap_scale, sizeof(D3DXVECTOR3));//mat
		}

		///////////////////////////////////////////////

		if (t_hd.ver >= EXPORTER_MESH_VER4) {

			MZF_READ(&pMeshNode->m_axis_rot, sizeof(D3DXVECTOR3));
			MZF_READ(&pMeshNode->m_axis_rot_angle, sizeof(float));

			MZF_READ(&pMeshNode->m_axis_scale, sizeof(D3DXVECTOR3));
			MZF_READ(&pMeshNode->m_axis_scale_angle, sizeof(float));

			MZF_READ(&pMeshNode->m_mat_etc, sizeof(D3DXMATRIX));//mat

		}

		int m_point_num;
		Read(m_point_num);

		//printf_s("m_point_num: %d\n", m_point_num);

		if (m_point_num) {
			MZF_READ(pMeshNode->m_point_list, sizeof(D3DXVECTOR3) * m_point_num);
		}

		int m_face_num;
		Read(m_face_num);

		//printf_s("m_face_num: %d\n", m_face_num);

		if (m_face_num) {


			if (t_hd.ver >= EXPORTER_MESH_VER6) {//ver 6

				MZF_READ(pMeshNode->m_face_list, sizeof(RFaceInfo) * m_face_num);
				MZF_READ(pMeshNode->m_face_normal_list, sizeof(RFaceNormalInfo) * m_face_num);

			}
			else if (t_hd.ver > EXPORTER_MESH_VER2) {//ver3 부터

				MZF_READ(pMeshNode->m_face_list, sizeof(RFaceInfo) * m_face_num);
			}
			else {									//ver3 이하

				MZF_READ(pInfo, sizeof(RFaceInfoOld) * m_face_num);
			}
		}

		if (t_hd.ver >= EXPORTER_MESH_VER6) {

			int m_point_color_num;
			Read(m_point_color_num);

			//printf_s("m_point_color_num: %d\n", m_point_color_num);

			if (m_point_color_num) {
				MZF_READ(pMeshNode->m_point_color_list, sizeof(D3DXVECTOR3) * m_point_color_num);
			}
		}

		MZF_READ(&pMeshNode->m_mtrl_id, 4);

		int m_physique_num;
		Read(m_physique_num);

		//printf_s("m_physique_num: %d\n", m_physique_num);

		if (m_physique_num)
		{
			for (int j = 0; j < m_physique_num; j++)
				MZF_READ(&pMeshNode->m_physique[j], sizeof(RPhysiqueInfo));
		}
	}

	fclose(pFile);

	return true;
}

int main()
{
	rapidxml::xml_document<> doc;
	WIN32_FIND_DATA data;

	HANDLE hFind = FindFirstFile("*", &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strlen(data.cFileName) > 4 && !strncmp(data.cFileName + (strlen(data.cFileName) - 4), ".elu", 4))
			{
				printf_s("%s\n", data.cFileName);
				auto node = doc.allocate_node(rapidxml::node_element, "parts");
				node->append_attribute(doc.allocate_attribute("file", doc.allocate_string(data.cFileName)));
				ReadElu(data.cFileName, doc, *node);
				doc.append_node(node);
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}

	std::ofstream file("parts_index.xml");

	file << doc;

	file.close();

	system("pause");
}