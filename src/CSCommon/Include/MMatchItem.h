#pragma once

#include "MXml.h"
#include "MUID.h"
#include "MSync.h"
#include "MBaseItem.h"
#include <map>
#include <list>
#include <vector>
#include <algorithm>

class MZFileSystem;

enum MMatchItemType
{
	MMIT_MELEE = 0,
	MMIT_RANGE,
	MMIT_EQUIPMENT,
	MMIT_CUSTOM,
	MMIT_END
};

enum MMatchItemSlotType
{
	MMIST_NONE = 0,
	MMIST_MELEE,
	MMIST_RANGE,
	MMIST_CUSTOM,
	MMIST_HEAD,
	MMIST_CHEST,
	MMIST_HANDS,
	MMIST_LEGS,
	MMIST_FEET,
	MMIST_FINGER,
	MMIST_EXTRA,
	MMIST_END
};

enum MMatchCharItemParts
{
	MMCIP_HEAD		= 0,
	MMCIP_CHEST		= 1,
	MMCIP_HANDS		= 2,
	MMCIP_LEGS		= 3,
	MMCIP_FEET		= 4,
	MMCIP_FINGERL	= 5,
	MMCIP_FINGERR	= 6,
	MMCIP_MELEE		= 7,
	MMCIP_PRIMARY	= 8,
	MMCIP_SECONDARY	= 9,
	MMCIP_CUSTOM1	= 10,
	MMCIP_CUSTOM2	= 11,
	MMCIP_END
};

enum MMatchCustomItemType
{
	MMCIT_MED_KIT		= 0,
	MMCIT_REPAIR_KIT,
	MMCIT_BULLET_KIT,
	MMCIT_DYNAMITE,
	MMCIT_REMOTE_CHARGE,
	MMCIT_DEMOLITION_TOOLS,
	MMCIT_FLASH_BANG,
	MMCIT_FRAGMENTATION,
	MMCIT_SMOKE_GRENADE,
	MMCIT_TEARGAS_GRENADE,
	MMCIT_FOOD,

	MMCIT_ENCHANT_FIRE,
	MMCIT_ENCHANT_COLD,
	MMCIT_ENCHANT_LIGHTNING,
	MMCIT_ENCHANT_POISON,

	MMCIT_END
};

enum MMatchMeleeItemType
{
	MIT_DAGGER			= 0,
	MIT_DUAL_DAGGER		= 1,
	MIT_KATANA			= 2,
	MIT_GREAT_SWORD		= 3,
	MIT_DOUBLE_KATANA	= 4,

	MIT_END
};

enum MMatchRangeItemType
{
	RIT_PISTOL			= 0,
	RIT_PISTOLx2		= 1,
	RIT_REVOLVER		= 2,
	RIT_REVOLVERx2		= 3,
	RIT_SMG				= 4,
	RIT_SMGx2			= 5,
	RIT_SHOTGUN			= 6,
	RIT_SAWED_SHOTGUN	= 7,
	RIT_RIFLE			= 8,
	RIT_MACHINEGUN		= 9,
	RIT_ROCKET			= 10,
	RIT_SNIFER			= 11,

	RIT_END
};

enum MMatchWeaponType
{
	MWT_NONE	= 0,
	// melee
	MWT_DAGGER,
	MWT_DUAL_DAGGER,
	MWT_KATANA,
	MWT_GREAT_SWORD,
	MWT_DOUBLE_KATANA,

	// range
	MWT_PISTOL,
	MWT_PISTOLx2,
	MWT_REVOLVER,
	MWT_REVOLVERx2,
	MWT_SMG,
	MWT_SMGx2,
	MWT_SHOTGUN,
	MWT_SAWED_SHOTGUN,
	MWT_RIFLE,
	MWT_MACHINEGUN,
	MWT_ROCKET,
	MWT_SNIFER,

	// custom
	MWT_MED_KIT,
	MWT_REPAIR_KIT,
	MWT_BULLET_KIT,
	MWT_FLASH_BANG,
	MWT_FRAGMENTATION,
	MWT_SMOKE_GRENADE,
	MWT_FOOD,
	MWT_SKILL,				// NPC용 - skill.xml에 기술되어 있는 걸로 공격한다.

