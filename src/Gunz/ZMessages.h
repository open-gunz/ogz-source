#pragma once

#include <string>
#include "MCommand.h"
#include "MZFileSystem.h"

#define ZOK										0			///< 에러없음
#define ZERR_UNKNOWN							-1			///< 알지 못하는 에러


// 클라이언트 에러메시지 관
#define MSG_WARNING								500			///< 주의
#define MSG_REROUTE_TO_WEBSITE					501			///< 주의


/// 클라이언트 인터페이스 관련
#define MSG_119_REPORT_WAIT_ONEMINUTE			1000		///< 1분후에 신고가능합니다.
#define MSG_119_REPORT_OK						1001		///< 119 신고하였습니다.
#define MSG_CANNOT_CHAT							1002		///< 채팅할 수 없습니다.
#define MSG_YOU_MUST_WRITE_MORE					1003		///< $1글자이상 글을 적어야 합니다.		// $1 - 글자수
#define MSG_LESS_ARGUMENT						1004		///< 인자가 부족합니다.
#define MSG_WRONG_ARGUMENT						1005		///< 인자가 틀렸습니다.
#define MSG_MUST_EXECUTE_LOBBY					1006		///< 로비에서만 실행가능합니다.
#define MSG_MUST_EXECUTE_STAGE					1007		///< 대기방에서만 실행가능합니다.
#define MSG_MUST_EXECUTE_GAME					1008		///< 게임에서만 실행가능합니다.
#define MSG_CANCELED							1009		///< 취소되었습니다.
#define MSG_CANNOT_CHAT_CMD						1010		///< 명령어를 사용할 수 없습니다.
#define MSG_SCREENSHOT_SAVED					1011		///< 스크린샷이 $1 로 저장되었습니다.
#define MSG_SCREENSHOT_CANT_SAVE				1012		///< 스크린샷 저장이 실패하였습니다.
#define MSG_RECORD_STARTING						1013		///< 녹화가 시작되었습니다.
#define MSG_RECORD_SAVED						1014		///< 녹화가 $1 로 저장되었습니다.
#define MSG_RECORD_CANT_SAVE					1015		///< 녹화 저장이 실패하였습니다.
#define MSG_VOTE_KICK							1016		///< '%s' 님의 강제퇴장에 대한 투표가 진행중입니다
#define MSG_VOTE_YESNO							1017		///< 찬성은 'Y' 반대는 'N' 를 누르시면 됩니다.
#define MSG_VOTE_SELECT_PLAYER_TO_KICK			1018		///< 추방투표의 대상을 선택하세요
#define MSG_VOTE_SELECT_PLAYER_CANCEL			1019		///< [ESC] 취소
#define MSG_VOTE_PASSED							1020		///< 투표가 가결되었습니다.
#define MSG_VOTE_REJECTED						1021		///< 투표가 부결되었습니다.
#define MSG_VOTE_ABORT							1022		///< 투표설정이 되어있지 않습니다.
#define MSG_VOTE_START							1023		///< '%s' 에 대한 투표가 시작되었습니다.
#define MSG_VOTE_VOTE_STOP						1025		///< 투표 대상 플레이어가 게임에서 나갔습니다.

#define MSG_CLAN_SPONSOR_AGREEMENT_LABEL		1105		///< $1님께서 $2님과 함께 '$3'클랜을 결성하려고 합니다. 동의하십니까?	
															// $1 - 클랜생성멤버, $2 - 클랜마스터, $3 - 클랜이름
