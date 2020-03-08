#include "stdafx.h"
#include "MMatchItem.h"
#include "MZFileSystem.h"
#include "MBaseStringResManager.h"
#include "MMatchUtil.h"
#include "MTime.h"
#include "MDebug.h"

#define DEFAULT_MELEE_WEAPON_RANGE 160

MUID MMatchItemMap::m_uidGenerate = MUID(0,0);
MCriticalSection MMatchItemMap::m_csUIDGenerateLock;

MMatchItemDesc::MMatchItemDesc() : m_nID(0), m_nTotalPoint(0), m_nType(MMIT_MELEE), m_nResSex(0),
m_nResLevel(0), m_nSlot(MMIST_NONE), m_nWeaponType(MWT_NONE),
m_nWeight(0), m_nBountyPrice(0),
m_nDamage(0), m_nDelay(0), m_pEffect(NULL), m_nControllability(0), m_nMagazine(0),
m_nReloadTime(0), m_bSlugOutput(0), m_nGadgetID(0), m_nHP(0), m_nAP(0), m_nMaxWT(0), m_nSF(0),
m_nFR(0), m_nCR(0), m_nPR(0), m_nLR(0), m_nColor(0xFFFFFFFF), m_nImageID(0), m_nBulletImageID(0),
m_nMagazineImageID(0), m_nMaxBullet(0), m_nLimitSpeed(100), m_nLimitJump(0), m_nLimitTumble(0), m_nLimitWall(0),
m_nEffectLevel(0), m_bIsCashItem(false), m_bDuplicate(true),
m_szName(), m_szDesc(), m_szMeshName(), m_szReloadSndName(), m_szFireSndName(), m_szDryfireSndName(),
m_Bonus()
{}

