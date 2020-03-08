#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZEquipmentListBox.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"

bool ZGameInterface::OnCommand(MCommand* pCommand)
{
#ifdef _BIRDTEST
	ZApplication::GetGameInterface()->OnBirdTestCommand(pCommand);
	return false;
#endif

	if(g_pGame!=NULL) {
		return g_pGame->OnCommand(pCommand);
	}

	if (ZGetQuest()->OnCommand(pCommand)) return true;

	switch(pCommand->GetID())
	{
	case MC_MATCH_RESPONSE_ACCOUNT_CHARLIST:
		{
			mlog("Responsed Account Charlist\n");

			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pCharListBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pCharListBlob);

			// 캐릭터 정보 가져오기
			ZCharacterSelectView::OnReceivedAccountCharInfo(pCharListBlob);
			
			// 캐릭터가 한명도 없으면 메시지 출력
			if (ZCharacterSelectView::GetNumOfCharacter() <= 0)
			{
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_NOCHARACTER);
			}

			// 캐릭터 하나 선택
			if ( ZCharacterSelectView::GetNumOfCharacter() > 0)
			{
				ZCharacterSelectView::SetSelectedCharacterFromLastChar();
				ZApplication::GetGameInterface()->ChangeSelectedChar( ZCharacterSelectView::GetSelectedCharacter() );
			}

			// 캐릭터 리스트가 날라오면 CHARSELECTION으로 STATE변환
			ZApplication::GetGameInterface()->SetState(GUNZ_CHARSELECTION);
		}
		break;
	case MC_MATCH_RESPONSE_ACCOUNT_CHARINFO:
		{
			char nCharNum;
			pCommand->GetParameter(&nCharNum, 0, MPT_CHAR);

			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pCharInfoBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pCharInfoBlob);

			ZCharacterSelectView::OnReceivedCharInfo(nCharNum, pCharInfoBlob);

			// 캐릭터 보이기
			if ( ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZApplication::GetGameInterface()->GetCharacterSelectView()->SelectChar(nCharNum);
			}
		}
		break;
	case MC_MATCH_RESPONSE_SELECT_CHAR:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) 
				{
					_ASSERT(0);
					break;
				}

				void* pCharBlob = pParam->GetPointer();
				MTD_CharInfo* pCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharBlob, 0);

				ZCharacterSelectView::SetLastChar(pCharInfo->szName);

				ZGetMyInfo()->InitCharInfo(pCharInfo->szName, pCharInfo->szClanName,
					pCharInfo->nClanGrade, (MMatchSex)pCharInfo->nSex, 
					                 (int)pCharInfo->nHair, (int)pCharInfo->nFace);
				ZGetMyInfo()->GetItemList()->SetEquipItemID(pCharInfo->nEquipedItemDesc);
				ZGetMyInfo()->SetBP(pCharInfo->nBP);
				ZGetMyInfo()->SetXP((int)pCharInfo->nXP);
				ZGetMyInfo()->SetLevel((int)pCharInfo->nLevel);

				pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB)
				{
					_ASSERT(0);
					break;
				}
				void* pExtraCharBlob = pParam->GetPointer();
				MTD_MyExtraCharInfo* pMyExtraCharInfo = (MTD_MyExtraCharInfo*)MGetBlobArrayElement(pExtraCharBlob, 0);

				if (pMyExtraCharInfo)
				{
					ZGetMyInfo()->SetLevelPercent((int)pMyExtraCharInfo->nLevelPercent);
				}

				// if playing a clanwar type server decide if go to clan channel upon login
				if ( ( (ZGetGameClient()->GetServerMode() != MSM_CLAN) && (ZGetGameClient()->GetServerMode() != MSM_TEST) ) || (!ZGetMyInfo()->IsClanJoined()) )
				{
					ZPostRequestRecommendChannel();
				}
				else
				{
					ZPostChannelRequestJoinFromChannelName(ZGetGameClient()->GetPlayerUID(), 
						MCHANNEL_TYPE_CLAN, ZGetMyInfo()->GetClanName());
				}
				ZPostUserOption();
			}
		}
		break;
	case MC_MATCH_RESPONSE_CREATE_CHAR:
		{
			int nResult;
			char szCharName[256];
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			pCommand->GetParameter(szCharName, 1, MPT_STR, sizeof(szCharName) );
			if (nResult == MOK)
			{
				ZCharacterSelectView::SetLastChar(szCharName);

				ZApplication::GetGameInterface()->ChangeToCharSelection();
			}
			else 
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;
	case MC_MATCH_RESPONSE_DELETE_CHAR:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == 0)
			{
				ZApplication::GetGameInterface()->ChangeToCharSelection();

				ZApplication::GetGameInterface()->ShowMessage( MSG_HAS_DELETED_CHAR );
			}
			else
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;
	case MC_MATCH_RESPONSE_EQUIP_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
		}
		break;
	case MC_MATCH_RESPONSE_TAKEOFF_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
		}
		break;
	case MC_MATCH_RESPONSE_SHOP_ITEMLIST:
		{
			u32* pnItemList;
			int nItemCount = 0;


			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) 
			{
				_ASSERT(0);
				break;
			}
			void* pItemListBlob = pParam->GetPointer();
			nItemCount = MGetBlobArrayCount(pItemListBlob);
			pnItemList = new u32[nItemCount];


			for (int i = 0; i < nItemCount; i++)
			{
				pnItemList[i] = *(u32*)MGetBlobArrayElement(pItemListBlob, i);
			}

			ZApplication::GetGameInterface()->OnResponseShopItemList(pnItemList, nItemCount);

			delete[] pnItemList;
		}
		break;
	case MC_MATCH_RESPONSE_CHARACTER_ITEMLIST:
		{
			int nBounty = 0;
			MUID uidEquipItems[MMCIP_END];
			int nItemCount = 0, nEquipItemCount = 0;

			pCommand->GetParameter(&nBounty, 0, MPT_INT);
			ZGetMyInfo()->SetBP(nBounty);

			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) 
			{
				_ASSERT(0);
				break;
			}
			void* pEquipItemListBlob = pParam->GetPointer();
			nEquipItemCount = MGetBlobArrayCount(pEquipItemListBlob);
			if (nEquipItemCount != MMCIP_END)
			{
				_ASSERT(0);
				break;
			}

			for (int i = 0; i < MMCIP_END; i++)
			{
				uidEquipItems[i] = *(MUID*)MGetBlobArrayElement(pEquipItemListBlob, i);
			}

			pParam = pCommand->GetParameter(2);
			void* pItemListBlob = pParam->GetPointer();
			nItemCount = MGetBlobArrayCount(pItemListBlob);
			std::unique_ptr<MTD_ItemNode[]> pItemNodes{ new MTD_ItemNode[nItemCount] };

			for (int i = 0; i < nItemCount; i++)
				pItemNodes[i] = *reinterpret_cast<MTD_ItemNode*>(MGetBlobArrayElement(pItemListBlob, i));

			ZApplication::GetGameInterface()->OnResponseCharacterItemList(uidEquipItems,
				pItemNodes.get(),
				nItemCount);
		}
		break;
	case MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			void* pItemListBlob = pParam->GetPointer();
			int nItemCount = MGetBlobArrayCount(pItemListBlob);

			MTD_AccountItemNode* pItemNodes = new MTD_AccountItemNode[nItemCount];

			ZGetMyInfo()->GetItemList()->ClearAccountItems();

			for (int i = 0; i < nItemCount; i++)
			{
				pItemNodes[i] = *(MTD_AccountItemNode*)MGetBlobArrayElement(pItemListBlob, i);
				ZGetMyInfo()->GetItemList()->AddAccountItem(pItemNodes[i].nAIID, pItemNodes[i].nItemID, pItemNodes[i].nRentMinutePeriodRemainder);
			}

			delete[] pItemNodes;

			ZGetMyInfo()->GetItemList()->SerializeAccountItem();
		}
		break;
	case MC_MATCH_RESPONSE_BRING_ACCOUNTITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZGetMyInfo()->GetItemList()->ClearAccountItems();
				ZGetMyInfo()->GetItemList()->SerializeAccountItem();

				ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_BRINGITEM);
			}
			else 
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_NOTBRINGITEM);
			}

			
		}
		break;
	case MC_MATCH_RESPONSE_BUY_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_BUYITEM);
			}
			else if ((nResult == MERR_TOO_EXPENSIVE_BOUNTY) || (nResult == MERR_TOO_MANY_ITEM))
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_NOTBUYITEM);
			}
		}
		break;
	case MC_MATCH_RESPONSE_SELL_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);
			}
			else
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;
	case MC_MATCH_RESPONSE_STAGE_JOIN:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
			ZApplication::GetGameInterface()->EnableLobbyInterface(true);
		}
		break;
	case MC_MATCH_STAGE_REQUIRE_PASSWORD:
		{
			MUID uidStage = MUID(0,0);
			char szStageName[256];
			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			pCommand->GetParameter(szStageName, 1, MPT_STR, sizeof(szStageName) );

			auto* pRoomList = (ZRoomListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Lobby_StageList" );
			if ( pRoomList != NULL ) pRoomList->SetPrivateStageUID(uidStage);

			ZApplication::GetGameInterface()->ShowPrivateStageJoinFrame(szStageName);
		}
		break;
	case MC_MATCH_RESPONSE_STAGE_CREATE:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}

			ZApplication::GetGameInterface()->EnableLobbyInterface(true);
		}
		break;
	case MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			void* pMySimpleCharInfoBlob = pParam->GetPointer();

			MTD_MySimpleCharInfo* pCharInfo;
			pCharInfo = (MTD_MySimpleCharInfo*)MGetBlobArrayElement(pMySimpleCharInfoBlob, 0);

			ZGetMyInfo()->SetLevel(pCharInfo->nLevel);
			ZGetMyInfo()->SetXP(pCharInfo->nXP);
			ZGetMyInfo()->SetBP(pCharInfo->nBP);
		}
		break;
	case MC_MATCH_STAGE_RESPONSE_QUICKJOIN:
		{
			if (ZApplication::GetGameInterface()->GetState() == GUNZ_LOBBY)
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult != MOK)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
				}

				MUID uidStage;
				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				ZPostRequestStageJoin(ZGetGameClient()->GetPlayerUID(), uidStage);
			}
		}
		break;