#define MSG_CLAN_SPONSOR_AGREEMENT_REJECT		1106		///< $1님께서 클랜 결성을 거절하였습니다.	// $1 - 클랜생성멤버 대상
#define MSG_CLAN_CREATED						1107		///< 클랜이 생성되었습니다.
#define MSG_CLAN_CLOSE_RESERVED					1108		///< 클랜 폐쇄 예약되었습니다.
#define MSG_CLAN_ENABLED_TO_MASTER				1109		///< 클랜마스터만 실행할 수 있습니다.
#define MSG_CLAN_ENABLED_TO_MASTER_AND_ADMIN	1110		///< 클랜마스터와 클랜운영자만 실행할 수 있습니다.
#define MSG_CLAN_JOINED_ALREADY					1111		///< 이미 클랜에 가입되어 있습니다.
#define MSG_CLAN_NOT_JOINED						1112		///< 클랜에 가입되어 있지 않습니다.
#define MSG_CLAN_JOINER_AGREEMENT_LABEL			1113		///< '$1' 클랜 가입을 동의하십니까?		// $1 - 클랜이름
#define MSG_CLAN_JOINER_AGREEMENT_REJECT		1114		///< 가입자가 거절하였습니다.
#define MSG_CLAN_JOINED							1115		///< 클랜에 가입되었습니다.
#define MSG_CLAN_LEAVED							1116		///< 클랜에서 탈퇴되었습니다.
#define MSG_CLAN_MASTER_CANNOT_LEAVED			1117		///< 클랜마스터는 클랜에서 탈퇴할 수 없습니다.
#define MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE	1118		///< 클랜에 가입되어 있으면 삭제할 수 없습니다. 클랜에서 탈퇴해주십시오.
#define MSG_CLAN_CHANGED_GRADE					1119		///< 권한이 변경되었습니다.
#define MSG_CLAN_EXPEL_MEMBER					1120		///< 탈퇴시켰습니다.
#define MSG_CLAN_CREATE_NEED_SOME_SPONSOR		1121		///< 클랜을 생성하기 위해서는 $1명의 창단 멤버를 선택하셔야 합니다.
#define MSG_CLAN_CONFIRM_CLOSE					1122		///< 정말로 클랜을 폐쇄시키겠습니까?
#define MSG_CLAN_CONFIRM_LEAVE					1123		///< 정말로 클랜에서 탈퇴하시겠습니까?
#define MSG_CLAN_WRONG_CLANNAME					1124		///< 클랜이름이 다릅니다.
#define MSG_CLAN_MEMBER_CONNECTED				1125		///< 클랜원 $1님이 접속하였습니다.

#define MSG_CANNOT_DELETE_CHAR_FOR_CASHITEM		1200		///< 캐쉬아이템을 장비하고있어 삭제할 수 없습니다.
#define MSG_HAS_DELETED_CHAR					1201		///< 캐릭터가 삭제되었습니다.

#define MSG_BACKTOTHEPREV						1202		///< $1초 뒤에 이전 해상도로 복귀 됩니다

// 로비 관련
#define MSG_LOBBY_WELCOME1						1300		///< 당신은 채널 '%1'에 입장하셨습니다.
#define MSG_LOBBY_CLAN_DETAIL					1301		///< 클랜마스터 : $1, $2명 접속중
#define MSG_LOBBY_WAITING						1302		///< %1 명 대기중
#define MSG_LOBBY_NO_CLAN						1303		///< 클랜에 가입되어있지 않습니다
// 추가된 부분. 11. 13.
#define MSG_LOBBY_LIMIT_LEVEL					1304		///< 레벨제한을 원치 않으시면 자유채널을 이용해 주시기 바랍니다.
#define MSG_LOBBY_LEAGUE						1305		///< 리그게임은 채널에 상관없이 모든사용자들과 겨루게 됩니다.
#define MSG_LOBBY_INVITATION					1306		///< $1님을 채팅방으로 초대합니다.
#define MSG_LOBBY_WHO_INVITATION				1307		///< $1님이 채팅방 $2로 초대하셨습니다.
#define MSG_LOBBY_WHO_CHAT_ROMM_JOIN			1308		///< 채팅방 '$1'에 '$2'님이 입장하셨습니다.
#define MSG_LOBBY_WHO_CHAT_ROOM_EXIT			1309		///< 채팅방 '$1'에서 '$2'님이 퇴장하셨습니다.
#define MSG_LOBBY_CHAT_ROOM_CHANGE				1310		///< 채팅방 '$1'로 전환합니다.
//#define MRESULT_WHO_CHAT_ROOM_INVITATION		1311		///< $1' 님이 채팅방 '$2'로 초대하셨습니다.
#define MRESULT_CHAT_ROOM						1312		///< 채팅방($1) $2 : $3
// #define MRESULT_WHISPER							100012	///< 귓속말($1) : $2
// 추가된 부분. 11. 15.
#define MSG_LOBBY_REQUESTING_CREATE_CHAT_ROOM	1313		///< 채팅방 개설 요청중...
#define MSG_LOBBY_REQUESTING_JOIN_CAHT_ROOM		1314		///< 채팅방 참가 요청중...
#define MSG_LOBBY_LEAVE_CHAT_ROOM				1315		///< 채팅방에서 탈퇴합니다.
#define MSG_LOBBY_CHOICE_CHAT_ROOM				1316		///< 채팅방을 선택합니다.
#define MSG_LOBBY_JOIN_CHANNEL					1317		///< 당신은 채널 '$1'에 입장했습니다.


