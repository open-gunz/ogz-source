#ifndef _RMeshNodeStringTable_h
#define _RMeshNodeStringTable_h

#include <string>
#include <unordered_map>

_NAMESPACE_REALSPACE2_BEGIN

enum RMeshNodeNameType
{
	RMeshNodeNameType_None = 0,

	RMeshNodeNameType_BipRoot,
	RMeshNodeNameType_BipHead,
	RMeshNodeNameType_BipHeadNub,

	RMeshNodeNameType_BipLCalf,
	RMeshNodeNameType_BipLClavicle,
	RMeshNodeNameType_BipLFinger0,
	RMeshNodeNameType_BipLFingerNub,
	RMeshNodeNameType_BipLFoot ,
	RMeshNodeNameType_BipLForeArm ,
	RMeshNodeNameType_BipLHand,
	RMeshNodeNameType_BipLThigh,
	RMeshNodeNameType_BipLToe0 ,
	RMeshNodeNameType_BipLToe0Nub ,
	RMeshNodeNameType_BipLUpperArm ,

	RMeshNodeNameType_BipNeck ,
	RMeshNodeNameType_BipPelvis ,

	RMeshNodeNameType_BipRCalf,
	RMeshNodeNameType_BipRClavicle,
	RMeshNodeNameType_BipRFinger0,
	RMeshNodeNameType_BipRFingerNub,
	RMeshNodeNameType_BipRFoot ,
	RMeshNodeNameType_BipRForeArm ,
	RMeshNodeNameType_BipRHand,
	RMeshNodeNameType_BipRThigh,
	RMeshNodeNameType_BipRToe0 ,
	RMeshNodeNameType_BipRToe0Nub ,
	RMeshNodeNameType_BipRUpperArm ,

	RMeshNodeNameType_Spine ,
	RMeshNodeNameType_Spine1 ,
	RMeshNodeNameType_Spine2 ,

	RMeshNodeNameType_Footsteps,

	RMeshNodeNameType_Ponytail1,
	RMeshNodeNameType_Ponytail11,
	RMeshNodeNameType_Ponytail12,
	RMeshNodeNameType_Ponytail1Nub,

	RMeshNodeNameType_eq_wd_katana,
	RMeshNodeNameType_eq_ws_pistol,
	RMeshNodeNameType_eq_wd_pistol,
	RMeshNodeNameType_eq_wd_shotgun,
	RMeshNodeNameType_eq_wd_rifle,
	RMeshNodeNameType_eq_wd_grenade,
	RMeshNodeNameType_eq_ws_dagger,
	RMeshNodeNameType_eq_wd_item,
	RMeshNodeNameType_eq_wd_rlauncher,
	RMeshNodeNameType_eq_ws_smg,
	RMeshNodeNameType_eq_wd_smg,
	RMeshNodeNameType_eq_wd_sword,
	RMeshNodeNameType_eq_wd_blade,
	RMeshNodeNameType_eq_wd_dagger,

	RMeshNodeNameType_End
};

typedef std::unordered_map<std::string, int> rmesh_node_table;

class RMeshNodeStringTable
{
public:
	RMeshNodeStringTable();
	~RMeshNodeStringTable();

	static RMeshNodeStringTable* GetInstance();

	void Destroy();

	int Add(const std::string& str, int);
	int Get(const std::string& str);
	int Get(char* str) { return Get(str); }

	rmesh_node_table m_table;
};

RMeshNodeStringTable* RGetMeshNodeStringTable();

_NAMESPACE_REALSPACE2_END

#endif