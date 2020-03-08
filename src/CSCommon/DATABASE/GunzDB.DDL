/*    이 SQL DDL 스크립트는 Microsoft Visual Studio(릴리스 날짜: LOCAL BUILD)에서 생성했습니다.                  */

/*    사용된 드라이브 : Microsoft Visual Studio - Microsoft SQL Server 드라이버                          */
/*    문서    : D:\My Projects\Maiet\CSCommon\Database\GunzDB.vsd                               */
/*    만든 시간: 2004년3월22일 5:42 오후                                                               */
/*    작업   : Visio 생성 마법사에서 수행                                                                */
/*    연결된 데이터 원본 : 연결되지 않았습니다.                                                                */
/*    연결된 서버      : 연결되지 않았습니다.                                                               */
/*    연결된 데이터베이스    : 적용할 수 없습니다.                                                             */



/* TestDB 데이터베이스를 만듭니다.                                                                       */
use master  

go

create database "TestDB"  

go

use "TestDB"  

go

/* 새 테이블 "ItemType"을(를) 만듭니다.                                                                 */
/* "ItemType" : Table of 아이템 타입                                                               */
/* 	"ItemTypeID" : ItemTypeID identifies 아이템 타입                                               */
/* 	"Name" : Name is of 아이템 타입                                                                */  
create table "ItemType" ( 
	"ItemTypeID" int not null,
	"Name" varchar(64) not null) ON 'PRIMARY'  

go

alter table "ItemType"
	add constraint "ItemType_PK" primary key clustered ("ItemTypeID")   


go

/* 새 테이블 "MonthRanking"을(를) 만듭니다.                                                             */
/* "MonthRanking" : Table of MonthRanking                                                     */  
create table "MonthRanking" ( 
	"id" int identity not null,
	"name" varchar(24) not null,
	"level" int not null,
	"point" int null,
	"rank" int null)  

go

alter table "MonthRanking"
	add constraint "PK_MonthRanking_ID" primary key ("id")   


go

/* 새 테이블 "TotalRanking"을(를) 만듭니다.                                                             */
/* "TotalRanking" : Table of TotalRanking                                                     */  
create table "TotalRanking" ( 
	"id" int identity not null,
	"rank" int not null,
	"name" varchar(24) not null,
	"level" smallint not null,
	"md1point" int not null,
	"md2point" int not null,
	"md3point" int not null,
	"md4point" int not null,
	"weekpoint" int not null,
	"monthpoint" int not null)  

go

alter table "TotalRanking"
	add constraint "PK_id" primary key ("id")   


go

/* 새 테이블 "ModeRanking"을(를) 만듭니다.                                                              */
/* "ModeRanking" : Table of ModeRanking                                                       */  
create table "ModeRanking" ( 
	"id" int identity not null,
	"name" varchar(24) not null,
	"level" int not null,
	"md1point" int null,
	"md2point" int null,
	"md3point" int null,
	"md4point" int null,
	"md1rank" int null,
	"md2rank" int null,
	"md3rank" int null,
	"md4rank" int null)  

go

alter table "ModeRanking"
	add constraint "PK_ModeRanking_ID" primary key ("id")   


go

/* 새 테이블 "MonthHonorRanking"을(를) 만듭니다.                                                        */
/* "MonthHonorRanking" : Table of MonthHonorRanking                                           */  
create table "MonthHonorRanking" ( 
	"id" int identity not null,
	"name" varchar(24) not null,
	"level" int not null,
	"point" int null,
	"rank" int null,
	"date" datetime null)  

go

alter table "MonthHonorRanking"
	add constraint "PK_MonthHonorRanking_ID" primary key ("id")   


go

/* 새 테이블 "WeekHonorRanking"을(를) 만듭니다.                                                         */
/* "WeekHonorRanking" : Table of WeekHonorRanking                                             */  
create table "WeekHonorRanking" ( 
	"id" int identity not null,
	"name" varchar(24) not null,
	"level" int not null,
	"point" int null,
	"rank" int null,
	"date" datetime null)  

go

alter table "WeekHonorRanking"
	add constraint "PK_WeekHonorRanking_ID" primary key ("id")   


go

/* 새 테이블 "WeekRanking"을(를) 만듭니다.                                                              */
/* "WeekRanking" : Table of WeekRanking                                                       */  
create table "WeekRanking" ( 
	"id" int identity not null,
	"name" varchar(24) not null,
	"level" int not null,
	"point" int null,
	"rank" int null)  

go

alter table "WeekRanking"
	add constraint "PK_WeekRanking_ID" primary key ("id")   


go

/* 새 테이블 "CashSetItem"을(를) 만듭니다.                                                              */
/* "CashSetItem" : Table of 세트 아이템 리스트                                                        */
/* 	"CSIID" : CSIID identifies 세트 아이템 리스트                                                     */
/* 	"CSSID" : CSSID identifies 세트 아이템 리스트                                                     */
/* 	"CSID" : SIID is of 세트 아이템 리스트                                                            */  
create table "CashSetItem" ( 
	"CSIID" int identity not null,
	"CSSID" int not null,
	"CSID" int not null) ON 'PRIMARY'  

go

alter table "CashSetItem"
	add constraint "CashSetItem_PK" primary key clustered ("CSIID")   


go

/* 새 테이블 "CashSetShop"을(를) 만듭니다.                                                              */
/* "CashSetShop" : Table of 넷마블의 세트 아이템 샵                                                     */
/* 	"CSSID" : CSSID identifies 넷마블의 세트 아이템 샵                                                  */
/* 	"Name" : Name is of 넷마블의 세트 아이템 샵                                                         */
/* 	"Description" : Description is of 넷마블의 세트 아이템 샵                                           */
/* 	"CashPrice" : CashPrice is of 넷마블의 세트 아이템 샵                                               */
/* 	"WebImgName" : WebImgName is of 넷마블의 세트 아이템 샵                                             */
/* 	"RegDate" : RegDate is of 넷마블의 세트 아이템 샵                                                   */  
create table "CashSetShop" ( 
	"CSSID" int identity not null,
	"Name" varchar(64) null,
	"Description" text null,
	"CashPrice" int not null,
	"WebImgName" varchar(64) null,
	"RegDate" datetime null) ON 'PRIMARY' TEXTIMAGE_ON 'PRIMARY'  

go

alter table "CashSetShop"
	add constraint "CashSetShop_PK" primary key clustered ("CSSID")   


go

/* 새 테이블 "CashShop"을(를) 만듭니다.                                                                 */
/* "CashShop" : Table of 넷마블의 상점 목록                                                           */
/* 	"CSID" : SIID identifies 넷마블의 상점 목록                                                       */
/* 	"ItemID" : ItemID is of 넷마블의 상점 목록                                                        */
/* 	"RegDate" : RegDate is of 넷마블의 상점 목록                                                      */  
create table "CashShop" ( 
	"CSID" int identity not null,
	"ItemID" int not null,
	"RegDate" datetime null) ON 'PRIMARY'  

go

alter table "CashShop"
	add constraint "CashShop_PK" primary key clustered ("CSID")   


go

/* 새 테이블 "ServerStatus"을(를) 만듭니다.                                                             */
/* "ServerStatus" : Table of 서버상태                                                             */
/* 	"ServerID" : 서버상태ServerID identifies 서버상태                                                 */
/* 	"CurrPlayer" : CurrPlayer is of 서버상태                                                      */
/* 	"MaxPlayer" : MaxPlayer is of 서버상태                                                        */
/* 	"Time" : 서버시간 is of 서버상태                                                                  */
/* 	"IP" : ServerStatus_IP is of 서버상태                                                         */  
create table "ServerStatus" ( 
	"ServerID" int not null,
	"CurrPlayer" smallint null,
	"MaxPlayer" smallint null,
	"Time" datetime null,
	"IP" varchar(32) null,
	"Port" int null,
	"ServerName" varchar(64) null,
	"Opened" tinyint null) ON 'PRIMARY'  

