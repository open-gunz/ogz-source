#ifndef MERRORTABLE_H
#define MERRORTABLE_H

#define MOK				0	///< 에러 없음

#define MERR_UNKNOWN	-1	///< 알지 못하는 에러

#define MERR_COMMUNICATOR_HAS_NOT_UID						100		///< 커뮤니케이터가 UID를 가지지 않은 경우, 아직 마스터나 혹은 UID를 발급받을 수 있는 커뮤니케이터와 접속하지 않았을 경우다.
#define MERR_COMMUNICATOR_HAS_NOT_UID_SPACE					101		///< 커뮤니케이터가 발급할 UID 가지고 있지 않다. UID를 예약중일 가능성이 높다.
#define MERR_COMMUNICATOR_INVALID_DIRECT_CONNECTION_POINTER	110		///< 하나의 프로세스내에 여러 커뮤니케이터가 동시에 떳을때, DirectConnectionPointer가 유효하지 않은 경우
#define MERR_COMMAND_HAS_INVALID_RECEIVER_UID				200		///< 커맨드에 설정된 Receiver UID가 유효하지 않을때

#define MERR_ZONESERVER_NOT_CONNECTED_TO_MASTER				300		///< Zone-Server가 Master-Controller와 연결되지 않았을때
#define MERR_ZONESERVER_TRY_TO_INVALID_MASTER				301		///< Zone-Server가 Master-Controller가 아니 커뮤니케이터에게 Master-Controller로 오인하고 연결을 시도할때

#define MERR_OBJECT_INVALID									1000	///< 유효하지 않은 오브젝트
#define MERR_OBJECT_INVALID_MODE							1001	///< 적합하지 않은 모드

#define MERR_MAP_CANNOT_OPEN_FILE							2000	///< 맵을 열지 못하는 경우


// 접속, 로그인 관련
#define MERR_CLIENT_WRONG_PASSWORD							10000	///< 패스워드가 잘못됨
#define MERR_CLIENT_CONNECTED_ALREADY						10001	///< 이미 접속중이다.
#define MERR_COMMAND_INVALID_VERSION						10002	///< 서버와 클라이언트가 버전이 다르다.
#define MERR_CLIENT_FULL_PLAYERS							10003	///< 서버 인원이 꽉찼다
#define MERR_CLIENT_MMUG_BLOCKED							10004	///< 블럭당했다.
#define MERR_FAILED_AUTHENTICATION							10005	///< 사용자 인증에 오류가 발생하였습니다.

// 캐릭터 관리 관련
#define MERR_CLIENT_EXIST_CHARNAME							10100	///< 서버에 이미 캐릭터 아이디가 존재
#define MERR_WRONG_WORD_NAME								10101	///< 캐릭터이름에 사용할 수 없는 문자가 있다.
#define MERR_TOO_SHORT_NAME									10102	///< 캐릭터이름 길이가 너무 짧다.
#define MERR_TOO_LONG_NAME									10103	///< 캐릭터 이름이 너무 길다
#define MERR_PLZ_INPUT_CHARNAME								10104	///< 캐릭터 이름을 입력해주세요
#define MERR_CHAR_NOT_EXIST									10110	///< 존재하지 않는 캐릭터이름
#define MERR_CANNOT_DELETE_CHAR								10111	///< 캐릭터를 삭제할 수 없습니다.

// 아이템 관련
#define MERR_TOO_EXPENSIVE_BOUNTY							20001	///< 바운티가 부족하다
#define MERR_CANNOT_BUY_ITEM								20002	///< 아이템을 살수 없다.
#define MERR_CANNOT_EQUIP_ITEM								20003	///< 아이템을 장비할 수 없다.
#define MERR_CANNOT_TAKEOFF_ITEM							20004	///< 장착 아이템 해제할 수 없다
#define MERR_CANNOT_SELL_ITEM								20005	///< 아이템을 팔 수 없다.
#define MERR_CANNOT_SELL_EQUIPED_ITEM						20006	///< 장비한 아이템은 팔 수 없다.
#define MERR_CANNOT_SELL_NONE_ITEM							20007	///< 해당 아이템이 없다.
#define MERR_TOO_MANY_ITEM									20008	///< 갖고 있는 아이템이 너무 많다
#define MERR_NO_SELITEM										20009	///< 선택한 아이템이 없다
#define MERR_TOO_HEAVY										20010	///< 장비아이템 무게가 너무 무겁다
#define MERR_LOW_LEVEL										20011	///< 레벨이 너무 낮다
#define MERR_CANNOT_EQUIP_EQUAL_ITEM						20012	///< 양슬롯에 같은 아이템 착용불가
#define MERR_CANNOT_TAKEOFF_ITEM_BY_WEIGHT					20013	///< 무게때문에 벗을 수 없다
#define MERR_CANNOT_SELL_CASHITEM							20014	///< 캐쉬아이템은 팔 수 없습니다.
#define MERR_BRING_ACCOUNTITEM_BECAUSEOF_SEX				20015	///< 성별이 맞지않아 아이템을 가져올 수 없다.
#define MERR_BRING_CONFIRM_ACCOUNTITEM_BECAUSEOF_LEVEL		20016	///< 레벨이 맞지않아 아이템을 가져올 수 없다. 그래도 가져오겠는가?
#define MERR_BRING_BACK_ACCOUNTITEM							20017	///< 중앙은행에 넣을 수 없습니다.
#define MERR_BRING_BACK_ACCOUNTITEM_FOR_CASHITEM			20018	///< 캐쉬아이템만 중앙은행에 넣을 수 있습니다.
#define MERR_TAKEOFF_ITEM_BY_LEVELDOWN						20019	///< 레벨다운으로 아이템이 벗겨졌습니다.