	// custom - enchant
	MWT_ENCHANT_FIRE,			
	MWT_ENCHANT_COLD,
	MWT_ENCHANT_LIGHTNING,
	MWT_ENCHANT_POISON,

	MWT_END
};

struct MMatchItemEffectDesc
{
	u32	m_nID;
	char				m_szName[128];
	int					m_nArea;
	u32	m_nTime;
	int					m_nModHP;
	int					m_nModAP;
	int					m_nModMaxWT;
	int					m_nModSF;
	int					m_nModFR;
	int					m_nModCR;
	int					m_nModPR;
	int					m_nModLR;
	int					m_nResAP;
	int					m_nResFR;
	int					m_nResCR;
	int					m_nResPR;
	int					m_nResLR;
	int					m_nStun;
	int					m_nKnockBack;
	int					m_nSmoke;
	int					m_nFlash;
	int					m_nTear;
	int					m_nFlame;
};

enum MMatchItemBonusType
{
	MIBT_SOLO = 0,
	MIBT_TEAM = 1,
	MIBT_QUEST = 2
};

struct MMatchItemBonus
{
	float				m_fXP_SoloBonus;
	float				m_fXP_TeamBonus;
	float				m_fXP_QuestBonus;

	float				m_fBP_SoloBonus;
	float				m_fBP_TeamBonus;
	float				m_fBP_QuestBonus;
};

struct MMatchItemDesc
{
	u32	m_nID;
	char				m_szName[128];
	int					m_nTotalPoint;
	MMatchItemType		m_nType;
	int					m_nResSex;
	int					m_nResLevel;
	MMatchItemSlotType	m_nSlot;
	MMatchWeaponType	m_nWeaponType;
	int					m_nWeight;
	int					m_nBountyPrice;
	bool				m_bIsCashItem;
	int					m_nDamage;
	int					m_nDelay;
	MMatchItemEffectDesc* m_pEffect;
	int					m_nControllability;
	int					m_nMagazine;
	int					m_nMaxBullet;
	int					m_nReloadTime;
	bool				m_bSlugOutput;
	int					m_nGadgetID;
	int					m_nHP;
	int					m_nAP;
	int					m_nMaxWT;
	int					m_nSF;
	int					m_nFR;
	int					m_nCR;
	int					m_nPR;
	int					m_nLR;
	int					m_nLimitSpeed;
	int					m_nLimitJump;
	int					m_nLimitTumble;
	int					m_nLimitWall;
	int					m_nRange;
	int					m_nEffectLevel;
	char				m_szDesc[8192];

	u32	m_nColor;
	char				m_szMeshName[128];
	int					m_nImageID;
	int					m_nBulletImageID;
	int					m_nMagazineImageID;
	char				m_szReloadSndName[256];
	char				m_szFireSndName[256];
	char				m_szDryfireSndName[256];

	MMatchItemBonus		m_Bonus;

	bool				m_bDuplicate;

	MMatchItemDesc();
	int GetBountyValue() { return (m_nBountyPrice / 4); }
	bool IsCashItem()	{ if (m_bIsCashItem) return true; return false; }
	bool IsEnchantItem() { if (m_nWeaponType>=MWT_ENCHANT_FIRE && m_nWeaponType<=MWT_ENCHANT_POISON) return true; return false; }
};

bool IsSuitableItemSlot(MMatchItemSlotType nSlotType, MMatchCharItemParts nParts);
MMatchCharItemParts GetSuitableItemParts(MMatchItemSlotType nSlotType);
MMatchItemSlotType	GetSuitableItemSlot(MMatchCharItemParts nParts);
bool IsWeaponItemSlotType(MMatchItemSlotType nSlotType);
bool IsWeaponCharItemParts(MMatchCharItemParts nParts);

char* GetItemSlotTypeStr(MMatchItemSlotType nSlotType);
char* GetCharItemPartsStr(MMatchCharItemParts nParts);

MMatchWeaponType GetWeaponType(MMatchMeleeItemType nMeleeItemType);
MMatchWeaponType GetWeaponType(MMatchRangeItemType nRangeItemType);
MMatchWeaponType GetWeaponType(MMatchCustomItemType nCustomItemType);

bool IsEnchantItem(MMatchItemDesc* pItemDesc);

