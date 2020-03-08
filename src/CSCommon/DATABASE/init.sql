use "GunzDB"  

go

/* 이용자 권한 목록(UserGrade) */
INSERT INTO UserGrade (UGradeID, Name) Values (0, '무료 계정')
INSERT INTO UserGrade (UGradeID, Name) Values (1, '정액 유저')
INSERT INTO UserGrade (UGradeID, Name) Values (253, 'Blocked')
INSERT INTO UserGrade (UGradeID, Name) Values (254, '개발자')
INSERT INTO UserGrade (UGradeID, Name) Values (255, '관리자')

/* 유료이용자 권한 목록(PremiumGrade) */
INSERT INTO PremiumGrade (PGradeID, Name) Values (0, '무료')

/* 게임타입 목록(GameType) */
INSERT INTO GameType (GameTypeID, Name) Values (0, 'Solo Death Match')
INSERT INTO GameType (GameTypeID, Name) Values (1, 'Team Death Match')
INSERT INTO GameType (GameTypeID, Name) Values (2, 'Solo Gladiator')
INSERT INTO GameType (GameTypeID, Name) Values (3, 'Team Gladiator')
INSERT INTO GameType (GameTypeID, Name) Values (4, 'Assassinate')

/* 화폐 단위(Currency) */
INSERT INTO Currency (CurrencyID, Name) Values (0, '원')

/* 현금 결제 방식(BillingMethod) */
INSERT INTO BillingMethod (BillingMethodID, Name) Values (0, '계좌이체')
INSERT INTO BillingMethod (BillingMethodID, Name) Values (1, '신용카드')
INSERT INTO BillingMethod (BillingMethodID, Name) Values (2, '핸드폰')
INSERT INTO BillingMethod (BillingMethodID, Name) Values (3, '700서비스')
INSERT INTO BillingMethod (BillingMethodID, Name) Values (4, 'ADSL')

/* 아이템 구매 방식(PurchaseMethod) */
INSERT Into PurchaseMethod (PurchaseMethodID, Name) Values (0, '현금')
INSERT Into PurchaseMethod (PurchaseMethodID, Name) Values (1, '바운티')
INSERT Into PurchaseMethod (PurchaseMethodID, Name) Values (2, '현금&바운티')
INSERT Into PurchaseMethod (PurchaseMethodID, Name) Values (3, '특수(이벤트)')

/* 맵 */
INSERT Into Map (MapID, Name, MaxPlayer) values (0, 'Mansion', 32)
INSERT Into Map (MapID, Name, MaxPlayer) values (1, 'Prison', 32)
INSERT Into Map (MapID, Name, MaxPlayer) values (2, 'Prison II', 32)
INSERT INTO Map (MapID, Name, MaxPlayer) values (3, 'Station', 32)

/* 서버상태 */
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (1, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (2, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (3, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (4, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (5, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (6, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (7, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (8, 0, 0)
INSERT Into ServerStatus (ServerID, CurrPlayer, MaxPlayer) Values (9, 0, 0)


go