#define MERR_CANNOT_JOIN_STAGE								30000	///< 스테이지에 조인할 수 없다.
#define MERR_CANNOT_JOIN_STAGE_BY_MAXPLAYERS				30001	///< 스테이지 인원이 꽉 찼다.
#define MERR_CANNOT_JOIN_STAGE_BY_PASSWORD					30002	///< 비밀번호가 틀렸습니다.
#define MERR_CANNOT_JOIN_STAGE_BY_LEVEL						30003	///< 레벨이 맞지 않습니다.
#define MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY				30004	///< 난입할 수 없습니다.
#define MERR_CANNOT_JOIN_STAGE_BY_BAN						30005	///< 입장이 금지되었습니다.
#define MERR_STAGE_NOT_EXIST								30006	///< 존재하지 않는 스테이지입니다
#define MERR_STAGE_ALREADY_EXIST							30007	///< 스테이지가 이미 존재합니다
#define MERR_CANNOT_CREATE_STAGE							30008	///< 방을 만들 수 없습니다.
#define MERR_CANNOT_NO_STAGE								30009	///< 방이 없습니다.

#define MERR_CANNOT_JOIN_CHANNEL							30020	///< 채널에 조인할 수 없다
#define MERR_CANNOT_JOIN_CHANNEL_BY_MAXPLAYERS				30021	///< 채널 인원이 꽉 찼다
#define MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL					30022	///< 레벨이 맞지 않다
#define MERR_CANNOT_JOIN_CHANNEL_BY_NEWBIE					30023	///< 가지고 있는 캐릭터의 레벨이 높아 더이상 입문채널에는 들어갈 수 없습니다.

#define MERR_NO_TARGET										30026	///< 대상이 없습니다.

// 클랜관련
#define MERR_CLAN_CANNOT_CREATE								30030	///< 클랜을 만들 수 없습니다.
#define MERR_CLAN_CANNOT_CLOSE								30031	///< 클랜을 폐쇄할 수 없습니다.
#define MERR_EXIST_CLAN										30032	///< 클랜이 이미 존재합니다.
#define MERR_CLAN_NO_SPONSOR								30033	///< 클랜생성멤버가 부족합니다.
#define MERR_CLAN_SPONSOR_JOINED_OTHERCLAN					30034	///< 클랜생성멤버가 이미 다른 클랜에 가입되어 있습니다.
#define MERR_CLAN_SPONSOR_NOT_LOBBY							30035	///< 클랜생성멤버가 로비에 없습니다.
#define MERR_CLAN_WRONG_CLANNAME							30036	///< 클랜이름이 잘못되었습니다.
#define MERR_CLAN_NOT_MASTER								30037	///< 클랜마스터가 아닙니다.
#define MERR_CLAN_NOT_MASTER_OR_ADMIN						30038	///< 클랜마스터나 운영자가 아닙니다.
#define MERR_CLAN_JOINER_JOINED_ALREADY						30039	///< 가입자가 이미 다른 클랜에 가입되어 있습니다.
#define MERR_CLAN_DONT_JOINED								30040	///< 가입처리되지 않았습니다.
#define MERR_CLAN_JOINED_ALREADY							30041	///< 이미 클랜에 가입되어 있습니다.
#define MERR_CLAN_JOINER_NOT_LOBBY							30042	///< 가입자가 로비에 없습니다.
#define MERR_CLAN_NOT_JOINED								30043	///< 클랜에 가입되어 있지 않습니다.
#define MERR_CLAN_CANNOT_LEAVE								30044	///< 클랜에서 탈퇴할 수 없습니다.
#define MERR_CLAN_MEMBER_FULL								30045	///< 더이상 클랜원을 추가할 수 없습니다.
#define MERR_CLAN_OFFLINE_TARGET							30046	///< 대상이 접속해있지 않습니다.
#define MERR_CLAN_OTHER_CLAN								30047	///< 같은 클랜이 아닙니다.
#define MERR_CLAN_CANNOT_CHANGE_GRADE						30048	///< 권한을 변경할 수 없습니다.
#define MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER				30049	///< 해당 클랜원이 없습니다.
#define MERR_CLAN_CREATING_LESS_BOUNTY						30050	///< 클랜을 생성하는데 필요한 바운티가 부족합니다.
#define MERR_CLAN_CREATING_LESS_LEVEL						30051	///< 10레벨 이상만 클랜을 생성할 수 있습니다.
#define MERR_CLAN_ANNOUNCE_DELETE							30052	///< 클랜 폐쇄신청 접수를 알려줌.

