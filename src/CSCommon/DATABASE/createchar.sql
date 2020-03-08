/* 캐릭터 추가 */
CREATE PROC [spInsertChar]
	@AID		int,
	@CharNum	smallint,
	@Name		varchar(24),
	@Sex		tinyint,
	@Hair		int,  
	@Face		int,
	@Costume	int
AS
IF EXISTS (SELECT CID FROM Character where (AID=@AID AND CharNum=@CharNum) OR (Name=@Name))
BEGIN	
	return(-1)
END

DECLARE @CharIdent 	int
DECLARE @ChestCIID	int
DECLARE @LegsCIID	int
DECLARE @MeleeCIID	int
DECLARE @PrimaryCIID	int
DECLARE @SecondaryCIID  int
DECLARE @Custom1CIID	int
DECLARE @Custom2CIID	int

INSERT INTO Character 
	(AID, Name, CharNum, Country, Level, Sex, Hair, Face, XP, BP, BonusRate, MaxWeight, SafeFalls, FR, CR, ER, WR, 
         GameCount, KillCount, DeathCount, RegDate, PlayTime, DeleteFlag)
Values
	(@AID, @Name, @CharNum, 0, 1, @Sex, @Hair, @Face, 0, 0, 0, 100, 500, 0, 0, 0, 0, 
         0, 0, 0, GETDATE(), 0, 0)

SET @CharIdent = @@IDENTITY


  /* Melee */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 1
    WHEN 1 THEN 2
    WHEN 2 THEN 1
    WHEN 3 THEN 2
    WHEN 4 THEN 2
    WHEN 5 THEN 1
    END
  )
  SET @MeleeCIID = @@IDENTITY

  /* Primary */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 5001
    WHEN 1 THEN 5002
    WHEN 2 THEN 4005
    WHEN 3 THEN 4001
    WHEN 4 THEN 4002
    WHEN 5 THEN 4006
    END
  )
  SET @PrimaryCIID = @@IDENTITY

  /* Secondary */
IF @Costume = 0 OR @Costume = 2
BEGIN
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 4001
    WHEN 1 THEN 0
    WHEN 2 THEN 5001
    WHEN 3 THEN 4006
    WHEN 4 THEN 0
    WHEN 5 THEN 4006
    END
  )
  SET @SecondaryCIID = @@IDENTITY
END

  /* Custom1 */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 30301
    WHEN 1 THEN 30301
    WHEN 2 THEN 30401
    WHEN 3 THEN 30401
    WHEN 4 THEN 30401
    WHEN 5 THEN 30101
    END
  )
  SET @Custom1CIID = @@IDENTITY

  /* Custom2 */
IF @Costume = 4 OR @Costume = 5
BEGIN
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 0
    WHEN 1 THEN 0
    WHEN 2 THEN 0
    WHEN 3 THEN 0
    WHEN 4 THEN 30001
    WHEN 5 THEN 30001
    END
  )
  SET @Custom2CIID = @@IDENTITY
END


IF @Sex = 0		/* 남자일 경우 */
BEGIN

  /* Chest */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 21001
    WHEN 1 THEN 21001
    WHEN 2 THEN 21001
    WHEN 3 THEN 21001
    WHEN 4 THEN 21001
    WHEN 5 THEN 21001
    END
  )
  SET @ChestCIID = @@IDENTITY

  /* Legs */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 23001
    WHEN 1 THEN 23001
    WHEN 2 THEN 23001
    WHEN 3 THEN 23001
    WHEN 4 THEN 23001
    WHEN 5 THEN 23001
    END
  )
  SET @LegsCIID = @@IDENTITY

END
ELSE
BEGIN			/* 여자일 경우 */

  /* Chest */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 21501
    WHEN 1 THEN 21501
    WHEN 2 THEN 21501
    WHEN 3 THEN 21501
    WHEN 4 THEN 21501
    WHEN 5 THEN 21501
    END
  )
  SET @ChestCIID = @@IDENTITY

  /* Legs */
  INSERT INTO CharacterItem (CID, ItemID) Values (@CharIdent, 
    CASE @Costume
    WHEN 0 THEN 23501
    WHEN 1 THEN 23501
    WHEN 2 THEN 23501
    WHEN 3 THEN 23501
    WHEN 4 THEN 23501
    WHEN 5 THEN 23501
    END
  )
  SET @LegsCIID = @@IDENTITY

END  

UPDATE Character
SET chest_slot = @ChestCIID, legs_slot = @LegsCIID, melee_slot = @MeleeCIID,
    primary_slot = @PrimaryCIID, secondary_slot = @SecondaryCIID, custom1_slot = @Custom1CIID,
    custom2_slot = @Custom2CIID
WHERE CID=@CharIdent
Go
