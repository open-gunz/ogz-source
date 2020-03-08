#include "stdafx.h"
#include "ZApplication.h"
#include "ZFile.h"
#include "ZGameClient.h"
#include "ZReplay.h"
#include "ZGame.h"
#include "ZNetCharacter.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "MMatchUtil.h"
#include "ZRuleDuel.h"
#include "ZRuleGunGame.h"

#include "RGMain.h"
#include "ZReplay.inl"

static void CreatePlayers(const std::vector<ReplayPlayerInfo>& Players)
{
#ifdef _DEBUG
	MLog("Size = %d\n", Players.size());
#endif
	for (auto& Player : Players)
	{
		ZCharacter* Char = nullptr;

#ifdef _DEBUG
		MLog("Name = %s\n", Player.Info.szName);
		MLog("Hero = %d\n", Player.IsHero);
#endif

		if (Player.IsHero)
		{
			g_pGame->m_pMyCharacter = new ZMyCharacter;
			g_pGame->CreateMyCharacter(Player.Info);
			Char = g_pGame->m_pMyCharacter;
			Char->Load(Player);
		}
		else
		{
			Char = new ZNetCharacter;
			Char->Load(Player);
			Char->Create(Player.Info);
		}

		ZGetCharacterManager()->Add(Char);

		Char->SetVisible(true);
	}
}

#define COPY_MEMBER(member) dest.member = src.member
void GetReplayStageSetting(REPLAY_STAGE_SETTING_NODE& dest, const MSTAGE_SETTING_NODE& src)
{
	COPY_MEMBER(uidStage);
	strcpy_safe(dest.szStageName, src.szStageName);
	strcpy_safe(dest.szMapName, src.szMapName);
	COPY_MEMBER(nMapIndex);
	COPY_MEMBER(nGameType);
	COPY_MEMBER(nRoundMax);
	COPY_MEMBER(nLimitTime);
	COPY_MEMBER(nLimitLevel);
	COPY_MEMBER(nMaxPlayers);
	COPY_MEMBER(bTeamKillEnabled);
	COPY_MEMBER(bTeamWinThePoint);
	COPY_MEMBER(bForcedEntryEnabled);
	COPY_MEMBER(bAutoTeamBalancing);
	COPY_MEMBER(ForceHPAP);
	COPY_MEMBER(HP);
	COPY_MEMBER(AP);
	COPY_MEMBER(NoFlip);
	COPY_MEMBER(SwordsOnly);
	COPY_MEMBER(VanillaMode);
	COPY_MEMBER(InvulnerabilityStates);
	memset(dest.Reserved, 0, sizeof(dest.Reserved));
}

static void ConvertStageSettingNode(const REPLAY_STAGE_SETTING_NODE& src, MSTAGE_SETTING_NODE& dest)
{
	COPY_MEMBER(uidStage);
	strcpy_safe(dest.szStageName, src.szStageName);
	strcpy_safe(dest.szMapName, src.szMapName);
	COPY_MEMBER(nMapIndex);
	COPY_MEMBER(nGameType);
	COPY_MEMBER(nRoundMax);
	COPY_MEMBER(nLimitTime);
	COPY_MEMBER(nLimitLevel);
	COPY_MEMBER(nMaxPlayers);
	COPY_MEMBER(bTeamKillEnabled);
	COPY_MEMBER(bTeamWinThePoint);
	COPY_MEMBER(bForcedEntryEnabled);
	COPY_MEMBER(bAutoTeamBalancing);
	COPY_MEMBER(ForceHPAP);
	COPY_MEMBER(HP);
	COPY_MEMBER(AP);
	COPY_MEMBER(NoFlip);
	COPY_MEMBER(SwordsOnly);
	COPY_MEMBER(VanillaMode);
	COPY_MEMBER(InvulnerabilityStates);
}
#undef COPY_MEMBER

