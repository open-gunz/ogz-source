#include "stdafx.h"
#include "ZCommandTable.h"
#include "MCommandRegistration.h"

void ZAddCommandTable(MCommandManager* CommandManager)
{
	BEGIN_CMD_DESC(CommandManager, 0);

	C(ZC_CON_CLEAR, "Con.Clear", "Clear Console", MCDT_LOCAL);
	C(ZC_CON_SIZE, "Con.Size", "Resize Console", MCDT_LOCAL);
		P(MPT_INT, "width");
		P(MPT_INT, "height");
	C(ZC_CON_HIDE, "Con.Hide", "Hide Console", MCDT_LOCAL);

	C(ZC_CON_CONNECT, "con", "Test Command for connection", MCDT_LOCAL);
	C(ZC_CON_DISCONNECT, "dis", "Test Command for disconnection", MCDT_LOCAL);

	C(ZC_TEST_INFO, "t", "Get Test Info", MCDT_LOCAL);

#ifdef _DEBUG
	C(ZC_TEST_SETCLIENT1, "sc1", "Set Client1", MCDT_LOCAL);
	C(ZC_TEST_SETCLIENT2, "sc2", "Set Client1", MCDT_LOCAL);
	C(ZC_TEST_SETCLIENT3, "sc3", "Set Client1", MCDT_LOCAL);

	C(ZC_TEST_SETCLIENTALL, "scall", "Set Client All", MCDT_LOCAL);
		P(MPT_STR, "MyIP");
#endif

#ifndef _PUBLISH
	C(ZC_TEST_BIRD1, "bird", "Bird Local Test", MCDT_LOCAL);
#endif

	C(ZC_BEGIN_PROFILE, "bp" , "Begin Profile", MCDT_LOCAL);
	C(ZC_END_PROFILE, "ep" , "End Profile", MCDT_LOCAL);




	C(ZC_CHANGESKIN, "ChangeSkin", "Change Interface Skin", MCDT_LOCAL);
		P(MPT_STR, "SkinName");
	C(ZC_REPORT_119, "Report119", "Report 119", MCDT_LOCAL);
	C(ZC_MESSAGE, "Message", "Message", MCDT_LOCAL);
		P(MPT_INT, "nMessageID");
	C(ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM, "Local.Event.Optain.Special.WorldItem", "Event Optain Special WorldItem", MCDT_LOCAL);
		P(MPT_INT, "WorldItemID");


	C(MC_QUEST_NPC_LOCAL_SPAWN,	"Quest.NPCLocalSpawn", "Npc Local Spawn", MCDT_LOCAL);
		P(MPT_UID,		"uidChar");
		P(MPT_UID,		"nNpcUID");
		P(MPT_UCHAR,	"nNpcType");
		P(MPT_UCHAR,	"PositionIndex");
}