// 래더 관련
#define MSG_LADDER_PROPOSAL_WAIT_TITLE				1400		///< 동의대기
#define MSG_LADDER_PROPOSAL_WAIT_DESC				1401		///< 같이 게임할 플레이어가 동의할 동안 기다려주십시오.
#define MSG_LADDER_REPLIER_AGREEMENT_LABEL			1402		///< $1님께서 팀게임에 초대하셨습니다. 동의하십니까?
#define MSG_LADDER_REPLIER_AGREEMENT_REJECT			1403		///< $1님께서 거절하였습니다.	// $1 - 대상
#define MSG_LADDER_CANCEL							1404		///< $1님께서 취소하였습니다.	// $1 - 대상
#define MSG_LADDER_FAILED							1405		///< 상대팀을 찾지못했습니다.
#define MSG_LADDER_INVALID_COUNT					1406		///< 적절한 사람수를 택해야합니다.

/// 네트워크 관련 메세지
#define MSG_CLANBATTLE_REPLIER_AGREEMENT_LABEL		1420		/// $1님께서 클랜전에 초대하셨습니다. 동의하십니까?


// 클랜전 관련
#define MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_3	1510		///< '$1' 클랜이 '$2' 클랜에게 승리하여 $3연승으로 기세를 올리고 있습니다.
#define MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_5	1511		///< '$1' 클랜이 '$2' 클랜에게 승리하여 $3연승째 행진중입니다.
#define MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_7	1512		///< '$1' 클랜이 '$2' 클랜에게 승리하여 거침없이 $3연승을 달리고 있습니다.
#define MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_10	1513		///< '$1' 클랜이 '$2' 클랜에게 승리하여 대망의 $3연승에 성공하였습니다.
#define MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_11	1514		///< '$1' 클랜이 '$2' 클랜에게 승리하여 $3연승을 이어가고 있습니다.
#define MSG_CLANBATTLE_BROADCAST_INTERRUPT_VICTORIES 1515		///< '$1' 클랜이 '$2' 클랜의 $3연승을 저지하였습니다.

// 듀얼 관련
#define MSG_DUEL_BROADCAST_RENEW_VICTORIES			1520		///< '$1' 님이 '$2'채널의 $3번 방에서 $4연승을 기록중입니다.
#define MSG_DUEL_BROADCAST_INTERRUPT_VICTORIES		1521		///< '$2' 님이 '$1'님의 $3연승을 저지하였습니다.


