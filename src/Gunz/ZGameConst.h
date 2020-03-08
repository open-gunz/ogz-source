#ifndef _ZGAME_CONST_H
#define _ZGAME_CONST_H


// 게임에서 쓰이는 상수값 모음
#define GRAVITY_CONSTANT			2500.f			// 중력의 영향
#define MAX_FALL_SPEED				3000.f			// 최대 낙하속도
#define POS_TOLER					50.0f

#define PEER_HP_TICK				1000	// 1.0 초마다 HP 투표에 관한 메시지를 보낸다
#define PEER_PING_TICK				3000	// 1.0 초마다 Ping 체크에 관한 메시지를 보낸다
#define MAX_WATER_DEEP				150

//#define DIE_CRITICAL_LINE			(-2500.0f)
#define DIE_CRITICAL_LINE			(-2500.0f)
#define CHARACTER_HISTROY_COUNT		200



#endif