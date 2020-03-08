#include "stdafx.h"

#include "ZStageSetting.h"
#include "ZGameInterface.h"
#include "MComboBox.h"
#include "MFrame.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZPost.h"
#include "MMatchGameType.h"
#include "ZChannelRule.h"
#include "ZConfiguration.h"
#include "ZGameTypeList.h"
#include "ZMap.h"

static struct _STAGESETTING_MAXPLAYER
{
	int					Value;
	char				szText[16];
	static constexpr auto Default = 1;
} StageSetting_MaxPlayer[] =
{ {4, "4"}, {8, "8"}, {10, "10"}, {12, "12"}, {16, "16"} };

static struct _STAGESETTING_MAXROUND
{
	int					Value;
	char				szText[32];
	static constexpr auto Default = 3;
} StageSetting_MaxRound[] = {
	{10, "10"}, {20, "20"}, {30, "30"}, {50, "50"}, {70, "70"}, {100, "100"} };

static struct _STAGESETTING_LIMITTIME_SINGLE
{
	int					Value;
	char				szText[32];
	static constexpr auto Default = 4;
} StageSetting_LimitTime_Single[] =
{ {99999, "무한"}, {10, "10분"}, {15, "15분"}, {20, "20분"}, {30, "30분"}, {60, "60분"} };
static struct _STAGESETTING_LIMITTIME_TEAM
{
	int					Value;
	char				szText[32];
	static constexpr auto Default = 4;
} StageSetting_LimitTime_Team[] =
{ {99999, "무한"}, {3, "3분"}, {5, "5분"}, {7, "7분"}, {10, "10분"}, {15, "15분"} };

static struct _STAGESETTING_LIMITLEVEL
{
	int					Value;
	char				szText[32];
	static constexpr auto Default = 0;
} StageSetting_LimitLevel[] =
{ {0, "없음"}, {5, "레벨차 5"}, {10, "레벨차 10"}, {15, "레벨차 15"} };

static struct _STAGESETTING_TEAM
{
	bool	Value;
	char	szText[32];
	static constexpr auto Default = 0;
} StageSetting_TeamKill[] =
{ {true, "허용"}, {false, "금지"} };

static struct _STAGESETTING_FORCEDENTRY
{
	bool	Value;
	char	szText[32];
	static constexpr auto Default = 0;
} StageSetting_ForcedEntry[] =
{ {true, "허용"}, {false, "금지"} };

static struct _STAGESETTING_OBSERVER
{
	bool	Value;
	char	szText[32];
	static constexpr auto Default = 1;
} StageSetting_Observer[] =
{ {true, "허용"}, {false, "금지"} };

static struct _STAGESETTING_VOTE
{
	bool	Value;
	char	szText[32];
	static constexpr auto Default = 0;
} StageSetting_Vote[] =
{ {true, "허용"}, {false, "금지"} };

static struct _STAGESETTING_TEAMBALANCING
{
	bool	Value;
	char	szText[32];
	static constexpr auto Default = 0;
} StageSetting_TeamBalancing[] =
{ {true, "ON"}, {false, "OFF"} };

static struct _STAGESETTING_NETCODE
{
	NetcodeType	Value;
	char	szText[32];
	static constexpr auto Default = 0;
} StageSetting_Netcode[3] =
{ { NetcodeType::ServerBased, "Server-based" },{ NetcodeType::P2PAntilead, "Peer to Peer Antilead" },{ NetcodeType::P2PLead, "Peer to Peer Lead" }, };