// 로딩 메시지
#define MSG_LOADING_MESSAGE11					1600		///< 로딩1 메시지
#define MSG_LOADING_MESSAGE12					1601		///< 
#define MSG_LOADING_MESSAGE13					1602		///< 
#define MSG_LOADING_MESSAGE21					1603		///< 로딩2 메시지
#define MSG_LOADING_MESSAGE22					1604		///< 
#define MSG_LOADING_MESSAGE23					1605		///< 
#define MSG_LOADING_MESSAGE31					1606		///< 로딩3 메시지
#define MSG_LOADING_MESSAGE32					1607		///< 
#define MSG_LOADING_MESSAGE33					1608		///< 
#define MSG_LOADING_MESSAGE41					1609		///< 로딩4 메시지
#define MSG_LOADING_MESSAGE42					1610		///< 
#define MSG_LOADING_MESSAGE43					1611		///< 
#define MSG_LOADING_MESSAGE51					1612		///< 로딩5 메시지
#define MSG_LOADING_MESSAGE52					1613		///< 
#define MSG_LOADING_MESSAGE53					1614		///<
#define MSG_LOADING_MESSAGE61					1615		///< 로딩6 메시지
#define MSG_LOADING_MESSAGE62					1616		///< 
#define MSG_LOADING_MESSAGE63					1617		///< 
#define MSG_LOADING_MESSAGE71					1618		///< 로딩7 메시지
#define MSG_LOADING_MESSAGE72					1619		///< 
#define MSG_LOADING_MESSAGE73					1620		///< 
#define MSG_LOADING_MESSAGE81					1621		///< 로딩8 메시지
#define MSG_LOADING_MESSAGE82					1622		///< 
#define MSG_LOADING_MESSAGE83					1623		///< 
#define MSG_LOADING_MESSAGE91					1624		///< 로딩9 메시지
#define MSG_LOADING_MESSAGE92					1625		///< 
#define MSG_LOADING_MESSAGE93					1626		///< 

// 캐릭터 정보 표시
#define MSG_CHARINFO_TITLE						1700		///< [캐릭터 정보]=======================
#define MSG_CHARINFO_NAME						1701		///< 이름
#define MSG_CHARINFO_CLAN						1702		///< 클랜
#define MSG_CHARINFO_LEVEL						1703		///< 레벨
#define MSG_CHARINFO_WINPERCENT					1704		///< 승률
#define MSG_CHARINFO_WIN						1705		///< 승
#define MSG_CHARINFO_LOSE						1706		///< 패
#define MSG_CHARINFO_CONNTIME					1707		///< 접속시간
#define MSG_CHARINFO_DAY						1708		///< 일
#define MSG_CHARINFO_HOUR						1709		///< 시간
#define MSG_CHARINFO_MINUTE						1710		///< 분
#define MSG_CHARINFO_SECOND						1711		///< 초
// 추가 부분
#define MSG_CHARINFO_XP							1712		///< XP
#define MSG_CHARINFO_BOUNTY						1713		///< Bounty
#define MSG_CHARINFO_HP							1714		///< HP
#define MSG_CHARINFO_AP							1715		///< AP
#define MSG_CHARINFO_WEIGHT						1716		///< WT
#define MSG_CHARINFO_LEVELMARKER				1717		///< Lv.