///////////////////////////////////////////////////////////////////////////////
// MMatchItemEffectDescMgr ////////////////////////////////////////////////////
MMatchItemEffectDescMgr::MMatchItemEffectDescMgr()
{

}
MMatchItemEffectDescMgr::~MMatchItemEffectDescMgr()
{
	Clear();
}
void MMatchItemEffectDescMgr::ParseEffect(MXmlElement& element)
{
	MMatchItemEffectDesc* pNewEffectDesc = new MMatchItemEffectDesc;
	memset(pNewEffectDesc, 0, sizeof(MMatchItemEffectDesc));

	int n = 0;
	element.GetAttribute(&n, MECTOK_ID);	pNewEffectDesc->m_nID = n;
	element.GetAttribute(pNewEffectDesc->m_szName, MECTOK_NAME);

	element.GetAttribute(&pNewEffectDesc->m_nArea, MECTOK_AREA);
	element.GetAttribute(&n, MECTOK_TIME);  pNewEffectDesc->m_nTime = n;
	element.GetAttribute(&pNewEffectDesc->m_nModHP, MECTOK_MOD_HP);
	element.GetAttribute(&pNewEffectDesc->m_nModAP, MECTOK_MOD_AP);
	element.GetAttribute(&pNewEffectDesc->m_nModMaxWT, MECTOK_MOD_MAXWT);
	element.GetAttribute(&pNewEffectDesc->m_nModSF, MECTOK_MOD_SF);
	element.GetAttribute(&pNewEffectDesc->m_nModFR, MECTOK_MOD_FR);
	element.GetAttribute(&pNewEffectDesc->m_nModCR, MECTOK_MOD_CR);
	element.GetAttribute(&pNewEffectDesc->m_nModPR, MECTOK_MOD_PR);
	element.GetAttribute(&pNewEffectDesc->m_nModLR, MECTOK_MOD_LR);
	element.GetAttribute(&pNewEffectDesc->m_nResAP, MECTOK_RES_AP);
	element.GetAttribute(&pNewEffectDesc->m_nResFR, MECTOK_RES_FR);
	element.GetAttribute(&pNewEffectDesc->m_nResCR, MECTOK_RES_CR);
	element.GetAttribute(&pNewEffectDesc->m_nResPR, MECTOK_RES_PR);
	element.GetAttribute(&pNewEffectDesc->m_nResLR, MECTOK_RES_LR);
	element.GetAttribute(&pNewEffectDesc->m_nStun, MECTOK_STUN);
	element.GetAttribute(&pNewEffectDesc->m_nKnockBack, MECTOK_KNOCKBACK);
	element.GetAttribute(&pNewEffectDesc->m_nSmoke, MECTOK_SMOKE);
	element.GetAttribute(&pNewEffectDesc->m_nFlash, MECTOK_FLASH);
	element.GetAttribute(&pNewEffectDesc->m_nTear, MECTOK_TEAR);
	element.GetAttribute(&pNewEffectDesc->m_nFlame, MECTOK_FLAME);


	insert(value_type(pNewEffectDesc->m_nID, pNewEffectDesc));

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pNewEffectDesc->m_nID);
	if (pItemDesc)
		pItemDesc->m_pEffect = pNewEffectDesc;
}
bool MMatchItemEffectDescMgr::ReadXml(const char* szFileName)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MECTOK_EFFECT))
		{
			ParseEffect(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;

}

bool MMatchItemEffectDescMgr::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	if (!xmlIniData.LoadFromFile(szFileName, pFileSystem))
	{
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MECTOK_EFFECT))
		{
			ParseEffect(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

void MMatchItemEffectDescMgr::Clear()
{
	while(!empty())
	{
		MMatchItemEffectDesc* pEffectDesc = (*begin()).second;
		delete pEffectDesc; pEffectDesc = NULL;
		erase(begin());
	}
}
MMatchItemEffectDesc* MMatchItemEffectDescMgr::GetEffectDesc(int nID)
{
	iterator itor = find(nID);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;
}

MMatchItemEffectDescMgr* MMatchItemEffectDescMgr::GetInstance()
{
	static MMatchItemEffectDescMgr m_ItemEffectDescMgr;
	return &m_ItemEffectDescMgr;
}
///////////////////////////////////////////////////////////////////////////////
// MMatchItemDescMgr //////////////////////////////////////////////////////////
MMatchItemDescMgr::MMatchItemDescMgr() : m_nChecksum(0)
{

}
MMatchItemDescMgr::~MMatchItemDescMgr()
{
	Clear();
}
bool MMatchItemDescMgr::ReadXml(const char* szFileName)
{
	m_nChecksum = MGetMZFileChecksum(szFileName);

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		m_nChecksum = 0;
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_ITEM))
		{
			ParseItem(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchItemDescMgr::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	m_nChecksum = 0;

	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem))
	{
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_ITEM))
		{
			ParseItem(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}
void MMatchItemDescMgr::Clear()
{
	while(!empty())
	{
		MMatchItemDesc* pItemDesc = (*begin()).second;
		delete pItemDesc; pItemDesc = NULL;
		erase(begin());
	}
}
MMatchItemDesc* MMatchItemDescMgr::GetItemDesc(u32 nID)
{
	iterator itor = find(nID);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;
}


void MMatchItemDescMgr::ParseItem(MXmlElement& element)
{
	MMatchItemDesc* pNewDesc = new MMatchItemDesc;

	// default 값 입력
	pNewDesc->m_bIsCashItem = false;
	pNewDesc->m_nLimitSpeed = 100;
	pNewDesc->m_nRange = DEFAULT_MELEE_WEAPON_RANGE;
	pNewDesc->m_nColor = 0xFFFFFFFF;

	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];

	int nAttrCount = element.GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		element.GetAttribute(i, szAttrName, szAttrValue);
		if (!_stricmp(szAttrName, MICTOK_ID))
		{
			pNewDesc->m_nID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_NAME))
		{
			strcpy_safe(pNewDesc->m_szName, MGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MICTOK_TYPE))
		{
			if (!_stricmp(szAttrValue, "melee")) pNewDesc->m_nType = MMIT_MELEE;
			else if (!_stricmp(szAttrValue, "range")) pNewDesc->m_nType = MMIT_RANGE;
			else if (!_stricmp(szAttrValue, "equip")) pNewDesc->m_nType = MMIT_EQUIPMENT;
			else if (!_stricmp(szAttrValue, "custom")) pNewDesc->m_nType = MMIT_CUSTOM;
			else if (!_stricmp(szAttrValue, "avatar")) return;
			else _ASSERT(0);
		}
		else if (!_stricmp(szAttrName, MICTOK_RES_SEX))
		{
			if (!_stricmp(szAttrValue, "m")) pNewDesc->m_nResSex = 0;
			else if (!_stricmp(szAttrValue, "f")) pNewDesc->m_nResSex = 1;
			else if (!_stricmp(szAttrValue, "a")) pNewDesc->m_nResSex = -1;
			else _ASSERT(0);
		}
		else if (!_stricmp(szAttrName, MICTOK_RES_LEVEL))
		{
			pNewDesc->m_nResLevel = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SLOT))
		{
			// 슬롯 타입
			if (!_stricmp(szAttrValue, "none"))			pNewDesc->m_nSlot = MMIST_NONE;
			else if (!_stricmp(szAttrValue, "melee"))	pNewDesc->m_nSlot = MMIST_MELEE;
			else if (!_stricmp(szAttrValue, "range"))	pNewDesc->m_nSlot = MMIST_RANGE;
			else if (!_stricmp(szAttrValue, "custom"))	pNewDesc->m_nSlot = MMIST_CUSTOM;
			else if (!_stricmp(szAttrValue, "head"))		pNewDesc->m_nSlot = MMIST_HEAD;
			else if (!_stricmp(szAttrValue, "chest"))	pNewDesc->m_nSlot = MMIST_CHEST;
			else if (!_stricmp(szAttrValue, "hands"))	pNewDesc->m_nSlot = MMIST_HANDS;
			else if (!_stricmp(szAttrValue, "legs"))		pNewDesc->m_nSlot = MMIST_LEGS;
			else if (!_stricmp(szAttrValue, "feet"))		pNewDesc->m_nSlot = MMIST_FEET;
			else if (!_stricmp(szAttrValue, "finger"))	pNewDesc->m_nSlot = MMIST_FINGER;
			else if (!_stricmp(szAttrValue, "extra"))	pNewDesc->m_nSlot = MMIST_EXTRA;
			else if (!_stricmp(szAttrValue, "dash"))	return;
			else _ASSERT(0);
		}
		else if (!_stricmp(szAttrName, MICTOK_WEAPON))
		{
			// 무기 타입
			if (strlen(szAttrValue) <= 0)					pNewDesc->m_nWeaponType = MWT_NONE;
			else if (!_stricmp(szAttrValue, "dagger"))		pNewDesc->m_nWeaponType = MWT_DAGGER;
			else if (!_stricmp(szAttrValue, "dualdagger"))	pNewDesc->m_nWeaponType = MWT_DUAL_DAGGER;
			else if (!_stricmp(szAttrValue, "katana"))		pNewDesc->m_nWeaponType = MWT_KATANA;
			else if (!_stricmp(szAttrValue, "greatsword"))	pNewDesc->m_nWeaponType = MWT_GREAT_SWORD;
			else if (!_stricmp(szAttrValue, "doublekatana"))	pNewDesc->m_nWeaponType = MWT_DOUBLE_KATANA;

			else if (!_stricmp(szAttrValue, "pistol"))		pNewDesc->m_nWeaponType = MWT_PISTOL;
			else if (!_stricmp(szAttrValue, "pistolx2"))		pNewDesc->m_nWeaponType = MWT_PISTOLx2;
			else if (!_stricmp(szAttrValue, "revolver"))		pNewDesc->m_nWeaponType = MWT_REVOLVER;
			else if (!_stricmp(szAttrValue, "revolverx2"))	pNewDesc->m_nWeaponType = MWT_REVOLVERx2;
			else if (!_stricmp(szAttrValue, "smg"))			pNewDesc->m_nWeaponType = MWT_SMG;
			else if (!_stricmp(szAttrValue, "smgx2"))		pNewDesc->m_nWeaponType = MWT_SMGx2;
			else if (!_stricmp(szAttrValue, "shotgun"))		pNewDesc->m_nWeaponType = MWT_SHOTGUN;
			else if (!_stricmp(szAttrValue, "sawedshotgun"))	pNewDesc->m_nWeaponType = MWT_SAWED_SHOTGUN;
			else if (!_stricmp(szAttrValue, "rifle"))		pNewDesc->m_nWeaponType = MWT_RIFLE;
			else if (!_stricmp(szAttrValue, "machinegun"))	pNewDesc->m_nWeaponType = MWT_MACHINEGUN;
			else if (!_stricmp(szAttrValue, "rocket"))		pNewDesc->m_nWeaponType = MWT_ROCKET;
			else if (!_stricmp(szAttrValue, "snifer"))		pNewDesc->m_nWeaponType = MWT_SNIFER;

			else if (!_stricmp(szAttrValue, "medkit"))		pNewDesc->m_nWeaponType = MWT_MED_KIT;
			else if (!_stricmp(szAttrValue, "repairkit"))	pNewDesc->m_nWeaponType = MWT_REPAIR_KIT;
			else if (!_stricmp(szAttrValue, "bulletkit"))	pNewDesc->m_nWeaponType = MWT_BULLET_KIT;
			else if (!_stricmp(szAttrValue, "flashbang"))	pNewDesc->m_nWeaponType = MWT_FLASH_BANG;
			else if (!_stricmp(szAttrValue, "frag"))			pNewDesc->m_nWeaponType = MWT_FRAGMENTATION;
			else if (!_stricmp(szAttrValue, "smoke"))		pNewDesc->m_nWeaponType = MWT_SMOKE_GRENADE;
			else if (!_stricmp(szAttrValue, "skill"))		pNewDesc->m_nWeaponType = MWT_SKILL;
			else if (!_stricmp(szAttrValue, "food"))			pNewDesc->m_nWeaponType = MWT_FOOD;

			// enchant
			else if (!_stricmp(szAttrValue, "enchant_fire"))			pNewDesc->m_nWeaponType = MWT_ENCHANT_FIRE;
			else if (!_stricmp(szAttrValue, "enchant_cold"))			pNewDesc->m_nWeaponType = MWT_ENCHANT_COLD;
			else if (!_stricmp(szAttrValue, "enchant_lightning"))	pNewDesc->m_nWeaponType = MWT_ENCHANT_LIGHTNING;
			else if (!_stricmp(szAttrValue, "enchant_poison"))		pNewDesc->m_nWeaponType = MWT_ENCHANT_POISON;

			else _ASSERT(0);
		}
		else if (!_stricmp(szAttrName, MICTOK_EFFECT_LEVEL))
		{
			// 이펙트레벨
			pNewDesc->m_nEffectLevel = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_WEIGHT))
		{
			pNewDesc->m_nWeight = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_BOUNTY_PRICE))
		{
			pNewDesc->m_nBountyPrice = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_DAMAGE))
		{
			pNewDesc->m_nDamage = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_ISCASHITEM))
		{
			if (!_stricmp(szAttrValue, "true"))
			{
				pNewDesc->m_bIsCashItem = true;
			}
			else
			{
				pNewDesc->m_bIsCashItem = false;
			}
		}
		else if (!_stricmp(szAttrName, MICTOK_EFFECT_ID))
		{
			//n = atoi(szAttrValue);
			// 이펙트 처리 해야 함
		}
		else if (!_stricmp(szAttrName, MICTOK_DELAY))
		{
			pNewDesc->m_nDelay = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_CONTROLLABILITY))
		{
			pNewDesc->m_nControllability = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_MAGAZINE))
		{
			pNewDesc->m_nMagazine = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_MAXBULLET))
		{
			pNewDesc->m_nMaxBullet = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_RELOADTIME))
		{
			pNewDesc->m_nReloadTime = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SLUGOUTPUT))
		{
			if (!_stricmp(szAttrValue, "true"))
			{
				pNewDesc->m_bSlugOutput = true;
			}
			else
			{
				pNewDesc->m_bSlugOutput = false;
			}
		}
		else if (!_stricmp(szAttrName, MICTOK_HP))
		{
			pNewDesc->m_nHP = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_AP))
		{
			pNewDesc->m_nAP = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_MAXWT))
		{
			pNewDesc->m_nMaxWT = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SF))
		{
			pNewDesc->m_nSF = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_FR))
		{
			pNewDesc->m_nFR = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_CR))
		{
			pNewDesc->m_nCR = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_PR))
		{
			pNewDesc->m_nPR = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_LR))
		{
			pNewDesc->m_nLR = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_LIMITSPEED))
		{
			pNewDesc->m_nLimitSpeed = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_LIMITJUMP))
		{
			pNewDesc->m_nLimitJump = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_LIMITTUMBLE))
		{
			pNewDesc->m_nLimitTumble = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_LIMITWALL))
		{
			pNewDesc->m_nLimitWall = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_RANGE))
		{
			pNewDesc->m_nRange = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_COLOR))
		{
			// color 처리해야 함..-_-z
		}
		else if (!_stricmp(szAttrName, MICTOK_DESC))
		{
			strcpy_safe(pNewDesc->m_szDesc, MGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MICTOK_MESH_NAME))
		{
			strcpy_safe(pNewDesc->m_szMeshName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_IMAGE_ID))
		{
			pNewDesc->m_nImageID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_BULLET_IMAGE_ID))
		{
			pNewDesc->m_nBulletImageID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_MAGAZINE_IMAGE_ID))
		{
			pNewDesc->m_nMagazineImageID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SOUND_RELOAD))
		{
			strcpy_safe(pNewDesc->m_szReloadSndName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SOUND_FIRE))
		{
			strcpy_safe(pNewDesc->m_szFireSndName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_SOUND_DRYFIRE))
		{
			strcpy_safe(pNewDesc->m_szDryfireSndName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_GADGET_ID))
		{
			pNewDesc->m_nGadgetID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_BONUS_XP_SOLO))
		{
			pNewDesc->m_Bonus.m_fXP_SoloBonus = (float)(atoi(szAttrValue)) / 100.0f;
		}
		else if (!_stricmp(szAttrName, MICTOK_BONUS_XP_TEAM))
		{
			pNewDesc->m_Bonus.m_fXP_TeamBonus = (float)(atoi(szAttrValue)) / 100.0f;
		}
		else if (!_stricmp(szAttrName, MICTOK_BONUS_XP_QUEST))
		{
			pNewDesc->m_Bonus.m_fXP_QuestBonus = (float)(atoi(szAttrValue)) / 100.0f;
		}
		else if( !_stricmp(szAttrName, MICTOK_BONUS_BP_SOLO) )
		{
			pNewDesc->m_Bonus.m_fBP_SoloBonus = static_cast< float >( atoi(szAttrValue) ) / 100.0f;
		}
		else if( !_stricmp(szAttrName, MICTOK_BONUS_BP_TEAM) )
		{
			pNewDesc->m_Bonus.m_fBP_TeamBonus = static_cast< float >( atoi(szAttrValue) ) / 100.0f;
		}
		else if( !_stricmp(szAttrName, MICTOK_BONUS_BP_QUEST) )
		{
			pNewDesc->m_Bonus.m_fBP_QuestBonus = static_cast< float >( atoi(szAttrValue) ) / 100.0f;
		}
		else if( !_stricmp(szAttrName, MICTOK_BONUS_DUPLICATE) )
		{
			if( 0 == _stricmp("false", szAttrValue) )
				pNewDesc->m_bDuplicate = false;
			else
				pNewDesc->m_bDuplicate = true;
		}
	}


