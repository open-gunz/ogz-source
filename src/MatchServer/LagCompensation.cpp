#include "stdafx.h"
#include "LagCompensation.h"
#include "GlobalTypes.h"
#include "RealSpace2.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "MUtil.h"
#include "MMatchConfig.h"
#include "MMatchServer.h"
#include "RBspObject.h"

static auto Log = [](auto&&... Args) {
	MGetMatchServer()->LogF(MMatchServer::LOG_ALL, std::forward<decltype(Args)>(Args)...); };

bool LagCompManager::Create()
{
	using namespace RealSpace2;
	if (!MGetServerConfig()->HasGameData())
	{
		Log("game_dir is empty! Server-based netcode will be disabled.");
		return false;
	}

	const char* path = MGetServerConfig()->GetGameDirectory();
	g_pFileSystem = new MZFileSystem();
	
	if (!g_pFileSystem->Create(path))
	{
		Log("g_pFileSystem->Create failed!");
		return false;
	}

	bool ret = false;
	
	ret = LoadAnimations("model/man/man01.xml", 0);
	if (!ret)
	{
		Log("LoadAnimations0 failed!");
		return false;
	}
	Log("Loaded male animations");

	ret = LoadAnimations("model/woman/woman01.xml", 1);
	if (!ret)
	{
		Log("LoadAnimations1 failed!");
		return false;
	}
	Log("Loaded female animations");

	SetAnimationMgr(MMS_MALE, &AniMgrs[MMS_MALE]);
	SetAnimationMgr(MMS_FEMALE, &AniMgrs[MMS_FEMALE]);

	for (auto& Map : g_MapDesc)
	{
		char Path[128];
		sprintf_safe(Path, "maps/%s/%s.rs", Map.szMapName, Map.szMapName);
		ret = Maps[Map.szMapName].Open(Path, RBspObject::ROpenMode::Runtime, nullptr, nullptr, true);
		if (!ret)
			Log("Failed to load map %s!", Map.szMapName);
		else
			Log("Loaded map %s", Map.szMapName);
	}

	//for (int AniIdx = 0; AniIdx < ZC_STATE_LOWER_END; AniIdx++)
	//{
	//	auto& AniItem = g_AnimationInfoTableLower[AniIdx];
	//	Log("%s", AniItem.Name);
	//	auto Ani = AniMgrs[0].GetAnimation(AniItem.Name);

	//	if (!Ani)
	//	{
	//		Log("Can't find animation!");
	//		continue;
	//	}

	//	if (!Ani->m_pAniData)
	//	{
	//		Log("Can't find ani data!");
	//		continue;
	//	}

	//	Log("Max frame %d", Ani->m_pAniData->m_max_frame);

	//	/*for (int i = 0; i < Ani->m_pAniData->m_max_frame; i++)
	//	{
	//		auto v = GetHeadPosition(rvector(0, 0, 0), MMatchSex(0), ZC_STATE_LOWER(AniIdx), i);
	//		Log("Frame %d: %f, %f, %f", i, v.x, v.y, v.z);
	//	}*/
	//}

	return true;
}

bool LagCompManager::LoadAnimations(const char* filename, int Index)
{
	using namespace RealSpace2;
	auto& AniMgr = AniMgrs[Index];

	MXmlDocument	XmlDoc;
	MXmlElement		DocNode, Node;

	XmlDoc.Create();

	char Path[256];
	Path[0] = 0;

	GetPath(filename, Path);

	char *buffer;
	MZFile mzf;

	if (g_pFileSystem) {
		if (!mzf.Open(filename, g_pFileSystem)) {
			if (!mzf.Open(filename))
			{
				Log("mzf.Open failed with g_pFileSystem!");
				return false;
			}
		}
	}
	else {
		if (!mzf.Open(filename))
		{
			Log("mzf.Open failed without g_pFileSystem!");
			return false;
		}
	}

	buffer = new char[mzf.GetLength() + 1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer, mzf.GetLength());

	if (!XmlDoc.LoadFromMemory(buffer))
	{
		Log("XmlDoc.LoadFromMemory failed!");
		return false;
	}

	delete[] buffer;

	mzf.Close();

	//-------->

	DocNode = XmlDoc.GetDocumentElement();

	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char PathFileName[256];
	char MotionTypeID[256];
	char MotionLoopTypeID[256];
	char SoundFileName[256];
	char GameMotion[256];
	char PathSoundFileName[256];

	int nMTypeID = -1;
	bool bSoundMap = false;
	AnimationLoopType MLoopType = RAniLoopType_Loop;

	int nCnt = DocNode.GetChildNodeCount();

	for (int i = 0; i < nCnt; i++) {

		Node = DocNode.GetChildNode(i);

		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddAnimation") == 0) {

			SoundFileName[0] = 0;
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(MotionTypeID, "motion_type");
			Node.GetAttribute(MotionLoopTypeID, "motion_loop_type");
			Node.GetAttribute(SoundFileName, "sound");
			Node.GetAttribute(&bSoundMap, "soundmap", false);
			Node.GetAttribute(GameMotion, "gm");

			if (strcmp(MotionLoopTypeID, "normal") == 0) {
				MLoopType = RAniLoopType_Normal;
			}
			else if (strcmp(MotionLoopTypeID, "loop") == 0) {
				MLoopType = RAniLoopType_Loop;
			}
			else if (strcmp(MotionLoopTypeID, "onceidle") == 0) {
				MLoopType = RAniLoopType_OnceIdle;
			}
			else if (strcmp(MotionLoopTypeID, "lastframe") == 0) {
				MLoopType = RAniLoopType_HoldLastFrame;
			}
			else if (strcmp(MotionLoopTypeID, "onceLowerbody") == 0) {
				MLoopType = RAniLoopType_OnceLowerBody;
			}

			//MLog("Name: %s, filename: %s, motion_type %s, motion_loop_type %s\n", IDName, FileName, MotionTypeID, MotionLoopTypeID);

			int nGameMotion = atoi(GameMotion);

			//if (GetToolMesh()) // 툴에서 사용되는거라면 무조건 바로 로딩~
			//	nGameMotion = 0;

			nMTypeID = atoi(MotionTypeID);

			if (Path[0]) {
				strcpy_safe(PathFileName, Path);
				strcat_safe(PathFileName, FileName);
			}
			else
				strcpy_safe(PathFileName, FileName);

			RAnimation* pAni = 0;

			//MLog("AddAnimation %s, %s\n", IDName, PathFileName);

			//if (nGameMotion == 1) { // 게임 모션은 나중에 로딩
			//	pAni = AniMgr.AddGameLoad(IDName, PathFileName, -1, nMTypeID);
			//}
			//else
			{
				pAni = AniMgr.Add(IDName, PathFileName, -1, nMTypeID);
			}

			if (pAni) {

				pAni->SetAnimationLoopType(MLoopType);

				if (SoundFileName[0] == 0) {
					int len = (int)strlen(FileName);
					strncpy_safe(SoundFileName, FileName, len - 8);
					SoundFileName[len - 8] = 0;

					strcpy_safe(PathSoundFileName, "/sound/effect/");
					strcat_safe(PathSoundFileName, SoundFileName);
				}

				pAni->SetSoundFileName(SoundFileName);
				pAni->SetSoundRelatedToMap(bSoundMap);
			}
		}
	}

	XmlDoc.Destroy();

	return true;
}

RealSpace2::RBspObject * LagCompManager::GetBspObject(const char * MapName)
{
	auto it = Maps.find(MapName);
	if (it == Maps.end())
		return nullptr;

	return &it->second;
}