class MMatchItemEffectDescMgr : public std::map<int, MMatchItemEffectDesc*>
{
protected:
	void ParseEffect(MXmlElement& element);
public:
	MMatchItemEffectDescMgr();
	virtual ~MMatchItemEffectDescMgr();
	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);
	void Clear();
	MMatchItemEffectDesc* GetEffectDesc(int nID);
	static MMatchItemEffectDescMgr* GetInstance();
};

inline MMatchItemEffectDescMgr* MGetMatchItemEffectDescMgr() {
	return MMatchItemEffectDescMgr::GetInstance(); }

class MMatchItemDescMgr : public std::map<int, MMatchItemDesc*>
{
	u32 m_nChecksum;
protected:
	void ParseItem(MXmlElement& element);
public:
	MMatchItemDescMgr();
	virtual ~MMatchItemDescMgr();
	bool ReadXml(const char* szFileName);
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);
	void Clear();
	MMatchItemDesc* GetItemDesc(u32 nID);
	static MMatchItemDescMgr* GetInstance();

	u32 GetChecksum() { return m_nChecksum; }

	static MMatchItemDescMgr DefaultInstance;
};
inline MMatchItemDescMgr* MGetMatchItemDescMgr() { return MMatchItemDescMgr::GetInstance(); }
void MSetMatchItemDescMgr(MMatchItemDescMgr*);



class MMatchItem : public MBaseItem
{
private:
protected:
	MUID				m_uidItem;			
	u32	m_nCIID;
	MMatchItemDesc*		m_pDesc;
	bool				m_bEquiped;
	u64					m_nRentItemRegTime;
protected:
	void SetDesc(MMatchItemDesc* pDesc) { m_pDesc = pDesc; }
public:
	MMatchItem();
	virtual ~MMatchItem() = default;
	bool Create(const MUID& uid, MMatchItemDesc* pDesc, int nCount = 1);
	void Destroy();

	MUID GetUID() { return m_uidItem; }


	bool IsEmpty() { return (((m_pDesc == NULL) || (m_nCount <= 0)) ? true : false); }
	void SetCIID(u32 nCIID) { m_nCIID = nCIID; }
	u32 GetCIID() { return m_nCIID; }
	auto* GetDesc() { return m_pDesc; }
	auto* GetDesc() const { return m_pDesc; }
	u32 GetDescID() { if (m_pDesc) return m_pDesc->m_nID; else return 0; }
	void SetCount(int n) { m_nCount = n; }
	int GetCount() { return m_nCount; }
	void Inc(int n=1) { m_nCount += n; }
	void Dec(int n=1) { m_nCount -= n; if (m_nCount < 0) m_nCount = 0; }
	void SetEquiped(bool bIsEquiped) { m_bEquiped = bIsEquiped; }
	bool IsEquiped() { return m_bEquiped; }
	auto GetRentItemRegTime() const { return m_nRentItemRegTime; }
	void SetRentItemRegTime(u64 nTime){ m_nRentItemRegTime = nTime; }

	MMatchItemType GetItemType();
};

class MMatchItemMap;

/// 장비하고있는 아이템
class MMatchEquipedItem
{
protected:
	MMatchItem*		m_pParts[MMCIP_END];
public:
	MMatchEquipedItem() { memset(m_pParts, 0, sizeof(m_pParts)); }
	virtual ~MMatchEquipedItem() { }
public:
	bool SetItem(MMatchCharItemParts parts, MMatchItem* pMatchItem);
	MMatchItem* GetItem(MMatchCharItemParts parts) { return m_pParts[parts]; }
	void Remove(MMatchCharItemParts parts);
	bool IsEmpty(MMatchCharItemParts parts) { if (m_pParts[parts] != NULL) return false; return true; }
	void GetTotalWeight(int* poutWeight, int* poutMaxWeight);
	bool IsEquipedItem(MMatchItem* pCheckItem, MMatchCharItemParts& outParts); // 해당 아이템이 장비중인지 체크
	void Clear();
	auto begin() const { return std::begin(m_pParts); }
	auto end() const { return std::end(m_pParts); }
};

class MMatchItemMap : public std::map<MUID, MMatchItem*>
{
private:
protected:
	static MUID				m_uidGenerate;
	static MCriticalSection	m_csUIDGenerateLock;
	bool					m_bDoneDbAccess;