#ifdef _DEBUG
	iterator tempitor = find(pNewDesc->m_nID);
	if (tempitor != end())
	{
		//_ASSERT(0);
		MLog("ZItem error: %s has identical id (%d) to %s\n",
			pNewDesc->m_szName, pNewDesc->m_nID, tempitor->second->m_szName);
		delete pNewDesc;
		return;
	}
#endif

	insert(value_type(pNewDesc->m_nID, pNewDesc));
}

MMatchItemDescMgr MMatchItemDescMgr::DefaultInstance;
static MMatchItemDescMgr* CurrentMatchItemDescMgr = &MMatchItemDescMgr::DefaultInstance;

MMatchItemDescMgr* MMatchItemDescMgr::GetInstance()
{
	return CurrentMatchItemDescMgr;
}

void MSetMatchItemDescMgr(MMatchItemDescMgr* New)
{
	CurrentMatchItemDescMgr = New;
}


///////////////////////////////////////////////////////////////////////////////
// MMatchItem /////////////////////////////////////////////////////////////////
MMatchItem::MMatchItem() : MBaseItem(), m_nCIID(0), m_pDesc(NULL), m_bEquiped(false), m_nRentItemRegTime(0)
{}

bool MMatchItem::Create(const MUID& uid, MMatchItemDesc* pDesc, int nCount)
{
	m_uidItem = uid;
	m_nCount = nCount;
	m_pDesc = pDesc;

	return true;
}