/// 게임중 메세지
#define MSG_GAME_JOIN_BATTLE					2000		///< $1님께서 게임에 참가하셨습니다.
#define MSG_GAME_LEAVE_BATTLE					2001		///< $1님께서 게임에서 퇴장하셨습니다.
#define MSG_GAME_LEVEL_UP						2002		///< $1님께서 레벨업 하셨습니다.
#define MSG_GAME_LEVEL_DOWN						2003		///< $1님께서 레벨다운 하셨습니다.
#define MSG_GAME_SCORESCREEN_STAGENAME			2004		///< 클랜전 ( %d vs %d )
// 추가된 부분.
#define MSG_GAME_FALL_NARAK						2005		///< 나락으로 떨어짐.
#define MSG_GAME_LOSE_BY_MY_BOMB				2006		///< 당신은 자신의 폭탄으로 인하여 패배 하였습니다.
#define MSG_GAME_LOSE_MYSELF					2007		///< 당신은 스스로 패배하였습니다.
#define MSG_GAME_WHO_LOSE_SELF					2008		///< $1님이 스스로 패배하였습니다.
#define MSG_GAME_WIN_FROM_WHO					2009		///< 당신은 $1님으로부터 승리하였습니다.
#define MSG_GAME_LOSE_FROM_WHO					2010		///< 당신은 $1님으로부터 패배 하였습니다.
#define MSG_GAME_WHO_WIN_FROM_OTHER				2011		///< $1님이 $2s님으로부터 승리하였습니다.
#define MSG_GAME_CLICK_FIRE						2012		///< 플레이하려면 Fire키를 눌러주세요!
#define MSG_GAME_WAIT_N_MIN						2013		///< $1초동안 기다려 주세요.
#define MSG_GAME_EXIT_N_MIN_AFTER				2014		///< $1초후에 게임에서 나갑니다.
// 추가된 부분. 11. 15.
#define MSG_GAME_WHISPER						2015		///< 귓속말(%s) : %s
#define MSG_GAME_BRINGITEM						2016		///< 아이템을 가져왔습니다.
#define MSG_GAME_NOTBRINGITEM					2017		///< 아이템을 가져올 수 없습니다.
#define MSG_GAME_BUYITEM						2018		///< 구입하였습니다.
#define MSG_GAME_NOTBUYITEM						2019		///< 구입할 수 없습니다.
#define MSG_GAME_SELLITEM						2020		///< 판매하였습니다.
#define MSG_GAME_NOCHARACTER					2021		///< 선택할 수 있는 캐릭터가 없습니다. 먼저 캐릭터를 생성해 주시기 바랍니다.z
#define MSG_GAME_NOTSELLITEM					2022		///< 판매할 수 없습니다.
#define MSG_GAME_NEXT_N_MIN_AFTER				2023		///< $1초후에 다음 라운드로 이동합니다.
#define MSG_GAME_GET_QUEST_ITEM					2024		///< 퀘스트 팀에서 $1를 획득하였습니다.
#define MSG_GAME_GET_QUEST_ITEM2				2025		///< $1 $2개 획득
#define MSG_GAME_OPEN_PORTAL					2026		///< 포탈이 열렸습니다.
#define MSG_GAME_MAKE_AUTO_BALANCED_TEAM		2027		///< 팀 밸런스가 적용되었습니다.
#define MSG_GANE_NO_QUEST_SCENARIO				2028		///< 맞는 시나리오가 없습니다.

#define MSG_ADMIN_ANNOUNCE						3000		///< 관리자 메시지.



// 메뉴
#define MSG_MENUITEM_NONE						8000		///< 메뉴없음

#define MSG_MENUITEM_FRIENDADD					8001		///< 친구추가
#define MSG_MENUITEM_FRIENDREMOVE				8002		///< 친구제거
#define MSG_MENUITEM_FRIENDWHERE				8003		///< 위치확인
#define MSG_MENUITEM_FRIENDFOLLOW				8004		///< 따라가기
#define MSG_MENUITEM_FRIENDWHISPER				8005		///< 귓말
#define MSG_MENUITEM_FRIENDKICK					8006		///< 추방
#define MSG_MENUITEM_FRIENDCLANINVITE			8007		///< 클랜가입권유

#define MSG_MENUITEM_CLANGRADEMASTER			8008		///< 클랜마스터위임
#define MSG_MENUITEM_CLANGRADEADMIN				8009		///< 운영자로 변경
#define MSG_MENUITEM_CLANMEMBER					8010		///< 클랜원으로 변경
#define MSG_MENUITEM_CLANKICK					8011		///< 클랜에서 방출
#define MSG_MENUITEM_CLANLEAVE					8012		///< 클랜탈퇴

#define MSG_MENUITEM_SENTTOBANK					8013		///< 중앙은행으로 보내기

#define MSG_MENUITEM_OK							8014		///< 확인
#define MSG_MENUITEM_CANCEL						8015		///< 취소
#define MSG_MENUITEM_YES						8016		///< 예
#define MSG_MENUITEM_NO							8017		///< 아니오
#define MSG_MENUITEM_MESSAGE					8018		///< 메세지


/// 용어
#define MSG_WORD_CLAN_NONE						9000		///< 없음
#define MSG_WORD_CLAN_MASTER					9001		///< 클랜마스터
#define MSG_WORD_CLAN_ADMIN						9002		///< 클랜운영자
#define MSG_WORD_CLAN_MEMBER					9003		///< 클랜원

