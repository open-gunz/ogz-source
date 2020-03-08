/* 계정 추가 */
CREATE PROC [spInsertAccount] 
	@UserID		varchar(20)
,	@Password	varchar(20)
, @Cert		tinyint
,	@Name		varchar(128)
, @Age		smallint
,	@Sex		tinyint

AS

DECLARE @AIDIdent 	int

INSERT INTO Account 
	(UserID, Cert, Name, Age, Sex, UGradeID, PGradeID, RegDate)
Values
	(@UserID, @Cert, @Name, @Age, @Sex, 0, 0, GETDATE())

SET @AIDIdent = @@IDENTITY

INSERT INTO Login
	(UserID, AID, Password)
Values 
	(@UserID, @AIDIdent, @Password)

GO	

/* LoginInfo 얻어옴 */
CREATE PROC [spGetLoginInfo]
	@UserID		varchar(20)
AS
SELECT AID, UserID, Password FROM Login WHERE UserID = @UserID

Go



/* 성별 정보 얻어옴 */
CREATE PROC [spGetSexInfo]
	@AID		int
AS
SELECT Sex, RegNum, Email FROM Account WHERE AID=@AID

Go

/* 계정 정보 얻어옴 */
CREATE PROC [spGetAccountInfo]
	@AID		int
AS
SELECT AID, UserID, UGradeID, PGradeID, RegDate, Name, Email, RegNum, Sex, ZipCode, Address
FROM Account WHERE AID=@AID

Go

/* 캐릭터 삭제 */
CREATE PROC [spDeleteChar]
	@AID		int,
	@CharNum	smallint,
	@CharName	varchar(24)
AS

UPDATE Character SET CharNum = -1, DeleteFlag = 1, Name='', DeleteName=@CharName
WHERE AID=@AID AND CharNum=@CharNum AND Name=@CharName

Go


/* 계정의 캐릭터 리스트 가져오기 - 성능상 문제가 있다. 나중에 더 좋은 방법을 찾아보자 */
CREATE PROC [spGetCharList]
	@AID		int
AS
SELECT *, 
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.head_slot = ci.CIID) head_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.chest_slot = ci.CIID) chest_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.hands_slot = ci.CIID) hands_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.legs_slot = ci.CIID) legs_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.feet_slot = ci.CIID) feet_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.fingerl_slot = ci.CIID) fingerl_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.fingerr_slot = ci.CIID) fingerr_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.melee_slot = ci.CIID) melee_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.primary_slot = ci.CIID) primary_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.secondary_slot = ci.CIID) secondary_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.custom1_slot = ci.CIID) custom1_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.custom2_slot = ci.CIID) custom2_ItemID
FROM Character AS c WITH (nolock)
WHERE c.AID=@AID AND c.DeleteFlag = 0

/* SELECT * FROM Character WHERE AID=@AID AND DeleteFlag=0 ORDER BY CharNum */
Go

/* 캐릭터 정보 가져오기 */
CREATE PROC [spGetCharInfoByCharNum]
	@AID		int
,	@CharNum	smallint
AS
SELECT * FROM Character WITH (nolock) WHERE AID=@AID and CharNum=@CharNum

Go

/* 캐릭터 정보 가져오기 */
CREATE PROC [spGetCharInfoByCID]
	@CID		int
AS
SELECT * FROM Character WITH (nolock) WHERE CID=@CID
Go


/* 캐릭터 정보 가져오기 - 장비 아이템 정보까지 - 성능상 문제있다. */
CREATE PROC [spGetCharInfoExByCID]
	@CID		int
AS
SELECT *, 
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.head_slot = ci.CIID) head_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.chest_slot = ci.CIID) chest_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.hands_slot = ci.CIID) hands_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.legs_slot = ci.CIID) legs_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.feet_slot = ci.CIID) feet_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.fingerl_slot = ci.CIID) fingerl_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.fingerr_slot = ci.CIID) fingerr_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.melee_slot = ci.CIID) melee_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.primary_slot = ci.CIID) primary_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.secondary_slot = ci.CIID) secondary_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.custom1_slot = ci.CIID) custom1_ItemID,
  (SELECT ci.ItemID FROM CharacterItem ci WHERE c.custom2_slot = ci.CIID) custom2_ItemID