	bool					m_bHasRentItem;			
public:
	MMatchItemMap();
	virtual ~MMatchItemMap();
	bool IsEmpty() { return empty(); }
	int GetCount() { return (int)size(); }
	virtual bool CreateItem(MUID& uid, int nCIID, int nItemDescID, 
		bool bRentItem = false,
		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED,
		int nCount = 1);
	bool RemoveItem(MUID& uidItem);
	virtual void Clear();
	MMatchItem* GetItem(MUID& uidItem);
	bool IsDoneDbAccess() { return m_bDoneDbAccess; }
	void SetDbAccess() { m_bDoneDbAccess = true; }
	bool HasRentItem() { return m_bHasRentItem; }
public:
	static MUID UseUID() {
		m_csUIDGenerateLock.lock();
		m_uidGenerate.Increase();	
		m_csUIDGenerateLock.unlock();
		return m_uidGenerate;
	}
};

struct MAccountItemNode
{
	int					nAIID;
	u32	nItemID;
	int					nRentMinutePeriodRemainder;
};


// effect
#define MECTOK_EFFECT		"EFFECT"
#define MECTOK_ID			"id"
#define MECTOK_NAME			"name"
#define MECTOK_AREA			"area"
#define MECTOK_TIME			"time"
#define MECTOK_MOD_HP		"mod_hp"
#define MECTOK_MOD_AP		"mod_ap"
#define MECTOK_MOD_MAXWT	"mod_maxwt"
#define MECTOK_MOD_SF		"mod_sf"
#define MECTOK_MOD_FR		"mod_fr"
#define MECTOK_MOD_CR		"mod_cr"
#define MECTOK_MOD_PR		"mod_pr"
#define MECTOK_MOD_LR		"mod_lr"
#define MECTOK_RES_AP		"res_ap"
#define MECTOK_RES_FR		"res_fr"
#define MECTOK_RES_CR		"res_cr"
#define MECTOK_RES_PR		"res_pr"
#define MECTOK_RES_LR		"res_lr"
#define MECTOK_STUN			"stun"
#define MECTOK_KNOCKBACK	"knockback"
#define MECTOK_SMOKE		"smoke"
#define MECTOK_FLASH		"flash"
#define MECTOK_TEAR			"tear"
#define MECTOK_FLAME		"flame"


// item
#define MICTOK_ITEM			"ITEM"
#define MICTOK_ID			"id"
#define MICTOK_NAME			"name"
#define MICTOK_TOTALPOINT	"totalpoint"
#define MICTOK_TYPE			"type"
#define MICTOK_RES_SEX		"res_sex"
#define MICTOK_RES_LEVEL	"res_level"
#define MICTOK_SLOT			"slot"
#define MICTOK_WEAPON		"weapon"
#define MICTOK_EFFECT_LEVEL	"effect_level"
#define MICTOK_WEIGHT		"weight"
#define MICTOK_BOUNTY_PRICE	"bt_price"
#define MICTOK_ISCASHITEM	"iscashitem"
#define MICTOK_DAMAGE		"damage"
#define MICTOK_DELAY		"delay"
#define MICTOK_EFFECT_ID	"effect_id"
#define MICTOK_CONTROLLABILITY	"ctrl_ability"
#define MICTOK_MAGAZINE		"magazine"
#define MICTOK_MAXBULLET	"maxbullet"
#define MICTOK_RELOADTIME	"reloadtime"
#define MICTOK_SLUGOUTPUT	"slut_output"
#define MICTOK_GADGET_ID	"gadget_id"
#define MICTOK_HP			"hp"
#define MICTOK_AP			"ap"
#define MICTOK_MAXWT		"maxwt"
#define MICTOK_SF			"sf"
#define MICTOK_FR			"fr"
#define MICTOK_CR			"cr"
#define MICTOK_PR			"pr"
#define MICTOK_LR			"lr"
#define MICTOK_LIMITSPEED	"limitspeed"
#define MICTOK_LIMITJUMP	"limitjump"
#define MICTOK_LIMITTUMBLE	"limittumble"
#define MICTOK_LIMITWALL	"limitwall"
#define MICTOK_RANGE		"range"

#define MICTOK_COLOR		"color"
#define MICTOK_DESC			"desc"