#define MSG_WORD_ADMIN							9004		///< 운영자
#define MSG_WORD_DEVELOPER						9005		///< 개발자
#define MSG_WORD_COMMANDS						9006		///< 명령어
#define MSG_WORD_HELP							9007		///< 도움말
#define MSG_WORD_USAGE							9008		///< 사용법

#define MSG_WORD_MALE							9100		///< 남성
#define MSG_WORD_FEMALE							9101		///< 여성
#define MSG_WORD_MALE_SHORT						9102		///< 남
#define MSG_WORD_FEMALE_SHORT					9103		///< 여

#define MSG_WORD_LOBBY							9200		///< 로비
#define MSG_WORD_STAGE							9201		///< 스테이지
#define MSG_WORD_SHOP							9202		///< 상점
#define MSG_WORD_EQUIPMENT						9203		///< 장비
#define MSG_WORD_CASH							9204		///< Cash

#define MSG_WORD_FORMEN							9302		///< 착용성별 : 남성용
#define MSG_WORD_FORWOMEN						9349		///< 착용성별 : 여성용
#define MSG_WORD_WEARABLE						9302		///< 착용성별
#define MSG_WORD_LIMITEDLEVEL					9303		///< 제한레벨
#define MSG_WORD_WEIGHT							9304		///< 무게
#define MSG_WORD_ATTRIBUTE_FIRE					9305		///< 불속성
#define MSG_WORD_ATTRIBUTE_COLD					9306		///< 얼음속성
#define MSG_WORD_ATTRIBUTE_POISON				9307		///< 독속성
#define MSG_WORD_ATTRIBUTE_LIGHTNING			9308		///< 번개속성
#define MSG_WORD_RUNTIME						9309		///< 지속시간
#define MSG_WORD_BULLET							9310		///< 탄수/탄창
#define MSG_WORD_ATTACK							9311		///< 공격력
#define MSG_WORD_DELAY							9312		///< 딜레이
#define MSG_WORD_MAXWEIGHT						9313		///< 최대무게
#define MSG_WORD_RUNSPEED						9314		///< 이동속도
#define MSG_WORD_DONOTJUMP						9315		///< 점프 불가
#define MSG_WORD_DONOTDASH						9316		///< 대쉬 불가
#define MSG_WORD_DONOTHANGWALL					9317		///< 벽타기 불가
#define MSG_WORD_DAMAGE							9318		///< 데미지
#define MSG_WORD_QUEST							9319		///< 퀘스트
#define MSG_WORD_ITEM							9320		///< 아이템
#define MSG_WORD_SACRIFICE						9321		///< 희생
#define MSG_WORD_PLAYERS						9322		///< 참가 인원
#define MSG_WORD_BLUETEAM						9323		///< 청팀
#define MSG_WORD_REDTEAM						9324		///< 홍팀
#define MSG_WORD_QUANTITY						9325		///< 현재 수량
#define MSG_WORD_FINDTEAM						9326		///< 상대팀 찾는 중
#define MSG_WORD_GETITEMQTY						9327		///< 획득 아이템 수
#define MSG_WORD_REMAINNPC						9328		///< 남은 NPC 수
#define MSG_WORD_RPROGRESS						9329		///< 진행도
#define MSG_WORD_REMAINTIME						9330		///< 남은 시간
#define MSG_WORD_QUESTLEVELMARKER				9331		///< ★
#define MSG_WORD_ENDKILL						9363		///< 목표 KILL수
#define MSG_WORD_ROUND							9364		///< 라운드

#define MSG_WORD_RATE							9332		///< 수집률
#define MSG_WORD_GRADE							9333		///< 등급
#define MSG_WORD_REGULAR						9334		///< Regular
#define MSG_WORD_LEGENDARY						9335		///< Legendary
#define MSG_WORD_BOSS							9336		///< Boss
#define MSG_WORD_ELITE							9337		///< Elite
#define MSG_WORD_VETERAN						9338		///< Veteran
#define MSG_WORD_VERYHARD						9339		///< 매우 강함
#define MSG_WORD_HARD							9340		///< 강함
#define MSG_WORD_NORAML							9341		///< 보통
#define MSG_WORD_WEAK							9342		///< 약함
#define MSG_WORD_VERYWEAK						9343		///< 매우 약함