FROM Character AS c WITH (nolock)
WHERE c.CID=@CID

Go


/* 캐릭터 정보 업데이트 - 아이템 정보는 업데이트 하지 않는다. */
CREATE PROC [spUpdateChar]
	@Name		varchar(24),
	@CharNum	smallint,
	@Level		smallint,
	@Sex		tinyint,
	@Hair		tinyint,
	@Face		tinyint,
	@XP		int,
	@BP		int,
	@BonusRate	float,
	@Prize		int,
	@HP		smallint,
	@AP		smallint,
	@MaxWeight	int,
	@SafeFalls	int,
	@FR		smallint,
	@CR		smallint,
	@ER		smallint,
	@WR		smallint
AS
UPDATE Character WITH (rowlock)
SET Name=@Name, Level=@Level, Sex=@Sex, Hair=@Hair, Face=@Face, XP=@XP, BP=@BP, BonusRate=@BonusRate,
  Prize=@Prize, HP=@HP, AP=@AP, MaxWeight=@MaxWeight, SafeFalls=@SafeFalls, FR=@FR,
  CR=@CR, ER=@ER, WR=@WR
WHERE Name=@Name and CharNum=@CharNum
Go

/* 캐릭터 정보 업데이트 - 한사람 죽일때마다 업데이트한다. */
CREATE PROC [spSimpleUpdateChar]
	@CID		int,
	@Name		varchar(24),
	@Level		smallint,
	@XP		int,
	@BP		int
AS
UPDATE Character WITH (rowlock)
SET Level=@Level, XP=@XP, BP=@BP
WHERE CID=@CID AND Name=@Name
Go
	

/* 캐릭터 아이템 추가 */
CREATE PROC [spInsertCharItem]
	@CID		int,
	@ItemID		int
AS
DECLARE @OrderCIID	int

INSERT INTO CharacterItem
	(CID, ItemID, RegDate)
Values
	(@CID, @ItemID, GETDATE())

SET @OrderCIID = @@IDENTITY
SELECT @OrderCIID as ORDERCIID

Go

/* 캐릭터 아이템 삭제 */
CREATE PROC [spDeleteCharItem]
	@CID		int,
	@CIID		int
AS

UPDATE CharacterItem SET CID=NULL
WHERE CID=@CID AND CIID=@CIID

/* 예전꺼
DELETE FROM CharacterItem 
WHERE CID=@CID AND CIID=@CIID
*/
Go

/* 캐릭터 아이템 보기 */
CREATE PROC [spSelectCharItem]
	@CID		int
AS
SELECT * FROM CharacterItem
WHERE CID=@CID ORDER BY CIID
Go

/* 캐릭터 장비 아이템 보기 */
CREATE PROC [spSelectCharEquipedItemCIID]
AS
SELECT head_slot, chest_slot, hands_slot, legs_slot, feet_slot, 
fingerl_slot, fingerr_slot, melee_slot, secondary_slot, custom1_slot,
custom2_slot 
FROM Character WHERE CID=1
Go

/* 캐릭터 장비 업데이트 */
CREATE PROC [spUpdateCharEquipedItem]
	@CID		int,
	@HEAD		int,
	@CHEST		int,
	@HANDS		int,
	@LEGS		int,
	@FEET		int,
	@FINGERL	int,
	@FINGERR	int,
	@MELEE		int,
	@PRIMARY	int,
	@SECONDARY	int,
	@CUSTOM1	int,
	@CUSTOM2	int
AS
UPDATE Character WITH (rowlock)
SET head_slot=@HEAD, chest_slot=@CHEST, hands_slot=@HANDS, legs_slot=@LEGS, feet_slot=@FEET,
  fingerl_slot=@FINGERL, fingerr_slot=@FINGERR, melee_slot=@MELEE, 
  primary_slot=@PRIMARY, secondary_slot=@SECONDARY,
  custom1_slot=@CUSTOM1, custom2_slot=@CUSTOM2
WHERE CID=@CID
Go

/* 접속 로그 추가 */
CREATE PROC [spInsertConnLog] 
	@AID		int
,	@UserID		varchar(20)
,	@IP		varchar(20)
AS
INSERT INTO ConnLog 
	(AID, UserID, Time, IP)