go

alter table "ServerStatus"
	add constraint "ServerStatus_PK" primary key clustered ("ServerID")   


go

/* 새 테이블 "PlayerLog"을(를) 만듭니다.                                                                */
/* "PlayerLog" : Table of 로그아웃시 로그                                                            */
/* 	"id" : 플레이어로그ID identifies 로그아웃시 로그                                                       */
/* 	"AID" : AID is of 로그아웃시 로그                                                                */
/* 	"CID" : CID identifies 로그아웃시 로그                                                           */
/* 	"DisTime" : DisTime is of 로그아웃시 로그                                                        */
/* 	"PlayTime" : PlayTime is of 로그아웃시 로그                                                      */
/* 	"Kills" : Kills is of 로그아웃시 로그                                                            */
/* 	"Deaths" : Deaths is of 로그아웃시 로그                                                          */  
create table "PlayerLog" ( 
	"id" int identity not null,
	"AID" int null,
	"CID" int null,
	"DisTime" datetime null,
	"PlayTime" int null,
	"Kills" int null,
	"Deaths" int null) ON 'PRIMARY'  

go

alter table "PlayerLog"
	add constraint "PlayerLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "ServerLog"을(를) 만듭니다.                                                                */
/* "ServerLog" : Table of 서버로그                                                                */
/* 	"id" : 서버로그id identifies 서버로그                                                             */
/* 	"ServerID" : ServerID is of 서버로그                                                          */
/* 	"PlayerCount" : PlayerCount is of 서버로그                                                    */
/* 	"GameCount" : 게임중인 방수 is of 서버로그                                                          */
/* 	"Time" : 동접시간 is of 서버로그                                                                  */  
create table "ServerLog" ( 
	"id" int identity not null,
	"ServerID" smallint null,
	"PlayerCount" smallint null,
	"GameCount" smallint null,
	"Time" smalldatetime null) ON 'PRIMARY'  

go

alter table "ServerLog"
	add constraint "PlayerCountLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "CharacterMakingLog"을(를) 만듭니다.                                                       */
/* "CharacterMakingLog" : Table of Entity4                                                    */
/* 	"id" : id identifies Entity4                                                              */
/* 	"AID" : AID is of Entity4                                                                 */
/* 	"CharName" : CharName is of Entity4                                                       */
/* 	"Type" : 캐릭터 만드는 타입 is of Entity4                                                         */
/* 	"Date" : Date is of Entity4                                                               */  
create table "CharacterMakingLog" ( 
	"id" int identity not null,
	"AID" int null,
	"CharName" varchar(32) null,
	"Type" varchar(20) null,
	"Date" datetime null) ON 'PRIMARY'  

go

alter table "CharacterMakingLog"
	add constraint "CharacterMakingLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "ShopLog"을(를) 만듭니다.                                                                  */
/* "ShopLog" : Table of Entity3                                                               */
/* 	"id" : id identifies Entity3                                                              */
/* 	"CID" : CID is of Entity3                                                                 */
/* 	"ItemID" : ItemID is of Entity3                                                           */
/* 	"Type" : 샵 구분 is of Entity3                                                               */  
create table "ShopLog" ( 
	"id" int identity not null,
	"CID" int null,
	"ItemID" int null,
	"Type" varchar(20) null) ON 'PRIMARY'  

go

alter table "ShopLog"
	add constraint "ShopLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "ChatLog"을(를) 만듭니다.                                                                  */
/* "ChatLog" : Table of 채팅 로그                                                                 */
/* 	"id" : id identifies 채팅 로그                                                                */
/* 	"CID" : CID is of 채팅 로그                                                                   */
/* 	"Msg" : Msg is of 채팅 로그                                                                   */
/* 	"Place" : Place is of 채팅 로그                                                               */
/* 	"Time" : 채팅시간 is of 채팅 로그                                                                 */  
create table "ChatLog" ( 
	"id" int identity not null,
	"CID" int null,
	"Msg" varchar(256) null,
	"Place" varchar(64) null,
	"Time" datetime null) ON 'PRIMARY'  

go

alter table "ChatLog"
	add constraint "ChatLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "Login"을(를) 만듭니다.                                                                    */
/* "Login" : Table of 로그인                                                                     */
/* 	"UserID" : UserID identifies 로그인                                                          */
/* 	"AID" : AID partly identifies 로그인                                                         */
/* 	"Password" : Password is of 로그인                                                           */
/* 	"LastConnDate" : LastConnDate is of 로그인                                                   */
/* 	"LastIP" : LastIP is of 로그인                                                               */  
create table "Login" ( 
	"UserID" varchar(20) not null,
	"AID" int not null,
	"Password" varchar(20) null,
	"LastConnDate" datetime null,
	"LastIP" varchar(20) null) ON 'PRIMARY'  

go

alter table "Login"
	add constraint "Login_PK" primary key clustered ("UserID")   


go

/* 새 테이블 "Ranking"을(를) 만듭니다.                                                                  */
/* "Ranking" : Table of 전체랭킹                                                                  */
/* 	"id" : id identifies 전체랭킹                                                                 */
/* 	"no" : no identifies 전체랭킹                                                                 */
/* 	"CID" : CID is of 전체랭킹                                                                    */
/* 	"CharName" : CharName is of 전체랭킹                                                          */
/* 	"Level" : Level is of 전체랭킹                                                                */
/* 	"XP" : XP is of 전체랭킹                                                                      */
/* 	"BP" : BP is of 전체랭킹                                                                      */
/* 	"GameCount" : GameCount is of 전체랭킹                                                        */
/* 	"KillCount" : KillCount is of 전체랭킹                                                        */
/* 	"DeathCount" : DeathCount is of 전체랭킹                                                      */  
create table "Ranking" ( 
	"id" int not null,
	"no" int not null,
	"CID" int not null,
	"CharName" varchar(24) not null,
	"Level" tinyint not null,
	"XP" int not null,
	"BP" int not null,
	"GameCount" int not null,
	"KillCount" int not null,
	"DeathCount" int not null) ON 'PRIMARY'  

go

alter table "Ranking"
	add constraint "Ranking_PK" primary key clustered ("id")   


go

/* 새 테이블 "KillLog"을(를) 만듭니다.                                                                  */
/* "KillLog" : Table of 게임 킬 로그                                                               */
/* 	"id" : id identifies 게임 킬 로그                                                              */
/* 	"AttackerCID" : AttackerCID is of 게임 킬 로그                                                 */
/* 	"VictimCID" : VictimCID is of 게임 킬 로그                                                     */
/* 	"Time" : 시간 is of 게임 킬 로그                                                                 */  
create table "KillLog" ( 
	"id" int identity not null,
	"AttackerCID" int null,
	"VictimCID" int null,
	"Time" datetime null) ON 'PRIMARY'  

go

alter table "KillLog"
	add constraint "KillLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "GuildItem"을(를) 만듭니다.                                                                */
/* "GuildItem" : Table of Entity2                                                             */
/* 	"GIID" : GIID identifies Entity2                                                          */
/* 	"GDID" : 길드ID is of Entity2                                                               */
/* 	"ItemID" : 아이템ID is of Entity2                                                            */  
create table "GuildItem" ( 
	"GIID" int identity not null,
	"GDID" int not null,
	"ItemID" int not null) ON 'PRIMARY'  

go

alter table "GuildItem"
	add constraint "테이블2_PK" primary key clustered ("GIID")   


go