static bool BuildStageSetting(MSTAGE_SETTING_NODE* pOutNode)
{
	ZeroMemory(pOutNode, sizeof(MSTAGE_SETTING_NODE));
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
		if (pCB == NULL) return false;
		bool bExistGameType = false;
		for (int i = 0; i < MMATCH_GAMETYPE_MAX; i++)
		{
			if (!strcmp(pCB->GetText(), ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(i))))
			{
				bExistGameType = true;
				pOutNode->nGameType = ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->nGameTypeID;
				break;
			}
		}
		if (!bExistGameType) return false;
	}

	strcpy_safe(pOutNode->szStageName, ZGetGameClient()->GetStageName());

	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("MapSelection");
		if (pCB)
			strcpy_safe(pOutNode->szMapName, pCB->GetText());
	}

	auto BuildStageSettingListItem = [&](const char* WidgetItemName,
		auto& NodeVariable, const auto& ItemList)
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget(WidgetItemName);
		if (pCB && (pCB->GetSelIndex() >= 0))
		{
			int nItemCount = 0;
			auto itr = ItemList.begin();
			for (int i = 0; i < pCB->GetSelIndex(); i++)
			{
				if (itr == ItemList.end())
				{
					itr = ItemList.begin();
					break;
				}
				itr++;
			}
			NodeVariable = (*itr)->m_nValue;
		}
	};

	ZGameTypeConfig* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(pOutNode->nGameType);
	if (pGameTypeCfg)
	{
		BuildStageSettingListItem("StageMaxPlayer", pOutNode->nMaxPlayers, pGameTypeCfg->m_MaxPlayers);
		BuildStageSettingListItem("StageRoundCount", pOutNode->nRoundMax, pGameTypeCfg->m_Round);
		BuildStageSettingListItem("StageLimitTime", pOutNode->nLimitTime, pGameTypeCfg->m_LimitTime);
	}

	auto BuildStageSettingItem = [&](const char* WidgetItemName, auto& NodeVariable, auto& ItemList)
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget(WidgetItemName);
		if (pCB == nullptr) return;
		if (pCB->GetSelIndex() < int(std::size(ItemList)))
		{
			NodeVariable = ItemList[pCB->GetSelIndex()].Value;
		}
	};

	BuildStageSettingItem("StageLevelLimit", pOutNode->nLimitLevel, StageSetting_LimitLevel);
	BuildStageSettingItem("StageTeamKill", pOutNode->bTeamKillEnabled, StageSetting_TeamKill);
	BuildStageSettingItem("StageIntrude", pOutNode->bForcedEntryEnabled, StageSetting_ForcedEntry);
	BuildStageSettingItem("StageTeamBalancing", pOutNode->bAutoTeamBalancing, StageSetting_TeamBalancing);

	if (pOutNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO
		|| pOutNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM)
	{
		pOutNode->Netcode = NetcodeType::P2PLead;
	}
	else
	{
		BuildStageSettingItem("StageNetcode", pOutNode->Netcode, StageSetting_Netcode);
	}

	auto ForceHPAPWidget = static_cast<MButton*>(pResource->FindWidget("StageForceHPAP"));
	if (ForceHPAPWidget)
		pOutNode->ForceHPAP = ForceHPAPWidget->GetCheck();

	auto GetWidgetInt = [&](const char* WidgetName, int& Ret)
	{
		auto Widget = static_cast<MEdit*>(pResource->FindWidget(WidgetName));
		if (!Widget)
			return;

		auto Text = Widget->GetText();
		auto MaybeInt = StringToInt<int>(Text);

		if (!MaybeInt.has_value())
			return;

		Ret = MaybeInt.value();
	};

	GetWidgetInt("StageHP", pOutNode->HP);
	GetWidgetInt("StageAP", pOutNode->AP);

	auto Widget = static_cast<MButton*>(pResource->FindWidget("StageNoFlip"));
	if (Widget)
		pOutNode->NoFlip = Widget->GetCheck();

	Widget = static_cast<MButton*>(pResource->FindWidget("StageSwordsOnly"));
	if (Widget)
		pOutNode->SwordsOnly = Widget->GetCheck();

	Widget = static_cast<MButton*>(pResource->FindWidget("StageVanillaMode"));
	if (Widget)
		pOutNode->VanillaMode = Widget->GetCheck();

	Widget = static_cast<MButton*>(pResource->FindWidget("StageInvulnerabilityStates"));
	if (Widget)
		pOutNode->InvulnerabilityStates = Widget->GetCheck();

	return true;
}