void MMatchItem::Destroy()
{
	m_pDesc = NULL;
	m_nCount = 0;
	m_uidItem = MUID(0,0);
}

MMatchItemType MMatchItem::GetItemType()
{
	if (m_pDesc == NULL) return MMIT_MELEE;

	return m_pDesc->m_nType;
}
///////////////////////////////////////////////////////////////////////////////
// MMatchItemMap //////////////////////////////////////////////////////////////
MMatchItemMap::MMatchItemMap()
{
	m_bDoneDbAccess = false;	
	m_bHasRentItem = false;
}
MMatchItemMap::~MMatchItemMap()
{
	Clear();
}

bool MMatchItemMap::CreateItem(MUID& uid, int nCIID, int nItemDescID, bool bRentItem, int nRentMinutePeriodRemainder, int nCount)
{
	MMatchItemDesc* pDesc = NULL;
	pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemDescID);
	if (pDesc == NULL) 
	{
		_ASSERT(0);
		return false;
	}

	MMatchItem* pNewItem = new MMatchItem();
	if (pNewItem == NULL) 
	{
		_ASSERT(0);
		return false;
	}
	pNewItem->Create(uid, pDesc);
	pNewItem->SetCIID(nCIID);

	if (bRentItem)
	{
		pNewItem->SetRentItem(nRentMinutePeriodRemainder);
		pNewItem->SetRentItemRegTime(GetGlobalTimeMS());
	}

	insert(value_type(uid, pNewItem));

	if (bRentItem) m_bHasRentItem = true;

	return true;
}