/* 새 테이블 "AccountItem"을(를) 만듭니다.                                                              */
/* "AccountItem" : Table of Entity1                                                           */
/* 	"AIID" : AIID identifies Entity1                                                          */
/* 	"AID" : 계정ID is of Entity1                                                                */
/* 	"ItemID" : 아이템ID is of Entity1                                                            */  
create table "AccountItem" ( 
	"AIID" int identity not null,
	"AID" int not null,
	"ItemID" int not null) ON 'PRIMARY'  

go

alter table "AccountItem"
	add constraint "테이블1_PK" primary key clustered ("AIID")   


go

/* 새 테이블 "CharacterItem"을(를) 만듭니다.                                                            */
/* "CharacterItem" : Table of 캐릭터아이템                                                          */
/* 	"CIID" : CIID identifies 캐릭터아이템                                                           */
/* 	"CID" : 캐릭터ID is of 캐릭터아이템                                                                */
/* 	"ItemID" : 아이템ID partly identifies 캐릭터아이템                                                 */
/* 	"RegDate" : RegDate is of 캐릭터아이템                                                          */  
create table "CharacterItem" ( 
	"CIID" int identity not null,
	"CID" int null,
	"ItemID" int not null,
	"RegDate" datetime null) ON 'PRIMARY'  

go

alter table "CharacterItem"
	add constraint "CharacterItem_PK" primary key clustered ("CIID")   


go

/* 새 테이블 "Server"을(를) 만듭니다.                                                                   */
/* "Server" : Table of 서버정보                                                                   */
/* 	"SID" : SID identifies 서버정보                                                               */
/* 	"IP" : IP is of 서버정보                                                                      */
/* 	"Port" : Port is of 서버정보                                                                  */
/* 	"Name" : Name is of 서버정보                                                                  */
/* 	"Type" : Type is of 서버정보                                                                  */
/* 	"ServiceType" : ServiceType is of 서버정보                                                    */  
create table "Server" ( 
	"SID" int not null,
	"IP" varchar(20) null,
	"Port" int null,
	"Name" varchar(32) null,
	"Type" tinyint null,
	"ServiceType" tinyint null) ON 'PRIMARY'  

go

alter table "Server"
	add constraint "Server_PK" primary key clustered ("SID")   


go

/* 새 테이블 "ConnLog"을(를) 만듭니다.                                                                  */
/* "ConnLog" : Table of 접속 로그                                                                 */
/* 	"id" : id identifies 접속 로그                                                                */
/* 	"AID" : AID is of 접속 로그                                                                   */
/* 	"UserID" : UserID is of 접속 로그                                                             */
/* 	"Time" : 접속시간 is of 접속 로그                                                                 */
/* 	"IP" : 접속IP is of 접속 로그                                                                   */  
create table "ConnLog" ( 
	"id" int identity not null,
	"AID" int null,
	"UserID" varchar(20) null,
	"Time" datetime null,
	"IP" varchar(20) null) ON 'PRIMARY'  

go

alter table "ConnLog"
	add constraint "ConnLog_PK" primary key clustered ("id")   


go

/* 새 테이블 "Effect"을(를) 만듭니다.                                                                   */
/* "Effect" : Table of 이펙트                                                                    */
/* 	"ID" : EffID identifies 이펙트                                                               */
/* 	"Name" : 이펙트 이름 is of 이펙트                                                                 */
/* 	"Area" : Effective Area is of 이펙트                                                         */
/* 	"Time" : Time is of 이펙트                                                                   */
/* 	"ModHP" : ModHP is of 이펙트                                                                 */
/* 	"ModAP" : ModAP is of 이펙트                                                                 */
/* 	"ModMaxWT" : ModMaxWT is of 이펙트                                                           */
/* 	"ModSF" : ModSF is of 이펙트                                                                 */
/* 	"ModFR" : ModFR is of 이펙트                                                                 */
/* 	"ModCR" : ModCR is of 이펙트                                                                 */
/* 	"ModPR" : ModPR is of 이펙트                                                                 */
/* 	"ModLR" : ModLR is of 이펙트                                                                 */
/* 	"ResAP" : ResAP is of 이펙트                                                                 */
/* 	"ResFR" : ResFR is of 이펙트                                                                 */
/* 	"ResCR" : ResCR is of 이펙트                                                                 */
/* 	"ResPR" : ResPR is of 이펙트                                                                 */
/* 	"ResLR" : ResLR is of 이펙트                                                                 */
/* 	"Stun" : Stun is of 이펙트                                                                   */
/* 	"KnockBack" : KnockBack is of 이펙트                                                         */
/* 	"Smoke" : Smoke is of 이펙트                                                                 */
/* 	"Flash" : Flash is of 이펙트                                                                 */
/* 	"Tear" : Tear is of 이펙트                                                                   */
/* 	"Flame" : Flame is of 이펙트                                                                 */  
create table "Effect" ( 
	"ID" int not null,
	"Name" varchar(32) not null,
	"Area" int null,
	"Time" int null,
	"ModHP" int null,
	"ModAP" int null,
	"ModMaxWT" int null,
	"ModSF" int null,
	"ModFR" int null,
	"ModCR" int null,
	"ModPR" int null,
	"ModLR" int null,
	"ResAP" int null,
	"ResFR" int null,
	"ResCR" int null,
	"ResPR" int null,
	"ResLR" int null,
	"Stun" int null,
	"KnockBack" int null,
	"Smoke" int null,
	"Flash" int null,
	"Tear" int null,
	"Flame" int null) ON 'PRIMARY'  

go

alter table "Effect"
	add constraint "Effect_PK" primary key clustered ("ID")   


go

/* 새 테이블 "FortGameHistory"을(를) 만듭니다.                                                          */
/* "FortGameHistory" : Table of 요새전 게임 히스토리                                                   */
/* 	"id" : 요새전게임ID identifies 요새전 게임 히스토리                                                     */
/* 	"FortID" : 요새ID is of 요새전 게임 히스토리                                                         */
/* 	"WinnerID" : 길드ID is of 요새전 게임 히스토리                                                       */
/* 	"StartTime" : 시작시간 is of 요새전 게임 히스토리                                                      */
/* 	"EndTime" : 종료시간 is of 요새전 게임 히스토리                                                        */
/* 	"LoserID" : LoserID is of 요새전 게임 히스토리                                                     */  
create table "FortGameHistory" ( 
	"id" int identity not null,
	"FortID" int not null,
	"WinnerID" int null,
	"StartTime" datetime not null,
	"EndTime" datetime not null,
	"LoserID" int null) ON 'PRIMARY'  

go

alter table "FortGameHistory"
	add constraint "FortGameHistory_PK" primary key clustered ("id")   


go

/* 새 테이블 "Currency"을(를) 만듭니다.                                                                 */
/* "Currency" : Table of 화폐 단위                                                                */
/* 	"CurrencyID" : 화폐단위ID identifies 화폐 단위                                                    */
/* 	"Name" : 화폐 단위이름 is of 화폐 단위                                                              */  
create table "Currency" ( 
	"CurrencyID" int not null,
	"Name" varchar(128) not null) ON 'PRIMARY'  

go

alter table "Currency"
	add constraint "Currency_PK" primary key clustered ("CurrencyID")   


go

/* 새 테이블 "PremiumAccountHistory"을(를) 만듭니다.                                                    */
/* "PremiumAccountHistory" : Table of 유료 계정 내역                                                */
/* 	"PremiumAccountHistoryID" : PremiumAccountHistoryID partly identifies 유료 계정 내역            */
/* 	"StartDate" : 시작기간 is of 유료 계정 내역                                                         */
/* 	"EndDate" : EndDate is of 유료 계정 내역                                                        */
/* 	"BillingDate" : 결제일 is of 유료 계정 내역                                                        */
/* 	"BillingMethodID" : 결제방법 is of 유료 계정 내역                                                   */
/* 	"Cost" : 결제 금액 is of 유료 계정 내역                                                             */
/* 	"CurrencyID" : CurrencyID is of 유료 계정 내역                                                  */
/* 	"AID" : 계정No identifies 유료 계정 내역                                                          */  
create table "PremiumAccountHistory" ( 
	"PremiumAccountHistoryID" int identity not null,
	"StartDate" datetime null,
	"EndDate" datetime null,
	"BillingDate" datetime null,
	"BillingMethodID" int not null,
	"Cost" int not null,
	"CurrencyID" int not null,
	"AID" int not null) ON 'PRIMARY'  