#define MSG_WORD_INFINITE						9344		///< 무한
#define MSG_WORD_NONE							9345		///< 없음
#define MSG_WORD_LEVELDIFF						9346		///< 레벨차
#define MSG_WORD_PERMIT							9347		///< 허용
#define MSG_WORD_PROHIBIT						9348		///< 금지

// 2006년 5월 3일 추가
#define MSG_WORD_TYPE							9365		///< Type
#define MSG_WORD_EXP							9366		///< Exp
#define MSG_WORD_KILL							9367		///< Kill
#define MSG_WORD_DEATH							9368		///< Death
#define MSG_WORD_PING							9369		///< Ping

#define MSG_WORD_HEAD							9370		///< < Head >
#define MSG_WORD_CHEST							9371		///< < Chest >
#define MSG_WORD_HANDS							9372		///< < Hands >
#define MSG_WORD_LEGS							9373		///< < Legs >
#define MSG_WORD_FEET							9374		///< < Feet >
#define MSG_WORD_LFINGER						9375		///< < Left Finger >
#define MSG_WORD_RFINGER						9376		///< < Right Finger >
#define MSG_WORD_MELEE							9377		///< < Melee >
#define MSG_WORD_WEAPON1						9378		///< < Primary Weapon >
#define MSG_WORD_WEAPON2						9379		///< < Secondary Weapon >
#define MSG_WORD_ITEM1							9380		///< < Item 1 >
#define MSG_WORD_ITEM2							9381		///< < Item 2 >

#define MSG_SERVER_DEBUG						9382		///< Debug Server
#define MSG_SERVER_MATCH						9383		///< Match Server
#define MSG_SERVER_CLAN							9384		///< Clan Server
#define MSG_SERVER_QUEST						9385		///< Quest Server
#define MSG_SERVER_EVENT						9386		///< Event Server
#define MSG_SERVER_IGUNZ						9387		///< iGunZ Server

#define MSG_MT_DEATHMATCH_SOLO					9350
#define MSG_MT_DEATHMATCH_TEAM					9351
#define MSG_MT_GLADIATOR_SOLO					9352
#define MSG_MT_GLADIATOR_TEAM					9353
#define MSG_MT_ASSASSINATE						9354
#define MSG_MT_TRAINING							9355
#define MSG_MT_CLASSIC_SOLO						9356
#define MSG_MT_CLASSIC_TEAM						9357
#define MSG_MT_SURVIVAL							9358
#define MSG_MT_QUEST							9359
#define MSG_MT_BERSERKER						9360
#define MSG_MT_DEATHMATCH_TEAM2					9361
#define MSG_MT_DUEL								9362

#define MSG_REMAIND_PERIOD						9400
#define MSG_JOINED_STAGE						9401
#define MSG_JOINED_STAGE2						9402

#define MSG_WORD_ON								9500
#define MSG_WORD_OFF							9501

#define MSG_DISCONNMSG_XTRAPHACK				9601

#define MSG_HACKING_DETECTED					20000
#define MSG_EXPIRED								20001
#define MSG_SHOPMSG								20002
#define MSG_DONOTSUPPORT_GPCARD					20003
#define MSG_DIRECTX_NOT_INSTALL					20004
#define MSG_DIRECTX_DOWNLOAD_URL				20005
#define MSG_CHARDELETE_ERROR					20006

#define MSG_WRONG_WORD_NAME					100300


const char* ZGetSexStr(MMatchSex nSex, bool bShort=false);
void ZGetTimeStrFromSec(char* poutStr, size_t maxlen, u32 nSec);
template <size_t size>
void ZGetTimeStrFromSec(char(&poutStr)[size], u32 nSec) {
	return ZGetTimeStrFromSec(poutStr, size, nSec);
}