void ZStageSetting::ShowStageSettingDialog(MSTAGE_SETTING_NODE* pStageSetting, bool bShowAll)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;


	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
		if (pCB)
		{
			for (int i = 0; i < MMATCH_GAMETYPE_MAX; i++)
			{
				if (pStageSetting->nGameType == ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->nGameTypeID)
				{
					int nGameType = i;

					for (int j = 0; j < pCB->GetCount(); j++)
					{
						if (!_stricmp(pCB->GetString(j), ZGetGameTypeManager()->GetInfo(MMATCH_GAMETYPE(i))->szGameTypeStr))
						{
							pCB->SetSelIndex(j);
							break;
						}
					}
				}
			}
		}
	}

	auto ShowStageSettingListItem = [&](auto&& WidgetItemName, auto&& NodeVariable, auto&& ItemList, auto&& MaxValue)
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget(WidgetItemName);
		if ( pCB)
		{
			int nSelect = pCB->GetSelIndex(), nItemCount = 0;
			pCB->RemoveAll();
			MGAMETYPECFGDATA::iterator itr = ItemList.begin();
			for ( ; itr != ItemList.end();  itr++)
			{
				if ( (*itr)->m_nValue <= MaxValue)
				{
					pCB->Add( (*itr)->m_szString);
					if ( (*itr)->m_nValue == NodeVariable)
						nSelect = nItemCount;
					nItemCount++;
				}
			}
			if ( nSelect >= nItemCount)
				nSelect = nItemCount - 1;
			pCB->SetSelIndex( nSelect);
		}
	};

	auto* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(pStageSetting->nGameType);
	if (pGameTypeCfg)
	{
		ShowStageSettingListItem("StageMaxPlayer",
			pStageSetting->nMaxPlayers, pGameTypeCfg->m_MaxPlayers,
			g_MapDesc[pStageSetting->nMapIndex].nMaxPlayers);

		ShowStageSettingListItem("StageRoundCount",
			pStageSetting->nRoundMax, pGameTypeCfg->m_Round, 99999);

		ShowStageSettingListItem("StageLimitTime",
			pStageSetting->nLimitTime, pGameTypeCfg->m_LimitTime, 99999);
	}

	[&]()
	{
		auto cb = static_cast<MComboBox*>(pResource->FindWidget("StageNetcode"));

		if (!cb)
			return;

		cb->RemoveAll();

		cb->Add("Server-based");
		cb->Add("Peer to Peer Antilead");
		cb->Add("Peer to Peer Lead");

		bool LeadOnly = IsSwordsOnly(pStageSetting->nGameType)
			|| pStageSetting->SwordsOnly
			|| MGetGameTypeMgr()->IsQuestDerived(pStageSetting->nGameType);

		if (LeadOnly)
			cb->SetSelIndex(2);
		else
			cb->SetSelIndex((int)pStageSetting->Netcode);

		cb->Enable(!LeadOnly);
	}();


	auto ShowStageSettingItem = [&](auto&& WidgetItemName, auto&& NodeVariable, auto&& ItemList)
	{
		auto* pCB = static_cast<MComboBox*>(pResource->FindWidget(WidgetItemName));
		if (!pCB)
			return;

		for (int i = 0; i < int(std::size(ItemList)); i++)
		{
			if (ItemList[i].Value == NodeVariable)
			{
				pCB->SetSelIndex(i);
				break;
			}
		}
	};

	ShowStageSettingItem("StageLevelLimit", pStageSetting->nLimitLevel, StageSetting_LimitLevel);
	ShowStageSettingItem("StageTeamKill", pStageSetting->bTeamKillEnabled, StageSetting_TeamKill);
	ShowStageSettingItem("StageIntrude", pStageSetting->bForcedEntryEnabled, StageSetting_ForcedEntry);
	ShowStageSettingItem("StageTeamBalancing", pStageSetting->bAutoTeamBalancing, StageSetting_TeamBalancing);


	MComboBox* pCBTeamBanlance = (MComboBox*)pResource->FindWidget("StageTeamBalancing");
	if (pCBTeamBanlance)
		pCBTeamBanlance->Enable(ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType));

	auto SetEditToInt = [&](const char* Name, int Value)
	{
		char buf[64];
		auto Widget = static_cast<MEdit*>(pResource->FindWidget(Name));
		if (Widget)
		{
			itoa_safe(Value, buf, 10);
			Widget->SetText(buf);
		}
	};

	auto SetButtonWidget = [&](const char* Name, bool Value, int Enable = -1)
	{
		auto Widget = static_cast<MButton*>(pResource->FindWidget(Name));
		if (Widget)
		{
			Widget->SetCheck(Value);
			if (Enable != -1)
				Widget->Enable(Enable != 0);
		}
	};

	SetButtonWidget("StageForceHPAP", pStageSetting->ForceHPAP);
	SetEditToInt("StageHP", pStageSetting->HP);
	SetEditToInt("StageAP", pStageSetting->AP);

	SetButtonWidget("StageNoFlip", pStageSetting->NoFlip);
	SetButtonWidget("StageSwordsOnly", pStageSetting->SwordsOnly, !IsSwordsOnly(pStageSetting->nGameType));
	SetButtonWidget("StageVanillaMode", pStageSetting->VanillaMode);
	SetButtonWidget("StageInvulnerabilityStates", pStageSetting->InvulnerabilityStates);

	if (bShowAll)
	{
		MWidget* pFindWidget = pResource->FindWidget("StageSettingFrame");
		if (pFindWidget != NULL) pFindWidget->Show(true, true);
	}
}