go

alter table "PremiumAccountHistory"
	add constraint "PremiumAccountHistory_PK" primary key clustered ("PremiumAccountHistoryID")   


go

/* 새 테이블 "ItemTradeCharacter"을(를) 만듭니다.                                                       */
/* "ItemTradeCharacter" : Table of 아이템거래한 캐릭터의 아이템 목록                                         */
/* 	"ItemTradeID" : 거래로그ID identifies 아이템거래한 캐릭터의 아이템 목록                                      */
/* 	"CID" : 캐릭터ID partly identifies 아이템거래한 캐릭터의 아이템 목록                                        */
/* 	"ItemID" : 거래한 아이템ID is of 아이템거래한 캐릭터의 아이템 목록                                             */  
create table "ItemTradeCharacter" ( 
	"ItemTradeID" int not null,
	"CID" int not null,
	"ItemID" int not null) ON 'PRIMARY'  

go

alter table "ItemTradeCharacter"
	add constraint "ItemTradeCharacter_PK" primary key clustered ("ItemTradeID", "CID")   


go

/* 새 테이블 "ItemTradeLog"을(를) 만듭니다.                                                             */
/* "ItemTradeLog" : Table of 아이템 거래 로그                                                        */
/* 	"id" : 거래로그ID identifies 아이템 거래 로그                                                        */
/* 	"Date" : 거래날짜 is of 아이템 거래 로그                                                             */
/* 	"SrcCharID" : 캐릭터1 ID is of 아이템 거래 로그                                                     */
/* 	"SrcCharItemID" : 캐릭터1 거래 아이템 is of 아이템 거래 로그                                             */
/* 	"SrcCharBounty" : 캐릭터1 거래바운티 is of 아이템 거래 로그                                              */
/* 	"DestCharID" : 캐릭터2 ID partly identifies 아이템 거래 로그                                        */
/* 	"DestCharItemId" : 캐릭터2 거래 아이템 is of 아이템 거래 로그                                            */
/* 	"DestCharBounty" : 캐릭터2 거래바운티 is of 아이템 거래 로그                                             */  
create table "ItemTradeLog" ( 
	"id" int identity not null,
	"Date" datetime not null,
	"SrcCharID" int null,
	"SrcCharItemID" int null,
	"SrcCharBounty" int null,
	"DestCharID" int not null,
	"DestCharItemId" int null,
	"DestCharBounty" int null) ON 'PRIMARY'  

go

alter table "ItemTradeLog"
	add constraint "ItemTradeHistory_PK" primary key clustered ("id")   


go

/* 새 테이블 "ItemPurchaseLogByBounty"을(를) 만듭니다.                                                  */
/* "ItemPurchaseLogByBounty" : Table of 바운티로 아이템 구매 로그                                        */
/* 	"id" : Index partly identifies 바운티로 아이템 구매 로그                                             */
/* 	"ItemID" : 사는 아이템ID partly identifies 바운티로 아이템 구매 로그                                      */
/* 	"CID" : 캐릭터ID is of 바운티로 아이템 구매 로그                                                        */
/* 	"Date" : 구매날짜 partly identifies 바운티로 아이템 구매 로그                                            */
/* 	"Bounty" : 구매바운티 is of 바운티로 아이템 구매 로그                                                     */
/* 	"CharBounty" : 캐릭터잔액 is of 바운티로 아이템 구매 로그                                                 */
/* 	"Type" : 구매타입(구매,판매) is of 바운티로 아이템 구매 로그                                                 */  
create table "ItemPurchaseLogByBounty" ( 
	"id" int identity not null,
	"ItemID" int not null,
	"CID" int null,
	"Date" datetime null,
	"Bounty" int null,
	"CharBounty" int null,
	"Type" varchar(20) null) ON 'PRIMARY'  

go

alter table "ItemPurchaseLogByBounty"
	add constraint "PurchaseItemByBountyHistory_PK" primary key clustered ("id")   


go

/* 새 테이블 "ItemPurchaseByCashHistory"을(를) 만듭니다.                                                */
/* "ItemPurchaseByCashHistory" : Table of 현금으로 아이템 결제한 히스토리                                   */
/* 	"id" : id partly identifies 현금으로 아이템 결제한 히스토리                                             */
/* 	"AID" : AID is of 현금으로 아이템 결제한 히스토리                                                       */
/* 	"ItemID" : 사는 아이템ID partly identifies 현금으로 아이템 결제한 히스토리                                   */
/* 	"Date" : 결제날짜 partly identifies 현금으로 아이템 결제한 히스토리                                         */
/* 	"BillingMethodID" : 결제방식ID partly identifies 현금으로 아이템 결제한 히스토리                            */
/* 	"Cash" : 결제액 is of 현금으로 아이템 결제한 히스토리                                                      */  
create table "ItemPurchaseByCashHistory" ( 
	"id" int identity not null,
	"AID" int null,
	"ItemID" int not null,
	"Date" datetime not null,
	"BillingMethodID" int null,
	"Cash" int null) ON 'PRIMARY'  

go

alter table "ItemPurchaseByCashHistory"
	add constraint "PurchaseItemByCashHistory_PK" primary key clustered ("id")   


go

/* 새 테이블 "PurchaseMethod"을(를) 만듭니다.                                                           */
/* "PurchaseMethod" : Table of 구매 방식(특수,현금,바운티,PC방등)                                          */
/* 	"PurchaseMethodID" : 구매방식ID identifies 구매 방식(특수,현금,바운티,PC방등)                              */
/* 	"Name" : 구매 방식(특수,현금,바운티,PC방등)이름 is of 구매 방식(특수,현금,바운티,PC방등)                              */  
create table "PurchaseMethod" ( 
	"PurchaseMethodID" int not null,
	"Name" varchar(256) null) ON 'PRIMARY'  

go

alter table "PurchaseMethod"
	add constraint "PurchaseMethod_PK" primary key clustered ("PurchaseMethodID")   


go

/* 새 테이블 "ItemPurchaseMethod"을(를) 만듭니다.                                                       */
/* "ItemPurchaseMethod" : Table of 아이템 구매가능한 방식 목록                                            */
/* 	"ItemID" : 아이템ID identifies 아이템 구매가능한 방식 목록                                               */
/* 	"PurchaseMethodID" : 구매방식ID partly identifies 아이템 구매가능한 방식 목록                             */  
create table "ItemPurchaseMethod" ( 
	"ItemID" int not null,
	"PurchaseMethodID" int not null) ON 'PRIMARY'  

go

alter table "ItemPurchaseMethod"
	add constraint "ItemPurchaseMethod_PK" primary key clustered ("ItemID", "PurchaseMethodID")   


go

/* 새 테이블 "GameLog"을(를) 만듭니다.                                                                  */
/* "GameLog" : Table of 게임 히스토리                                                               */
/* 	"id" : 게임ID identifies 게임 히스토리                                                            */
/* 	"GameName" : GameName is of 게임 히스토리                                                       */
/* 	"MasterCID" : MasterCID is of 게임 히스토리                                                     */
/* 	"Map" : MAP1 is of 게임 히스토리                                                                */
/* 	"GameType" : GAMETYPE2 is of 게임 히스토리                                                      */
/* 	"Round" : 라운드수 is of 게임 히스토리                                                              */
/* 	"StartTime" : 시작시간 is of 게임 히스토리                                                          */
/* 	"PlayerCount" : PlayerCount is of 게임 히스토리                                                 */
/* 	"Players" : Players is of 게임 히스토리                                                         */  
create table "GameLog" ( 
	"id" int identity not null,
	"GameName" varchar(64) null,
	"MasterCID" int null,
	"Map" varchar(32) null,
	"GameType" varchar(24) null,
	"Round" int null,
	"StartTime" datetime not null,
	"PlayerCount" tinyint null,
	"Players" varchar(1000) null) ON 'PRIMARY'  