#ifdef _QUEST_ITEM
	case MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (!pParam) break;
			void* pQuestItemBlob = pParam->GetPointer();
			if (!pQuestItemBlob) break;
			int	nQuestItemCount	= MGetBlobArrayCount(pQuestItemBlob);
			if (nQuestItemCount == 0) break;
			std::unique_ptr<MTD_QuestItemNode[]> QuestItemNodes{ new MTD_QuestItemNode[nQuestItemCount] };

			for(int i = 0; i < nQuestItemCount; ++i)
			{
				QuestItemNodes[i] =
					*(reinterpret_cast<MTD_QuestItemNode*>(MGetBlobArrayElement(pQuestItemBlob, i)));
			}

			ZGetGameInterface()->OnResponseCharacterItemList_QuestItem(QuestItemNodes.get(),
				nQuestItemCount);
		}
		break;

	case MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM:
		{
			int		nResult;
			MUID	uidRequester;
			int		nSlotIndex;
			int		nItemID;

			if (!pCommand->GetParameter(&nResult, 0, MPT_INT)) return false;
			if (!pCommand->GetParameter(&uidRequester, 1, MPT_UID)) return false;
			if (!pCommand->GetParameter(&nSlotIndex, 2, MPT_INT)) return false;
			if (!pCommand->GetParameter(&nItemID, 3, MPT_INT)) return false;

			ZApplication::GetStageInterface()->OnResponseDropSacrificeItemOnSlot(
				nResult, uidRequester,
				nSlotIndex, nItemID);
		}
		break;

	case  MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM:
		{
			int		nResult;
			MUID	uidRequester;
			int		nSlotIndex;
			int		nItemID;

			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &uidRequester, 1, MPT_UID );
			pCommand->GetParameter( &nSlotIndex, 2, MPT_INT );
			pCommand->GetParameter( &nItemID, 3, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseCallbackSacrificeItem( nResult, uidRequester, nSlotIndex, nItemID );
		}
		break;

	case MC_MATCH_RESPONSE_BUY_QUEST_ITEM :
		{
			int nResult;
			int nBP;
            
			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &nBP, 1, MPT_INT );

			ZApplication::GetGameInterface()->OnResponseBuyQuestItem( nResult, nBP );
		}
		break;

	case MC_MATCH_RESPONSE_SELL_QUEST_ITEM :
		{
			int nResult;
			int nBP;
			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &nBP, 1, MPT_INT );

			ZApplication::GetGameInterface()->OnResponseSellQuestItem( nResult, nBP );

		}
		break;

	case MC_QUEST_RESPONSE_QL :
		{
			int nQL;
			pCommand->GetParameter( &nQL, 0, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseQL( nQL );
		}
		break;

	case MC_MATCH_RESPONSE_SLOT_INFO :
		{
			MUID	uidOwner1;
			MUID	uidOwner2;
			int		nItemID1;
			int		nItemID2;

			pCommand->GetParameter( &uidOwner1, 0, MPT_UID );
			pCommand->GetParameter( &nItemID1, 1, MPT_INT );
			pCommand->GetParameter( &uidOwner2, 2, MPT_UID );
			pCommand->GetParameter( &nItemID2, 3, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseSacrificeSlotInfo( uidOwner1, nItemID1, uidOwner2, nItemID2 );
		}
		break;

	case MC_GAME_START_FAIL :
	{
		int nType;
		int nState;

		pCommand->GetParameter(&nType, 0, MPT_INT);
		pCommand->GetParameter(&nState, 1, MPT_INT);

		if( ALL_PLAYER_NOT_READY == nType )
		{
			ZGetGameInterface()->PlayVoiceSound( VOICE_PLAYER_NOT_READY, 1500);
		}
		else if( QUEST_START_FAILED_BY_SACRIFICE_SLOT == nType )
		{
			ZApplication::GetStageInterface()->OnQuestStartFailed( nState );

			ZGetGameInterface()->PlayVoiceSound( VOICE_QUEST_START_FAIL, 2800);
		}

		// Stage UI Enable
		ZApplication::GetStageInterface()->ChangeStageEnableReady( false);
	}
		break;

	case MC_QUEST_STAGE_GAME_INFO :
		{
			char nQL, nMapsetID;
			unsigned int nScenarioID;
			pCommand->GetParameter( &nQL,			0, MPT_CHAR );
			pCommand->GetParameter( &nMapsetID,		1, MPT_CHAR );
			pCommand->GetParameter( &nScenarioID,	2, MPT_UINT );

			ZApplication::GetStageInterface()->OnResponseQL( nQL );
			ZApplication::GetStageInterface()->OnStageGameInfo((int)nQL, (int)nMapsetID, nScenarioID);
		}
		break;

	case MC_MATCH_VOTE_STOP :
		{
			ZApplication::GetStageInterface()->OnStopVote();
		}
		break;

#endif // _QUEST_ITEM

#ifdef _LOCATOR
	case MC_RESPONSE_SERVER_LIST_INFO :
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) 
				return false;

			void* pBlob = pParam->GetPointer();
			if( 0 == pBlob ) 
				return false;

			int nCount = MGetBlobArrayCount(pBlob);

			if( 0 < nCount )
				ZApplication::GetGameInterface()->OnResponseServerStatusInfoList( nCount, pBlob );
		}
		break;

	case MC_RESPONSE_BLOCK_COUNTRY_CODE_IP :
		{
			char szCountryCode[ 8 ];
			char szRoutingURL[ 65 ];
			pCommand->GetParameter( szCountryCode, 0, MPT_STR, 7 );
			pCommand->GetParameter( szRoutingURL, 1, MPT_STR, 65 );

			if( 3 > strlen(szCountryCode) )
				return false;

			ZApplication::GetGameInterface()->OnResponseBlockCountryCodeIP( szCountryCode, szRoutingURL );
		}
		break;

	case MC_RESPONSE_BLOCK_COUNTRYCODE :
		{
			int test = 0;
		}
		break;
#endif

	case MC_MATCH_DISCONNMSG :
		{
			DWORD dwMsgID;
			pCommand->GetParameter( &dwMsgID, 0, MPT_UINT );

			ZApplication::GetGameInterface()->OnDisconnectMsg( dwMsgID );
		}
		break;

	case MC_MATCH_CLAN_ACCOUNCE_DELETE :
		{
			char szDeleteDate[ 128 ] = {0,};
			pCommand->GetParameter( szDeleteDate, 0, MPT_STR, 128 );

			ZApplication::GetGameInterface()->OnAnnounceDeleteClan( szDeleteDate );
		}
		break;

	default:
#ifdef _DEBUG
		mlog("ZGameInterface::OnCommand -- Hit default case for command ID %d\n",
			pCommand->GetID());
#endif
		break;

	}

	return false;
}