void ZStageSetting::InitStageSettingDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	auto InitStageSettingItem = [&](auto&& WidgetItemName, auto&& ItemList)
	{
		auto* pCB = static_cast<MComboBox*>(pResource->FindWidget(WidgetItemName));
		if (!pCB)
			return;

		pCB->RemoveAll();
		for (auto&& Item : ItemList)
		{
			pCB->Add(Item.szText);
		}
		pCB->SetSelIndex(ItemList[0].Default);
	};

	InitStageSettingGameType();

	strcpy_safe(StageSetting_LimitLevel[0].szText, ZMsg(MSG_WORD_NONE));
	for (int i = 1; i < 4; ++i) {
		sprintf_safe(StageSetting_LimitLevel[i].szText, "%s %d", ZMsg(MSG_WORD_LEVELDIFF), i * 5);
	}
	InitStageSettingItem("StageLevelLimit", StageSetting_LimitLevel);

	strcpy_safe(StageSetting_TeamKill[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_TeamKill[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	InitStageSettingItem("StageTeamKill", StageSetting_TeamKill);

	strcpy_safe(StageSetting_ForcedEntry[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_ForcedEntry[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	InitStageSettingItem("StageIntrude", StageSetting_ForcedEntry);

	strcpy_safe(StageSetting_TeamBalancing[0].szText, ZMsg(MSG_WORD_ON));
	strcpy_safe(StageSetting_TeamBalancing[1].szText, ZMsg(MSG_WORD_OFF));
	InitStageSettingItem("StageTeamBalancing", StageSetting_TeamBalancing);

	strcpy_safe(StageSetting_Observer[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_Observer[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	InitStageSettingItem("StageObserver", StageSetting_Observer);

	strcpy_safe(StageSetting_Vote[0].szText, ZMsg(MSG_WORD_PERMIT));
	strcpy_safe(StageSetting_Vote[1].szText, ZMsg(MSG_WORD_PROHIBIT));
	InitStageSettingItem("StageVote", StageSetting_Vote);

	InitStageSettingItem("StageNetcode", StageSetting_Netcode);

	auto SetButton = [&](const char* Name, bool Value) {
		auto Widget = static_cast<MButton*>(pResource->FindWidget(Name));
		if (Widget) {
			Widget->SetCheck(Value);
		}
	};

	auto SetEdit = [&](const char* Name, const char* Value) {
		auto Widget = static_cast<MEdit*>(pResource->FindWidget(Name));
		if (Widget) {
			Widget->SetText(Value);
		}
	};

	SetButton("StageForceHPAP", true);
	SetEdit("StageHP", "100");
	SetEdit("StageAP", "50");
	SetButton("StageNoFlip", true);
	SetButton("StageSwordsOnly", true);
}

void ZStageSetting::ApplyStageSettingDialog()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	MWidget* pWidget = pResource->FindWidget("StageSettingFrame");
	if (pWidget != NULL) pWidget->Show(false);

	PostDataToServer();
}

void ZStageSetting::PostDataToServer()
{
	MSTAGE_SETTING_NODE SettingNode;
	BuildStageSetting(&SettingNode);
	ZPostStageSetting(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), &SettingNode);
}


void ZStageSetting::InitStageSettingGameType()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	MComboBox* pCB = (MComboBox*)pResource->FindWidget("StageType");
	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) return;
	if (pCB == NULL) return;

	MChannelRuleGameTypeList* pGameTypeList = pRule->GetGameTypeList();

	pCB->RemoveAll();

	for (MChannelRuleGameTypeList::iterator itor = pGameTypeList->begin(); itor != pGameTypeList->end(); ++itor)
	{
		int nGameType = (*itor);
		if ((nGameType < 0) || (nGameType >= MMATCH_GAMETYPE_MAX)) continue;
		pCB->Add(ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(nGameType)));
	}

#ifdef _QUEST
	{
		if ((ZGetGameClient()) && (ZGetGameClient()->GetServerMode() == MSM_TEST))
		{
			pCB->Add(ZGetGameTypeManager()->GetGameTypeStr(MMATCH_GAMETYPE(MMATCH_GAMETYPE_QUEST)));
		}
	}
#endif

	pCB->SetSelIndex(MMATCH_GAMETYPE_DEFAULT);
}


void ZStageSetting::InitStageSettingGameFromGameType()
{
	if (!ZGetGameClient() || ZGetGameClient()->IsForcedEntry())
		return;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	int nGameType = 0;
	MMATCH_GAMETYPE nPrevGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();

	MComboBox* pCBType = (MComboBox*)pResource->FindWidget("StageType");
	if (pCBType)
	{
		constexpr int GametypeNameIDList[] = {
			MSG_MT_DEATHMATCH_SOLO, MSG_MT_DEATHMATCH_TEAM,
			MSG_MT_GLADIATOR_SOLO, MSG_MT_GLADIATOR_TEAM,
			MSG_MT_ASSASSINATE, MSG_MT_TRAINING,
			MSG_MT_SURVIVAL, MSG_MT_QUEST,
			MSG_MT_BERSERKER, MSG_MT_DEATHMATCH_TEAM2,
			MSG_MT_DUEL,
		};

		for (int i = 0; i < int(std::size(GametypeNameIDList)); ++i)
		{
			auto id = GametypeNameIDList[i];
			if (strcmp(pCBType->GetString(pCBType->GetSelIndex()), ZMsg(id)) == 0)
			{
				nGameType = i;
				break;
			}
		}
	}

	// Set game type
	ZGetGameClient()->GetMatchStageSetting()->SetGameType((MMATCH_GAMETYPE)nGameType);

	auto InitStageSettingListItem = [&](auto&& WidgetItemName, auto&& ItemList, auto&& nItemDefaultIndex)
	{
		auto* pCB = static_cast<MComboBox*>(pResource->FindWidget(WidgetItemName));
		if (!pCB)
			return;

			int nSelect = pCB->GetSelIndex(), nItemCount = 0;
			pCB->RemoveAll();
			for (auto&& val : ItemList)
				pCB->Add(val->m_szString);
			pCB->SetSelIndex(nItemDefaultIndex);
	};

	// Set game setting
	ZGameTypeConfig* pGameTypeCfg = ZGetConfiguration()->GetGameTypeList()->GetGameTypeCfg(nGameType);
	if (pGameTypeCfg)
	{
		InitStageSettingListItem("StageMaxPlayer", pGameTypeCfg->m_MaxPlayers, pGameTypeCfg->m_nDefaultMaxPlayers);
		InitStageSettingListItem("StageRoundCount", pGameTypeCfg->m_Round, pGameTypeCfg->m_nDefaultRound);
		InitStageSettingListItem("StageLimitTime", pGameTypeCfg->m_LimitTime, pGameTypeCfg->m_nDefaultLimitTime);
	}

	// Set map
	InitMapSelectionWidget();
}