go

alter table "GameLog"
	add constraint "GameHistory_PK" primary key clustered ("id")   


go

/* 새 테이블 "BillingMethod"을(를) 만듭니다.                                                            */
/* "BillingMethod" : Table of 현금 결제 방식                                                        */
/* 	"BillingMethodID" : 결제방식ID identifies 현금 결제 방식                                            */
/* 	"Name" : 현금 결제 방식이름 is of 현금 결제 방식                                                        */  
create table "BillingMethod" ( 
	"BillingMethodID" int not null,
	"Name" varchar(256) not null) ON 'PRIMARY'  

go

alter table "BillingMethod"
	add constraint "BillingMethod_PK" primary key clustered ("BillingMethodID")   


go

/* 새 테이블 "GameType"을(를) 만듭니다.                                                                 */
/* "GameType" : Table of 게임타입                                                                 */
/* 	"GameTypeID" : 게임타입ID identifies 게임타입                                                     */
/* 	"Name" : 게임타입이름 is of 게임타입                                                                */  
create table "GameType" ( 
	"GameTypeID" int not null,
	"Name" varchar(256) null) ON 'PRIMARY'  

go

alter table "GameType"
	add constraint "GameType_PK" primary key clustered ("GameTypeID")   


go

/* 새 테이블 "Map"을(를) 만듭니다.                                                                      */
/* "Map" : Table of 맵                                                                         */
/* 	"MapID" : 맵ID identifies 맵                                                                */
/* 	"Name" : 맵이름 is of 맵                                                                      */
/* 	"MaxPlayer" : Max인원 is of 맵                                                               */  
create table "Map" ( 
	"MapID" int not null,
	"Name" varchar(256) not null,
	"MaxPlayer" int not null) ON 'PRIMARY'  

go

alter table "Map"
	add constraint "Map_PK" primary key clustered ("MapID")   


go

/* 새 테이블 "Fort"을(를) 만듭니다.                                                                     */
/* "Fort" : Table of 요새                                                                       */
/* 	"FortID" : 요새ID identifies 요새                                                             */
/* 	"Level" : 난이도 is of 요새                                                                    */
/* 	"TrialLevel" : 방어전회수 is of 요새                                                             */
/* 	"GDID" : 점령길드ID is of 요새                                                                  */
/* 	"OccupationDate" : 최초 점령일 is of 요새                                                        */
/* 	"InitBonusRate" : 초기 보너스 is of 요새                                                         */
/* 	"CurrentBonusRate" : 현재 보너스 is of 요새                                                      */
/* 	"MapID" : 맵ID is of 요새                                                                    */  
create table "Fort" ( 
	"FortID" int identity not null,
	"Level" smallint null constraint "FortLevel_Chk" check ([Level] >= 1 and [Level] <= 3) ,
	"TrialLevel" smallint not null,
	"GDID" int not null,
	"OccupationDate" datetime null,
	"InitBonusRate" float null,
	"CurrentBonusRate" float null,
	"MapID" int not null) ON 'PRIMARY'  

go

alter table "Fort"
	add constraint "Fort_PK" primary key clustered ("FortID")   


go

/* 새 테이블 "TaskCharacter"을(를) 만듭니다.                                                            */
/* "TaskCharacter" : Table of 태스크를 수행하는 캐릭터                                                   */
/* 	"TaskID" : 태스크ID identifies 태스크를 수행하는 캐릭터                                                 */
/* 	"CID" : 캐릭터ID partly identifies 태스크를 수행하는 캐릭터                                             */  
create table "TaskCharacter" ( 
	"TaskID" int not null,
	"CID" int not null) ON 'PRIMARY'  

go

alter table "TaskCharacter"
	add constraint "TaskCharacter_PK" primary key clustered ("TaskID", "CID")   


go

/* 새 테이블 "Task"을(를) 만듭니다.                                                                     */
/* "Task" : Table of 태스크                                                                      */
/* 	"TaskID" : 태스크ID identifies 태스크                                                           */
/* 	"Title" : Title is of 태스크                                                                 */
/* 	"Comment" : Comment is of 태스크                                                             */
/* 	"TrialLevel" : 시도 레벨 is of 태스크                                                            */
/* 	"ClientCharID" : 의뢰인ID is of 태스크                                                          */
/* 	"TargetCharID" : 타겟ID is of 태스크                                                           */
/* 	"RewardXP" : Reward XP is of 태스크                                                          */
/* 	"RewardBP" : Reward BP is of 태스크                                                          */
/* 	"RegDate" : 등록일 is of 태스크                                                                 */
/* 	"MaxContractor" : 최대 가용 청부인수 is of 태스크                                                    */  
create table "Task" ( 
	"TaskID" int identity not null,
	"Title" varchar(256) null,
	"Comment" varchar(256) null,
	"TrialLevel" int not null,
	"ClientCharID" int not null,
	"TargetCharID" int not null,
	"RewardXP" int not null,
	"RewardBP" int not null,
	"RegDate" datetime not null,
	"MaxContractor" int null) ON 'PRIMARY'  

go

alter table "Task"
	add constraint "Task_PK" primary key clustered ("TaskID")   


go

/* 새 테이블 "FriendList"을(를) 만듭니다.                                                               */
/* "FriendList" : Table of 친구 목록                                                              */
/* 	"CID" : 캐릭터ID identifies 친구 목록                                                            */
/* 	"FriendID" : 친구ID partly identifies 친구 목록                                                 */  
create table "FriendList" ( 
	"CID" int not null,
	"FriendID" int not null) ON 'PRIMARY'  

go

alter table "FriendList"
	add constraint "FriendList_PK" primary key clustered ("CID", "FriendID")   


go

