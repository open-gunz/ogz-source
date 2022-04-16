#ifndef _ZGLOBAL_H
#define _ZGLOBAL_H

//#include "RMesh.h"
#include "ZApplication.h"

// 전역적으로 사용하는 것은 이곳에다 넣도록 하자
//#ifdef LOCALE_BRAZIL
//#define APPLICATION_NAME		"The Duel"
//#else
//#define APPLICATION_NAME		"Gunz"
//#endif

#define APPLICATION_NAME "Open GunZ"

// 저장되는 폴더
//#ifdef LOCALE_BRAZIL
//#define GUNZ_FOLDER			"/The Duel"
//#else
//#define GUNZ_FOLDER			"/Gunz"
//#endif
#define GUNZ_FOLDER			"/Open GunZ"
#define SCREENSHOT_FOLDER	"/Screenshots"
#define REPLAY_FOLDER		"/Replay"
#define EMBLEM_FOLDER		"/Emblem"


class MZFileSystem;
class MMessenger;

class ZApplication;
class ZGameClient;
class ZSoundEngine;
class ZGameInterface;
class ZEffectManager;
class ZScreenEffectManager;
class ZDirectInput;
class ZCombatInterface;
class ZCamera;
class ZGame;
class ZQuest;
class ZGameTypeManager;
class ZWorldManager;
class ZMessengerManager;
class ZEmblemInterface;
class ZInput;

extern ZDirectInput	g_DInput;


bool					ZIsLaunchDevelop(void);

ZApplication*			ZGetApplication(void);
ZGameClient*			ZGetGameClient(void);
RMeshMgr*				ZGetNpcMeshMgr(void);
RMeshMgr*				ZGetMeshMgr(void);
RMeshMgr*				ZGetWeaponMeshMgr(void);
RAniEventMgr*			ZGetAniEventMgr(void);
ZSoundEngine*			ZGetSoundEngine(void);
ZGameInterface*			ZGetGameInterface(void);
ZCombatInterface*		ZGetCombatInterface(void);
ZEffectManager*			ZGetEffectManager(void);
ZScreenEffectManager*	ZGetScreenEffectManager(void);
MZFileSystem*			ZGetFileSystem(void);
ZDirectInput*			ZGetDirectInput(void);
ZCamera*				ZGetCamera(void);
ZGame*					ZGetGame(void);
ZQuest*					ZGetQuest(void);
ZGameTypeManager*		ZGetGameTypeManager(void);
ZInput*					ZGetInput(void);

inline ZEmblemInterface*	ZGetEmblemInterface() { return ZApplication::GetInstance()->GetEmblemInterface(); }
inline ZWorldManager*		ZGetWorldManager(void) { return ZApplication::GetInstance()->GetWorldManager(); }
inline ZWorld*				ZGetWorld(void)	{ return ZGetWorldManager()->GetCurrent(); }
inline ZOptionInterface*	ZGetOptionInterface(void) { return ZApplication::GetInstance()->GetOptionInterface(); }

// Returns true if the key is pressed down this frame.
#define ZIsActionKeyDown(_ActionID)	(ZGetInput()->IsActionKeyDown(_ActionID))
// Returns true if the key is not pressed down this frame.
#define ZIsActionKeyUp(_ActionID)	(!ZGetInput()->IsActionKeyDown(_ActionID))

// Returns true if the key was pressed down last frame.
#define ZWasActionKeyDownLast(_ActionID)	(ZGetInput()->WasActionKeyDownLast(_ActionID))
// Returns true if the key was not pressed down last frame.
#define ZWasActionKeyUpLast(_ActionID)	(!ZGetInput()->WasActionKeyDownLast(_ActionID))

// Returns true if the action key was just pressed down this frame, i.e. it was up last frame and
// down this frame.
#define ZWasActionKeyPressed(_ActionID) (ZWasActionKeyUpLast(_ActionID) && ZIsActionKeyDown(_ActionID))
// Returns true if the action key was just released this frame, i.e. it was down last frame and
// up this frame.
#define ZWasActionKeyReleased(_ActionID) (ZWasActionKeyDownLast(_ActionID) && ZIsActionKeyUp(_ActionID))


#endif