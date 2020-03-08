#include "stdafx.h"
#include "RMeshNodeStringTable.h"

_NAMESPACE_REALSPACE2_BEGIN

RMeshNodeStringTable::RMeshNodeStringTable()
{
	Add("Bip01", RMeshNodeNameType_BipRoot);
	Add("Bip01 Head", RMeshNodeNameType_BipHead);
	Add("Bip01 HeadNub", RMeshNodeNameType_BipHeadNub);

	Add("Bip01 L Calf", RMeshNodeNameType_BipLCalf);
	Add("Bip01 L Clavicle", RMeshNodeNameType_BipLClavicle);
	Add("Bip01 L Finger0", RMeshNodeNameType_BipLFinger0);
	Add("Bip01 L Finger0Nub", RMeshNodeNameType_BipLFingerNub);
	Add("Bip01 L Foot", RMeshNodeNameType_BipLFoot);
	Add("Bip01 L ForeArm", RMeshNodeNameType_BipLForeArm);
	Add("Bip01 L Hand", RMeshNodeNameType_BipLHand);
	Add("Bip01 L Thigh", RMeshNodeNameType_BipLThigh);
	Add("Bip01 L Toe0", RMeshNodeNameType_BipLToe0);
	Add("Bip01 L Toe0Nub", RMeshNodeNameType_BipLToe0Nub);
	Add("Bip01 L UpperArm", RMeshNodeNameType_BipLUpperArm);

	Add("Bip01 Neck", RMeshNodeNameType_BipNeck);
	Add("Bip01 Pelvis", RMeshNodeNameType_BipPelvis);

	Add("Bip01 R Calf", RMeshNodeNameType_BipRCalf);
	Add("Bip01 R Clavicle", RMeshNodeNameType_BipRClavicle);
	Add("Bip01 R Finger0", RMeshNodeNameType_BipRFinger0);
	Add("Bip01 R Finger0Nub", RMeshNodeNameType_BipRFingerNub);
	Add("Bip01 R Foot", RMeshNodeNameType_BipRFoot);
	Add("Bip01 R ForeArm", RMeshNodeNameType_BipRForeArm);
	Add("Bip01 R Hand", RMeshNodeNameType_BipRHand);
	Add("Bip01 R Thigh", RMeshNodeNameType_BipRThigh);
	Add("Bip01 R Toe0", RMeshNodeNameType_BipRToe0);
	Add("Bip01 R Toe0Nub", RMeshNodeNameType_BipRToe0Nub);
	Add("Bip01 R UpperArm", RMeshNodeNameType_BipRUpperArm);

	Add("Bip01 Spine", RMeshNodeNameType_Spine);
	Add("Bip01 Spine1", RMeshNodeNameType_Spine1);
	Add("Bip01 Spine2", RMeshNodeNameType_Spine2);

	Add("Bip01 Footsteps", RMeshNodeNameType_Footsteps);

	Add("Bip01 Ponytail1", RMeshNodeNameType_Ponytail1);
	Add("Bip01 Ponytail11", RMeshNodeNameType_Ponytail11);
	Add("Bip01 Ponytail12", RMeshNodeNameType_Ponytail12);
	Add("Bip01 Ponytail1Nub", RMeshNodeNameType_Ponytail1Nub);
	/*
		Add("eq_wd_katana"   ,RMeshNodeNameType_eq_wd_katana );
		Add("eq_ws_pistol"   ,RMeshNodeNameType_eq_ws_pistol );
		Add("eq_wd_pistol"   ,RMeshNodeNameType_eq_wd_pistol );
		Add("eq_wd_shotgun"  ,RMeshNodeNameType_eq_wd_shotgun );
		Add("eq_wd_rifle"    ,RMeshNodeNameType_eq_wd_rifle );
		Add("eq_wd_grenade"  ,RMeshNodeNameType_eq_wd_grenade );
		Add("eq_ws_dagger"   ,RMeshNodeNameType_eq_ws_dagger );
		Add("eq_wd_item"     ,RMeshNodeNameType_eq_wd_item );
		Add("eq_wd_rlauncher",RMeshNodeNameType_eq_wd_rlauncher );
		Add("eq_ws_smg"      ,RMeshNodeNameType_eq_ws_smg );
		Add("eq_wd_smg"      ,RMeshNodeNameType_eq_wd_smg );
		Add("eq_wd_sword"    ,RMeshNodeNameType_eq_wd_sword );
		Add("eq_wd_blade"    ,RMeshNodeNameType_eq_wd_blade );
		Add("eq_wd_dagger"   ,RMeshNodeNameType_eq_wd_dagger );
	*/
}

RMeshNodeStringTable::~RMeshNodeStringTable()
{

}

int RMeshNodeStringTable::Add(const std::string& str,int id)
{
	m_table.insert(rmesh_node_table::value_type(str, id));
	return 1;
}

int RMeshNodeStringTable::Get(const std::string& str)
{
	rmesh_node_table::iterator itor = m_table.find(str);
	if (itor != m_table.end()) {
		return (*itor).second;
	}
	return -1;
}

RMeshNodeStringTable* RMeshNodeStringTable::GetInstance()
{
	static RMeshNodeStringTable m_RMNStringTable;
	return &m_RMNStringTable;
}

RMeshNodeStringTable* RGetMeshNodeStringTable()
{
	return RMeshNodeStringTable::GetInstance();
}

_NAMESPACE_REALSPACE2_END