bool MMatchItemMap::RemoveItem(MUID& uidItem)
{
	iterator itor = find(uidItem);
	if (itor != end())
	{
		MMatchItem* pItem = (*itor).second;
		delete pItem; pItem = NULL;
		erase(itor);
	}
	else
	{
		return false;
	}

	return true;
}

MMatchItem* MMatchItemMap::GetItem(MUID& uidItem)
{
	iterator itor = find(uidItem);
	if (itor != end())
	{
		return (*itor).second;
	}

	return NULL;
}

void MMatchItemMap::Clear()
{	
	m_bDoneDbAccess = false;
	m_bHasRentItem = false;

	while(!empty())
	{
		MMatchItem* pItem = (*begin()).second;
		delete pItem; pItem = NULL;
		erase(begin());
	}
}


///////////////////////////////////////////////////////////////////////////////
// MMatchEquipedItem //////////////////////////////////////////////////////////
bool MMatchEquipedItem::SetItem(MMatchCharItemParts parts, MMatchItem* pMatchItem)
{
	if (m_pParts[parts] != NULL)
	{
		m_pParts[parts]->SetEquiped(false);
	}

	m_pParts[parts] = pMatchItem;
	pMatchItem->SetEquiped(true);

	return true;
}

void MMatchEquipedItem::GetTotalWeight(int* poutWeight, int* poutMaxWeight)
{
	int weight = 0;
	int maxwt = 0;

	for (int i = 0; i < MMCIP_END; i++)
	{
		if (m_pParts[i] != NULL)
		{
			weight += (m_pParts[i]->GetDesc()->m_nWeight);
			maxwt += (m_pParts[i]->GetDesc()->m_nMaxWT);
		}
	}
	
	*poutWeight = weight;
	*poutMaxWeight = maxwt;
}

