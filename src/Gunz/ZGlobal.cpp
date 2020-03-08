#include "stdafx.h"
#include "ZGlobal.h"
#include "ZApplication.h"
#include "ZQuest.h"

bool ZIsLaunchDevelop(void) { 
	return ZApplication::GetInstance()->IsLaunchDevelop(); 
}
bool ZIsLaunchTest(void) { 
	return ZApplication::GetInstance()->IsLaunchTest(); 
}

ZApplication* ZGetApplication(void) {
	return ZApplication::GetInstance();
}

ZGameClient* ZGetGameClient(void) { 
	return ZApplication::GetGameClient(); 
}

RMeshMgr* ZGetNpcMeshMgr(void) { 
	return ZApplication::GetNpcMeshMgr(); 
}

RMeshMgr* ZGetMeshMgr(void) { 
	return ZApplication::GetMeshMgr(); 
}

RMeshMgr* ZGetWeaponMeshMgr(void) { 
	return ZApplication::GetWeaponMeshMgr(); 
}

RAniEventMgr* ZGetAniEventMgr(void) {
	return ZApplication::GetAniEventMgr();
}

ZSoundEngine* ZGetSoundEngine(void) { 
	return ZApplication::GetSoundEngine(); 
}

ZGameInterface*	ZGetGameInterface(void) { 
	return ZApplication::GetGameInterface(); 
}

ZCombatInterface*	ZGetCombatInterface(void) { 
	return ZApplication::GetGameInterface() ? 
		ZApplication::GetGameInterface()->GetCombatInterface() : NULL;
}

ZCamera* ZGetCamera(void) {
	return ZApplication::GetGameInterface() ? 
		ZApplication::GetGameInterface()->GetCamera() : NULL;
}

ZEffectManager*	ZGetEffectManager(void) { 
//	return &g_pGame->m_EffectManager; 
	return ZGetGameInterface()->GetEffectManager(); 
}

ZScreenEffectManager* ZGetScreenEffectManager(void) { 
	return ZGetGameInterface()->GetScreenEffectManager(); 
}

MZFileSystem* ZGetFileSystem(void) { 
	return ZApplication::GetFileSystem(); 
}

ZDirectInput* ZGetDirectInput(void) { 
	return &g_DInput; 
}

ZGame* ZGetGame(void) {
	return ZApplication::GetGameInterface() ? 
		ZApplication::GetGameInterface()->GetGame() : NULL;
}

ZQuest* ZGetQuest(void)
{
	return (ZApplication::GetGameInterface()) ? 
		ZApplication::GetGameInterface()->GetQuest() : NULL;
}

ZGameTypeManager* ZGetGameTypeManager(void)
{
	return (ZApplication::GetGameInterface()) ? 
		ZApplication::GetGameInterface()->GetGameTypeManager() : NULL;
}

extern ZInput* g_pInput;	// TODO: Á¤¸®
ZInput*	ZGetInput(void)
{
	return g_pInput;
}
