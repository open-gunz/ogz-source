#ifndef _ZTIPS_H
#define _ZTIPS_H

#include "MZFileSystem.h"
#include <string>
#include <vector>
#include <list>
using namespace std;

enum ZTIPS_CATEGORY
{
	ZTIPS_CATEGORY_NORMAL		=0,		// 일반
	ZTIPS_CATEGORY_SOLO			=1,		// 개인전
	ZTIPS_CATEGORY_TEAM			=2,		// 팀전
	ZTIPS_CATEGORY_CLANSERVER	=3,		// 클랜전
	ZTIPS_CATEGORY_NEWBIE		=4,		// 왕초보
	ZTIPS_CATEGORY_BEGINNER		=5,		// 초보
	ZTIPS_CATEGORY_ROOKIE		=6,		// 중수
	ZTIPS_CATEGORY_MASTERY		=7,		// 고수

	ZTIPS_CATEGORY_END
};

class ZTips
{
private:
	// type
	struct ZTIPS_MSG
	{
		int		nMSGID;
		int		nVectorIndex;
	};

	// data
	vector<string>		m_StringVector;
	vector<ZTIPS_MSG>	m_MsgsVectors[ZTIPS_CATEGORY_END];

	bool		m_bIsShowedNewbieTips;	// 왕초보용 메세지를 이미 보여줬는지 여부. 이게 여기있는게 적당한건지 잘 모르겠음. -_-ㅋ

	// func
	void ParseTips(MXmlElement* pElement);
public:
	ZTips();
	~ZTips() { }
	bool Initialize(MZFileSystem *pfs, const MLanguage LangID );
	void Finalize();
	const char* GetRandomTips();
	const char* GetTips(ZTIPS_CATEGORY nCategory, int nID=-1);

	bool IsShowedNewbieTips()		{ return m_bIsShowedNewbieTips; }
	void SetShowedNewbieTips(bool bShowed)	{ m_bIsShowedNewbieTips = bShowed; }
};



#define ZTIPMSG_ID_NEWBIE_SHOW_HELP				1



#endif