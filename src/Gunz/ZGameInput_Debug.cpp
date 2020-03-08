#include "stdafx.h"
#include "ZGameInput.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "ZModule_ElementalDamage.h"
#include "ZModule_Skills.h"
#include "ZWorldItem.h"
#include "ZMapDesc.h"
#include "ZGlobal.h"
#include "ZScreenEffectManager.h"
#include "ZInput.h"
#include "RSolidBsp.h"

extern int  g_debug_render_mode;
extern bool g_bVertex_Soft; 


bool ZGameInput::OnDebugEvent(MEvent* pEvent)
{
#ifdef _PUBLISH

	return false;
#endif

	static bool bMusicMute = false;

	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		{
			switch (pEvent->nKey)
			{
			case VK_END:
				{
					ZGetGameInterface()->m_bTeenVersion = !ZGetGameInterface()->m_bTeenVersion;
				}
				return true;
			case VK_INSERT:
				{
					g_debug_render_mode++;
					if(g_debug_render_mode > 3)
						g_debug_render_mode = 0;
				}
				return true;
			case VK_DELETE:
				{
					g_bVertex_Soft = !g_bVertex_Soft;
				}
				return true;
			case VK_F9:
				{
					static int nIndex = 0;
					nIndex++; if (nIndex >= 2) nIndex = 0;

					if (nIndex == 0)
					{
						ZGetGameInterface()->m_bViewUI = true;
						ZGetGameInterface()->GetGame()->m_pMyCharacter->SetVisible(true);

					}
					else if (nIndex == 1)
					{
						ZGetGameInterface()->m_bViewUI = false;
						g_pGame->m_pMyCharacter->SetVisible(false);
						ZGetGameInterface()->GetCombatInterface()->ShowCrossHair(false);
					}
				}
				return false;

			case VK_F8:
				{
					RSolidBspNode::m_bTracePath = !RSolidBspNode::m_bTracePath;
				}break;

			case 'U': {

				ZC_ENCHANT zctype;

				if(g_pGame && g_pGame->m_pMyCharacter ) {

					zctype = g_pGame->m_pMyCharacter->GetEnchantType();

					if(zctype==ZC_ENCHANT_FIRE) {
						ZModule_FireDamage *pModule = (ZModule_FireDamage*)g_pGame->m_pMyCharacter->GetModule(ZMID_FIREDAMAGE);
						pModule->BeginDamage(g_pGame->m_pMyCharacter->GetUID(),5,10);
					}
					else if(zctype==ZC_ENCHANT_COLD) {
						ZModule_ColdDamage *pModule = (ZModule_ColdDamage*)g_pGame->m_pMyCharacter->GetModule(ZMID_COLDDAMAGE);
						pModule->BeginDamage(10,50);
					}
					else if(zctype==ZC_ENCHANT_LIGHTNING) {
						ZModule_LightningDamage *pModule = (ZModule_LightningDamage*)g_pGame->m_pMyCharacter->GetModule(ZMID_LIGHTNINGDAMAGE);
						pModule->BeginDamage(g_pGame->m_pMyCharacter->GetUID(),5,10);
					}
					else if(zctype==ZC_ENCHANT_POISON) {
						ZModule_PoisonDamage *pModule = (ZModule_PoisonDamage*)g_pGame->m_pMyCharacter->GetModule(ZMID_POISONDAMAGE);
						pModule->BeginDamage(g_pGame->m_pMyCharacter->GetUID(),5,10);
					}
				}

				//g_pGame->m_pMyCharacter->ShotBlocked();
//				ZApplication::GetSoundEngine()->StopMusic();
				//ZApplication::GetSoundEngine()->load_preset();
					  }break;

			case 'M':
				{
					if (ZApplication::GetInstance()->GetLaunchMode()==ZApplication::ZLAUNCH_MODE_STANDALONE_GAME)
					{

						// 혼자테스트할때 되살아나기
						if (g_pGame->GetMatch()->IsTeamPlay())
						{
							ZCharacter* pCharacter = g_pGame->m_pMyCharacter;
							pCharacter->InitStatus();
							rvector pos=rvector(0,0,0), dir=rvector(0,1,0);

							static int nTeamIndex = 0;
							static int nSpawnIndex = 0;

							ZMapSpawnData* pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetTeamData(nTeamIndex, nSpawnIndex);
							if (pSpawnData != NULL)
							{
								pos = pSpawnData->m_Pos;
								dir = pSpawnData->m_Dir;
							}

							pCharacter->SetPosition(pos);
							pCharacter->SetDirection(dir);

							nSpawnIndex++;
							if (nSpawnIndex >= 16) 
							{
								nSpawnIndex = 0;
								nTeamIndex++;
								if (nTeamIndex >= 2) nTeamIndex=0;
							}
						}
						else
						{
							if(g_pGame->m_CharacterManager.size()==1)
								ZGetGameInterface()->RespawnMyCharacter();
						}
					}

				}break;

			case 'C' : {
//				ZModule_Skills *pmod = (ZModule_Skills *)g_pGame->m_pMyCharacter->GetModule(ZMID_SKILLS);
//				pmod->Excute(0,MUID(0,0),rvector(0,0,0));
					
//				g_pGame->UpdateCombo(true);


					   }break;

			case 'G' : {
//						MNewMemories::Dump();
						//g_pGame->m_pMyCharacter->m_bGuardTest=!g_pGame->m_pMyCharacter->m_bGuardTest;
					   }break;

				// 테스트용..
			case 'I' : g_pGame->m_pMyCharacter->AddIcon(rand()%5); return true;
				// 테스트^^

			case 'F' : 
				{
					static bool toggle = false;
					if(toggle)
						ZGetInput()->SetDeviceForcesXY(1,1);
					else
						ZGetInput()->SetDeviceForcesXY(0,0);
					toggle=!toggle;
					return true;
				}
			case 'L' :
				{
//					rvector pos = g_pGame->m_pMyCharacter->GetPosition();
//					pos.x += 1000.0f;
//					ZApplication::GetSoundEngine()->PlayNPCSound(NPC_GOBLIN, NPC_SOUND_WOUND, pos, true);

					ZApplication::GetGameInterface()->FinishGame();
//					ZGetScreenEffectManager()->AddScreenEffect("teamredwin");

					/*
					g_pGame->m_pMyCharacter->m_Position = rvector( 2712.99805 , -1691.46191 , 2649.13403 );
					g_pGame->m_pMyCharacter->Move(rvector( 2.561 , -7.040 , -6.471 ));
					*/

					/*
					g_pGame->m_pMyCharacter->m_Position = rvector( 2713.05347 , -1691.56250 , 2929.06738 );
					g_pGame->m_pMyCharacter->Move(rvector( 0.00000000 , 0.00000000 , -2.07031250 ));
					*/

					/*
					g_pGame->m_pMyCharacter->m_Position = rvector( 1648.73877 ,8691.30176 ,1501.03381 -120);
					g_pGame->m_pMyCharacter->Move(rvector( -0.134,0.004, -0.986 ));
					g_pGame->m_pMyCharacter->m_Velocity = rvector( -450,0,0);
					
					ZPostLocalMessage(MSG_HACKING_DETECTED);
					

					ZGetEffectManager()->AddShotgunEffect(rvector(0,0,100),rvector(0,0,100),rvector(0,1,0),g_pGame->m_pMyCharacter);
					*/
				}break;

			case 'K':
				{
//					rvector pos = g_pGame->m_pMyCharacter->GetPosition();
//					pos.x += 1000.0f;
//					ZApplication::GetSoundEngine()->PlayNPCSound(NPC_GOBLIN, NPC_SOUND_WOUND, pos, false);

//					ZGetGameInterface()->GetCamera()->Shock(2000.f, .5f, rvector(0.0f, 0.0f, -1.0f));
					ZGetScreenEffectManager()->ShockBossGauge(35.0f);

					static int n = 0; n++;
					ZGetScreenEffectManager()->AddKO(n);

//					g_pGame->m_pMyCharacter->OnBlast(rvector(1,0,0));
//					ZGetGameInterface()->GetCamera()->Shock(500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
				}break;
			case 'B':
				{
					// test
					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					pCombatInterface->SetObserverMode(!pCombatInterface->GetObserver()->IsVisible());
				}
				return true;
			case 'N':
				{
					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					if(pCombatInterface->GetObserverMode())
						pCombatInterface->GetObserver()->ChangeToNextTarget();
				}
				return true;
			case 'P':
				{
					// test
//					g_pGame->m_pMyCharacter->OnKnockback(const rvector(1.0f, 0.0f, 0.0f),200.f);
					ZGetEffectManager()->AddMethorEffect(rvector(0,0,0) , 1 );

				}
				return true;
			case 'O':
				{
					if (ZApplication::GetInstance()->GetLaunchMode()==ZApplication::ZLAUNCH_MODE_STANDALONE_AI)
					{
						ZGetObjectManager()->ClearNPC();
						// npc 생성 test
						MUID uidNPC = MUID(0,0);
						uidNPC.High = rand() % RAND_MAX;
						uidNPC.Low = rand() % RAND_MAX;

						int nNPCType = rand() % NPC_GOBLIN_KING+1;

						nNPCType = NPC_GOBLIN_GUNNER;
						rvector ranpos = rvector(0, 0, 0);

						MQuestNPCInfo* pNPCInfo = NULL;

						if(ZGetQuest())
							pNPCInfo = 	ZGetQuest()->GetNPCInfo(MQUEST_NPC(nNPCType));

						ZActor* pNewActor = ZActor::CreateActor(MQUEST_NPC(nNPCType), 1.0f, 0);
						if (pNewActor)
						{
							pNewActor->SetUID(uidNPC);
							pNewActor->SetPosition(ranpos);
							pNewActor->SetMyControl(true);

							if(pNewActor->m_pVMesh && pNPCInfo) {

								D3DCOLORVALUE color;

								color.r = pNPCInfo->vColor.x;
								color.g = pNPCInfo->vColor.y;
								color.b = pNPCInfo->vColor.z;
								color.a = 1.f;

								pNewActor->m_pVMesh->SetNPCBlendColor(color);//색을 지정한 경우..
							}

							ZGetObjectManager()->Add(pNewActor);
							ZGetEffectManager()->AddReBirthEffect(ranpos);
						}
					}
				}
				return true;
			case VK_F1: 

				//	도움말을 보여준다.테스트후 제거..
				//---------------------------------------------------------
/*
				if( g_pGame )
					g_pGame->m_HelpScreen.ChangeMode();
				break;
*/
				g_pGame->m_pMyCharacter->SetVisible(!g_pGame->m_pMyCharacter->IsVisible());
				break;

				//---------------------------------------------------------

			case VK_F2: 
				{
					ZGetGameInterface()->ShowInterface(
						!ZGetGameInterface()->IsShowInterface());

				}
				break;
			case VK_F3:
				if (ZIsLaunchDevelop())
				{
					ZApplication::GetGameInterface()->GetScreenDebugger()->SwitchDebugInfo();
				}
				return true;

			case VK_F4: g_pGame->m_bShowWireframe=!g_pGame->m_bShowWireframe;
				return false;
			case VK_F5: {
				m_bCTOff = !m_bCTOff;
				RMesh::SetTextureRenderOnOff(m_bCTOff);
						}
						return true;
			case VK_F7: 
				{
					extern bool g_bProfile;
					if(g_bProfile)
					{
						ZPOSTCMD0(ZC_END_PROFILE);
						ZChatOutput("Profile saved.");
					}
					else
					{
						ZPOSTCMD0(ZC_BEGIN_PROFILE);
						ZChatOutput("Profile started.");
					}
				}return true;
			case VK_NUMPAD1:
				ZGetGameInterface()->TestChangeParts(0);
				return true;
			case VK_NUMPAD2:
				ZGetGameInterface()->TestChangeParts(1);
				return true;
			case VK_NUMPAD3:
				ZGetGameInterface()->TestChangeParts(2);
				return true;
			case VK_NUMPAD4:
				ZGetGameInterface()->TestChangeParts(3);
				return true;
			case VK_NUMPAD5:
				ZGetGameInterface()->TestChangeParts(4);
				return true;
			case VK_NUMPAD6:
				ZGetGameInterface()->TestChangeParts(5);
				return true;
			case VK_NUMPAD7:
				ZGetGameInterface()->TestChangeWeapon();
				return true;
			case VK_NUMPAD0:
				ZGetGameInterface()->TestToggleCharacter();
				return true;
			case VK_NUMPAD9:
				bMusicMute = !bMusicMute;
				ZGetSoundEngine()->SetMusicMute(bMusicMute);
				return true;
#ifdef USING_VERTEX_SHADER
			case 'V':
				RShaderMgr::mbUsingShader = !RShaderMgr::mbUsingShader;
				return false;
#endif			

			}

		}
		break;

	}

	return false;
}

