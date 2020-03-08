#ifndef RealSoundEffectPlayer_H
#define RealSoundEffectPlayer_H

#include <list>
#include "map"
#include "string"
using namespace std;

#include "RealSoundDef.h"

class RealSoundEffectPlay;
class RealSoundEffectSource;

typedef map<string, RealSoundEffectPlay*>		SNDBufList;
typedef SNDBufList::iterator								SNDB_Iter;

/// 소리를 내는 독립적인 객체를 위한 사운드 플레이어
/// 소리의 위치를 일괄적으로 바꿀 수 있다.
class RealSoundEffectPlayer
{
protected:
	list<RealSoundEffectPlay*>	m_SEs;
	float	m_fX, m_fY, m_fZ;

	SNDBufList	mSndBuffer;
	
public:
	RealSoundEffectPlayer(void);
	virtual ~RealSoundEffectPlayer(void);

	RealSoundEffectPlay* Play(RealSoundEffectSource* pSES, float x, float y, float z, bool bLoop=false, bool bDupCheck=false, float fDigVol=1.0f, bool b3D=true, float fMinDistance=10.0f, float fMaxDistance=1000.0f, RealSoundEffectMode nMode=RSEM_NORMAL);

	bool IsPlaying(void);

	void RemoveLoopPlay(void);

	void Move(RealSoundEffectPlayer* pSource);

	void SetPos(float x, float y, float z);
	void GetPos(float* x, float* y, float* z);

	void Run(void);

	int GetSoundEffectNum() { return (int)m_SEs.size();	}
};

#endif