/* 새 테이블 "Item"을(를) 만듭니다.                                                                     */
/* "Item" : Table of 아이템                                                                      */
/* 	"ItemID" : 아이템ID identifies 아이템                                                           */
/* 	"Name" : 아이템이름 is of 아이템                                                                  */
/* 	"TotalPoint" : TotalPoint is of 아이템                                                       */
/* 	"ItemTypeID" : 종류 is of 아이템                                                               */
/* 	"ResSex" : 성별제한 is of 아이템                                                                 */
/* 	"ResRace" : 종족제한 is of 아이템                                                                */
/* 	"ResLevel" : 레벨제한 is of 아이템                                                               */
/* 	"Slot" : 슬롯 is of 아이템                                                                     */
/* 	"Weight" : 무게 is of 아이템                                                                   */
/* 	"CashPrice" : 현금 판매 금액 is of 아이템                                                          */
/* 	"BountyPrice" : 바운티 판매 금액 is of 아이템                                                       */
/* 	"Value" : Value is of 아이템                                                                 */
/* 	"Damage" : Bonus Damage is of 아이템                                                         */
/* 	"Delay" : Delay is of 아이템                                                                 */
/* 	"EffectID" : 이펙트 is of 아이템                                                                */
/* 	"Controllability" : 제어성 is of 아이템                                                         */
/* 	"Magazine" : 장탄수 is of 아이템                                                                */
/* 	"ReloadTime" : 재장전시간 is of 아이템                                                            */
/* 	"SlugOutput" : SlugOutput is of 아이템                                                       */
/* 	"Gadget" : 무기옵션 is of 아이템                                                                 */
/* 	"HP" : HP조정수치 is of 아이템                                                                   */
/* 	"AP" : AP조정수치 is of 아이템                                                                   */
/* 	"MAXWT" : MAX WT 조정수치 is of 아이템                                                           */
/* 	"SF" : SF조정수치 is of 아이템                                                                   */
/* 	"FR" : FRModifier is of 아이템                                                               */
/* 	"CR" : CRModifier is of 아이템                                                               */
/* 	"PR" : PRModifier is of 아이템                                                               */
/* 	"LR" : LRModifier is of 아이템                                                               */
/* 	"BlendColor" : 블렌딩되는 색깔 is of 아이템                                                         */
/* 	"ModelName" : 모델 파일 이름 is of 아이템                                                          */
/* 	"Description" : Description is of 아이템                                                     */
/* 	"WebImgName" : WebImgName is of 아이템                                                       */  
create table "Item" ( 
	"ItemID" int not null,
	"Name" varchar(256) not null,
	"TotalPoint" int null,
	"ItemTypeID" int null,
	"ResSex" tinyint null,
	"ResRace" tinyint null,
	"ResLevel" int null,
	"Slot" tinyint null,
	"Weight" int null,
	"CashPrice" int null,
	"BountyPrice" int null,
	"Value" int null,
	"Damage" int null,
	"Delay" int null,
	"EffectID" int null,
	"Controllability" int null,
	"Magazine" int null,
	"ReloadTime" int null,
	"SlugOutput" tinyint null,
	"Gadget" int null,
	"HP" int null,
	"AP" int null,
	"MAXWT" int null,
	"SF" int null,
	"FR" int null,
	"CR" int null,
	"PR" int null,
	"LR" int null,
	"BlendColor" int null,
	"ModelName" varchar(64) null,
	"Description" text null,
	"WebImgName" varchar(64) null) ON 'PRIMARY' TEXTIMAGE_ON 'PRIMARY'  

go

alter table "Item"
	add constraint "Item_PK" primary key clustered ("ItemID")   


go

/* 새 테이블 "GuildMember"을(를) 만듭니다.                                                              */
/* "GuildMember" : Table of 길드회원                                                              */
/* 	"GDID" : 길드ID identifies 길드회원                                                             */
/* 	"CID" : 캐릭터ID partly identifies 길드회원                                                      */  
create table "GuildMember" ( 
	"GDID" int not null,
	"CID" int not null) ON 'PRIMARY'  

go

alter table "GuildMember"
	add constraint "GuildMember_PK" primary key clustered ("GDID", "CID")   


go

/* 새 테이블 "Guild"을(를) 만듭니다.                                                                    */
/* "Guild" : Table of 길드                                                                      */
/* 	"GDID" : 길드ID identifies 길드                                                               */
/* 	"Level" : 레벨 is of 길드                                                                     */
/* 	"TotalBounty" : 누적바운티 is of 길드                                                            */
/* 	"CurrentBounty" : 잔여바운티 is of 길드                                                          */
/* 	"RegDate" : 생성일 is of 길드                                                                  */
/* 	"BonusRate" : 바운티 보너스율 is of 길드                                                           */
/* 	"BountyRate" : 바운티 상납율 is of 길드                                                           */
/* 	"Items" : Items is of 길드                                                                  */
/* 	"IsActive" : 액티브 여부 is of 길드                                                              */  
create table "Guild" ( 
	"GDID" int identity not null,
	"Level" smallint null,
	"TotalBounty" int not null,
	"CurrentBounty" int not null,
	"RegDate" datetime null,
	"BonusRate" float not null,
	"BountyRate" float not null,
	"Items" binary(4000) null,
	"IsActive" int not null) ON 'PRIMARY'  

go

alter table "Guild"
	add constraint "Guild_PK" primary key clustered ("GDID")   


go

/* 새 테이블 "Character"을(를) 만듭니다.                                                                */
/* "Character" : Table of 캐릭터                                                                 */
/* 	"CID" : 캐릭터ID identifies 캐릭터                                                              */
/* 	"AID" : AID partly identifies 캐릭터                                                         */
/* 	"Name" : 캐릭터이름 is of 캐릭터                                                                  */
/* 	"Level" : 레벨 is of 캐릭터                                                                    */
/* 	"Sex" : 성별 is of 캐릭터                                                                      */
/* 	"CharNum" : 계정의 캐릭인덱스 is of 캐릭터                                                           */
/* 	"Country" : Country is of 캐릭터                                                             */
/* 	"Race" : 종족 is of 캐릭터                                                                     */
/* 	"Hair" : Hair is of 캐릭터                                                                   */
/* 	"Face" : Face is of 캐릭터                                                                   */
/* 	"XP" : 캐릭터 총경험치 is of 캐릭터                                                                 */
/* 	"BP" : 캐릭터 총바운티 is of 캐릭터                                                                 */
/* 	"BonusRate" : 바운티 보너스율 is of 캐릭터                                                          */
/* 	"Prize" : 현상금 is of 캐릭터                                                                   */
/* 	"HP" : 기본HP is of 캐릭터                                                                     */
/* 	"AP" : 기본AP is of 캐릭터                                                                     */
/* 	"MaxWeight" : 소지가능무게 is of 캐릭터                                                            */
/* 	"SafeFalls" : safe falls is of 캐릭터                                                        */
/* 	"FR" : 화 방어력 is of 캐릭터                                                                    */
/* 	"CR" : 수 방어력 is of 캐릭터                                                                    */
/* 	"ER" : 지 방어력 is of 캐릭터                                                                    */
/* 	"WR" : 풍 방어력 is of 캐릭터                                                                    */
/* 	"head_slot" : Head 슬롯 아이템 is of 캐릭터                                                       */
/* 	"chest_slot" : Chest 슬롯 아이템 is of 캐릭터                                                     */
/* 	"hands_slot" : hands 슬롯 아이템 is of 캐릭터                                                     */
/* 	"legs_slot" : legs 슬롯 아이템 is of 캐릭터                                                       */
/* 	"feet_slot" : feet 슬롯 아이템 is of 캐릭터                                                       */
/* 	"fingerl_slot" : lfinger 슬롯 아이템 is of 캐릭터                                                 */
/* 	"fingerr_slot" : rfinger 슬롯 아이템 is of 캐릭터                                                 */
/* 	"melee_slot" : melee 슬롯 아이템 is of 캐릭터                                                     */
/* 	"primary_slot" : primary 슬롯 아이템 is of 캐릭터                                                 */
/* 	"secondary_slot" : secondary 슬롯 아이템 is of 캐릭터                                             */
/* 	"custom1_slot" : item1 슬롯 아이템 is of 캐릭터                                                   */
/* 	"custom2_slot" : item2 슬롯 아이템 is of 캐릭터                                                   */
/* 	"RegDate" : RegDate is of 캐릭터                                                             */
/* 	"LastTime" : LastTime is of 캐릭터                                                           */
/* 	"PlayTime" : PlayTime is of 캐릭터                                                           */
/* 	"GameCount" : GameCount is of 캐릭터                                                         */
/* 	"KillCount" : KillCount is of 캐릭터                                                         */
/* 	"DeathCount" : DeathCount is of 캐릭터                                                       */
/* 	"DeleteFlag" : 삭제한 캐릭인지 여부                                                                */
/* 	"DeleteName" : 삭제된 캐릭 이름                                                                  */  
create table "Character" ( 
	"CID" int identity not null,
	"AID" int not null,
	"Name" varchar(24) not null,
	"Level" smallint not null,
	"Sex" tinyint not null,
	"CharNum" smallint not null,
	"Country" tinyint null,
	"Race" tinyint null,
	"Hair" tinyint null,
	"Face" tinyint null,
	"XP" int not null,
	"BP" int not null,
	"BonusRate" float not null,
	"Prize" int null,
	"HP" smallint null,
	"AP" smallint null,
	"MaxWeight" int null,
	"SafeFalls" int null,
	"FR" int null,
	"CR" int null,
	"ER" int null,
	"WR" int null,
	"head_slot" int null,
	"chest_slot" int null,
	"hands_slot" int null,
	"legs_slot" int null,
	"feet_slot" int null,
	"fingerl_slot" int null,
	"fingerr_slot" int null,
	"melee_slot" int null,
	"primary_slot" int null,
	"secondary_slot" int null,
	"custom1_slot" int null,
	"custom2_slot" int null,
	"RegDate" datetime null,
	"LastTime" datetime null,
	"PlayTime" int null,
	"GameCount" int null,
	"KillCount" int null,
	"DeathCount" int null,
	"DeleteFlag" tinyint null,
	"DeleteName" varchar(24) null,
	"md1point" int default 0 not null,
	"md2point" int default 0 not null,
	"md3point" int default 0 not null,
	"md4point" int default 0 not null,
	"weekpoint" int default 0 not null,
	"monthpoint" int default 0 not null) ON 'PRIMARY'  

