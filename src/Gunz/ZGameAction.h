#ifndef _ZGAMEACTION_H
#define _ZGAMEACTION_H

#include "MCommand.h"

// 이 enum 은 게임 프로토콜과 관련이 있으므로 변경은 불가. 추가만 하자
enum ZREACTIONID{	// 내 반응을 다른사람에게 알린다
	ZR_CHARGING = 0,		// 기 모으기를 시작
	ZR_CHARGED,				// 기가 모엿다
	ZR_BE_UPPERCUT,			// 띄워졌다
	ZR_BE_DASH_UPPERCUT,	// 단검대쉬기술 맞았을때
	ZR_DISCHARGED,			// 기 모인게 풀렸다

	ZR_END
};

class ZCharacter;
class ZMyCharacter;

/// ZGame에서 캐릭터들간의 액션에 관한 것들(ZGame의 OnPeer씨리즈들)은 이곳으로 빼냅시다.
class ZGameAction
{
	bool OnReaction(MCommand* pCommand);
	bool OnPeerSkill(MCommand* pCommand);
	bool OnEnchantDamage(MCommand* pCommand);

private:
	void OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter);
	void OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter);	// 강베기 스플래시
	void OnPeerSkill_Dash(ZCharacter *pOwnerCharacter);

public:
	bool OnCommand(MCommand* pCommand);
};


#define CHARGED_TIME			15.f		// 강베기 지속시간 (단위:초)
#define COUNTER_CHARGED_TIME	1.f			// 반격기 강베기 지속시간


#endif