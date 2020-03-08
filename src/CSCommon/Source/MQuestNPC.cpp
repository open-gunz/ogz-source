#include "stdafx.h"
#include "MQuestNPC.h"
#include "MXml.h"
#include "MZFileSystem.h"
#include "stdlib.h"
#include "MMath.h"
#include "MBaseStringResManager.h"

MQuestNPCSpawnType MQuestNPCInfo::GetSpawnType()
{
	if (nGrade >= NPC_GRADE_BOSS)
	{
		return MNST_BOSS;
	}
	else if (nNPCAttackTypes & NPC_ATTACK_RANGE)
	{
		return MNST_RANGE;
	}

	return MNST_MELEE;
}

///////////////////////////////////////////////////////////////////////////////

void MQuestNPCCatalogue::Clear()
{
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		delete (*itor).second;
	}

	clear();
}

MQuestNPCCatalogue::MQuestNPCCatalogue()
{
	m_GlobalAIValue.m_fAttack_ShakingRatio = 0.0f;
	m_GlobalAIValue.m_fPathFinding_ShakingRatio = 0.0f;
	m_GlobalAIValue.m_fSpeed_ShakingRatio = 0.0f;
	for (int i=0;i<NPC_INTELLIGENCE_STEPS;i++)
		m_GlobalAIValue.m_fPathFindingUpdateTime[i] = 1;
	for (int i=0;i<NPC_AGILITY_STEPS;i++)
		m_GlobalAIValue.m_fPathFindingUpdateTime[i] = 1;
}

MQuestNPCCatalogue::~MQuestNPCCatalogue() 
{ 
	Clear();
}

MQuestNPCInfo* MQuestNPCCatalogue::GetInfo(MQUEST_NPC nNpc)
{
	iterator itor = find(nNpc);
	if (itor != end())
	{
		return (*itor).second;
	}

	_ASSERT(0);
	return NULL;
}

MQuestNPCInfo* MQuestNPCCatalogue::GetPageInfo( int nPage)
{
	int nCount = 0;
	for ( iterator itor = begin();  itor != end();  itor++)
	{
		if ( nCount++ == nPage)
			return (*itor).second;
	}

	return NULL;
}

void MQuestNPCCatalogue::Insert(MQuestNPCInfo* pNPCInfo)
{
	if (!pNPCInfo)
	{
		assert(false);
		return;
	}

	MQUEST_NPC nID = pNPCInfo->nID;
	insert(value_type(nID, pNPCInfo));
}


MQuestNPCInfo* MQuestNPCCatalogue::GetIndexInfo( int nIndex )
{
	map< int, MQUEST_NPC >::iterator it = m_MonsterBibleCatalogue.find( nIndex );
	if( m_MonsterBibleCatalogue.end() == it )
		return 0;

	return GetInfo( it->second );
}


//////////////////////////////////////////////////////
#define MTOK_NPC						"NPC"
#define MTOK_NPC_AI_VALUE				"AI_VALUE"

// 기본정보
#define	MTOK_NPC_AI_SHAKING							"SHAKING"
#define	MTOK_NPC_AI_SHAKING_ATTR_PATHFINDING_UPDATE	"pathfinding_update"
#define	MTOK_NPC_AI_SHAKING_ATTR_ATTACK_UPDATE		"attack_update"
#define	MTOK_NPC_AI_SHAKING_ATTR_SPEED				"speed"
#define	MTOK_NPC_AI_INTELLIGENCE					"INTELLIGENCE"
#define	MTOK_NPC_AI_AGILITY							"AGILITY"
#define	MTOK_NPC_AI_TIME							"TIME"
#define	MTOK_NPC_AI_ATTR_STEP						"step"


#define MTOK_NPC_TAG_COLLISION			"COLLISION"
#define MTOK_NPC_TAG_ATTACK				"ATTACK"
#define MTOK_NPC_TAG_SPEED				"SPEED"
#define MTOK_NPC_TAG_FLAG				"FLAG"
#define MTOK_NPC_TAG_SKILL				"SKILL"
#define MTOK_NPC_TAG_DROP				"DROP"
#define MTOK_NPC_TAG_SOUND				"SOUND"