go

alter table "Character"
	add constraint "Character_PK" primary key clustered ("CID")   


go

/* 새 테이블 "PremiumGrade"을(를) 만듭니다.                                                             */
/* "PremiumGrade" : Table of 유료이용자 권한                                                         */
/* 	"PGradeID" : 유료이용자권한ID identifies 유료이용자 권한                                                */
/* 	"Name" : 권한 이름 is of 유료이용자 권한                                                             */  
create table "PremiumGrade" ( 
	"PGradeID" int not null,
	"Name" varchar(128) not null) ON 'PRIMARY'  

go

alter table "PremiumGrade"
	add constraint "PremiumGrade_PK" primary key clustered ("PGradeID")   


go

/* 새 테이블 "UserGrade"을(를) 만듭니다.                                                                */
/* "UserGrade" : Table of 이용자 권한                                                              */
/* 	"UGradeID" : 이용자권한ID identifies 이용자 권한                                                    */
/* 	"Name" : 권한 이름 is of 이용자 권한                                                               */  
create table "UserGrade" ( 
	"UGradeID" int not null,
	"Name" varchar(128) not null) ON 'PRIMARY'  

go

alter table "UserGrade"
	add constraint "UserGrade_PK" primary key clustered ("UGradeID")   


go

/* 새 테이블 "Account"을(를) 만듭니다.                                                                  */
/* "Account" : Table of 계정                                                                    */
/* 	"AID" : 계정ID identifies 계정                                                                */
/* 	"UserID" : 사용자ID identifies 계정                                                            */
/* 	"UGradeID" : 등급 partly identifies 계정                                                      */
/* 	"PGradeID" : 유료이용자권한 is of 계정                                                             */
/* 	"RegDate" : 등록일자 is of 계정                                                                 */
/* 	"Name" : 계정이름 is of 계정                                                                    */
/* 	"Email" : email is of 계정                                                                  */
/* 	"RegNum" : 주민등록번호 is of 계정                                                                */
/* 	"Age" : Age is of 계정                                                                      */
/* 	"Sex" : 성별 is of 계정                                                                       */
/* 	"ZipCode" : 우편번호 is of 계정                                                                 */
/* 	"Address" : 주소 is of 계정                                                                   */
/* 	"Country" : 출신국가 is of 계정                                                                 */
/* 	"LastCID" : LastCID is of 계정                                                              */  
create table "Account" ( 
	"AID" int identity not null,
	"UserID" varchar(20) not null,
	"UGradeID" int not null,
	"PGradeID" int not null,
	"RegDate" datetime not null,
	"Name" varchar(50) not null,
	"Email" varchar(50) null,
	"RegNum" varchar(50) null,
	"Age" smallint null,
	"Sex" tinyint null,
	"ZipCode" varchar(50) null,
	"Address" varchar(256) null,
	"Country" varchar(50) null,
	"LastCID" int null,
	"Cert" tinyint null) ON 'PRIMARY'  

go

alter table "Account"
	add constraint "Account_PK" primary key clustered ("AID")   


go

/* "MonthRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                          */
create index "INDEX_MonthRanking_rank" on "MonthRanking" (
	"rank")  


go

/* "TotalRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                          */
create index "INDEX_TotalRanking_name" on "TotalRanking" (
	"name")  


go

create index "INDEX_TotalRanking_rank" on "TotalRanking" (
	"rank")  


go

/* "ModeRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                           */
create index "INDEX_ModeRanking_md1rank" on "ModeRanking" (
	"md1rank")  


go

create index "INDEX_ModeRanking_md2rank" on "ModeRanking" (
	"md2rank")  


go

create index "INDEX_ModeRanking_md3rank" on "ModeRanking" (
	"md3rank")  


go

create index "INDEX_ModeRanking_md4rank" on "ModeRanking" (
	"md4rank")  


go

create index "INDEX_ModeRanking_name" on "ModeRanking" (
	"name")  


go

/* "MonthHonorRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                     */
create index "INDEX_MonthHonorRanking_date_rank" on "MonthHonorRanking" (
	"date",
	"rank")  


go

/* "WeekHonorRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                      */
create index "INDEX_WeekHonorRanking_date_rank" on "WeekHonorRanking" (
	"date",
	"rank")  


go

/* "WeekRanking" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                           */
create index "INDEX_WeekRanking_rank" on "WeekRanking" (
	"rank")  


go

/* "CashSetShop" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                           */
create index "IX_CashSetShop_RegDate" on "CashSetShop" (
	"RegDate") ON 'PRIMARY'  


go

/* "CashShop" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                              */
create index "IX_CashShop_RegDate" on "CashShop" (
	"RegDate") ON 'PRIMARY'  


go

/* "CharacterItem" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                         */
create index "CharacterItemCIDIndex" on "CharacterItem" (
	"CID")  


go

create index "CharacterItemItemIDIndex" on "CharacterItem" (
	"ItemID")  


go

/* "Character" 테이블에 대한 나머지 키, 제약 조건 및 인덱스를 추가합니다.                                             */
create index "IX_Character_AID" on "Character" (
	"AID",
	"CharNum")  


go

create index "IX_Character_Name" on "Character" (
	"Name")  


go

/* "CashSetItem" 테이블에 외래 키 제약 조건을 추가합니다.                                                      */
alter table "CashSetItem"
	add constraint "Item_CashSetItem_FK1" foreign key (
		"CSID")
	 references "CashShop" (
		"CSID") on update no action on delete no action  

go

alter table "CashSetItem"
	add constraint "CashSetShop_CashSetItem_FK1" foreign key (
		"CSSID")
	 references "CashSetShop" (
		"CSSID") on update no action on delete no action  

go

/* "CashShop" 테이블에 외래 키 제약 조건을 추가합니다.                                                         */
alter table "CashShop"
	add constraint "Item_CashShop_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "GuildItem" 테이블에 외래 키 제약 조건을 추가합니다.                                                        */
alter table "GuildItem"
	add constraint "Guild_테이블2_FK1" foreign key (
		"GDID")
	 references "Guild" (
		"GDID") on update no action on delete no action  

go

alter table "GuildItem"
	add constraint "Item_테이블2_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "AccountItem" 테이블에 외래 키 제약 조건을 추가합니다.                                                      */
alter table "AccountItem"
	add constraint "Account_테이블1_FK1" foreign key (
		"AID")
	 references "Account" (
		"AID") on update no action on delete no action  

go

alter table "AccountItem"
	add constraint "Item_테이블1_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "CharacterItem" 테이블에 외래 키 제약 조건을 추가합니다.                                                    */
