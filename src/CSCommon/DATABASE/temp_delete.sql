UPDATE Character SET head_slot = NULL, chest_slot=NULL, hands_slot=NULL, 
legs_slot=NULL, feet_slot=NULL, fingerl_slot=NULL, fingerr_slot=NULL, melee_slot=NULL,
primary_slot=NULL, secondary_slot=NULL, custom1_slot=NULL, custom2_slot=NULL
DELETE FROM CharacterITEM
DELETE FROM Character
DELETE FROM ITEM