#define MTOK_NPC_ATTR_ID				"id"
#define MTOK_NPC_ATTR_NAME				"name"
#define MTOK_NPC_ATTR_DESC				"desc"
#define MTOK_NPC_ATTR_SKILL				"skill"
#define MTOK_NPC_ATTR_MESHNAME			"meshname"
#define MTOK_NPC_ATTR_GRADE				"grade"
#define MTOK_NPC_ATTR_MAXHP				"max_hp"
#define MTOK_NPC_ATTR_MAXAP				"max_ap"
#define MTOK_NPC_ATTR_RADIUS			"radius"
#define MTOK_NPC_ATTR_HEIGHT			"height"
#define MTOK_NPC_ATTR_TYPE				"type"
#define MTOK_NPC_ATTR_RANGE				"range"
#define MTOK_NPC_ATTR_WEAPONITEM_ID		"weaponitem_id"
#define MTOK_NPC_ATTR_DEFAULT			"default"
#define MTOK_NPC_ATTR_ROTATE			"rotate"
#define MTOK_NPC_ATTR_SCALE				"scale"
#define MTOK_NPC_ATTR_DC				"dc"
#define MTOK_NPC_ATTR_HITRATE			"hitrate"
#define MTOK_NPC_ATTR_TABLE				"table"
#define MTOK_NPC_ATTR_INT				"int"
#define MTOK_NPC_ATTR_AGILITY			"agility"
#define MTOK_NPC_ATTR_COOLTIME			"cooltime"
#define MTOK_NPC_ATTR_VIEW_ANGLE		"view_angle"
#define MTOK_NPC_ATTR_DYINGTIME			"dyingtime"
#define MTOK_NPC_ATTR_PICK				"pick"
#define MTOK_NPC_ATTR_ANGLE				"angle"
#define MTOK_NPC_ATTR_TREMBLE			"tremble"


#define MTOK_NPC_ATTR_NEVER_BLASTED			"never_blasted"
#define MTOK_NPC_ATTR_NEVER_DAMAGED_MELEE	"never_damaged_melee"
#define MTOK_NPC_ATTR_NEVER_DAMAGED_RANGE	"never_damaged_range"
#define MTOK_NPC_ATTR_NEVER_PUSHED			"never_pushed"
#define MTOK_NPC_ATTR_NEVER_ATTACK_CANCEL	"never_attack_cancel"

#define MTOK_NPC_ATTR_SHADOW				"shadow"

#define MTOK_NPC_ATTR_ATTACK				"attack"
#define MTOK_NPC_ATTR_DEATH					"death"
#define MTOK_NPC_ATTR_WOUND					"wound"


bool MQuestNPCCatalogue::ReadXml(const char* szFileName)
{
	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName)) {
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPC))
		{
			ParseNPC(chrElement);
		}
		else if (!_stricmp(szTagName, MTOK_NPC_AI_VALUE))
		{
			ParseGlobalAIValue(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MQuestNPCCatalogue::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem)) {
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPC)) {
			ParseNPC(chrElement);
		}
		else if (!_stricmp(szTagName, MTOK_NPC_AI_VALUE))
		{
			ParseGlobalAIValue(chrElement);
		}
	}

	xmlIniData.Destroy();

	return true;
}

