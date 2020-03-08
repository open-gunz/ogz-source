#include "stdafx.h"
//#include <winsock2.h>
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MMatchGlobal.h"
#include "MMatchConfig.h"
#include "MUtil.h"


#define CYCLE_MATCHSTAGELISTUPDATE			1000
#define CYCLE_MATCHCHANNELLISTUPDATE		1000
#define CYCLE_MATCHCHANNELPLAYERLISTUPDATE	1000
#define CYCLE_MATCHCHANNELCLANMEMBER		1000

#define CYCLE_MATCH_STANDBY_CLANLIST_UPDATE	1000		// 클랜전 대기 클랜 리스트 업데이트 시간은 10초이다.


const u32 MMatchDisconnStatusInfo::MINTERVAL_DISCONNECT_STATUS_MIN = (5 * 1000);

MMatchObject::MMatchObject(const MUID& uid) : MObject(uid) 
{ 
	m_pCharInfo = NULL;
	m_pFriendInfo = NULL;

	m_dwIP = 0;
	memset(m_szIP, 0, sizeof(char)*64);	
	m_nPort=0;
	
	m_uidStage = MUID(0,0);
	m_uidChatRoom = MUID(0,0);

	m_bBridgePeer = false;
	m_bRelayPeer = false;
	m_uidAgent = MUID(0,0);

	m_nPlayerFlags = 0;
	m_nUserOptionFlags = 0;

	m_ChannelInfo.Clear();

	m_bStageListTransfer = false;
	m_nStageListChecksum = 0;
	m_nStageListLastChecksum = 0;
	m_nTimeLastStageListTrans = 0;
	m_nStageCursor = 0;

	m_RefreshClientChannelImpl.SetMatchObject(this);
	m_RefreshClientClanMemberImpl.SetMatchObject(this);

	m_nTeam=MMT_ALL;
	SetLadderGroupID(0);
	m_nStageState = MOSS_NONREADY;
	m_bEnterBattle=false;
	m_bAlive=false;
	m_bForcedEntried = false;
	m_bLadderChallenging = false;
	m_nKillCount = 0;
	m_nDeathCount = 0;
	m_nPlace = MMP_OUTSIDE;
	m_bLaunchedGame = false;
	m_nAllRoundDeathCount = 0;
	m_nAllRoundKillCount = 0;
	m_bNewbie = false;
	m_nDeadTime = 0;

	m_GameInfo.bJoinedGame = false;

	m_bDBFriendListRequested = false;

	m_nTickLastPacketRecved = 0;
	m_bHacker = false;

	m_dwLastHackCheckedTime			= GetGlobalTimeMS();
	m_dwLastRecvNewHashValueTime	= GetGlobalTimeMS();
	m_bIsRequestNewHashValue		= false;

	LastSpawnTime					= GetGlobalTimeMS();

	m_nLastPingTime = m_nQuestLatency = 0;
	m_bQuestRecvPong = true;
}

MMatchObject::~MMatchObject()
{
	FreeCharInfo();
	FreeFriendInfo();
}

void MMatchObject::FreeCharInfo()
{
	if (m_pCharInfo) {
		delete m_pCharInfo;
		m_pCharInfo = NULL;
	}
	else
	{
//		_ASSERT(0);
	}
}

void MMatchObject::FreeFriendInfo()
{
	if (m_pFriendInfo) {
		delete m_pFriendInfo;
		m_pFriendInfo = NULL;
	}
	m_bDBFriendListRequested = false;
}

void MMatchObject::SetTeam(MMatchTeam nTeam)
{ 
	if (IsAdminGrade(this) && CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		nTeam = MMT_SPECTATOR;

	m_nTeam = nTeam;
}

void MMatchObject::SetStageCursor(int nStageCursor)
{
	m_nStageCursor = nStageCursor;
}

void MMatchObject::SetPlace(MMatchPlace nPlace)
{
	m_nPlace = nPlace;

	switch(m_nPlace) {
	case MMP_OUTSIDE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	case MMP_LOBBY:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(true);
		}
		break;
	case MMP_STAGE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	case MMP_BATTLE:
		{
			MRefreshClientChannelImpl* pChannelImpl = GetRefreshClientChannelImplement();
			pChannelImpl->Enable(false);
		}
		break;
	default:
		{
		}break;
	};
}

