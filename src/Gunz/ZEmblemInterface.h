#pragma once

#include <string>
#include <map>
#include "MMatchGlobal.h"
#include "MBitmap.h"

struct ZEmblemInfoNode final
{
	u64			m_nClanID{};
	u32			m_nNumOfClan{};
	MBitmap*	m_pBitmapEmblem{};

	~ZEmblemInfoNode() {
		SAFE_DELETE(m_pBitmapEmblem);
	}
};

using EmblemInfoMapList = std::map<int, ZEmblemInfoNode>;


class ZEmblemInterface
{
public:
	~ZEmblemInterface();

	void Create();
	void Destroy();

	bool AddClanInfo(UINT nClanID);
	bool DeleteClanInfo(UINT nClanID);
	bool ClearClanInfo();
	bool ReloadClanInfo(UINT nClanID);

	bool ZEmblemInterface::FindClanInfo(UINT nClanID, EmblemInfoMapList::iterator* pIterator);

	MBitmap* GetClanEmblem(UINT nClanID);

	EmblemInfoMapList	m_EmblemInfoMap;
	MBitmap*			m_pBitmapNoEmblem;
};