Values
	(@AID, @UserID, GETDATE(), @IP)
GO	

/* 게임 로그 추가 */
CREATE PROC [spInsertGameLog]
	@GameName	varchar(64),
	@Map		varchar(32),
	@GameType	varchar(24),
	@Round		int,
	@MasterCID	int,
	@PlayerCount	tinyint,
	@Players	varchar(1000)

AS
INSERT INTO GameLog
	(GameName, Map, GameType, Round, MasterCID, StartTime, PlayerCount, Players)
VALUES
	(@GameName, @Map, @GameType, @Round, @MasterCID, GETDATE(), @PlayerCount, @Players)
GO

/* 킬 로그 추가 */
CREATE PROC [spInsertKillLog]
	@AttackerCID	int,
	@VictimCID	int
AS
INSERT INTO KillLog
	(AttackerCID, VictimCID, Time)
VALUES
	(@AttackerCID, @VictimCID, GETDATE())
Go

/* 아이템 구매(바운티) 로그 */
CREATE PROC [spInsertItemPurchaseLogByBounty]
	@ItemID		int,
	@CID		int,
	@Bounty		int,
	@CharBounty	int,
	@Type		varchar(20)
AS
INSERT INTO ItemPurchaseLogByBounty
	(ItemID, CID, Date, Bounty, CharBounty, Type)
VALUES
	(@ItemID, @CID, GETDATE(), @Bounty, @CharBounty, @Type)
Go

/* 캐릭터 생성 로그 */
CREATE PROC [spInsertCharMakingLog]
	@AID		int,
	@CharName	varchar(32),
	@Type		varchar(20)
AS
INSERT INTO CharacterMakingLog
	(AID, CharName, Type, Date)
VALUES
	(@AID, @CharName, @Type, GETDATE())
Go

/* 서버 로그 */
CREATE PROC [spInsertServerLog]
  @ServerID     smallint,
	@PlayerCount	smallint,
	@GameCount	  smallint
AS
INSERT INTO ServerLog
	(ServerID, PlayerCount, GameCount, Time)
VALUES
	(@ServerID, @PlayerCount, @GameCount, GETDATE())
Go

/* 플레이어 로그 */
CREATE PROC [spInsertPlayerLog]
  @AID          int,
	@CID          int,
	@PlayTime     int,
	@Kills        int,
	@Deaths       int
AS
INSERT INTO PlayerLog
	(AID, CID, DisTime, PlayTime, Kills, Deaths)
VALUES
	(@AID, @CID, GETDATE(), @PlayTime, @Kills, @Deaths)
Go

/* 서버 동접자 상태 */
CREATE PROC [spUpdateServerStatus]
  @CurrPlayer   smallint,
  @ServerID     int
AS

UPDATE ServerStatus 
Set CurrPlayer=@CurrPlayer, Time=GETDATE() 
WHERE ServerID=@ServerID

Go

/* 레벨 업데이트 */
CREATE PROC [spUpdateCharLevel]
  @Level        smallint,
  @CID          int
AS

UPDATE Character 
Set Level=@Level 
WHERE CID=@CID

Go


/* BP 업데이트 */
CREATE PROC [spUpdateCharBP]
  @BPInc        int,
  @CID          int
AS

UPDATE Character 
SET BP=BP+(@BPInc) 
WHERE CID=@CID

Go


/* 캐릭터 정보(XP, BP, KillCount, DeathCount) 업데이트 */
CREATE PROC [spUpdateCharInfoData]
  @XPInc        int,
  @BPInc        int,
  @KillInc      int,
  @DeathInc     int,
  @CID          int
AS
  
UPDATE Character 
SET XP=XP+(@XPInc), BP=BP+(@BPInc), KillCount=KillCount+(@KillInc), DeathCount=DeathCount+(@DeathInc)
WHERE CID=@CID

Go



/* 캐릭터 플레이 시간 업데이트 */
CREATE PROC [spUpdateCharPlayTime]
  @PlayTimeInc    int,
  @CID            int
AS

UPDATE Character 
SET PlayTime=PlayTime+(@PlayTimeInc), LastTime=GETDATE() 
WHERE CID=@CID

Go