void MMatchObject::Tick(u64 nTime)
{
	MMatchServer* pServer = MMatchServer::GetInstance();

	if (CheckStageListTransfer() == true) {

		MMatchChannel* pChannel = pServer->FindChannel(GetChannelUID());
		if ( ( (MGetServerConfig()->GetServerMode() == MSM_CLAN) || (MGetServerConfig()->GetServerMode() == MSM_TEST) ) &&
			(pChannel) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
		{
			if ((unsigned int)(nTime - m_nTimeLastStageListTrans) > CYCLE_MATCH_STANDBY_CLANLIST_UPDATE) {
				u32 nCurrStageListChecksum = pServer->GetLadderMgr()->GetChecksum(m_nStageCursor, 
																			TRANS_STANDBY_CLANLIST_NODE_COUNT);
				if (nCurrStageListChecksum != GetStageListChecksum()) {
					m_nTimeLastStageListTrans = nTime;

					pServer->StandbyClanList(GetUID(), m_nStageCursor, true);
					UpdateStageListChecksum(nCurrStageListChecksum);
				}
			}
		}
		else
		{
			if ((unsigned int)(nTime - m_nTimeLastStageListTrans) > CYCLE_MATCHSTAGELISTUPDATE) {
				u32 nCurrStageListChecksum = pServer->GetStageListChecksum(m_ChannelInfo.uidChannel, 
																		m_nStageCursor, TRANS_STAGELIST_NODE_COUNT);
				if (nCurrStageListChecksum != GetStageListChecksum()) {
					m_nTimeLastStageListTrans = nTime;

					pServer->StageList(GetUID(), m_nStageCursor, true);
					UpdateStageListChecksum(nCurrStageListChecksum);
				}
			}
		}
	}

	if (CheckChannelListTransfer() == true) {
		if ((unsigned int)(nTime - m_ChannelInfo.nTimeLastChannelListTrans) > CYCLE_MATCHCHANNELLISTUPDATE) {
			if (pServer->GetChannelListChecksum() != GetChannelListChecksum()) {
				m_ChannelInfo.nTimeLastChannelListTrans = nTime;

				if ((m_ChannelInfo.nChannelListType != MCHANNEL_TYPE_CLAN) || (GetChannelListChecksum() == 0))
				{
					pServer->ChannelList(GetUID(), m_ChannelInfo.nChannelListType);
					UpdateChannelListChecksum(pServer->GetChannelListChecksum());				
				}
			}
		}
	}
	if (GetRefreshClientChannelImplement()->IsEnable()) {
		if (nTime - GetRefreshClientChannelImplement()->GetLastUpdatedTime() > CYCLE_MATCHCHANNELPLAYERLISTUPDATE) {
			GetRefreshClientChannelImplement()->SetLastUpdatedTime(nTime);

			MMatchChannel* pChannel = pServer->FindChannel(GetChannelUID());
			if (pChannel) {
				pChannel->SyncPlayerList(this, GetRefreshClientChannelImplement()->GetCategory());
			}
		}
	}
	if (GetRefreshClientClanMemberImplement()->IsEnable()) {
		if (nTime - GetRefreshClientClanMemberImplement()->GetLastUpdatedTime() > CYCLE_MATCHCHANNELCLANMEMBER) {
			GetRefreshClientClanMemberImplement()->SetLastUpdatedTime(nTime);

			MMatchClan* pClan = pServer->FindClan(GetCharInfo()->m_ClanInfo.m_nClanID);
			if (pClan) {
				pClan->SyncPlayerList(this, GetRefreshClientClanMemberImplement()->GetCategory());
			}
		}
	}

	m_DisconnStatusInfo.Update( nTime );

	auto* Stage = MGetMatchServer()->FindStage(m_uidStage);
	if (Stage && Stage->GetStageSetting()->GetNetcode() == NetcodeType::ServerBased)
	{
		if (nTime - LastHPAPInfoTime > 1000 && IsAlive())
		{
			MGetMatchServer()->PostHPAPInfo(*this, HP, AP);
			LastHPAPInfoTime = nTime;
		}

		if (!BasicInfoHistory.empty())
		{
			// TODO: Make less ungood!
			Origin = BasicInfoHistory.front().position;
			Direction = BasicInfoHistory.front().direction;
			Velocity = BasicInfoHistory.front().velocity;
		}
	}
}


void MMatchObject::OnStageJoin()
{
	SetAllRoundDeathCount(0);	
	SetAllRoundKillCount(0);
	SetStageListTransfer(false);	
	SetForcedEntry(false);
	SetPlace(MMP_STAGE);
	m_GameInfo.bJoinedGame = false;
	m_nDeadTime = 0;
}

void MMatchObject::OnEnterBattle()
{
	SetAlive(false);
	SetEnterBattle(true);
	SetKillCount(0);
	SetDeathCount(0);
	SetAllRoundDeathCount(0);	
	SetAllRoundKillCount(0);

	SetMaxHPAP();
}

void MMatchObject::OnLeaveBattle()
{
	SetEnterBattle(false);
	SetKillCount(0);
	SetDeathCount(0);
	SetAlive(false);
	SetStageState(MOSS_NONREADY);
	SetLaunchedGame(false);
	BasicInfoHistory.clear();
}


void MMatchObject::OnInitRound()
{
	if (GetTeam() != MMT_SPECTATOR)
	{
		SetAlive(true);
	}
	SetKillCount(0);
	SetDeathCount(0);

	m_GameInfo.bJoinedGame = true;
	m_nDeadTime = 0;

	ResetHPAP();
}

void MMatchObject::SetChannelListTransfer(const bool bVal, const MCHANNEL_TYPE nChannelType)
{ 
	if ((nChannelType < 0) || (nChannelType >= MCHANNEL_TYPE_MAX)) 
	{
		_ASSERT(0);
		return;
	}

	m_ChannelInfo.bChannelListTransfer = bVal; 
	m_ChannelInfo.nChannelListType = nChannelType;
	UpdateChannelListChecksum(0); 
}

void MMatchObject::GetPositions(v3* Head, v3* Foot, double Time) const
{
	auto GetItemDesc = [&](MMatchCharItemParts slot) -> MMatchItemDesc*
	{
		auto item = m_pCharInfo->m_EquipedItem.GetItem(slot);
		if (!item)
			return nullptr;

		auto id = item->GetDescID();

		auto ItemDesc = MGetMatchItemDescMgr()->GetItemDesc(id);

		return ItemDesc;
	};

	BasicInfoHistoryManager::Info Info;
	Info.Head = Head;
	Info.Pos = Foot;
	BasicInfoHistory.GetInfo(Info, Time, GetItemDesc, m_pCharInfo->m_nSex, !IsAlive());
}

void MMatchObject::SetMaxHPAP()
{
	auto CharInfo = GetCharInfo();
	if (!CharInfo)
		return;

	auto* Stage = MGetMatchServer()->FindStage(m_uidStage);
	if (!Stage)
		return;

	if (Stage->GetStageSetting()->IsForcedHPAP())
	{
		MaxHP = Stage->GetStageSetting()->GetForcedHP();
		MaxAP = Stage->GetStageSetting()->GetForcedAP();
		return;
	}

	auto SetP = [&](int& MaxP, int Default, auto ItemP)
	{
		MaxP = Default;
		for (auto Item : GetCharInfo()->m_EquipedItem)
		{
			if (!Item)
				continue;

			MaxP += Item->GetDesc()->*ItemP;
		}
	};

	SetP(MaxHP, 100, &MMatchItemDesc::m_nHP);
	SetP(MaxAP, 0, &MMatchItemDesc::m_nAP);
}

void MMatchObject::ResetHPAP()
{
	HP = MaxHP;
	AP = MaxAP;
}

void MMatchObject::OnDamaged(const MMatchObject& Attacker, const v3& SrcPos,
	ZDAMAGETYPE DamageType, MMatchWeaponType WeaponType, int Damage, float PiercingRatio)
{
	int HPDamage = int(Damage * PiercingRatio);
	int APDamage = Damage - HPDamage;

	if (AP - APDamage < 0)
	{
		HPDamage += APDamage - AP;
		APDamage -= APDamage - AP;
	}

	HP -= HPDamage;
	AP -= APDamage;

	MGetMatchServer()->PostDamage(GetUID(), Attacker.GetUID(), DamageType, WeaponType, Damage, PiercingRatio);

	if (HP <= 0)
	{
		MGetMatchServer()->OnGameKill(Attacker.GetUID(), GetUID());
	}
}

void MMatchObject::Heal(int Amount)
{
	auto HPDiff = min(HP + Amount, MaxHP) - HP;
	HP += HPDiff;
	AP += Amount - HPDiff;
	AP = min(AP, MaxAP);
}

bool MMatchObject::CheckEnableAction(MMO_ACTION nAction)
{
	switch (nAction)
	{
	case MMOA_STAGE_FOLLOW:		// 따라가기가 가능한 상태인지 여부
		{
			if (GetPlace() != MMP_LOBBY) return false;
			if (IsLadderChallenging()) return false;

			return true;
		}
		break;
	default:
		_ASSERT(0);
	}

	return true;
}

void MMatchObject::CheckNewbie(int nCharMaxLevel)
{
#define NEWBIE_LEVEL_CUTLINE		20		// 가지고 있는 캐릭터들의 최고레벨이 21레벨이상이면 뉴비가 아니다.

	if (nCharMaxLevel > NEWBIE_LEVEL_CUTLINE) m_bNewbie = false;
	else m_bNewbie = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void MMatchCharInfo::EquipFromItemList()
{
	m_EquipedItem.Clear();
	
	for (MMatchItemMap::iterator itor = m_ItemList.begin(); itor != m_ItemList.end(); ++itor)
	{
		MMatchItem* pItem = (*itor).second;
		pItem->SetEquiped(false);
	}

	for (int i = 0; i < MMCIP_END; i++)
	{
		if (m_nEquipedItemCIID[i] == 0) continue;

		for (MMatchItemMap::iterator itor = m_ItemList.begin(); itor != m_ItemList.end(); ++itor)
		{
			MMatchItem* pItem = (*itor).second;
			if (m_nEquipedItemCIID[i] == pItem->GetCIID())
			{
				m_EquipedItem.SetItem(MMatchCharItemParts(i), pItem);
				break;
			}
		}
	}
}

void MMatchCharInfo::ClearItems()
{
	m_EquipedItem.Clear();
	m_ItemList.Clear();
}

void MMatchCharInfo::Clear()
{
	m_nCID					= 0;
	m_nCharNum				= 0;
	m_nLevel				= 0;
	m_nSex					= MMS_MALE;
	m_nFace					= 0;
	m_nHair					= 0;
	m_nXP					= 0;
	m_nBP					= 0;
	m_fBonusRate			= DEFAULT_CHARINFO_BONUSRATE;
	m_nPrize				= DEFAULT_CHARINFO_PRIZE;
	m_nHP					= 0;
	m_nAP					= 0;
	m_nMaxWeight			= DEFAULT_CHARINFO_MAXWEIGHT;
	m_nSafeFalls			= DEFAULT_CHARINFO_SAFEFALLS;
	m_nFR					= 0;
	m_nCR					= 0;
	m_nER					= 0;
	m_nWR					= 0;
	m_nTotalPlayTimeSec		= 0;
	m_nConnTime				= 0;
	m_nTotalKillCount		= 0;
	m_nTotalDeathCount		= 0;
	m_nConnKillCount		= 0;
	m_nConnDeathCount		= 0;

	memset(m_szName, 0, sizeof(m_szName));
	memset(m_nEquipedItemCIID, 0, sizeof(m_nEquipedItemCIID));
	memset(&m_ClanInfo, 0, sizeof(MMatchCharClanInfo));

	ClearItems();
}

void MMatchCharInfo::GetTotalWeight(int* poutWeight, int* poutMaxWeight)
{
	int nWeight, nMaxWeight;

	m_EquipedItem.GetTotalWeight(&nWeight, &nMaxWeight);
	nMaxWeight = nMaxWeight + m_nMaxWeight;

	*poutWeight = nWeight;
	*poutMaxWeight = nMaxWeight;
}


bool IsEquipableItem(u32 nItemID, int nPlayerLevel, MMatchSex nPlayerSex)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;

	// 성별 제한 조건
	if (pItemDesc->m_nResSex != -1)
	{
		if (pItemDesc->m_nResSex != int(nPlayerSex)) return false;
	}

	if (MGetServerConfig()->GetServerMode() != MSM_EVENT) {	// EVENT때 레벨제한 없이 장착한다
		// 레벨 제한 조건
		if (pItemDesc->m_nResLevel > nPlayerLevel) return false;
	}

	return true;
}



void MMatchObject::SetFriendInfo(MMatchFriendInfo* pFriendInfo)
{
	m_bDBFriendListRequested = true;
	m_pFriendInfo = pFriendInfo;
}


void MMatchObject::SetCharInfo(MMatchCharInfo* pCharInfo)
{ 
	m_pCharInfo = pCharInfo; 
#ifdef _QUEST_ITEM
	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		m_pCharInfo->m_DBQuestCachingData.SetCharObject( this );
	}
#endif
}

void MMatchObject::OnDead()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	m_nDeadTime = pServer->GetTickTime();
	SetAlive(false);
	DeathCount();
}

void MMatchObject::OnKill()
{
	KillCount();
}

bool MMatchObject::IsEnabledRespawnDeathTime(u64 nNowTime) const
{
	if ((nNowTime - m_nDeadTime) > (RESPAWN_DELAYTIME_AFTER_DYING-500)) return true;
	return false;
}

void MMatchObject::UpdateTickLastPacketRecved()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	m_nTickLastPacketRecved = pServer->GetTickTime();
}