void MMatchEquipedItem::Remove(MMatchCharItemParts parts)
{
	if (m_pParts[parts] != NULL) m_pParts[parts]->SetEquiped(false);
	m_pParts[parts] = NULL;
}

void MMatchEquipedItem::Clear()
{
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (m_pParts[i] != NULL)
		{
			m_pParts[i]->SetEquiped(false);
		}
		m_pParts[i] = NULL;
	}
}


bool MMatchEquipedItem::IsEquipedItem(MMatchItem* pCheckItem, MMatchCharItemParts& outParts)
{
	if (pCheckItem == NULL) return false;
	for (int i = 0; i < MMCIP_END; i++)
	{
		if (m_pParts[i] == pCheckItem)
		{
			outParts = MMatchCharItemParts(i);
			return true;
		}
	}

	return false;
}

bool IsSuitableItemSlot(MMatchItemSlotType nSlotType, MMatchCharItemParts nParts)
{
	if (nSlotType == MMIST_MELEE)
	{
		if (nParts == MMCIP_MELEE) return true;
		else return false;
	}
	if (nSlotType == MMIST_RANGE)
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY)) return true;
		else return false;
	}
	if (nSlotType == MMIST_CUSTOM)
	{
		if ((nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2)) return true;
		else return false;
	}
	if (nSlotType == MMIST_HEAD)
	{
		if (nParts == MMCIP_HEAD) return true;
		else return false;
	}
	if (nSlotType == MMIST_CHEST)
	{
		if (nParts == MMCIP_CHEST) return true;
		else return false;
	}
	if (nSlotType == MMIST_HANDS)
	{
		if (nParts == MMCIP_HANDS) return true;
		else return false;
	}
	if (nSlotType == MMIST_LEGS)
	{
		if (nParts == MMCIP_LEGS) return true;
		else return false;
	}
	if (nSlotType == MMIST_FEET)
	{
		if (nParts == MMCIP_FEET) return true;
		else return false;
	}
	if (nSlotType == MMIST_FINGER)
	{
		if ((nParts == MMCIP_FINGERL) || (nParts == MMCIP_FINGERR)) return true;
		else return false;
	}

	return false;
};