static bool ChangeGameState(const REPLAY_STAGE_SETTING_NODE& rssn)
{
	MSTAGE_SETTING_NODE stageSetting;
	memset(&stageSetting, 0, sizeof(MSTAGE_SETTING_NODE));

	ConvertStageSettingNode(rssn, stageSetting);

	ZGetGameClient()->GetMatchStageSetting()->UpdateStageSetting(&stageSetting);
	ZApplication::GetStageInterface()->SetMapName(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	if (!ZGetGameInterface()->SetState(GUNZ_GAME))
		return false;

	ZGetCharacterManager()->Clear();
	ZGetObjectManager()->Clear();

	return true;
}

static bool ApplyGunGameWeapons(const std::vector<MTD_GunGameWeaponInfo>& WeaponInfos)
{
	int size = int(WeaponInfos.size());

	auto&& CharMgr = *ZGetCharacterManager();
	auto it = CharMgr.begin();

	auto* Rule = static_cast<ZRuleGunGame*>(ZGetGameInterface()->GetGame()->GetMatch()->GetRule());

	for (int i = 0; i < size; ++i)
	{
		if (it == CharMgr.end())
		{
			MLog("Internal error: There are fewer characters than there are "
				"MTD_GunGameWeaponInfos (%d chars, %d gg infos)\n", i, size);
			return false;
		}

		Rule->SetPlayerWeapons(it->second, WeaponInfos[i].WeaponIDs);

		++it;
	}

	if (it != CharMgr.end())
	{
		MLog("Internal error: There are more characters than there are "
			"MTD_GunGameWeaponInfos (%d chars, %d gg infos)\n", int(CharMgr.size()), size);
		return false;
	}

	return true;
}

struct ReplayVersionItems
{
	const char* Filename;
	bool Loaded = false;
	bool FailedToLoad = false;
	MMatchItemDescMgr ItemDescMgr;
};

ReplayVersionItems OfficialItems{"system/official_zitem.xml"};
ReplayVersionItems FGItems      {"system/fg_zitem.xml"};
ReplayVersionItems DGItems      {"system/dg_zitem.xml"};

bool SetReplayVersionItems(ReplayVersion Version)
{
	ReplayVersionItems* rvi;
	if (Version.Server == ServerType::Official)
		rvi = &OfficialItems;
	else if (Version.Server == ServerType::FreestyleGunz)
		rvi = &FGItems;
	else if (Version.Server == ServerType::DarkGunz)
		rvi = &DGItems;
	else if (Version.Server == ServerType::RefinedGunz)
		return true;
	else
		return false;

	if (!rvi->Loaded)
	{
		rvi->FailedToLoad = !rvi->ItemDescMgr.ReadXml(ZGetFileSystem(), rvi->Filename);
		rvi->Loaded = true;
	}

	if (rvi->FailedToLoad)
		return false;

	MSetMatchItemDescMgr(&rvi->ItemDescMgr);
	return true;
}

static bool LoadReplayData(ZReplayLoader& Loader, const char* filename)
{
	try
	{
		if (!Loader.LoadFile(filename))
			return false;

		auto Version = Loader.GetVersion();

		auto VersionString = Version.GetVersionString();

		MLog("Replay header loaded -- %s\n", VersionString.c_str());

		if (Version.Server == ServerType::None)
			return false;

		bool DefaultItems = !SetReplayVersionItems(Version);

		REPLAY_STAGE_SETTING_NODE StageSetting;
		Loader.GetStageSetting(StageSetting);

		if (!ChangeGameState(StageSetting))
			return false;

		std::vector<MTD_GunGameWeaponInfo> GunGameWeaponInfos;
		if (StageSetting.nGameType == MMATCH_GAMETYPE_DUEL)
		{
			ZRuleDuel* pDuel = static_cast<ZRuleDuel*>(ZGetGameInterface()->GetGame()->GetMatch()->GetRule());
			Loader.GetDuelQueueInfo(&pDuel->QInfo);
		}
		else if (StageSetting.nGameType == MMATCH_GAMETYPE_GUNGAME)
		{
			GunGameWeaponInfos = Loader.GetGunGameWeaponInfo();
		}

		{
			auto CharInfos = Loader.GetCharInfo();
			if (DefaultItems)
			{
				for (auto& CharInfo : CharInfos)
				{
					auto& Items = CharInfo.Info.nEquipedItemDesc;
					Items[MMCIP_MELEE] = 2;         // Rusty Sword
					Items[MMCIP_PRIMARY] = 6001;    // Breaker 5
					Items[MMCIP_SECONDARY] = 6002;  // Breaker 6
					Items[MMCIP_CUSTOM1] = 30001;   // Medical Kit MK-1
					Items[MMCIP_SECONDARY] = 30101; // Repair Kit RK-1
				}
			}
			CreatePlayers(CharInfos);
		}

		if (StageSetting.nGameType == MMATCH_GAMETYPE_GUNGAME)
		{
			if (!ApplyGunGameWeapons(GunGameWeaponInfos))
			{
				return false;
			}
		}

		auto PerCommand = [&](MCommand *Command, float Time)
		{
			ZObserverCommandItem *pZCommand = new ZObserverCommandItem;

			pZCommand->pCommand = Command;
			pZCommand->fTime = Time;

			g_pGame->GetReplayCommandList()->push_back(pZCommand);
		};

		Loader.GetCommands(PerCommand, true);
	}
	catch (EOFException& e)
	{
		MLog("Unexpected EOF while reading replay %s at position %d\n", filename, e.GetPosition());
		return false;
	}
	catch (...)
	{
		MLog("Something went wrong while reading replay %s\n", filename);
		return false;
	}

	return true;
}

bool CreateReplayGame(const char *SelectedFilename)
{
	static std::string LastFile;

	const char* Filename = nullptr;

	if (SelectedFilename)
	{
		Filename = SelectedFilename;
		LastFile = SelectedFilename;
	}
	else if (!LastFile.empty())
	{
		Filename = LastFile.c_str();
	}
	else
	{
		return false;
	}

	ZReplayLoader loader;

	if (!LoadReplayData(loader, Filename))
		return false;

	ZGetGame()->OnLoadReplay(&loader);

	return true;
}

bool ZReplayLoader::LoadFile(const char* FileName)
{
	auto pair = ReadZFile(FileName);

	if (!pair.first || pair.second.empty())
		return false;

	InflatedFile = std::move(pair.second);

	return true;
}

template <typename T>
bool ZReplayLoader::CreateCommandFromStream(const char* pStream, MCommand& Command, T& Alloc)
{
	if (Version.Server == ServerType::Official && Version.nVersion <= 2)
	{
		CreateCommandFromStreamVersion2(pStream, Command);
		return true;
	}

	bool ReadSerial = !(Version.Server == ServerType::Official && Version.nVersion == 11);

	return Command.SetData(pStream, ZGetGameClient()->GetCommandManager(), 65535, ReadSerial, Alloc);
}


bool ZReplayLoader::CreateCommandFromStreamVersion2(const char* pStream, MCommand& Command)
{
	MCommandManager* pCM = ZGetGameClient()->GetCommandManager();
	
	BYTE nParamCount = 0;
	unsigned short int nDataCount = 0;

	// Get Total Size
	unsigned short nTotalSize = 0;
	memcpy(&nTotalSize, pStream, sizeof(nTotalSize));
	nDataCount += sizeof(nTotalSize);

	// Command
	unsigned short int nCommandID = 0;
	memcpy(&nCommandID, pStream+nDataCount, sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	MCommandDesc* pDesc = pCM->GetCommandDescByID(nCommandID);
	if (pDesc == NULL)
	{
		mlog("Error(MCommand::SetData): Wrong Command ID(%d)\n", nCommandID);
		_ASSERT(0);

		return true;
	}
	Command.SetID(pDesc);

	if (ParseVersion2Command(pStream+nDataCount, &Command))
	{
		return true;
	}

	// Parameters
	memcpy(&nParamCount, pStream+nDataCount, sizeof(nParamCount));
	nDataCount += sizeof(nParamCount);
	for(int i=0; i<nParamCount; i++)
	{
		BYTE nType;
		memcpy(&nType, pStream+nDataCount, sizeof(BYTE));
		nDataCount += sizeof(BYTE);

		MCommandParameter* pParam = MakeVersion2CommandParameter((MCommandParameterType)nType, pStream, &nDataCount);
		if (pParam == NULL) return false;
		
		Command.m_Params.push_back(pParam);
	}

	return true;
}

bool ZReplayLoader::ParseVersion2Command(const char* pStream, MCommand* pCmd)
{
	switch (pCmd->GetID())
	{
	case MC_PEER_HPINFO:
	case MC_PEER_HPAPINFO:
	case MC_MATCH_OBJECT_CACHE:
	case MC_MATCH_STAGE_ENTERBATTLE:
	case MC_MATCH_STAGE_LIST:
	case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
	case MC_MATCH_GAME_RESPONSE_SPAWN:
	case MC_PEER_DASH:
	case MC_MATCH_BRIDGEPEER:
	case MC_MATCH_SPAWN_WORLDITEM:
		{

		}
		break;
	default:
		return false;
	};

	BYTE nParamCount = 0;
	unsigned short int nDataCount = 0;
	vector<MCommandParameter*> TempParams;

	// Count
	memcpy(&nParamCount, pStream+nDataCount, sizeof(nParamCount));
	nDataCount += sizeof(nParamCount);

	for(int i=0; i<nParamCount; i++)
	{
		BYTE nType;
		memcpy(&nType, pStream+nDataCount, sizeof(BYTE));
		nDataCount += sizeof(BYTE);

		MCommandParameter* pParam = MakeVersion2CommandParameter((MCommandParameterType)nType, pStream, &nDataCount);
		if (pParam == NULL) return false;
		
		TempParams.push_back(pParam);
	}


	switch (pCmd->GetID())
	{
	case MC_PEER_HPAPINFO:
		{
			void* pBlob = TempParams[1]->GetPointer();
			struct REPLAY2_HP_AP_INFO 
			{
				MUID muid;
				float fHP;
				float fAP;
			};

			REPLAY2_HP_AP_INFO* pBlobData = (REPLAY2_HP_AP_INFO*)MGetBlobArrayElement(pBlob, 0);
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fHP));
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fAP));
		}
		break;
	case MC_PEER_HPINFO:
		{
			void* pBlob = TempParams[1]->GetPointer();
			struct REPLAY2_HP_INFO 
			{
				MUID muid;
				float fHP;
			};

			REPLAY2_HP_INFO* pBlobData = (REPLAY2_HP_INFO*)MGetBlobArrayElement(pBlob, 0);
			pCmd->AddParameter(new MCmdParamFloat(pBlobData->fHP));
		}
		break;
	case MC_MATCH_OBJECT_CACHE:
		{
			unsigned int nType;
			TempParams[0]->GetValue(&nType);
			MCmdParamBlob* pBlobParam = ((MCmdParamBlob*)TempParams[1])->Clone();

			pCmd->AddParameter(new MCmdParamUChar((unsigned char)nType));
			pCmd->AddParameter(pBlobParam);
		}
		break;
	case MC_MATCH_STAGE_ENTERBATTLE:
		{
			MUID uidPlayer, uidStage;
			int nParam;
			
			TempParams[0]->GetValue(&uidPlayer);
			TempParams[1]->GetValue(&uidStage);
			TempParams[2]->GetValue(&nParam);

			struct REPLAY2_ExtendInfo
			{
				char			nTeam;
				unsigned char	nPlayerFlags;
				unsigned char	nReserved1;
				unsigned char	nReserved2;
			};

			struct REPLAY2_PeerListNode
			{
				MUID				uidChar;
				char				szIP[64];
				unsigned int		nPort;
				MTD_CharInfo		CharInfo;
				REPLAY2_ExtendInfo	ExtendInfo;
			};


			void* pBlob = TempParams[3]->GetPointer();
			REPLAY2_PeerListNode* pNode = (REPLAY2_PeerListNode*)MGetBlobArrayElement(pBlob, 0);


			void* pNewBlob = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
			MTD_PeerListNode* pNewNode = (MTD_PeerListNode*)MGetBlobArrayElement(pNewBlob, 0);
			pNewNode->uidChar = pNode->uidChar;
			pNewNode->dwIP = GetIPv4Number(pNode->szIP);
			pNewNode->nPort = pNode->nPort;
			memcpy(&pNewNode->CharInfo, &pNode->CharInfo, sizeof(MTD_CharInfo));
			pNewNode->ExtendInfo.nTeam = pNode->ExtendInfo.nTeam;
			pNewNode->ExtendInfo.nPlayerFlags = pNode->ExtendInfo.nPlayerFlags;
			pNewNode->ExtendInfo.nReserved1 = pNode->ExtendInfo.nReserved1;
			pNewNode->ExtendInfo.nReserved2 = pNode->ExtendInfo.nReserved1;
			

			pCmd->AddParameter(new MCmdParamUChar((unsigned char)nParam));
			pCmd->AddParameter(new MCommandParameterBlob(pNewBlob, MGetBlobArraySize(pNewBlob)));

			MEraseBlobArray(pNewBlob);
		}
		break;
	case MC_MATCH_STAGE_LIST:
		{
			_ASSERT(0);
		}
		break;
	case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
		{
			_ASSERT(0);
		}
		break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
		{
			MUID uidChar;
			rvector pos, dir;

			TempParams[0]->GetValue(&uidChar);
			TempParams[1]->GetValue(&pos);
			TempParams[2]->GetValue(&dir);

			pCmd->AddParameter(new MCmdParamUID(uidChar));
			pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
			pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
		}
		break;
	case MC_PEER_DASH:
		{
			rvector pos, dir;
			int nSelType;

			TempParams[0]->GetValue(&pos);
			TempParams[1]->GetValue(&dir);
			TempParams[2]->GetValue(&nSelType);

			ZPACKEDDASHINFO pdi;
			pdi.posx = Roundf(pos.x);
			pdi.posy = Roundf(pos.y);
			pdi.posz = Roundf(pos.z);

			pdi.dirx = DirElementToShort(dir.x);
			pdi.diry = DirElementToShort(dir.y);
			pdi.dirz = DirElementToShort(dir.z);

			pdi.seltype = (BYTE)nSelType;

			pCmd->AddParameter(new MCommandParameterBlob(&pdi,sizeof(ZPACKEDDASHINFO)));
		}
		break;
	case MC_MATCH_SPAWN_WORLDITEM:
		{
			struct REPLAY2_WorldItem
			{
				unsigned short	nUID;
				unsigned short	nItemID;
				unsigned short  nItemSubType;
				float			x;
				float			y;
				float			z;
			};


			void* pBlob = TempParams[0]->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);

			void* pNewBlob = MMakeBlobArray(sizeof(MTD_WorldItem), nCount);

			for (int i = 0; i < nCount; i++)
			{
				REPLAY2_WorldItem* pNode = (REPLAY2_WorldItem*)MGetBlobArrayElement(pBlob, i);
				MTD_WorldItem* pNewNode = (MTD_WorldItem*)MGetBlobArrayElement(pNewBlob, i);

				pNewNode->nUID = pNode->nUID;
				pNewNode->nItemID = pNode->nItemID;
				pNewNode->nItemSubType = pNode->nItemSubType;
				pNewNode->x = (short)Roundf(pNode->x);
				pNewNode->y = (short)Roundf(pNode->y);
				pNewNode->z = (short)Roundf(pNode->z);
			}
			pCmd->AddParameter(new MCommandParameterBlob(pNewBlob, MGetBlobArraySize(pNewBlob)));
			MEraseBlobArray(pNewBlob);

		}
		break;
	case MC_MATCH_BRIDGEPEER:
		{
			_ASSERT(0);
		}
		break;
	};


	for(int i=0; i<(int)TempParams.size(); i++){
		delete TempParams[i];
	}
	TempParams.clear();


	return true;
}