// 동의관련

// 래더관련
#define MERR_LADDER_CANNOT_CHALLENGE						30070	///< 게임할 수 없습니다.
#define MERR_LADDER_WRONG_TEAM_MEMBER						30071	///< 팀 멤버가 다릅니다.
#define MERR_LADDER_NO_TEAM_MEMBER							30072	///< 팀 멤버가 없습니다.
#define MERR_LADDER_EXIST_CANNOT_CHALLENGE_MEMBER			30073	///< 팀게임에 신청할 수 없는 멤버가 있습니다.

// 클랜전관련
#define MERR_CB_WRONG_TEAM_MEMBER							30080	///< 같은 클랜원이 아닙니다.
#define MERR_CB_NO_TEAM_MEMBER								30081	///< 멤버가 없습니다.
#define MERR_CB_EXIST_CANNOT_CHALLENGE_MEMBER				30082	///< 클랜전에 신청할 수 없는 멤버가 있습니다.


// 클라이언트
#define MERR_NOT_SUPPORT									30103	///< 지원하지 않습니다.
#define MERR_CANNOT_ABUSE									30104	///< 올바른 표현이 아닙니다.
#define MERR_CANNOT_INPUT_SAME_CHAT_MSG						30105	///< 같은 대화 내용 3번이상 연속해서 쓸 수 없다
#define MERR_CHAT_PENALTY_FOR_ONE_MINUTE					30106	///< 1분간 채팅을 금지합니다.

#define MERR_CLIENT_CONNECT_FAILED							90000	///< Server에 연결실패할때
#define MERR_CLIENT_DISCONNECTED							90002	///< Server와 연결이 끊어졌을때



// 메세지 결과값
#define MRESULT_CLAN_JOINED									100000	///< 클랜에 가입되었습니다.
#define MRESULT_CLAN_CREATED								100001	///< 클랜이 생성되었습니다.

// 따라가기 관련
#define MERR_CANNOT_FOLLOW									110000	///< 들어갈수 없습니다.
#define MERR_CANNOT_FOLLOW_BY_PASSWORD						110001	///< 비밀번호가 필요한 방이라 따라 들어갈수 없다.
// #define MERR_CANNOT_FOLLOW_BY_CHANNER						110002	///< 다른 채널에 있어 따라 들어갈수 없다.

// 투표관련.
#define MERR_CANNOT_CALL_VOTE								120000 // 투표를 신청할수 없습니다.


// Disconnect message
#define MERR_FIND_HACKER									130001	/// 해킹 검출.
#define MERR_BLOCK_HACKER									130002	/// 해킹 전과자로 블럭.
#define MERR_BLOCK_BADUSER									130003	/// 불량 유저로 블럭.




// 서버 관련 메시지
#define MERR_CANNOT_VOTE									200000 // 투표 불가능.
#define MERR_CANNOT_VOTE_LADERGAME							200001 // 래더게임은 투표가 허용되지 않습니다.
#define MERR_VOTE_ALREADY_START								200002 // 이미 투표가 진행중입니다.
#define MERR_VOTE_FAILED									200003 // 투표에 실패했습니다.
#define MERR_TIME_10REMAINING								200004 // 시간제한 10초 남았습니다.
#define MERR_TIME_30REMAINING								200005 // 시간제한 30초 남았습니다.
#define MERR_TIME_60REMAINING								200006 // 시간제한 60초 남았습니다.
#define MERR_PERSONNEL_TOO_MUCH								200007 // 인원이 너무 많습니다.
#define MERR_HE_IS_NOT_READY								200008 //  '%s'님은 준비가 안되었습니다.


#endif