alter table "CharacterItem"
	add constraint "Item_CharacterItem_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

alter table "CharacterItem"
	add constraint "Character_CharacterItem_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

/* "FortGameHistory" 테이블에 외래 키 제약 조건을 추가합니다.                                                  */
alter table "FortGameHistory"
	add constraint "Fort_FortGameHistory_FK1" foreign key (
		"FortID")
	 references "Fort" (
		"FortID") on update no action on delete no action  

go

alter table "FortGameHistory"
	add constraint "Guild_FortGameHistory_FK1" foreign key (
		"WinnerID")
	 references "Guild" (
		"GDID") on update no action on delete no action  

go

alter table "FortGameHistory"
	add constraint "Guild_FortGameHistory_FK2" foreign key (
		"LoserID")
	 references "Guild" (
		"GDID") on update no action on delete no action  

go

/* "PremiumAccountHistory" 테이블에 외래 키 제약 조건을 추가합니다.                                            */
alter table "PremiumAccountHistory"
	add constraint "Currency_PremiumAccountHistory_FK1" foreign key (
		"CurrencyID")
	 references "Currency" (
		"CurrencyID") on update no action on delete no action  

go

alter table "PremiumAccountHistory"
	add constraint "BillingMethod_PremiumAccountHistory_FK1" foreign key (
		"BillingMethodID")
	 references "BillingMethod" (
		"BillingMethodID") on update no action on delete no action  

go

alter table "PremiumAccountHistory"
	add constraint "Account_PremiumAccountHistory_FK1" foreign key (
		"AID")
	 references "Account" (
		"AID") on update no action on delete no action  

go

/* "ItemTradeCharacter" 테이블에 외래 키 제약 조건을 추가합니다.                                               */
alter table "ItemTradeCharacter"
	add constraint "ItemTradeHistory_ItemTradeCharacter_FK1" foreign key (
		"ItemTradeID")
	 references "ItemTradeLog" (
		"id") on update no action on delete no action  

go

alter table "ItemTradeCharacter"
	add constraint "Character_ItemTradeCharacter_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "ItemTradeCharacter"
	add constraint "Item_ItemTradeCharacter_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "ItemTradeLog" 테이블에 외래 키 제약 조건을 추가합니다.                                                     */
alter table "ItemTradeLog"
	add constraint "Character_ItemTradeHistory_FK1" foreign key (
		"DestCharID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "ItemTradeLog"
	add constraint "Character_ItemTradeHistory_FK2" foreign key (
		"SrcCharID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "ItemTradeLog"
	add constraint "Item_ItemTradeHistory_FK1" foreign key (
		"SrcCharItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

alter table "ItemTradeLog"
	add constraint "Item_ItemTradeHistory_FK2" foreign key (
		"DestCharItemId")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "ItemPurchaseLogByBounty" 테이블에 외래 키 제약 조건을 추가합니다.                                          */
alter table "ItemPurchaseLogByBounty"
	add constraint "Item_PurchaseItemByBountyHistory_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

alter table "ItemPurchaseLogByBounty"
	add constraint "Character_PurchaseItemByBountyHistory_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

/* "ItemPurchaseByCashHistory" 테이블에 외래 키 제약 조건을 추가합니다.                                        */
alter table "ItemPurchaseByCashHistory"
	add constraint "BillingMethod_PurchaseItemByCashHistory_FK1" foreign key (
		"BillingMethodID")
	 references "BillingMethod" (
		"BillingMethodID") on update no action on delete no action  

go

alter table "ItemPurchaseByCashHistory"
	add constraint "Item_PurchaseItemByCashHistory_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

/* "ItemPurchaseMethod" 테이블에 외래 키 제약 조건을 추가합니다.                                               */
alter table "ItemPurchaseMethod"
	add constraint "Item_ItemPurchaseMethod_FK1" foreign key (
		"ItemID")
	 references "Item" (
		"ItemID") on update no action on delete no action  

go

alter table "ItemPurchaseMethod"
	add constraint "PurchaseMethod_ItemPurchaseMethod_FK1" foreign key (
		"PurchaseMethodID")
	 references "PurchaseMethod" (
		"PurchaseMethodID") on update no action on delete no action  

go

/* "Fort" 테이블에 외래 키 제약 조건을 추가합니다.                                                             */
alter table "Fort"
	add constraint "Guild_Fort_FK1" foreign key (
		"GDID")
	 references "Guild" (
		"GDID") on update no action on delete no action  

go

alter table "Fort"
	add constraint "Map_Fort_FK1" foreign key (
		"MapID")
	 references "Map" (
		"MapID") on update no action on delete no action  

go

/* "TaskCharacter" 테이블에 외래 키 제약 조건을 추가합니다.                                                    */
alter table "TaskCharacter"
	add constraint "Task_TaskCharacter_FK1" foreign key (
		"TaskID")
	 references "Task" (
		"TaskID") on update no action on delete no action  

go

alter table "TaskCharacter"
	add constraint "Character_TaskCharacter_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

/* "Task" 테이블에 외래 키 제약 조건을 추가합니다.                                                             */
alter table "Task"
	add constraint "Character_Task_FK1" foreign key (
		"ClientCharID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "Task"
	add constraint "Character_Task_FK2" foreign key (
		"TargetCharID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

/* "FriendList" 테이블에 외래 키 제약 조건을 추가합니다.                                                       */
alter table "FriendList"
	add constraint "Character_FriendList_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "FriendList"
	add constraint "Character_FriendList_FK2" foreign key (
		"FriendID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

/* "Item" 테이블에 외래 키 제약 조건을 추가합니다.                                                             */
alter table "Item"
	add constraint "Effect_Item_FK1" foreign key (
		"EffectID")
	 references "Effect" (
		"ID") on update no action on delete no action  

go

alter table "Item"
	add constraint "ItemType_Item_FK1" foreign key (
		"ItemTypeID")
	 references "ItemType" (
		"ItemTypeID") on update no action on delete no action  

go

/* "GuildMember" 테이블에 외래 키 제약 조건을 추가합니다.                                                      */
alter table "GuildMember"
	add constraint "Character_GuildMember_FK1" foreign key (
		"CID")
	 references "Character" (
		"CID") on update no action on delete no action  

go

alter table "GuildMember"
	add constraint "Guild_GuildMember_FK1" foreign key (
		"GDID")
	 references "Guild" (
		"GDID") on update no action on delete no action  

go

/* "Character" 테이블에 외래 키 제약 조건을 추가합니다.                                                        */
alter table "Character"
	add constraint "Account_Character_FK1" foreign key (
		"AID")
	 references "Account" (
		"AID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK1" foreign key (
		"head_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK2" foreign key (
		"chest_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK3" foreign key (
		"hands_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK4" foreign key (
		"legs_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK5" foreign key (
		"feet_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK6" foreign key (
		"fingerl_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK7" foreign key (
		"fingerr_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK8" foreign key (
		"melee_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK9" foreign key (
		"primary_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK10" foreign key (
		"secondary_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK11" foreign key (
		"custom1_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

alter table "Character"
	add constraint "CharacterItem_Character_FK12" foreign key (
		"custom2_slot")
	 references "CharacterItem" (
		"CIID") on update no action on delete no action  

go

/* "Account" 테이블에 외래 키 제약 조건을 추가합니다.                                                          */
alter table "Account"
	add constraint "UserGrade_Account_FK1" foreign key (
		"UGradeID")
	 references "UserGrade" (
		"UGradeID") on update no action on delete no action  

go

alter table "Account"
	add constraint "PremiumGrade_Account_FK1" foreign key (
		"PGradeID")
	 references "PremiumGrade" (
		"PGradeID") on update no action on delete no action  

go


/* Microsoft Visual Studio가 생성한 SQL DDL 스크립트의 끝입니다.                                           */