void MQuestNPCCatalogue::ParseNPC(MXmlElement& element)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];

	MQuestNPCInfo* pNPCInfo = new MQuestNPCInfo();
	pNPCInfo->SetDefault();

	// NPC 태그 속성값 --------------------
	int nAttrCount = element.GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		element.GetAttribute(i, szAttrName, szAttrValue);
		if (!_stricmp(szAttrName, MTOK_NPC_ATTR_ID))
		{
			pNPCInfo->nID = (MQUEST_NPC)atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NAME))
		{
			strcpy_safe(pNPCInfo->szName, MGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_DESC))
		{
			strcpy_safe(pNPCInfo->szDesc, MGetStringResManager()->GetStringFromXml(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_MESHNAME))
		{
			strcpy_safe(pNPCInfo->szMeshName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_GRADE))
		{
			if(!_stricmp(szAttrValue,		"veteran"))		pNPCInfo->nGrade = NPC_GRADE_VETERAN;
			else if(!_stricmp(szAttrValue,	"elite"))		pNPCInfo->nGrade = NPC_GRADE_ELITE;
			else if(!_stricmp(szAttrValue,	"boss"))		pNPCInfo->nGrade = NPC_GRADE_BOSS;
			else if(!_stricmp(szAttrValue,	"legendary"))	pNPCInfo->nGrade = NPC_GRADE_LEGENDARY;
			else											pNPCInfo->nGrade = NPC_GRADE_REGULAR;
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_MAXHP))
		{
			pNPCInfo->nMaxHP = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_MAXAP))
		{
			pNPCInfo->nMaxAP = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_INT))
		{
			pNPCInfo->nIntelligence = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_AGILITY))
		{
			pNPCInfo->nAgility = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_SCALE))
		{
			sscanf(szAttrValue,"%f %f %f",&pNPCInfo->vScale.x,&pNPCInfo->vScale.y,&pNPCInfo->vScale.z);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_DC))
		{
			pNPCInfo->nDC = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_VIEW_ANGLE))
		{
			int nAngle = atoi(szAttrValue);
			pNPCInfo->fViewAngle = ToRadian(nAngle);
		}
		else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_DYINGTIME))
		{
			pNPCInfo->fDyingTime = (float)atof(szAttrValue);
		}
	}


	int iChildCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	for (int i = 0; i < iChildCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		// COLLISION 태그 --------------------
		if (!_stricmp(szTagName, MTOK_NPC_TAG_COLLISION))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_RADIUS))
				{
					pNPCInfo->fCollRadius = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_HEIGHT))
				{
					pNPCInfo->fCollHeight = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_PICK))
				{
					if(!_stricmp(szAttrValue, "true")) pNPCInfo->bColPick = true;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_TREMBLE))
				{
					pNPCInfo->fTremble = (float)atof(szAttrValue);
				}
			}
		}
		// ATTACK 태그 -----------------------
		else if (!_stricmp(szTagName, MTOK_NPC_TAG_ATTACK))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_TYPE))
				{
					if(!_stricmp(szAttrValue,		"melee"))   pNPCInfo->nNPCAttackTypes = NPC_ATTACK_MELEE;
					else if(!_stricmp(szAttrValue,	"range"))   pNPCInfo->nNPCAttackTypes = NPC_ATTACK_RANGE;
					else if(!_stricmp(szAttrValue,	"magic"))   pNPCInfo->nNPCAttackTypes = NPC_ATTACK_MAGIC;
					else										pNPCInfo->nNPCAttackTypes = NPC_ATTACK_NONE;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_RANGE))
				{
					pNPCInfo->fAttackRange = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_ANGLE))
				{
					int nAngle = atoi(szAttrValue);
					pNPCInfo->fAttackRangeAngle = ToRadian(nAngle);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_WEAPONITEM_ID))
				{
					pNPCInfo->nWeaponItemID = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_COOLTIME))
				{
					pNPCInfo->fAttackCoolTime = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_HITRATE))
				{
					int nPercent = atoi(szAttrValue);
					pNPCInfo->fAttackHitRate = (float)nPercent / 100.0f;
				}
			}
		}
		// SPEED 태그 ------------------------
		else if (!_stricmp(szTagName, MTOK_NPC_TAG_SPEED))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_DEFAULT))
				{
					pNPCInfo->fSpeed = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_ROTATE))
				{
					pNPCInfo->fRotateSpeed = (float)atof(szAttrValue);
				}
			}
		}
		// FLAG 태그 -------------------------
		else if (!_stricmp(szTagName, MTOK_NPC_TAG_FLAG))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NEVER_BLASTED))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bNeverBlasted = true;
					else								pNPCInfo->bNeverBlasted = false;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NEVER_DAMAGED_MELEE))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bMeleeWeaponNeverDamaged = true;
					else								pNPCInfo->bMeleeWeaponNeverDamaged = false;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NEVER_DAMAGED_RANGE))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bRangeWeaponNeverDamaged = true;
					else								pNPCInfo->bRangeWeaponNeverDamaged = false;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_SHADOW))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bShadow = true;
					else								pNPCInfo->bShadow = false;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NEVER_PUSHED))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bNeverPushed = true;
					else								pNPCInfo->bNeverPushed = false;
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_NEVER_ATTACK_CANCEL))
				{
					if(!_stricmp(szAttrValue, "true"))	pNPCInfo->bNeverAttackCancel = true;
					else								pNPCInfo->bNeverAttackCancel = false;
				}
			}
		}
		else if (!_stricmp(szTagName, MTOK_NPC_TAG_SKILL))
		{
			int nSkillID;
			chrElement.GetAttribute(&nSkillID,MTOK_NPC_ATTR_ID,0);
			_ASSERT(pNPCInfo->nSkills<MAX_SKILL);
			if(pNPCInfo->nSkills<MAX_SKILL)
			{
				pNPCInfo->nSkillIDs[pNPCInfo->nSkills++]=nSkillID;
				// 기술이 하나라도 있으면 플래그 체크해둔다
				pNPCInfo->nNPCAttackTypes|=NPC_ATTACK_MAGIC;
			}
		}
		// SOUND 태그 --------------------
		if (!_stricmp(szTagName, MTOK_NPC_TAG_SOUND))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_ATTACK))
				{
					strcpy_safe(pNPCInfo->szSoundName[NPC_SOUND_ATTACK], szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_WOUND))
				{
					strcpy_safe(pNPCInfo->szSoundName[NPC_SOUND_WOUND], szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_ATTR_DEATH))
				{
					strcpy_safe(pNPCInfo->szSoundName[NPC_SOUND_DEATH], szAttrValue);
				}
			}
		}
		// DROP 태그 -------------------------
		else if (!_stricmp(szTagName, MTOK_NPC_TAG_DROP))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_ATTR_TABLE))
				{
					strcpy_safe(pNPCInfo->szDropTableName, szAttrValue);
				}
			}
		}
	}

	Insert( pNPCInfo );
}