MMatchCharItemParts GetSuitableItemParts(MMatchItemSlotType nSlotType)
{
	switch (nSlotType)
	{
	case MMIST_MELEE:
		return MMCIP_MELEE;
		break;
	case MMIST_RANGE:
		return MMCIP_PRIMARY;
		break;
	case MMIST_CUSTOM:
		return MMCIP_CUSTOM1;
		break;
	case MMIST_HEAD:
		return MMCIP_HEAD;
		break;
	case MMIST_CHEST:
		return MMCIP_CHEST;
		break;
	case MMIST_HANDS:
		return MMCIP_HANDS;
		break;
	case MMIST_LEGS:
		return MMCIP_LEGS;
		break;
	case MMIST_FEET:
		return MMCIP_FEET;
		break;
	case MMIST_FINGER:
		return MMCIP_FINGERL;
		break;
	default:
		_ASSERT(0);
	}

	return MMCIP_END;
}

MMatchItemSlotType	GetSuitableItemSlot(MMatchCharItemParts nParts)
{
	switch (nParts)
	{
	case MMCIP_HEAD:
		return MMIST_HEAD;
		break;
	case MMCIP_CHEST:
		return MMIST_CHEST;
		break;
	case MMCIP_HANDS:
		return MMIST_HANDS;
		break;
	case MMCIP_LEGS:
		return MMIST_LEGS;
		break;
	case MMCIP_FEET:
		return MMIST_FEET;
		break;
	case MMCIP_FINGERL:
	case MMCIP_FINGERR:
		return MMIST_FINGER;
		break;
	case MMCIP_MELEE:
		return MMIST_MELEE;
		break;
	case MMCIP_PRIMARY:
	case MMCIP_SECONDARY:
		return MMIST_RANGE;
		break;
	case MMCIP_CUSTOM1:
	case MMCIP_CUSTOM2:
		return MMIST_CUSTOM;
		break;
	default:
		_ASSERT(0);
	}

	return MMIST_END;
}


bool IsWeaponItemSlotType(MMatchItemSlotType nSlotType)
{
	if ((nSlotType == MMIST_MELEE) || (nSlotType == MMIST_RANGE) ||
		(nSlotType == MMIST_CUSTOM)) return true;

	return false;
}
bool IsWeaponCharItemParts(MMatchCharItemParts nParts)
{
	if ((nParts == MMCIP_MELEE) || (nParts == MMCIP_PRIMARY) ||
		(nParts == MMCIP_SECONDARY) || (nParts == MMCIP_CUSTOM1) ||
		(nParts == MMCIP_CUSTOM2)) return true;

	return false;
}


char* GetItemSlotTypeStr(MMatchItemSlotType nSlotType)
{
	static char st_SlotTypeStr[MMIST_END][32] = { "없음",		// MMIST_NONE
												"근접무기",		// MMIST_MELEE
												"원거리무기",	// MMIST_RANGE
												"아이템",		// MMIST_CUSTOM
												"머리",			// MMIST_HEAD
												"가슴",			// MMIST_CHEST
												"손",			// MMIST_HANDS
												"다리",			// MMIST_LEGS
												"발",			// MMIST_FEET
												"손가락",		// MMIST_FINGER
												"특별"};		// MMIST_EXTRA

	return st_SlotTypeStr[nSlotType];
}