MCommandParameter* ZReplayLoader::MakeVersion2CommandParameter(MCommandParameterType nType, const char* pStream, unsigned short int* pnDataCount)
{
	MCommandParameter* pParam = NULL;

	switch(nType) 
	{
	case MPT_INT:
		pParam = new MCommandParameterInt;
		break;
	case MPT_UINT:
		pParam = new MCommandParameterUInt;
		break;
	case MPT_FLOAT:
		pParam = new MCommandParameterFloat;
		break;
	case MPT_STR:
		{
			pParam = new MCommandParameterString;
			MCommandParameterString* pStringParam = (MCommandParameterString*)pParam;

			const char* pStreamData = pStream + *pnDataCount;

			int nValueSize = 0;
			memcpy(&nValueSize, pStreamData, sizeof(nValueSize));
			pStringParam->m_Value = new char[nValueSize];
			memcpy(pStringParam->m_Value, pStreamData+sizeof(nValueSize), nValueSize);
			int nParamSize = nValueSize+sizeof(nValueSize);

			*pnDataCount += nParamSize;
			return pParam;
		}
		break;
	case MPT_VECTOR:
		pParam = new MCommandParameterVector;
		break;
	case MPT_POS:
		pParam = new MCommandParameterPos;
		break;
	case MPT_DIR:
		pParam = new MCommandParameterDir;
		break;
	case MPT_BOOL:
		pParam = new MCommandParameterBool;
		break;
	case MPT_COLOR:
		pParam = new MCommandParameterColor;
		break;
	case MPT_UID:
		pParam = new MCommandParameterUID;
		break;
	case MPT_BLOB:
		pParam = new MCommandParameterBlob;
		break;
	case MPT_CHAR:
		pParam = new MCommandParameterChar;
		break;
	case MPT_UCHAR:
		pParam = new MCommandParameterUChar;
		break;
	case MPT_SHORT:
		pParam = new MCommandParameterShort;
		break;
	case MPT_USHORT:
		pParam = new MCommandParameterUShort;
		break;
	case MPT_INT64:
		pParam = new MCommandParameterInt64;
		break;
	case MPT_UINT64:
		pParam = new MCommandParameterUInt64;
		break;
	default:
		mlog("Error(MCommand::SetData): Wrong Param Type\n");
		_ASSERT(false);		// Unknow Parameter!!!
		return NULL;
	}

	*pnDataCount += pParam->SetData(pStream + *pnDataCount);

	return pParam;
}