void MQuestNPCCatalogue::ParseGlobalAIValue(MXmlElement& element)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];

	int iChildCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	for (int i = 0; i < iChildCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		// SHAKING 태그 --------------------
		if (!_stricmp(szTagName, MTOK_NPC_AI_SHAKING))
		{
			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPC_AI_SHAKING_ATTR_PATHFINDING_UPDATE))
				{
					m_GlobalAIValue.m_fPathFinding_ShakingRatio = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_AI_SHAKING_ATTR_ATTACK_UPDATE))
				{
					m_GlobalAIValue.m_fAttack_ShakingRatio = (float)atof(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPC_AI_SHAKING_ATTR_SPEED))
				{
					m_GlobalAIValue.m_fSpeed_ShakingRatio = (float)atof(szAttrValue);
				}
			}
		}
		// INTELLIGENCE 태그 -----------------------
		else if (!_stricmp(szTagName, MTOK_NPC_AI_INTELLIGENCE))
		{
			int nChild = chrElement.GetChildNodeCount();
			MXmlElement TimeElement;
			char szChrTagName[64];
			for (int j = 0; j < nChild; j++)
			{
				TimeElement = chrElement.GetChildNode(j);
				TimeElement.GetTagName(szChrTagName);
				if (szChrTagName[0] == '#') continue;

				if (!_stricmp(szChrTagName, MTOK_NPC_AI_TIME))
				{
					int nStep = 0;
					float fTime = 1.0f;

					TimeElement.GetContents(&fTime);
					TimeElement.GetAttribute(&nStep, MTOK_NPC_AI_ATTR_STEP);

					nStep -= 1;
					if ((nStep >= 0) && (nStep < NPC_INTELLIGENCE_STEPS))
					{
						m_GlobalAIValue.m_fPathFindingUpdateTime[nStep] = fTime;
					}
				}
			}
		}
		// AGILITY 태그 -----------------------
		else if (!_stricmp(szTagName, MTOK_NPC_AI_AGILITY))
		{
			int nChild = chrElement.GetChildNodeCount();
			MXmlElement TimeElement;
			char szChrTagName[64];
			for (int j = 0; j < nChild; j++)
			{
				TimeElement = chrElement.GetChildNode(j);
				TimeElement.GetTagName(szChrTagName);
				if (szChrTagName[0] == '#') continue;

				if (!_stricmp(szChrTagName, MTOK_NPC_AI_TIME))
				{
					int nStep = 0;
					float fTime = 1.0f;

					TimeElement.GetContents(&fTime);
					TimeElement.GetAttribute(&nStep, MTOK_NPC_AI_ATTR_STEP);

					nStep -= 1;
					if ((nStep >= 0) && (nStep < NPC_AGILITY_STEPS))
					{
						m_GlobalAIValue.m_fAttackUpdateTime[nStep] = fTime;
					}
				}
			}
		}
	}


}
////////////////////////////////////////////////////////////////////////////////////////////////////