#define MICTOK_MESH_NAME			"mesh_name"
#define MICTOK_IMAGE_ID				"image_id"
#define MICTOK_BULLET_IMAGE_ID		"bullet_image_id"
#define MICTOK_MAGAZINE_IMAGE_ID	"magazine_image_id"

#define MICTOK_SOUND_RELOAD			"snd_reload"
#define MICTOK_SOUND_FIRE			"snd_fire"
#define MICTOK_SOUND_DRYFIRE		"snd_dryfire"

#define MICTOK_BONUS_XP_SOLO		"xp_solo_bonus"
#define MICTOK_BONUS_XP_TEAM		"xp_team_bonus"
#define MICTOK_BONUS_XP_QUEST		"xp_quest_bonus"

#define MICTOK_BONUS_BP_SOLO		"bp_solo_bonus"
#define MICTOK_BONUS_BP_TEAM		"bp_team_bonus"
#define MICTOK_BONUS_BP_QUEST		"bp_quest_bonus"

#define MICTOK_BONUS_DUPLICATE		"duplicate"

/*
== XML 기술 설명 ==
+ zitem.xml 설명

+ 태그이름 : ITEM
+ attribute 설명
id : 아이템 아이디(int)
name : 아이템 이름(string[128])
mesh_name : 메쉬 이름(str[256]) , weapon.xml의 name항목의 이름
totalpoint : 종합 점수(int)
type : 종류(melee, range, equip, custom)
res_sex : 성별 제한사항(m, f, a)
res_level : 레벨 제한사항(int) , 제한이 없을경우 0으로 세팅하도록 한다.
slot : 슬롯(none, melee, range, head, chest, hands, legs, feet, finger, custom)
weapon : 무기타입(none, dagger, katana, pistol, smg, shotgun, rifle, machinegun, 
                  rocket, snifer, medkit, flashbang, frag, smoke, 
				  enchant_fire, enchant_cold, enchant_lightning, enchant_poison)
weight : 무게(int)
bt_price : 판매가(int)
iscashitem : 캐쉬아이템인지 여부(true, false)
damage : 데미지(int)
delay : 딜레이(int)
effect_id : 이펙트 아이디(int) , effect.xml의 항목과 연동되는 ID
ctrl_ability : 제어성(int)
magazine : 장탄수(int)
reloadtime : 장전시간(int)
slug_output : 탄피적출(true, false)
gadget_id : 옵션 아이디(int)
hp : hp 캐릭터 수정치(int)
ap : ap 캐릭터 수정치(int)
maxwt : max wt 캐릭터 수정치(int)
sf : sf 캐릭터 수정치(int)
fr : fr 캐릭터 수정치(int)
cr : cr 캐릭터 수정치(int)
pr : pr 캐릭터 수정치(int)
lr : lr 캐릭터 수정치(int)
desc : 설명(string[65535])
color : 색깔(color)
image_id : ui에서 보이는 image 아이디(int) , 지금은 쓰이지 않음
bullet_image_id : ui에서 보이는 bullet 이미지 아이디(int) , 지금은 쓰이지 않음
magazine_image_id : ui에서 보이는 magazine 이미지 아이디(int) , 지금은 쓰이지 않음
snd_reload : reload때 나오는 사운드 이름(str[128])
snd_fire : fire때 나오는 사운드 이름(str[128])
snd_dryfire : dryfire 때 나오는 사운드 이름(str[128])
range : melee 공격의 범위(int)


+ effect.xml 설명

- 태그이름 : EFFECT
+ attribute 설명
id : 이펙트 아이디(int)
name : 이펙트 이름(string[128])
area : 이펙트 영역(int)
time : 이펙트 시간(msec)
mod_hp : hp 수정치 
mod_ap : ap 수정치
mod_maxwt : max wt 수정치
mod_sf : sf 수정치
mod_fr : fr 수정치
mod_cr : cr 수정치
mod_pr : pr 수정치
mod_lr : lr 수정치
res_ap : ap 저항판정
res_fr : fr 저항판정
res_cr : cr 저항판정
res_pr : pr 저항판정
res_lr : lr 저항판정
stun : stun 수정치
knockback : knockback 수정치
smoke : smoke 수정치
flash : flash 수정치
tear : tear 수정치
flame : frame 수정치

duplicate : 중복 착용 여부.(true, false)
 */