char* GetCharItemPartsStr(MMatchCharItemParts nParts)
{
	static char st_CharItemPartsStr[MMCIP_END][32] = { "머리",	// MMCIP_HEAD
														"가슴",		// MMCIP_CHEST
														"손",		// MMCIP_HANDS
														"다리",		// MMCIP_LEGS
														"발",		// MMCIP_FEET
														"왼쪽손가락",	// MMCIP_FINGERL
														"오른쪽손가락", // MMCIP_FINGERR
														"근접무기",		// MMCIP_MELEE
														"주무기",		// MMCIP_PRIMARY
														"보조무기",		// MMCIP_SECONDARY
														"아이템1",		// MMCIP_CUSTOM1
														"아이템2"		// MMCIP_CUSTOM2
														};

	return st_CharItemPartsStr[nParts];
}


MMatchWeaponType GetWeaponType(MMatchMeleeItemType nMeleeItemType)
{
	switch (nMeleeItemType)
	{
	case MIT_DAGGER:		return MWT_DAGGER; 
	case MIT_DUAL_DAGGER:	return MWT_DUAL_DAGGER;
	case MIT_KATANA:		return MWT_KATANA;
	case MIT_GREAT_SWORD:	return MWT_GREAT_SWORD;
	case MIT_DOUBLE_KATANA:	return MWT_DOUBLE_KATANA;
	default:
			// 없는 타입
			_ASSERT(0);
	}

	return MWT_NONE;
}

MMatchWeaponType GetWeaponType(MMatchRangeItemType nRangeItemType)
{
	switch (nRangeItemType)
	{

	case RIT_PISTOL:		return MWT_PISTOL;
	case RIT_PISTOLx2:		return MWT_PISTOLx2;

	case RIT_REVOLVER:		return MWT_REVOLVER;
	case RIT_REVOLVERx2:	return MWT_REVOLVERx2;

	case RIT_SMG:			return MWT_SMG;
	case RIT_SMGx2:			return MWT_SMGx2;

	case RIT_SHOTGUN:		return MWT_SHOTGUN;
	case RIT_SAWED_SHOTGUN:	return MWT_SAWED_SHOTGUN;

	case RIT_RIFLE:			return MWT_RIFLE;
	case RIT_MACHINEGUN:	return MWT_MACHINEGUN;
	case RIT_ROCKET:		return MWT_ROCKET;
	case RIT_SNIFER:		return MWT_SNIFER;
	default:
			// 없는 타입
			_ASSERT(0);
	}

	return MWT_NONE;
}

MMatchWeaponType GetWeaponType(MMatchCustomItemType nCustomItemType)
{
	switch (nCustomItemType)
	{
	case MMCIT_MED_KIT:			return MWT_MED_KIT;
	case MMCIT_REPAIR_KIT:		return MWT_REPAIR_KIT;
	case MMCIT_BULLET_KIT:		return MWT_BULLET_KIT;
	case MMCIT_FLASH_BANG:		return MWT_FLASH_BANG;
	case MMCIT_FRAGMENTATION:	return MWT_FRAGMENTATION;
	case MMCIT_SMOKE_GRENADE:	return MWT_SMOKE_GRENADE;
	case MMCIT_ENCHANT_FIRE:		return MWT_ENCHANT_FIRE;
	case MMCIT_ENCHANT_COLD:		return MWT_ENCHANT_COLD;
	case MMCIT_ENCHANT_LIGHTNING:	return MWT_ENCHANT_LIGHTNING;
	case MMCIT_ENCHANT_POISON:		return MWT_ENCHANT_POISON;
	case MMCIT_FOOD:				return MWT_FOOD;

	default:
		{
			// 없는 타입
			_ASSERT(0);
		}
	}
	return MWT_NONE;
}

bool IsEnchantItem(MMatchItemDesc* pItemDesc)
{
	if (pItemDesc->m_nType == MMIT_CUSTOM)
	{
		if ((pItemDesc->m_nWeaponType == MWT_ENCHANT_FIRE) || 
			(pItemDesc->m_nWeaponType == MWT_ENCHANT_COLD) || 
			(pItemDesc->m_nWeaponType == MWT_ENCHANT_LIGHTNING) || 
			(pItemDesc->m_nWeaponType == MWT_ENCHANT_POISON) )
			return true;
	}

	return false;
}