void MQuestNPCSetCatalogue::Clear()
{
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		delete (*itor).second;
	}

	clear();

	m_NameMap.clear();
}

MQuestNPCSetCatalogue::MQuestNPCSetCatalogue()
{

}

MQuestNPCSetCatalogue::~MQuestNPCSetCatalogue() 
{ 
	Clear();
}

MQuestNPCSetInfo* MQuestNPCSetCatalogue::GetInfo(int nID)
{
	iterator itor = find(nID);
	if (itor != end())
	{
		return (*itor).second;
	}

	return NULL;
}

MQuestNPCSetInfo* MQuestNPCSetCatalogue::GetInfo(const char* szName)
{
	char szLwrName[64];
	strcpy_safe(szLwrName, szName);
	strlwr_safe(szLwrName);
	auto strName = szLwrName;

	auto itor = m_NameMap.find(strName);
	if (itor != m_NameMap.end())
	{
		return (*itor).second;
	}

	_ASSERT(0);
	return NULL;
}

void MQuestNPCSetCatalogue::Insert(MQuestNPCSetInfo* pNPCSetInfo)
{
	int nID = pNPCSetInfo->nID;

	if (GetInfo(nID))
	{
		_ASSERT(0);
		return;
	}

	insert(value_type(nID, pNPCSetInfo));

	char szLwrName[64];
	strcpy_safe(szLwrName, pNPCSetInfo->szName);

	strlwr_safe(szLwrName);
	auto strName = szLwrName;
	m_NameMap.emplace(strName, pNPCSetInfo);
}

#define MTOK_NPCSET							"NPCSET"
#define MTOK_NPCSET_TAG_ADDNPC				"ADDNPC"

#define MTOK_NPCSET_ATTR_ID					"id"
#define MTOK_NPCSET_ATTR_NAME				"name"
#define MTOK_NPCSET_ATTR_BASENPC			"basenpc"
#define MTOK_NPCSET_ATTR_NPC_ID				"npc_id"
#define MTOK_NPCSET_ATTR_MIN_RATE			"min_rate"
#define MTOK_NPCSET_ATTR_MAX_RATE			"max_rate"
#define MTOK_NPCSET_ATTR_MAX_COUNT			"max_count"

bool MQuestNPCSetCatalogue::ReadXml(const char* szFileName)
{
	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName)) {
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPCSET))
		{
			ParseNPCSet(chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MQuestNPCSetCatalogue::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem)) {
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPCSET)) {
			ParseNPCSet(chrElement);
		}
	}

	xmlIniData.Destroy();

	return true;
}

void MQuestNPCSetCatalogue::ParseNPCSet(MXmlElement& element)
{
	char szTemp[256]="";
	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];
	char szTagName[128];

	MQuestNPCSetInfo* pNPCSetInfo = new MQuestNPCSetInfo();

	int nAttrCount = element.GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		element.GetAttribute(i, szAttrName, szAttrValue);
		if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_ID))
		{
			pNPCSetInfo->nID = MQUEST_NPC(atoi(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_NAME))
		{
			strcpy_safe(pNPCSetInfo->szName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_BASENPC))
		{
			pNPCSetInfo->nBaseNPC = MQUEST_NPC(atoi(szAttrValue));
		}
	}


	int iChildCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	for (int i = 0; i < iChildCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPCSET_TAG_ADDNPC))
		{
			MNPCSetNPC add_npc;

			int nAttrCount = chrElement.GetAttributeCount();
			for (int i = 0; i < nAttrCount; i++)
			{
				chrElement.GetAttribute(i, szAttrName, szAttrValue);
				if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_NPC_ID))
				{
					add_npc.nNPC = MQUEST_NPC(atoi(szAttrValue));
				}
				else if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_MIN_RATE))
				{
					add_npc.nMinRate = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_MAX_RATE))
				{
					add_npc.nMaxRate = atoi(szAttrValue);
				}
				else if (!_stricmp(szAttrName, MTOK_NPCSET_ATTR_MIN_RATE))
				{
					add_npc.nMaxSpawnCount = atoi(szAttrValue);
				}
			}

			pNPCSetInfo->vecNPCs.push_back(add_npc);
		}
	}

	Insert(pNPCSetInfo);
}