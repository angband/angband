/* monsters.c: monster definitions

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "monster.h"

/*
		Attack types:
		1	Normal attack
		2	Poison Strength
		3	Confusion attack
		4	Fear attack
		5	Fire attack
		6	Acid attack
		7	Cold attack
		8	Lightning attack
		9	Corrosion attack
		10	Blindness attack
		11	Paralysis attack
		12	Steal Money
		13	Steal Object
		14	Poison
		15	Lose dexterity
		16	Lose constitution
		17	Lose intelligence
		18	Lose wisdom
		19	Lose experience
		20	Aggravation
		21	Disenchant
		22	Eats food
		23	Eat light
		24	Energy drain from pack
		25      Drain all stats
		99	Blank

		Attack descriptions:
		1	hits you.
		2	bites you.
		3	claws you.
		4	stings you.
		5	touches you.
		6	kicks you.
		7	gazes at you.
		8	breathes on you.
		9	spits on you.
		10	makes a horrible wail.
		11	embraces you.
		12	crawls on you.
		13	releases a cloud of spores.
		14	begs you for money.
		15	You've been slimed.
		16	crushes you.
		17	tramples you.
		18	drools on you.
		19	insults you.

		20	butts you.
		21	charges you.
		22	engulfs you.
                23      talks to you about mushrooms and dogs

		99	is repelled.

	Example:  For a creature which bites for 1d6, then stings for
		  2d4 and loss of dex you would use:
			{1,2,1,6},{15,4,2,4}

	Sleep (sleep)	:	A measure in turns of how fast creature
				will notice player (on the average).
	Area of affect (aaf) :	Max range that creature is able to "notice"
				the player.
									*/

creature_type c_list[MAX_CREATURES] = {

{"Filthy street urchin"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,40,4,1,11,'p',{1,4},{72,148,0,0},0,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Filthy street urchin"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(GROUP),(NONE8),(NONE8)
			    ,0,40,4,1,11,'p',{1,4},{72,148,0,0},0,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Scrawny cat"              ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
                            ,(NONE8)
			    ,0,10,30,1,11,'f',{1,2},{49,0,0,0},0,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Scruffy little dog"       ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
                            ,(NONE8)
			    ,0,5,20,1,11,'C',{1,3},{24,0,0,0},0,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Farmer Maggot"            ,(MV_ATT_NORM|CARRY_OBJ|HAS_90),(NONE8)
                            ,(UNIQUE|MAX_HP|CHARM_SLEEP|GOOD),(NONE8),(NONE8)
			    ,0,3,40,10,11,'h',{25,15},{283,283,0,0},0,4
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blubbering idiot"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,0,6,1,11,'p',{1,2},{79,0,0,0},0,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Boil-covered wretch"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,0,6,1,11,'p',{1,2},{79,0,0,0},0,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Village idiot"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,0,6,1,12,'p',{4,4},{79,0,0,0},0,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Pitiful looking beggar"   ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,40,10,1,11,'p',{1,4},{72,0,0,0},0,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Mangy looking leper"	    ,(MV_ATT_NORM|MV_20|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,50,10,1,11,'p',{1,1},{72,0,0,0},0,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Squint eyed rogue"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|
			      MV_ATT_NORM|THRO_DR|PICK_UP)
			    ,(NONE8),(EVIL),(NONE8),(NONE8)
			    ,0,99,10,8,11,'p',{2,8},{5,149,0,0},0,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Singing, happy drunk"	    ,(CARRY_GOLD|HAS_60|
			      MV_ATT_NORM|MV_40|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,0,10,1,11,'p',{2,3},{72,0,0,0},0,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Aimless looking merchant" ,(CARRY_GOLD|HAS_60|
			      MV_ATT_NORM|MV_40|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,255,10,1,11,'p',{3,3},{2,0,0,0},0,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Mean looking mercenary"   ,(CARRY_GOLD|CARRY_OBJ|HAS_90|
			      MV_ATT_NORM|MV_40|THRO_DR|PICK_UP)
			    ,(NONE8),(EVIL),(NONE8),(NONE8)
			    ,0,250,10,20,11,'p',{5,8},{9,0,0,0},0,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Battle scarred veteran"   ,(CARRY_GOLD|CARRY_OBJ|HAS_90|
			      MV_ATT_NORM|MV_40|THRO_DR|PICK_UP)
			    ,(NONE8),(NONE8),(NONE8),(NONE8)
			    ,0,250,10,30,11,'p',{7,8},{15,0,0,0},0,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Grey mold"		   ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL),(NONE8)
			   ,(NONE8),3,0,2,1,11,'m',{1,2},{3,3,0,0},1,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Grey mushroom patch"	   ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL),(NONE8)
			   ,(NONE8),1,0,2,1,11,',',{1,2},{91,0,0,0},1,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Giant yellow centipede"   ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,2,30,8,12,11,'c',{2,6},{26,60,0,0},1,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Giant white centipede"    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),2,40,7,10,11,'c',{3,5},{25,59,0,0},1,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"White icky thing"	    ,(MV_ATT_NORM|MV_75),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),2,10,12,7,11,'i',{3,5},{63,0,0,0},1,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Clear icky thing"	    ,(MV_ATT_NORM|MV_75|MV_INVIS),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,1,10,12,6,11,'i',{2,5},{63,0,0,0},1,1
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Giant white mouse"	    ,(MV_ATT_NORM|MV_40|MULTIPLY),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,1,20,8,4,11,'r',{1,3},{25,0,0,0},1,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Large brown snake"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,3,99,4,35,10,'R',{4,6},{26,73,0,0},1,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Large white snake"	    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,2,99,4,30,11,'R',{3,6},{24,0,0,0},1,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Small kobold"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|
			     THRO_DR|MV_ATT_NORM),(NONE8),(EVIL)
			    ,(NONE8),(NONE8)
			    ,5,10,20,16,11,'k',{2,7},{4,0,0,0},1,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Kobold"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|
			     THRO_DR|MV_ATT_NORM),(NONE8),(EVIL)
			    ,(NONE8),(NONE8)
			    ,5,10,20,16,11,'k',{3,7},{5,0,0,0},1,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"White worm mass"	    ,(MULTIPLY|MV_75|MV_ATT_NORM),(NONE8)
			    ,(ANIMAL|HURT_LIGHT|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,2,10,7,1,10,'w',{4,4},{173,0,0,0},1,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Floating eye"		    ,(MV_ONLY_ATT),(NONE8),(ANIMAL|HURT_LIGHT)
			    ,(NONE8),(NONE8)
			    ,1,10,2,6,11,'e',{3,6},{146,0,0,0},1,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Rock lizard"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,2,15,20,4,11,'R',{3,4},{24,0,0,0},1,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Jackal"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL|GROUP)
			    ,(NONE8),(NONE8)
			    ,1,10,10,3,11,'C',{1,4},{24,0,0,0},1,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Soldier ant"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,3,10,10,3,11,'a',{2,5},{25,0,0,0},1,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Fruit bat"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,1,10,20,3,12,'b',{1,6},{24,0,0,0},1,1
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Shrieker mushroom patch"  ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,1,0,2,1,11,',',{1,1},{203,0,0,0},2,1
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Blubbering icky thing"    ,(CARRY_GOLD|CARRY_OBJ|HAS_90|
			      PICK_UP|THRO_CREAT|MV_40|MV_ATT_NORM)
			    ,(NONE8),(ANIMAL|IM_POISON),(NONE8),(NONE8)
			    ,8,10,14,4,11,'i',{5,6},{174,210,0,0},2,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Metallic green centipede" ,(MV_40|MV_ATT_NORM),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,3,10,5,4,12,'c',{4,4},{68,0,0,0},2,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Novice warrior"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(NONE8),(NONE8),(NONE8),(NONE8)
			    ,6,5,20,16,11,'p',{9,4},{6,5,0,0},2,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Novice rogue"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,6,5,20,12,11,'p',{8,4},{5,148,0,0},2,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Novice priest"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(0xCL|CAUSE_LIGHT|FEAR)
			    ,(NONE8),(HEAL),(NONE8)
			    ,7,10,20,10,11,'p',{7,4},{4,0,0,0},2,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Novice mage"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(0xCL|CONFUSION|MAG_MISS|BLINK)
			    ,(NONE8),(NONE8),(NONE8)
			    ,7,5,20,6,11,'p',{6,4},{3,0,0,0},2,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Yellow mushroom patch"   ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL),(NONE8)
			   ,(NONE8),2,0,2,1,11,',',{1,1},{100,0,0,0},2,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"White jelly"		    ,(MV_ONLY_ATT),(NONE8)
			    ,(CHARM_SLEEP|ANIMAL|HURT_LIGHT|IM_POISON),(NONE8)
			    ,(NONE8),10,99,2,1,12,'j',{8,8},{168,0,0,0},2,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Giant green frog"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),6,30,12,8,11,'R',{2,8},{26,0,0,0},2,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Giant black ant"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),8,80,8,20,11,'a',{3,6},{27,0,0,0},2,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Salamander"		    ,(MV_ATT_NORM|MV_20),(NONE8),(IM_FIRE|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,10,80,8,20,11,'R',{4,6},{105,0,0,0},2,1
#ifdef TC_COLOR
  , RED
#endif
},

{"White harpy"		    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL|EVIL),(NONE8)
			  ,(NONE8),5,10,16,17,11,'H',{2,5},{49,49,25,0},2,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blue yeek"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			     MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),4,10,18,14,11,'y',{2,6},{4,0,0,0},2,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Grip, Farmer Maggot's dog" ,(MV_ATT_NORM|MV_20),(NONE8),(UNIQUE|MAX_HP|
			      CHARM_SLEEP|ANIMAL),(NONE8),(NONE8)
			     ,30,0,30,30,12,'C',{5,5},{27,0,0,0},2,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Fang, Farmer Maggot's dog" ,(MV_ATT_NORM|MV_20),(NONE8),(UNIQUE|MAX_HP|
			      CHARM_SLEEP|ANIMAL),(NONE8),(NONE8)
			     ,30,0,30,30,12,'C',{5,5},{27,0,0,0},2,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Green worm mass"	    ,(MULTIPLY|MV_75|MV_ATT_NORM),(NONE8)
			    ,(ANIMAL|HURT_LIGHT|IM_ACID),(NONE8)
			    ,(NONE8),3,10,7,3,10,'w',{6,4},{140,0,0,0},2,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Large black snake"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),9,75,5,38,10,'R',{4,8},{27,74,0,0},2,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Cave spider"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL|GROUP),(NONE8)
			    ,(NONE8),7,80,8,16,12,'S',{2,6},{27,0,0,0},2,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Wild cat"                 ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8)
                            ,(NONE8),8,0,40,12,12,'f',{3,5},{51,51,0,0},2,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Smeagol"		    ,(HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR|MV_75|
			      MV_ATT_NORM|MV_INVIS),(NONE8),(EVIL|UNIQUE)
			    ,(NONE8),(NONE8)
			    ,16,5,20,12,13,'h',{11,4},{3,148,0,0},3,2
#ifdef TC_COLOR
  , LIGHTGRAY  /* he's invis because amazingly stealthy, so make grey
  		instead of clear-colored (if he was clear, then he should
  		still have The Ring, which he doesn't) -CFT */
#endif
},

{"Green ooze"		    ,(HAS_90|CARRY_GOLD|CARRY_OBJ|MV_75|
			      MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,4,80,8,16,12,'j',{3,4},{140,0,0,0},3,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Poltergeist"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|HAS_90|PICK_UP|
			     MV_INVIS|THRO_WALL|MV_40|MV_75|
			     MV_ATT_NORM),(BLINK|0xFL)
			    ,(CHARM_SLEEP|HURT_LIGHT|EVIL|NO_INFRA|UNDEAD|IM_FROST)
			    ,(NONE8),(NONE8)
			    ,8,10,8,15,13,'G',{2,5},{93,0,0,0},3,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Metallic blue centipede"  ,(MV_40|MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),7,15,6,6,12,'c',{4,5},{69,0,0,0},3,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Giant white louse"	    ,(MULTIPLY|MV_ATT_NORM|MV_75)
			    ,(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,1,10,6,5,12,'l',{1,1},{24,0,0,0},3,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Black naga"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|MV_ATT_NORM|
			      MV_20),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,20,120,16,40,11,'n',{6,8},{75,0,0,0},3,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Spotted mushroom patch"   ,(MV_ONLY_ATT),(NONE8)
			    ,(CHARM_SLEEP|ANIMAL|IM_POISON),(NONE8)
			    ,(NONE8),3,0,2,1,11,',',{1,1},{175,0,0,0},3,1
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Silver jelly"		    ,(MV_ONLY_ATT),(0xFL|MANA_DRAIN),
			     (CHARM_SLEEP|ANIMAL|HURT_LIGHT|IM_POISON),(NONE8)
			  ,(NONE8),12,99,2,1,12,'j',{10,8},{213,213,0,0},3,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Yellow jelly"		    ,(MV_ONLY_ATT),(0xFL|MANA_DRAIN),
			     (CHARM_SLEEP|ANIMAL|HURT_LIGHT|IM_POISON),(NONE8)
			    ,(NONE8),12,99,2,1,12,'j',{10,8},{169,0,0,0},3,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Scruffy looking hobbit"   ,(CARRY_GOLD|CARRY_OBJ|HAS_60|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,4,10,16,8,11,'h',{3,5},{3,148,0,0},3,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Giant white ant"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,7,80,8,16,11,'a',{3,6},{27,0,0,0},3,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Yellow mold"		   ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL),(NONE8)
			   ,(NONE8),9,99,2,10,11,'m',{8,8},{3,0,0,0},3,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Metallic red centipede"   ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),12,20,8,9,12,'c',{4,8},{69,0,0,0},3,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Yellow worm mass"	    ,(MULTIPLY|MV_75|MV_ATT_NORM),(NONE8)
			    ,(ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,4,10,7,4,10,'w',{4,8},{182,0,0,0},3,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Clear worm mass"	    ,(MULTIPLY|MV_INVIS|MV_75|MV_ATT_NORM),(NONE8)
			    ,(ANIMAL|HURT_LIGHT|IM_POISON),(NONE8),(NONE8)
			    ,4,10,7,1,10,'w',{4,4},{173,0,0,0},3,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Radiation eye"	    ,(MV_ONLY_ATT),(0xBL|MANA_DRAIN)
			    ,(ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,6,10,2,6,11,'e',{3,6},{88,0,0,0},3,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Cave lizard"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,8,80,8,16,11,'R',{3,6},{28,0,0,0},4,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Novice ranger"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(0x9L|MAG_MISS)
			    ,(NONE8),(NONE8),(NONE8)
			    ,18,5,20,6,11,'p',{6,8},{4,4,0,0},4,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Novice paladin"           ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			     MV_ATT_NORM),(0x9L|CAUSE_LIGHT|FEAR),(NONE8)
			    ,(NONE8),(NONE8)
			    ,20,5,20,16,11,'p',{6,8},{6,6,0,0},4,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blue jelly"		    ,(MV_ONLY_ATT),(NONE8)
			    ,(CHARM_SLEEP|ANIMAL|HURT_LIGHT|IM_FROST|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,14,99,2,1,11,'j',{12,8},{125,0,0,0},4,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Creeping copper coins"    ,(HAS_1D2|CARRY_GOLD|MV_ATT_NORM)
			    ,(NONE8),(CHARM_SLEEP|ANIMAL|IM_POISON|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,9,10,3,24,10,'$',{7,8},{3,170,0,0},4,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Giant white rat"	    ,(MULTIPLY|MV_20|MV_ATT_NORM),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,1,30,8,7,11,'r',{2,2},{153,0,0,0},4,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blue worm mass"	    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8)
			    ,(ANIMAL|HURT_LIGHT|IM_FROST|NO_INFRA),(NONE8)
			    ,(NONE8),5,10,7,12,10,'w',{5,8},{129,0,0,0},4,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Large grey snake"	    ,(MV_20|MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),14,50,6,41,10,'R',{6,8},{28,75,0,0},4,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Bullroarer the Hobbit"    ,(CARRY_OBJ|HAS_2D2|THRO_DR|
			      MV_ATT_NORM),(NONE8),(UNIQUE|GOOD|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,90,10,16,8,12,'h',{8,8},{5,149,148,0},5,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Novice mage"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(0xCL|CONFUSION|MAG_MISS|BLINK|
			      BLINDNESS),(GROUP),(NONE8),(NONE8)
			    ,7,20,20,6,11,'p',{6,4},{3,0,0,0},5,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Green naga"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|
			     MV_ATT_NORM|MV_20),(NONE8),(EVIL|IM_ACID),(NONE8)
			 ,(NONE8),30,120,18,40,11,'n',{9,8},{75,118,0,0},5,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Blue ooze"		    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|MV_75|
			      MV_ATT_NORM),(NONE8),(ANIMAL|GROUP|IM_FROST|NO_INFRA),(NONE8)
			    ,(NONE8),7,80,8,16,11,'j',{3,4},{129,0,0,0},3,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Green glutton ghost"	    ,(CARRY_GOLD|CARRY_OBJ|HAS_60|HAS_90|PICK_UP|
			      THRO_WALL|MV_INVIS|MV_ATT_NORM|MV_40|MV_75)
			   ,(NONE8),(CHARM_SLEEP|EVIL|NO_INFRA|UNDEAD|IM_FROST),(NONE8)
			   ,(NONE8),15,10,10,20,13,'G',{3,4},{211,0,0,0},5,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Green jelly"		    ,(MV_ONLY_ATT),(NONE8)
			    ,(CHARM_SLEEP|ANIMAL|HURT_LIGHT|IM_ACID),(NONE8)
			    ,(NONE8),18,99,2,1,12,'j',{22,8},{136,0,0,0},5,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Large kobold"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL),(NONE8),(NONE8)
			    ,25,30,20,32,11,'k',{13,9},{9,0,0,0},5,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Skeleton kobold"	    ,(THRO_DR|MV_ATT_NORM),(NONE8)
			    ,(CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|
			     IM_POISON)
			    ,(NONE8),(NONE8)
			    ,12,40,20,26,11,'s',{5,8},{5,0,0,0},5,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Grey icky thing"	    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),10,15,14,12,11,'i',{4,8},{66,0,0,0},5,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Disenchanter eye"	    ,(MV_ONLY_ATT),(MANA_DRAIN|0x9L)
			    ,(ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,20,10,2,10,10,'e',{7,8},{207,0,0,0},5,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Red worm mass"	    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8)
			    ,(ANIMAL|HURT_LIGHT|IM_FIRE),(NONE8),(NONE8)
			    ,6,10,7,12,10,'w',{5,8},{111,0,0,0},5,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Copperhead snake"	    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,15,1,6,20,11,'R',{4,6},{158,0,0,0},5,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Purple mushroom patch"    ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,15,0,2,1,11,',',{1,1},{183,183,183,0},6,2
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Novice priest"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			     MV_ATT_NORM),(0xCL|CAUSE_LIGHT|FEAR),(GROUP)
			    ,(HEAL),(NONE8)
			    ,7,5,20,10,11,'p',{7,4},{4,0,0,0},6,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Novice warrior"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			     MV_ATT_NORM),(NONE8),(GROUP),(NONE8),(NONE8)
			    ,6,5,20,16,11,'p',{9,4},{6,5,0,0},6,2
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Novice rogue"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|PICK_UP|
			     MV_ATT_NORM),(NONE8),(EVIL|GROUP),(NONE8)
			    ,(NONE8)
			    ,6,5,20,12,11,'p',{8,4},{5,148,0,0},6,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Brown mold"		    ,(MV_ONLY_ATT),(NONE8),(CHARM_SLEEP|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,20,99,2,12,11,'m',{15,8},{89,0,0,0},6,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Giant brown bat"	    ,(MV_40|MV_20|MV_ATT_NORM),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,10,30,10,15,13,'b',{3,8},{26,0,0,0},6,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Novice archer"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_1D2)
			    ,(0x3L),(NONE8),(NONE8),(ARROW)
			    ,20,5,20,10,12,'p',{6,8},{3,3,0,0},6,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Creeping silver coins"    ,(HAS_1D2|CARRY_GOLD|HAS_60|MV_ATT_NORM)
			    ,(NONE8),(CHARM_SLEEP|ANIMAL|IM_POISON|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,18,10,4,30,10,'$',{12,8},{5,171,0,0},6,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Snaga"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(ORC|EVIL|GROUP|HURT_LIGHT),(NONE8)
			    ,(NONE8),15,30,20,32,11,'o',{8,8},{7,0,0,0},6,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Rattlesnake"		    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,20,1,6,24,11,'R',{6,7},{159,0,0,0},6,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Cave orc"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(ORC|EVIL|GROUP|HURT_LIGHT),(NONE8)
			    ,(NONE8),20,30,20,32,11,'o',{11,9},{7,0,0,0},7,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Wood spider"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL|GROUP|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,15,80,8,16,12,'S',{3,6},{26,165,0,0},7,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Manes"		    ,(THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(DEMON|EVIL|GROUP|IM_FIRE),(NONE8)
			    ,(NONE8),16,30,20,32,11,'I',{8,8},{7,0,0,0},7,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Bloodshot eye"	    ,(MV_ONLY_ATT),(0x7L|MANA_DRAIN)
			    ,(ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,15,10,2,6,11,'e',{5,8},{143,0,0,0},7,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Red naga"		    ,(CARRY_GOLD|CARRY_OBJ|HAS_60|MV_ATT_NORM|
			      MV_20),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,40,120,20,40,11,'n',{11,8},{76,82,0,0},7,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Red jelly"		    ,(MV_ONLY_ATT),(NONE8)
			    ,(CHARM_SLEEP|ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,26,99,2,1,11,'j',{26,8},{87,0,0,0},7,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Giant red frog"	    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8),16,50,12,16,11,'R',{5,8},{83,0,0,0},7,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Green icky thing"	    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL),(NONE8)
			   ,(NONE8),18,20,14,12,11,'i',{5,8},{137,0,0,0},7,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Zombie kobold"	    ,(THRO_DR|MV_ATT_NORM),(NONE8),
			     (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|
			     IM_POISON),(NONE8),(NONE8)
			     ,14,30,20,14,11,'z',{6,8},{1,1,0,0},7,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Lost soul"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|HAS_90|
			      PICK_UP|MV_INVIS|THRO_WALL|MV_ATT_NORM|
			      MV_20|MV_40),(0xFL|TELE|MANA_DRAIN)
			    ,(CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,18,10,12,10,11,'G',{2,8},{11,185,0,0},7,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Dark elf"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_90|THRO_DR|
			      MV_ATT_NORM),(0xAL|CONFUSION),(EVIL|HURT_LIGHT),
				(DARKNESS)
			    ,(NONE8),25,20,20,16,11,'h',{7,10},{5,5,0,0},7,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Night lizard"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,35,30,20,16,11,'R',{4,8},{29,29,0,0},7,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Mughash the Kobold Lord"  ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,100,20,20,20,11,'k',{12,12},{9,9,9,0},7,3
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Wormtongue, Agent of Saruman",
			     (MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(0x5L|FROST_BOLT|SLOW)
			    ,(EVIL|MAX_HP|UNIQUE|SPECIAL)
			    ,(TRAP_CREATE|HEAL|ST_CLOUD),(NONE8)
			    ,150,20,20,30,11,'p',{25,10},{4,4,148,0},8,1
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Lagduf, the Snaga"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(EVIL|MAX_HP|UNIQUE|GOOD),(NONE8)
			    ,(NONE8)
			    ,80,30,20,32,11,'o',{16,12},{9,9,8,8},8,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Brown yeek"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			      MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,11,10,18,18,11,'y',{4,8},{5,0,0,0},8,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Novice ranger"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|
			     MV_ATT_NORM),(0x9L|MAG_MISS),(GROUP)
			    ,(NONE8),(NONE8)
			    ,18,5,20,6,11,'p',{6,8},{4,4,0,0},8,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Giant salamander"	    ,(MV_ATT_NORM|MV_20),(0x9L|BREATH_FI)
			    ,(ANIMAL|IM_FIRE),(NONE8)
			    ,(NONE8),50,1,6,40,11,'R',{6,7},{106,0,0,0},8,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Green mold"		    ,(MV_ONLY_ATT),(NONE8),
			     (CHARM_SLEEP|ANIMAL|IM_ACID),(NONE8),(NONE8)
			    ,28,75,2,14,11,'m',{21,8},{94,0,0,0},8,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Skeleton orc"		    ,(MV_ATT_NORM|THRO_DR),
			     (NONE8),(CHARM_SLEEP|ORC|UNDEAD|EVIL|IM_POISON|
			     IM_FROST|NO_INFRA),(NONE8),(NONE8)
			    ,26,40,20,36,11,'s',{10,8},{14,0,0,0},8,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Seedy looking human"	    ,(HAS_1D2|CARRY_OBJ|CARRY_GOLD|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,22,20,20,26,11,'p',{8,9},{17,0,0,0},8,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Lemure"		    ,(THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(DEMON|EVIL|GROUP|IM_FIRE)
			    ,(NONE8),(NONE8)
			    ,16,30,20,32,11,'I',{13,9},{7,0,0,0},8,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Hill orc"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(ORC|EVIL|GROUP),(NONE8),(NONE8)
			    ,25,30,20,32,11,'o',{13,9},{9,0,0,0},8,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Bandit"		    ,(HAS_1D2|CARRY_OBJ|CARRY_GOLD|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,26,10,20,24,11,'p',{8,8},{13,148,0,0},8,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Yeti"			    ,(THRO_DR|MV_ATT_NORM),(NONE8),(ANIMAL|IM_FROST|
				NO_INFRA),(NONE8),(NONE8)
			    ,30,10,20,24,11,'Y',{11,9},{51,51,27,0},9,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Bloodshot icky thing"	    ,(MV_ATT_NORM|MV_40),(0xBL|MANA_DRAIN)
			    ,(ANIMAL|IM_POISON),(NONE8),(NONE8)
			    ,24,20,14,18,11,'i',{7,8},{65,139,0,0},9,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Giant grey rat"	    ,(MULTIPLY|MV_ATT_NORM|MV_20),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,2,20,8,12,11,'r',{2,3},{154,0,0,0},9,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Black harpy"		    ,(MV_ATT_NORM|MV_20),(NONE8),(EVIL|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,19,10,16,22,12,'H',{3,8},{50,50,26,0},9,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Orc shaman"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x8L|MAG_MISS|CAUSE_LIGHT|BLINK)
			    ,(EVIL|ORC|HURT_LIGHT)
			    ,(NONE8),(NONE8)
			    ,30,20,20,15,11,'o',{9,8},{5,5,0,0},9,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Baby blue dragon"	    ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_L),(IM_LIGHTNING|EVIL|DRAGON|
			     MAX_HP),(NONE8),(NONE8)
			    ,35,70,20,30,11,'d',{10,10},{51,51,28,0},9,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Baby white dragon"	    ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_FR),(IM_FROST|EVIL|DRAGON|MAX_HP|
			    	NO_INFRA),(NONE8),(NONE8)
			    ,35,70,20,30,11,'d',{10,10},{51,51,28,0},9,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Baby green dragon"	    ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_G),(IM_POISON|EVIL|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,35,70,20,30,11,'d',{10,10},{51,51,28,0},9,2
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Baby black dragon"	    ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_A),(IM_ACID|EVIL|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,35,70,20,30,11,'d',{10,10},{51,51,28,0},9,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Baby red dragon"	    ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_FI),(IM_FIRE|EVIL|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,35,70,20,30,11,'d',{10,11},{51,51,28,0},9,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Giant red ant"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,22,60,12,34,11,'a',{4,8},{27,85,0,0},9,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Brodda, the Easterling"   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(MAX_HP|UNIQUE|GOOD|EVIL),(NONE8),(NONE8)
			    ,100,20,20,25,11,'p',{30,7},{10,10,10,10},9,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"King cobra"		    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,28,1,8,30,11,'R',{8,10},{144,161,0,0},9,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Giant spider"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL|IM_POISON),(NONE8)
			    ,(NONE8)
			    ,35,80,8,16,11,'S',{10,10},{32,156,156,32},10,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Dark elven mage"	    ,(CARRY_OBJ|HAS_1D2|THRO_DR|MV_ATT_NORM)
			    ,(0x5L|BLINDNESS|MAG_MISS|CONFUSION),(EVIL|
			     IM_POISON|HURT_LIGHT),(ST_CLOUD|DARKNESS),(NONE8)
			    ,50,20,20,16,12,'h',{7,10},{5,5,0,0},10,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Orfax, Son of Boldor"    ,(CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x4L|MONSTER|TELE_TO|BLINK|CONFUSION|SLOW)
			    ,(INTELLIGENT|ANIMAL|EVIL|UNIQUE|GOOD|MAX_HP)
			    ,(HEAL),(NONE8)
			   ,80,10,18,20,12,'y',{12,10},{8,7,268,268},10,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Dark elven warrior"	    ,(CARRY_OBJ|CARRY_GOLD|HAS_1D2|THRO_DR|
			      MV_ATT_NORM),(NONE),(EVIL|HURT_LIGHT),
				(NONE8),(NONE8)
			    ,50,20,20,16,11,'h',{10,11},{7,7,0,0},10,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Clear mushroom patch"	    ,(MULTIPLY|MV_ONLY_ATT|MV_INVIS)
			    ,(NONE8),(CHARM_SLEEP|ANIMAL|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,3,0,4,1,12,',',{1,1},{70,0,0,0},10,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Grishnakh, the Hill Orc"  ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,160,20,20,20,11,'o',{15,15},{10,9,10,9},10,3
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Giant white tick"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL|IM_POISON),(NONE8)
			 ,(NONE8),27,20,12,40,10,'t',{12,8},{160,0,0,0},10,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Hairy mold"		   ,(MV_ONLY_ATT),(NONE8),(ANIMAL|CHARM_SLEEP),(NONE8)
			 ,(NONE8),32,70,2,15,11,'m',{15,8},{151,0,0,0},10,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Disenchanter mold"	    ,(MV_ONLY_ATT),(MANA_DRAIN|0xBL)
			    ,(ANIMAL|CHARM_SLEEP),(NONE8),(NONE8)
			    ,40,70,2,20,11,'m',{16,8},{206,0,0,0},10,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Pseudo dragon"	    ,(MV_ATT_NORM|CARRY_OBJ|CARRY_GOLD|HAS_60)
			    ,(0xBL|FEAR|CONFUSION),(DRAGON|MAX_HP)
			    ,(NONE8),(BREATH_LT|BREATH_DA)
			    ,150,40,20,30,11,'d',{22,9},{51,51,28,0},10,2
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Tengu"		    ,(THRO_DR|MV_ATT_NORM)
			    ,(0x3L|BLINK|TELE_TO),(DEMON|EVIL|IM_FIRE),(NONE8)
			   ,(NONE8),40,30,20,32,12,'I',{16,9},{7,0,0,0},10,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Creeping gold coins"	    ,(MV_ATT_NORM|HAS_1D2|HAS_90|CARRY_GOLD),(NONE8)
			    ,(ANIMAL|IM_POISON|NO_INFRA|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,32,10,5,36,10,'$',{18,8},{14,172,0,0},10,3
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Wolf"			  ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|GROUP),(NONE8)
			  ,(NONE8),30,20,30,30,12,'C',{6,6},{29,0,0,0},10,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Giant fruit fly"	    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,4,10,8,14,12,'F',{2,2},{25,0,0,0},10,6
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Panther"                  ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
                            ,25,0,40,30,12,'f',{10,8},{54,54,0,0},10,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Brigand"		    ,(HAS_1D2|CARRY_OBJ|CARRY_GOLD|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,35,10,20,32,11,'p',{9,8},{13,149,0,0},10,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Baby multi-hued dragon"   ,(MV_ATT_NORM|HAS_1D2|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0xBL|BREATH_FI|BREATH_FR|BREATH_G|BREATH_A|
			     BREATH_L)
			    ,(IM_FIRE|IM_FROST|IM_POISON|IM_ACID|IM_LIGHTNING|
			     EVIL|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,45,70,20,30,11,'d',{10,13},{51,51,28,0},11,2
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Hippogriff"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,30,10,12,14,11,'H',{20,9},{14,35,0,0},11,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Orc zombie"		    ,(THRO_DR|MV_ATT_NORM),(NONE8)
			    ,(CHARM_SLEEP|EVIL|UNDEAD|ORC|IM_FROST|NO_INFRA|
			     IM_POISON),(NONE8),(NONE8)
			    ,30,25,20,24,11,'z',{11,8},{3,3,3,0},11,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Gnome mage"		    ,(HAS_1D2|CARRY_OBJ|CARRY_GOLD|MV_ATT_NORM|
			     THRO_DR)
			    ,(0x4L|BLINK|FROST_BOLT|MONSTER),(EVIL)
			    ,(DARKNESS),(NONE8)
			    ,38,10,18,20,11,'h',{7,8},{4,0,0,0},11,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Black mamba"		    ,(MV_ATT_NORM|MV_40),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,40,1,10,32,12,'R',{10,8},{163,0,0,0},12,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"White wolf"		    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|GROUP|
			      IM_FROST),(NONE8),(NONE8)
			    ,30,20,30,30,12,'C',{7,7},{26,27,0,0},12,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Grape jelly"		    ,(MV_ONLY_ATT),(MANA_DRAIN|0xBL)
			    ,(HURT_LIGHT|CHARM_SLEEP|IM_POISON),(NONE8)
			  ,(NONE8),60,99,2,1,11,'j',{52,8},{186,0,0,0},12,3
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Nether worm mass"	    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8)
			    ,(ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,6,3,10,15,10,'w',{5,8},{186,0,0,0},12,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Golfimbul, the Hill Orc Chief"   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
			    ,(NONE8),(ORC|EVIL|IM_POISON|IM_FROST|IM_FIRE|
			     IM_LIGHTNING|MAX_HP|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,230,20,20,60,11,'o',{30,8},{10,10,9,9},12,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Master yeek"		    ,(CARRY_GOLD|CARRY_OBJ|HAS_60|THRO_DR|
			     MV_ATT_NORM),(0x4L|BLINK|TELE|MONSTER|
			     BLINDNESS|SLOW),(ANIMAL|EVIL),(ST_CLOUD),(NONE8)
			    ,28,10,18,24,11,'y',{12,9},{7,0,0,0},12,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Priest"		    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|MV_ATT_NORM|
			    THRO_DR),(0x3L|CAUSE_SERIOUS|MONSTER|FEAR)
			    ,(EVIL|INTELLIGENT)
			    ,(HEAL),(NONE8)
			    ,36,40,20,22,11,'p',{12,8},{12,12,0,0},12,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Dark elven priest"	    ,(CARRY_OBJ|HAS_1D2|THRO_DR|MV_ATT_NORM)
			    ,(0x5L|BLINDNESS|CAUSE_SERIOUS|CONFUSION)
			    ,(EVIL|INTELLIGENT|HURT_LIGHT)
			    ,(HEAL|DARKNESS),(NONE8)
			    ,50,30,20,30,12,'h',{7,10},{8,9,0,0},12,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Air spirit"		    ,(THRO_DR|MV_INVIS|MV_ATT_NORM|MV_75),(NONE8)
			    ,(EVIL|NO_INFRA|IM_POISON|
			      CHARM_SLEEP),(NONE8),(NONE8)
			    ,40,20,12,40,13,'E',{8,8},{2,0,0,0},12,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Skeleton human"	    ,(THRO_DR|MV_ATT_NORM),(NONE8),
			     (EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|NO_INFRA|
			     IM_POISON),(NONE8),(NONE8)
			    ,38,30,20,30,11,'s',{10,8},{7,0,0,0},12,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Zombie human"		    ,(THRO_DR|MV_ATT_NORM),(NONE8),
			     (EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|NO_INFRA|
			     IM_POISON),(NONE8),(NONE8)
			    ,34,20,20,24,11,'z',{12,8},{3,3,0,0},12,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Tiger"                    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
                            ,40,0,40,40,12,'f',{12,10},{54,54,29,0},12,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Moaning spirit"	    ,(CARRY_GOLD|CARRY_OBJ|HAS_60|HAS_90|
			      THRO_WALL|MV_INVIS|MV_ATT_NORM|MV_20)
			    ,(0xFL|TELE|FEAR)
			    ,(CHARM_SLEEP|EVIL|UNDEAD|IM_FROST|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,44,10,14,20,12,'G',{5,8},{99,178,0,0},12,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Swordsman"		    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|THRO_DR|
			      MV_ATT_NORM),(NONE8),(NONE8)
			    ,(NONE8),(NONE8)
			    ,40,20,20,34,11,'p',{12,8},{18,18,0,0},12,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Stegocentipede"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,40,30,12,30,12,'c',{13,8},{34,34,62,0},12,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Spotted jelly"	    ,(THRO_DR|MV_ONLY_ATT|PICK_UP),(NONE8)
			    ,(IM_ACID|IM_POISON|ANIMAL|CHARM_SLEEP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,33,1,12,18,12,'j',{13,8},{115,138,138,0},12,3
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Drider"		    ,(MV_ATT_NORM),(0x8L|CAUSE_LIGHT|CONFUSION)
			    ,(EVIL|IM_POISON),(DARKNESS),(NONE8)
			    ,55,80,8,30,11,'S',{10,13},{10,10,156,0},13,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Killer brown beetle"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,38,30,10,40,11,'K',{13,8},{41,0,0,0},13,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Boldor, King of the Yeeks",(CARRY_OBJ|HAS_90|HAS_1D2|THRO_DR|
			     MV_ATT_NORM),(0x3L|BLINK|TELE|MONSTER|
			     BLINDNESS|SLOW),(MAX_HP|
			     INTELLIGENT|ANIMAL|EVIL|UNIQUE|GOOD),(HEAL)
			  ,(NONE8),200,10,18,24,12,'y',{20,9},{8,8,7,0},13,3
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Ogre"			    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL|GROUP|GIANT),(NONE8),(NONE8)
			   ,50,30,20,33,11,'O',{13,9},{16,0,0,0},13,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Creeping mithril coins"   ,(MV_ATT_NORM|HAS_2D2|HAS_90|CARRY_GOLD),(NONE8)
			    ,(ANIMAL|IM_POISON|NO_INFRA),(NONE8),(NONE8)
			   ,45,10,5,50,11,'$',{20,8},{14,172,0,0},13,4
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Illusionist"		    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|THRO_DR|
			      MV_ATT_NORM),(0x3L|BLINK|TELE|BLINDNESS|
			      CONFUSION|SLOW|HOLD_PERSON),(EVIL|INTELLIGENT)
			    ,(HASTE|DARKNESS),(NONE8)
			    ,50,10,20,10,11,'p',{12,8},{11,0,0,0},13,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Druid"		    ,(MV_ATT_NORM|CARRY_OBJ|CARRY_GOLD|HAS_1D2|
			     THRO_DR),(0x3L|BLINK|HOLD_PERSON|BLINDNESS|
			     SLOW|FIRE_BOLT),(EVIL|INTELLIGENT)
			    ,(HASTE|LIGHT_BOLT),(NONE8)
			    ,50,10,20,10,11,'p',{12,12},{13,13,0,0},13,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Black orc"		    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL|ORC|GROUP|HURT_LIGHT)
			    ,(NONE8),(NONE8)
			    ,45,20,20,36,11,'o',{12,10},{17,17,0,0},13,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Ochre jelly"		    ,(THRO_DR|MV_ATT_NORM|PICK_UP),(NONE8)
			    ,(IM_ACID|IM_POISON|ANIMAL|CHARM_SLEEP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,40,1,12,18,12,'j',{13,8},{115,138,138,0},13,3
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Giant flea"		    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,4,10,8,25,12,'F',{2,2},{25,0,0,0},14,1
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Ufthak of Cirith Ungol"   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(ORC|EVIL|IM_POISON|IM_FROST|MAX_HP|
			     UNIQUE|GOOD),(NONE8),(NONE8)
			    ,200,20,20,50,11,'o',{40,8},{17,17,17,17},14,3
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Giant white dragon fly"   ,(MV_ATT_NORM|MV_40),(BREATH_FR|0xAL)
			    ,(ANIMAL|IM_FROST|NO_INFRA),(NONE8),(NONE8)
			    ,60,50,20,20,11,'F',{5,8},{122,0,0,0},14,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blue icky thing"	    ,(MV_ATT_NORM|MV_40|THRO_DR|MULTIPLY)
			    ,(0x8L|FEAR|BLINDNESS|CONFUSION)
			    ,(ANIMAL|IM_POISON|EVIL),(NONE8),(NONE8)
			    ,20,20,15,20,10,'i',{10,6},{174,210,3,3},14,4
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Hill giant"		    ,(MV_ATT_NORM|HAS_60|CARRY_GOLD|CARRY_OBJ|
			     THRO_DR|MV_ATT_NORM),(NONE8),(EVIL|GIANT)
			    ,(NONE8),(NONE8)
			    ,60,50,20,45,11,'P',{16,10},{19,19,0,0},14,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Flesh golem"		    ,(MV_ATT_NORM),(NONE8),(IM_LIGHTNING|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,50,10,12,30,11,'g',{12,8},{5,5,0,0},14,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Warg"			    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|EVIL|GROUP)
			    ,(NONE8),(NONE8)
			    ,40,40,20,20,12,'C',{8,8},{31,0,0,0},14,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Giant black louse"	    ,(MULTIPLY|MV_ATT_NORM|MV_40),(NONE8),(NONE8)
			    ,(NONE8),(NONE8)
			    ,3,10,6,7,12,'l',{1,2},{25,0,0,0},14,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Lurker"                   ,(MV_ONLY_ATT|MV_INVIS),(NONE8)
                            ,(NO_INFRA|CHARM_SLEEP|MAX_HP),(NONE8),(NONE8)
			    ,80,10,30,25,11,'.',{20,10},{7,7,0,0},14,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Wererat"		    ,(MV_ATT_NORM|CARRY_GOLD|HAS_60|THRO_DR)
			    ,(0x9L|CAUSE_SERIOUS|BLINK|FROST_BOLT)
			    ,(EVIL|ANIMAL),(ST_CLOUD),(NONE8)
			    ,45,10,10,10,11,'r',{20,8},{54,54,36,0},15,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Black ogre"		    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|THRO_DR
			      |MV_ATT_NORM|MV_20)
			    ,(NONE8),(EVIL|GROUP|GIANT),(NONE8),(NONE8)
			    ,75,30,20,33,11,'O',{20,9},{16,16,0,0},15,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Magic mushroom patch"	    ,(MV_ONLY_ATT),(0x1L|BLINK|FEAR|SLOW)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(DARKNESS),(NONE8)
			    ,10,0,40,10,13,',',{1,1},{0,0,0,0},15,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Guardian naga"	    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|HAS_60|
			     MV_ATT_NORM|MV_20),(NONE8),(EVIL),(NONE8)
			     ,(NONE8)
			    ,80,120,20,65,11,'n',{24,11},{77,31,31,0},15,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Light hound"		   ,(MV_ATT_NORM),(0x5L),(ANIMAL|GROUP),(NONE8)
			    ,(BREATH_LT)
			    ,50,0,30,30,11,'Z',{6,6},{29,0,0,0},15,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Dark hound"		  ,(MV_ATT_NORM),(0x5L),(ANIMAL|GROUP),(NONE8)
			    ,(BREATH_DA)
			    ,50,0,30,30,11,'Z',{6,6},{29,0,0,0},15,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Half-orc"		    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL|ORC|GROUP),(NONE8),(NONE8)
			    ,50,20,20,40,11,'o',{16,10},{17,17,0,0},15,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Giant tarantula"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,70,80,8,32,12,'S',{10,15},{156,156,156,0},15,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Giant clear centipede"    ,(MV_INVIS|MV_ATT_NORM),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,30,30,12,30,11,'c',{5,8},{34,62,0,0},15,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Mirkwood spider"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL|GROUP|IM_POISON|
			    EVIL),(NONE8),(NONE8)
			    ,25,80,15,25,12,'S',{9,8},{31,156,156,0},15,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Frost giant"		    ,(MV_ATT_NORM|HAS_60|CARRY_GOLD|CARRY_OBJ|
			     THRO_DR|MV_ATT_NORM),(NONE8)
			    ,(EVIL|IM_FROST|NO_INFRA|GIANT),(NONE8),(NONE8)
			    ,75,50,20,50,11,'P',{17,10},{120,16,0,0},15,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Griffon"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,70,10,12,15,11,'H',{30,8},{17,36,0,0},15,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Homonculous"		    ,(THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(DEMON|EVIL|IM_FIRE),(NONE8),(NONE8)
			    ,40,30,20,32,11,'I',{8,8},{145,9,0,0},15,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Gnome mage"		    ,(CARRY_OBJ|CARRY_GOLD|HAS_60|MV_ATT_NORM|THRO_DR)
			    ,(0x4L|BLINK|FROST_BOLT|MONSTER),(EVIL|GROUP)
			    ,(DARKNESS),(NONE8)
			    ,40,20,20,20,11,'h',{7,8},{4,0,0,0},15,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Ethereal hound"	    ,(MV_ATT_NORM|MV_INVIS),(NONE8),(ANIMAL|GROUP)
			    ,(NONE8),(NONE8)
			    ,50,0,30,30,11,'Z',{10,6},{29,29,29,0},15,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Clay golem"		    ,(MV_ATT_NORM),(NONE8),
			     (HURT_ROCK|IM_FIRE|IM_LIGHTNING|IM_FROST|
			      IM_POISON|NO_INFRA|CHARM_SLEEP),(NONE8),(NONE8)
			    ,50,10,12,30,11,'g',{14,8},{7,7,0,0},15,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Umber hulk"		    ,(MV_ATT_NORM),(NONE8)
			    ,(EVIL|ANIMAL|BREAK_WALL|HURT_ROCK|IM_POISON|
			     NO_INFRA),(NONE8),(NONE8)
			    ,75,10,20,50,11,'U',{20,10},{92,5,5,36},16,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Orc captain"		    ,(HAS_90|CARRY_GOLD|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL|ORC),(NONE8),(NONE8)
			    ,40,20,20,59,11,'o',{20,10},{17,17,17,0},16,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Gelatinous cube"	    ,(THRO_DR|MV_ATT_NORM|PICK_UP|HAS_4D2|
			      CARRY_GOLD|CARRY_OBJ|HAS_60|HAS_90),(NONE8)
			    ,(IM_ACID|IM_FIRE|IM_LIGHTNING|IM_POISON|IM_FROST|
			      ANIMAL|CHARM_SLEEP|MAX_HP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,80,1,12,18,11,'j',{45,8},{115,115,115,0},16,4
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Giant green dragon fly"   ,(MV_ATT_NORM|MV_75),(BREATH_G|0xAL),
			     (IM_POISON|ANIMAL),(NONE8),(NONE8)
			    ,70,50,12,20,11,'F',{3,8},{156,0,0,0},16,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Fire giant"		    ,(MV_ATT_NORM|HAS_60|CARRY_GOLD|CARRY_OBJ|
			     THRO_DR|MV_ATT_NORM)
			    ,(NONE8),(EVIL|IM_FIRE|GIANT),(NONE8),(NONE8)
			    ,54,50,20,60,11,'P',{20,8},{102,102,0,0},16,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Hummerhorn"		    ,(MULTIPLY|MV_ATT_NORM|MV_75),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,4,10,8,14,12,'F',{2,2},{234,0,0,0},16,5
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Ulfast, Son of Ulfang"     ,(HAS_90|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL|UNIQUE|GOOD|MAX_HP)
			     ,(NONE8),(NONE8)
			     ,200,40,20,40,11,'p',{20,17},{18,18,18,18},16,3
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Quasit"		    ,(MV_ATT_NORM|MV_20|MV_INVIS|
			      CARRY_OBJ|HAS_1D2),(0xAL|BLINK|TELE_TO|TELE|FEAR
			     |CONFUSION|BLINDNESS)
			    ,(INTELLIGENT|DEMON|IM_FIRE|EVIL)
			    ,(TELE_LEV),(NONE8)
			    ,50,20,20,30,11,'I',{6,8},{176,51,51,0},16,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Imp"			     ,(MV_ATT_NORM|MV_20|MV_INVIS|
			      CARRY_OBJ|HAS_1D2),(0xAL|BLINK|TELE_TO|TELE|FEAR
			     |CONFUSION|BLINDNESS),
			     (DEMON|IM_FIRE|EVIL|NO_INFRA|INTELLIGENT)
			    ,(TELE_LEV),(NONE8)
			    ,55,20,20,30,11,'I',{6,8},{152,152,0,0},17,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Forest troll"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|CARRY_GOLD|
			     CARRY_OBJ),(NONE8),(TROLL|EVIL|HURT_LIGHT|GROUP)
			    ,(NONE8),(NONE8)
			    ,70,40,20,50,11,'T',{20,10},{3,3,29,0},17,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Nar, the Dwarf"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|CARRY_OBJ|
			     HAS_1D2),(0x6L|CAUSE_SERIOUS|BLINDNESS|CONFUSION)
			    ,(CHARM_SLEEP|IM_POISON|IM_FIRE|IM_FROST|GOOD|
			     MAX_HP|UNIQUE),(MIND_BLAST|HEAL),(NONE8)
			    ,250,25,25,70,11,'h',{45,10},{18,18,18,18},17,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"2-headed hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_1D2)
			    ,(0xBL|FEAR),(ANIMAL),(NONE8),(NONE8)
			    ,80,20,20,60,11,'R',{100,3},{36,36,0,0},17,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Water spirit"		    ,(MV_ATT_NORM|MV_20),(NONE8),
			     (EVIL|IM_POISON|NO_INFRA|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,58,40,12,28,12,'E',{9,8},{13,13,0,0},17,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Giant brown scorpion"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,62,20,12,44,11,'S',{11,8},{34,86,0,0},17,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Earth spirit"		    ,(MV_ATT_NORM|MV_20|THRO_WALL|THRO_DR|PICK_UP)
			    ,(NONE8),(EVIL|HURT_ROCK|IM_POISON|NO_INFRA|
			     IM_FIRE|IM_FROST|IM_LIGHTNING|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,64,50,10,40,12,'E',{13,8},{7,7,0,0},17,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Fire spirit"		    ,(MV_ATT_NORM|MV_20),
			     (NONE8),(EVIL|IM_POISON|IM_FIRE|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,75,20,16,30,12,'E',{10,9},{101,101,0,0},18,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Fire hound"		  ,(MV_ATT_NORM),(0xAL|BREATH_FI)
			    ,(ANIMAL|GROUP|IM_FIRE),(NONE8)
			    ,(NONE8)
			    ,70,0,30,30,11,'Z',{10,6},{105,105,105,0},18,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Cold hound"		  ,(MV_ATT_NORM),(0xAL|BREATH_FR)
			    ,(ANIMAL|GROUP|IM_FROST|NO_INFRA),(NONE8)
			    ,(NONE8)
			    ,70,0,30,30,11,'Z',{10,6},{122,54,29,0},18,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Energy hound"		   ,(MV_ATT_NORM),(0xAL|BREATH_L)
			    ,(ANIMAL|GROUP|IM_LIGHTNING),(NONE8)
			    ,(NONE8)
			    ,70,0,30,30,11,'Z',{10,6},{131,131,131,0},18,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Mimic"                    ,(MV_ONLY_ATT),(0x6L|FROST_BOLT|BLINDNESS|FEAR|
			     CONFUSION|CAUSE_SERIOUS),(NO_INFRA|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,60,0,25,30,11,'!',{10,10},{152,12,12,0},18,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Blink dog"		    ,(MV_ATT_NORM|MV_20),(0x4L|BLINK|TELE_TO)
			    ,(ANIMAL|GROUP),(NONE8),(NONE8)
			    ,50,10,20,20,12,'C',{8,8},{31,0,0,0},18,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Uruk"		    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|CARRY_OBJ|HAS_60)
			    ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|GROUP)
			    ,(NONE8),(NONE8)
			    ,68,20,20,50,11,'o',{10,8},{18,18,0,0},18,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Shagrat, the Orc Captain", (MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			     ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			     ,(NONE8),(NONE8)
			     ,400,20,20,60,11,'o',{40,10},{20,20,18,18},18,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Gorbag, the Orc Captain", (MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			     ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			     ,(NONE8),(NONE8)
			     ,400,20,20,60,11,'o',{40,10},{20,20,18,18},18,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Shambling mound"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_90)
			    ,(NONE8),(CHARM_SLEEP|ANIMAL|EVIL),(NONE8),(NONE8)
			    ,75,40,20,16,11,',',{20,6},{203,7,7,0},18,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Stone giant"		    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|
			     CARRY_OBJ|HAS_60),(NONE8),(EVIL|GIANT)
			    ,(NONE8),(NONE8)
			    ,90,50,20,75,11,'P',{24,8},{20,20,0,0},18,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Giant black dragon fly"   ,(MV_ATT_NORM|MV_75),(BREATH_A|0x9L),
			     (IM_ACID|ANIMAL),(NONE8),(NONE8)
			    ,68,50,12,20,12,'F',{3,8},{0,0,0,0},18,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Stone golem"		    ,(MV_ATT_NORM),(NONE8),(HURT_ROCK|IM_FIRE|
			     IM_FROST|IM_LIGHTNING|IM_POISON|CHARM_SLEEP|
			     NO_INFRA),(NONE8),(NONE8)
			    ,100,10,12,75,10,'g',{28,8},{9,9,0,0},19,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Red mold"		    ,(MV_ONLY_ATT),(NONE8),(IM_FIRE|IM_POISON|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,64,70,2,16,11,'m',{17,8},{108,0,0,0},19,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Giant gold dragon fly"    ,(MV_ATT_NORM|MV_75),(0x9L),
			     (IM_FIRE|ANIMAL),(BREATH_SD),(NONE8)
			    ,78,50,12,20,12,'F',{3,8},{26,0,0,0},18,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Bolg, Son of Azog"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
			    ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,800,20,20,50,12,'o',{50,10},{19,19,19,19},20,4
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Phase spider"		    ,(MV_ATT_NORM),(0x5L|BLINK|TELE_TO)
			    ,(ANIMAL|GROUP|IM_POISON),(NONE8),(NONE8)
			    ,60,80,15,25,12,'S',{6,8},{31,156,156,0},20,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"3-headed hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_2D2|HAS_1D2)
			    ,(0x9L|FEAR),(ANIMAL),(NONE8),(NONE8)
			    ,350,20,20,65,12,'R',{100,5},{36,36,36,0},20,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Earth hound"		    ,(MV_ATT_NORM),(0xAL)
			    ,(ANIMAL|GROUP),(BREATH_SH)
			    ,(NONE8)
			    ,200,0,30,30,11,'Z',{15,8},{31,31,58,58},20,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Air hound"		    ,(MV_ATT_NORM),(0xAL|BREATH_G)
			    ,(ANIMAL|GROUP|IM_POISON),(NONE8)
			    ,(NONE8)
			    ,200,0,30,30,11,'Z',{15,8},{31,31,58,58},20,1
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Sabre-tooth tiger"        ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
                            ,120,0,40,50,12,'f',{20,14},{56,56,32,32},20,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Water hound"		    ,(MV_ATT_NORM),(0xAL|BREATH_A)
			    ,(ANIMAL|GROUP|IM_ACID),(NONE8)
			    ,(NONE8)
			    ,200,0,30,30,11,'Z',{15,8},{113,113,58,58},20,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Chimera"		    ,(MV_ATT_NORM),(0xAL|BREATH_FI),(IM_FIRE)
			    ,(NONE8),(NONE8)
			    ,200,10,12,15,11,'H',{13,8},{32,105,105,0},20,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Quylthulg"		    ,(MV_INVIS),(0x4L|BLINK|MONSTER),(CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,250,0,10,1,11,'Q',{6,8},{0,0,0,0},20,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Sasquatch"		    ,(MV_ATT_NORM|THRO_DR),(NONE8),(ANIMAL|IM_FROST|
			    NO_INFRA),(NONE8),(NONE8)
			    ,180,10,15,40,12,'Y',{20,19},{56,56,37,0},20,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Werewolf"		    ,(MV_ATT_NORM|MV_20|PICK_UP|THRO_DR)
			    ,(NONE8),(ANIMAL|EVIL),(NONE8),(NONE8)
			    ,150,70,15,30,11,'C',{20,22},{29,29,32,0},20,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Dark elven lord"	    ,(CARRY_OBJ|HAS_2D2|THRO_DR|MV_ATT_NORM)
			    ,(0x5L|BLINDNESS|FROST_BOLT|FIRE_BOLT|CONFUSION)
			    ,(EVIL|HURT_LIGHT),(HASTE|DARKNESS),(NONE8)
			    ,500,30,20,40,12,'h',{18,15},{20,18,0,0},20,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Cloud giant"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|
			      CARRY_GOLD|HAS_90)
			    ,(NONE8),(EVIL|GIANT|IM_LIGHTNING),(NONE8),(NONE8)
			    ,125,50,20,60,11,'P',{24,10},{130,130,0,0},20,1
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Ugluk, the Uruk"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			     ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			     ,(NONE8),(NONE8)
			    ,550,20,20,90,11,'o',{40,16},{18,18,18,18},20,4
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Lugdush, the Uruk"    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(NONE8),(ORC|EVIL|IM_POISON|IM_FROST|IM_FIRE|
			     MAX_HP|GOOD|UNIQUE|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,550,20,20,95,11,'o',{40,18},{20,20,18,18},21,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Blue dragon bat"	    ,(MV_ATT_NORM|MV_40),(BREATH_L|0x4L)
			    ,(ANIMAL|IM_LIGHTNING),(NONE8),(NONE8)
			    ,54,50,12,26,13,'b',{4,4},{131,0,0,0},21,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Mimic"                    ,(MV_ONLY_ATT),(0x5L|FIRE_BOLT|CAUSE_SERIOUS|FEAR|
			     BLINDNESS|CONFUSION|MONSTER),(NO_INFRA|
			     CHARM_SLEEP),(NONE8),(NONE8)
			    ,60,0,30,40,11,'?',{10,14},{152,132,12,12},21,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Fire vortex"		    ,(MV_ATT_NORM|MV_40),(BREATH_FI|0x6L)
			    ,(IM_FIRE|CHARM_SLEEP),(NONE8),(NONE8)
			    ,100,0,100,30,11,'v',{9,9},{239,0,0,0},21,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Water vortex"		    ,(MV_ATT_NORM|MV_40),(BREATH_A|0x6L)
			    ,(IM_ACID|CHARM_SLEEP),(NONE8),(NONE8)
			    ,100,0,100,30,11,'v',{9,9},{240,0,0,0},21,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Cold vortex"		    ,(MV_ATT_NORM|MV_40),(BREATH_FR|0x6L)
			    ,(IM_FROST|CHARM_SLEEP|NO_INFRA),(NONE8),(NONE8)
			    ,100,0,100,30,11,'v',{9,9},{241,0,0,0},21,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Energy vortex"	    ,(MV_ATT_NORM|MV_40),(BREATH_L|0x6L)
			    ,(IM_LIGHTNING|CHARM_SLEEP),(NONE8),(NONE8)
			    ,130,0,100,30,11,'v',{12,12},{242,0,0,0},21,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Mummified orc"	    ,(MV_ATT_NORM|CARRY_GOLD|CARRY_OBJ|HAS_90|THRO_DR)
			    ,(NONE8),(EVIL|ORC|UNDEAD|IM_FROST|NO_INFRA|
			     IM_POISON|CHARM_SLEEP),(NONE8),(NONE8)
			    ,56,75,20,28,11,'M',{15,8},{13,13,0,0},21,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Killer stag beetle"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,80,30,12,55,11,'K',{20,8},{41,10,0,0},22,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Iron golem"		    ,(MV_ATT_NORM),(SLOW|0x7L)
			    ,(IM_FIRE|IM_FROST|IM_LIGHTNING|IM_POISON|
			      NO_INFRA|CHARM_SLEEP),(NONE8),(NONE8)
			    ,160,10,12,80,11,'g',{80,12},{10,0,0,0},22,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Giant yellow scorpion"    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8)
			    ,(NONE8)
			    ,60,20,12,38,11,'S',{12,8},{31,167,0,0},22,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Black ooze"		    ,(CARRY_GOLD|CARRY_OBJ|HAS_60|PICK_UP|MULTIPLY|
			      THRO_DR|THRO_CREAT|MV_ATT_NORM|MV_40)
			    ,(0xBL|MANA_DRAIN),(IM_POISON|ANIMAL),(NONE8)
			    ,(NONE8)
			    ,7,1,10,6,9,'j',{6,8},{138,0,0,0},23,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Hardened warrior"	    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,60,40,20,40,11,'p',{15,11},{18,18,0,0},23,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Azog, King of the Uruk-Hai",(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
			   ,(NONE8),(ORC|EVIL|IM_POISON|MAX_HP|UNIQUE|GOOD)
			   ,(NONE8),(NONE8)
			   ,1111,20,20,80,12,'o',{60,15},{23,23,23,0},23,5
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Master rogue"		    ,(HAS_2D2|CARRY_GOLD|CARRY_OBJ
			     |THRO_DR|PICK_UP|MV_ATT_NORM),(NONE8)
			    ,(EVIL),(NONE8),(NONE8)
			   ,110,40,20,30,12,'p',{15,9},{16,16,231,0},23,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Red dragon bat"	    ,(MV_ATT_NORM|MV_40),(BREATH_FI|0x4L)
			    ,(IM_FIRE|ANIMAL),(NONE8),(NONE8)
			    ,60,50,12,28,13,'b',{3,8},{105,0,0,0},23,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Killer blue beetle"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL),(NONE8)
			  ,(NONE8),85,30,14,55,11,'K',{20,8},{44,0,0,0},23,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Giant bronze dragon fly"  ,(MV_ATT_NORM|MV_75),(0x9L),
			     (CHARM_SLEEP|ANIMAL),(BREATH_CO),(NONE8)
			    ,70,50,12,20,12,'F',{3,8},{0,0,0,0},18,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Forest wight"		    ,(MV_ATT_NORM|MV_20|THRO_DR|CARRY_GOLD|
			      CARRY_OBJ|HAS_60|HAS_90)
			    ,(0xAL|FEAR|MANA_DRAIN)
			    ,(EVIL|NO_INFRA|UNDEAD|IM_FROST|IM_POISON|
			      HURT_LIGHT|CHARM_SLEEP),(NONE8),(NONE8)
			    ,140,30,20,30,11,'W',{12,8},{5,5,187,0},24,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Ibun, Son of Mim"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(0x8L|SLOW|FIRE_BOLT),(UNIQUE|IM_FROST|IM_FIRE|
			     EVIL|GOOD|CHARM_SLEEP|MAX_HP),(HEAL),(NONE8)
			    ,300,10,20,80,11,'h',{55,15},{19,19,19,204},24,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Khim, Son of Mim"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(0x8L|SLOW|FIRE_BOLT),(UNIQUE|IM_FROST|IM_FIRE|
			     EVIL|GOOD|CHARM_SLEEP|MAX_HP),(HEAL),(NONE8)
			    ,300,10,20,80,11,'h',{55,15},{19,19,19,204},24,2
#ifdef TC_COLOR
  , RED
#endif
},

{"4-headed hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_4D2)
			    ,(0x7L|FEAR),(ANIMAL),(NONE8),(NONE8)
			    ,350,20,20,70,12,'R',{100,6},{36,36,36,36},24,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Mummified human"	    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_90)
			    ,(NONE8),(EVIL|NO_INFRA|UNDEAD|IM_FROST|
			     IM_POISON|CHARM_SLEEP),(NONE8),(NONE8)
			    ,70,60,20,34,11,'M',{17,9},{13,13,0,0},24,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Vampire bat"		    ,(MV_ATT_NORM|MV_40),(NONE8)
			    ,(EVIL|UNDEAD|IM_FROST|IM_POISON|
			     CHARM_SLEEP|NO_INFRA),(NONE8),(NONE8)
			    ,150,50,12,40,12,'b',{9,10},{236,236,0,0},24,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Sangahyando of Umbar"	    ,(MV_ATT_NORM|CARRY_OBJ|HAS_1D2|HAS_90|THRO_DR)
			    ,(0x4L|SLOW),(CHARM_SLEEP|IM_FIRE|UNIQUE|EVIL|
			     IM_LIGHTNING|GOOD|MAX_HP),(FORGET),(NONE8)
			,400,25,25,80,11,'p',{80,10},{22,22,22,22},24,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Angamaite of Umbar"	    ,(MV_ATT_NORM|CARRY_OBJ|HAS_1D2|HAS_90|THRO_DR)
			    ,(0x4L|SLOW),(CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|
			     GOOD|EVIL|MAX_HP|UNIQUE),(FORGET),(NONE8)
			,400,25,25,80,11,'p',{80,10},{22,22,22,22},24,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Banshee"		    ,(MV_ATT_NORM|MV_20|MV_40|CARRY_GOLD|CARRY_OBJ|
			     HAS_1D2|THRO_WALL|MV_INVIS|PICK_UP)
			    ,(0xFL|TELE|MANA_DRAIN)
			    ,(NO_INFRA|UNDEAD|EVIL|IM_FROST|
			     CHARM_SLEEP|IM_POISON),(NONE8),(NONE8)
			    ,60,10,20,24,12,'G',{6,8},{99,188,0,0},24,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Pukelman"		    ,(MV_ATT_NORM),(ACID_BOLT|SLOW|CONFUSION|0x4L)
			    ,(IM_FIRE|IM_FROST|IM_LIGHTNING|IM_POISON|
			      NO_INFRA|CHARM_SLEEP|HURT_ROCK),(NONE8),(NONE8)
			    ,600,10,12,80,11,'g',{80,12},{10,19,0,0},25,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Dark elven druid"	    ,(MV_ATT_NORM|CARRY_OBJ|HAS_1D2|THRO_DR)
			    ,(0x6L|MONSTER|CONFUSION),(IM_POISON|
			     CHARM_SLEEP|EVIL|HURT_LIGHT),
				(HEAL|S_SPIDER|DARKNESS),(NONE8)
			    ,500,10,15,75,12,'h',{20,20},{6,6,20,0},25,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Stone troll"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|CARRY_GOLD|
			     CARRY_OBJ),(NONE8)
			    ,(TROLL|EVIL|HURT_LIGHT|HURT_ROCK|GROUP)
			    ,(NONE8),(NONE8)
			    ,85,50,20,40,11,'T',{23,10},{5,5,41,0},25,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Troll priest"		    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|CARRY_OBJ|
			     HAS_90),(0x5L|CAUSE_LIGHT|BLINK|FEAR|MAG_MISS)
			    ,(TROLL|EVIL|HURT_LIGHT|CHARM_SLEEP|MAX_HP)
			    ,(DARKNESS),(NONE8)
			    ,100,30,20,50,11,'T',{23,13},{7,7,41,0},25,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Wereworm"		    ,(MV_ATT_NORM),(NONE8),(IM_ACID|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,300,20,15,70,11,'w',{100,11},{32,139,224,156},
								    25,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Carrion crawler"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,60,10,15,40,11,'c',{20,12},{253,253,0,0},25,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Killer red beetle"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,85,30,14,50,11,'K',{20,8},{84,0,0,0},25,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Giant grey ant lion"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT),(NONE8),(ANIMAL)
			    ,(NONE8),(NONE8)
			    ,90,40,10,40,11,'a',{19,8},{39,0,0,0},26,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Ulwarth, Son of Ulfang"      ,(HAS_90|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,500,40,20,40,11,'p',{80,11},{22,22,22,0},26,4
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Displacer beast"          ,(MV_ATT_NORM|MV_INVIS),(NONE8),(ANIMAL)
                            ,(NONE8),(NONE8)
			    ,100,20,35,100,11,'f',{25,10},{37,9,9,9},26,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Giant fire tick"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|IM_FIRE)
			    ,(NONE8),(NONE8)
			    ,90,20,14,54,11,'t',{16,8},{109,0,0,0},26,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Cave ogre"		    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|THRO_DR
			      |MV_ATT_NORM)
			    ,(NONE8),(EVIL|GROUP|GIANT),(NONE8),(NONE8)
			    ,42,30,20,33,11,'O',{30,9},{20,20,0,0},26,1
#ifdef TC_COLOR
  , BROWN
#endif
},

{"White wraith"		    ,(CARRY_GOLD|CARRY_OBJ|HAS_1D2|THRO_DR|
			     MV_ATT_NORM),(0x8L|FEAR|CAUSE_SERIOUS)
			    ,(UNDEAD|EVIL|NO_INFRA|IM_FROST|IM_POISON|
			      CHARM_SLEEP|HURT_LIGHT),(DARKNESS),(NONE8)
			    ,175,10,20,40,11,'W',{15,8},{5,5,189,0},26,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Monadic deva"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_2D2)
			    ,(0x3L|FEAR|BLINDNESS|CONFUSION)
			    ,(IM_POISON|IM_ACID|CHARM_SLEEP|MAX_HP)
			    ,(FORGET),(NONE8)
			    ,220,255,30,60,11,'A',{25,12}
			    ,{17,17,17,17},26,6
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Mim, Betrayer of Turin"   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
			    ,(0x6L|FEAR|ACID_BOLT),(EVIL|MAX_HP|IM_FROST|
			     IM_FIRE|IM_POISON|IM_ACID|IM_LIGHTNING|UNIQUE|
			     GOOD|CHARM_SLEEP),(ACID_BALL|HEAL),(NONE8)
			    ,1000,20,20,80,12,'h',{100,11},{20,20,20,204},
								   27,4
#ifdef TC_COLOR
  , RED
#endif
},

{"Killer fire beetle"	    ,(MV_ATT_NORM),(NONE8),(IM_FIRE|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,95,30,14,45,11,'K',{13,8},{41,110,0,0},27,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Creeping adamantite coins",(MV_ATT_NORM|HAS_2D2|HAS_90|CARRY_GOLD),(NONE8)
			    ,(ANIMAL|IM_POISON|NO_INFRA),(NONE8),(NONE8)
			   ,45,10,5,50,12,'$',{20,25},{161,172,10,10},27,4
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Algroth"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|CARRY_GOLD|
			     CARRY_OBJ),(NONE8),(TROLL|EVIL|GROUP)
			    ,(NONE8),(NONE8)
			    ,150,40,20,60,11,'T',{21,12},{238,238,29,0},27,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Vibration hound"	      ,(MV_ATT_NORM),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(BREATH_SD)
			    ,(NONE8)
			    ,250,0,30,30,11,'Z',{25,10},{36,36,58,58},27,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Nexus hound"		  ,(MV_ATT_NORM),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(BREATH_NE)
			    ,(NONE8)
			    ,250,0,30,30,11,'Z',{25,10},{37,37,58,58},27,3
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Ogre mage"		,(CARRY_GOLD|CARRY_OBJ|HAS_1D2|THRO_DR|MV_ATT_NORM)
			    ,(0x4L|HOLD_PERSON|FROST_BALL|MONSTER)
			    ,(EVIL|GIANT),(HEAL|TRAP_CREATE),(NONE8)
			  ,300,30,20,40,11,'O',{30,12},{20,20,20,20},27,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Lokkak, the Ogre Chieftan",(MV_ATT_NORM|CARRY_OBJ|HAS_2D2|THRO_DR)
			    ,(NONE8),(GIANT|EVIL|IM_POISON|MAX_HP|GOOD|UNIQUE)
			    ,(NONE8),(NONE8)
			,1500,20,20,100,12,'O',{90,16},{235,235,235,0},27,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Vampire"		    ,(HAS_1D2|CARRY_GOLD|CARRY_OBJ|HAS_60|
			      THRO_DR|MV_ATT_NORM)
			    ,(0x9L|HOLD_PERSON|FEAR|TELE_TO|CAUSE_SERIOUS)
			    ,(UNDEAD|EVIL|NO_INFRA|IM_FROST|IM_POISON|
			     CHARM_SLEEP|HURT_LIGHT)
			    ,(MIND_BLAST|FORGET|DARKNESS),(NONE8)
			    ,175,10,20,45,11,'V',{25,12},{5,5,190,0},27,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Gorgimera"		    ,(MV_ATT_NORM),(0x8L|BREATH_FI),(IM_FIRE)
			    ,(NONE8),(NONE8)
			  ,200,10,12,55,11,'H',{25,20},{105,105,32,223},27,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Colbran"		    ,(MV_ATT_NORM),(0x3L)
			    ,(IM_LIGHTNING|IM_POISON|NO_INFRA|CHARM_SLEEP)
			    ,(LIGHT_BOLT),(NONE8)
			    ,900,10,12,80,12,'g',{80,12},{130,130,0,0},27,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Spirit naga"		    ,(HAS_2D2|CARRY_OBJ|HAS_90|
			     MV_ATT_NORM|MV_INVIS)
			    ,(0x4L|BLINDNESS)
			    ,(EVIL|CHARM_SLEEP)
			    ,(MIND_BLAST|DARKNESS|HEAL),(NONE8)
			    ,60,120,20,75,11,'n',{30,15},{77,77,31,31},28,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"5-headed hydra"	    ,(MV_ATT_NORM|CARRY_GOLD|HAS_4D2|HAS_1D2)
			    ,(0x5L|FEAR),(ANIMAL|IM_POISON)
			    ,(ST_CLOUD),(NONE8)
		       ,350,20,20,80,12,'R',{100,8},{163,163,163,163},28,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Black knight"		   ,(HAS_1D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM|CARRY_GOLD)
			    ,(0x8L|CAUSE_CRIT|BLINDNESS|FEAR),(EVIL)
			    ,(DARKNESS),(NONE8)
			    ,240,10,20,70,12,'p',{30,10},{23,23,23,0},28,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Uldor the Accursed"	  ,(HAS_1D2|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			    ,600,40,20,70,11,'p',{50,20},{22,22,22,18},28,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Mage"			    ,(HAS_1D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x3L|TELE|TELE_TO|BLINDNESS|FROST_BOLT|FIRE_BOLT
			     |CONFUSION|MONSTER),(EVIL|INTELLIGENT)
			    ,(LIGHT_BOLT|HASTE),(NONE8)
			    ,150,10,20,40,11,'p',{15,8},{14,14,0,0},28,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Mind flayer"		    ,(HAS_1D2|CARRY_OBJ|HAS_60|THRO_DR|MV_ATT_NORM)
			    ,(0x8L|HOLD_PERSON|FEAR|BLINDNESS)
			    ,(EVIL|CHARM_SLEEP|MAX_HP)
			    ,(MIND_BLAST|BRAIN_SMASH|FORGET),(NONE8)
			    ,200,10,20,60,11,'h',{18,8},{225,225,0,0},28,1
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Draebor, the Imp"	    ,(MV_ATT_NORM|MV_20|MV_INVIS|
			     CARRY_OBJ|HAS_4D2),(0x5L|BLINK|TELE_TO|TELE|FEAR
			    |CONFUSION|BLINDNESS)
			    ,(DEMON|IM_FIRE|MAX_HP|EVIL|UNIQUE
			      |GOOD|INTELLIGENT)
			    ,(TELE_LEV|TELE_AWAY),(NONE8)
			    ,750,20,20,50,12,'I',{40,13},{152,152,17,0},28,5
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Basilisk"		    ,(MV_ATT_NORM|CARRY_OBJ|HAS_1D2|THRO_DR)
			    ,(NONE8),(ANIMAL|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,300,30,15,90,12,'R',{20,30},{146,39,39,39},28,3
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Ice troll"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|CARRY_GOLD|
			     HAS_60),(NONE8),
			    (EVIL|TROLL|IM_FROST|HURT_LIGHT|GROUP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,160,50,20,56,11,'T',{24,10},{4,4,123,4},28,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Giant purple worm"	    ,(MV_ATT_NORM),(NONE8),(IM_ACID|ANIMAL)
			    ,(NONE8),(NONE8)
			    ,400,30,14,65,11,'w',{65,8},{7,113,166,0},29,3
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Movanic deva"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_2D2)
			    ,(0x3L|FEAR|BLINDNESS|CONFUSION)
			    ,(IM_POISON|IM_FIRE|IM_FROST|CHARM_SLEEP|MAX_HP|
			      INTELLIGENT)
			    ,(HEAL|HASTE),(NONE8)
			    ,400,255,30,68,11,'A',{25,16}
			    ,{18,18,18,18},29,6
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Catoblepas"		    ,(MV_ATT_NORM|CARRY_GOLD|HAS_2D2)
			    ,(NONE8)
			    ,(IM_POISON|ANIMAL),(NONE8),(NONE8)
			    ,400,40,15,55,11,'q',{30,10}
			    ,{221,222,228,39},29,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Mimic"                    ,(MV_ONLY_ATT),(0x4L|FEAR|CONFUSION|BLINDNESS|
			     FIRE_BOLT|FROST_BOLT|ACID_BOLT|CAUSE_SERIOUS|
			     MONSTER),(CHARM_SLEEP|NO_INFRA),(LIGHT_BOLT|
			     FORGET),(NONE8)
			,200,100,30,60,12,']',{10,35},{152,152,152,152},29,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Young blue dragon"	    ,(MV_ATT_NORM|HAS_1D2|HAS_60|HAS_90|THRO_DR|
			      CARRY_GOLD|CARRY_OBJ),(0xBL|FEAR|BREATH_L)
			    ,(IM_LIGHTNING|EVIL|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,300,70,20,50,11,'d',{33,8},{52,52,29,0},29,1
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Young white dragon"	    ,(MV_ATT_NORM|HAS_1D2|HAS_60|HAS_90|THRO_DR|
			      CARRY_GOLD|CARRY_OBJ),(0xBL|FEAR|BREATH_FR)
			    ,(IM_FROST|EVIL|DRAGON|MAX_HP|NO_INFRA),(NONE8)
			    ,(NONE8),275,70,20,50,11,'d',{32,8},{52,52,29,0},29,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Young green dragon"	    ,(MV_ATT_NORM|HAS_1D2|HAS_60|HAS_90|THRO_DR|
			      CARRY_GOLD|CARRY_OBJ),(0xBL|FEAR|BREATH_G)
			    ,(IM_POISON|EVIL|DRAGON|MAX_HP),(NONE8),(NONE8)
			    ,290,70,20,60,11,'d',{32,8},{52,52,29,0},29,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Young bronze dragon"	    ,(MV_ATT_NORM|HAS_2D2|HAS_60|HAS_90|
			     CARRY_GOLD|CARRY_OBJ|THRO_DR),
			     (0xBL|FEAR)
			    ,(DRAGON|MAX_HP|CHARM_SLEEP)
			    ,(BREATH_CO),(NONE8)
			    ,310,150,20,63,11,'d',{34,8},{52,52,29,0},29,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Mithril golem"	    ,(MV_ATT_NORM|CARRY_GOLD|HAS_2D2),(NONE8)
			    ,(IM_FROST|IM_FIRE|IM_LIGHTNING|IM_POISON
			      |NO_INFRA|CHARM_SLEEP),(NONE8),(NONE8)
			  ,500,10,12,100,11,'g',{80,15},{20,20,23,23},30,4
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Shadow drake"		    ,(MV_ATT_NORM|MV_20|MV_INVIS|THRO_DR|HAS_2D2|
			    CARRY_OBJ|PICK_UP),(0x6L|FEAR|CONFUSION|SLOW)
			    ,(ANIMAL|EVIL|IM_FROST|DRAGON)
			    ,(HASTE|DARKNESS),(NONE8)
			  ,700,30,25,50,11,'d',{20,10},{122,122,122,0},30,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Skeleton troll"	    ,(MV_ATT_NORM|THRO_DR),(NONE8)
			    ,(UNDEAD|EVIL|TROLL|NO_INFRA|IM_FROST|CHARM_SLEEP|
			     IM_POISON),(NONE8),(NONE8)
			    ,225,20,20,55,11,'s',{20,10},{5,5,41,0},30,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Manticore"		    ,(MV_ATT_NORM),(0x5L),(EVIL|MAX_HP),(MISSILE)
			    ,(NONE8)
			    ,300,10,12,15,12,'H',{25,10},{17,17,17,17},30,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Giant static ant"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|IM_LIGHTNING)
			    ,(NONE8),(NONE8)
			    ,80,60,10,50,11,'a',{8,8},{134,0,0,0},30,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Giant army ant"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT),(NONE8)
			    ,(ANIMAL|GROUP)
			    ,(NONE8),(NONE8)
			    ,90,40,10,40,12,'a',{19,6},{39,0,0,0},30,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Grave wight"		    ,(MV_ATT_NORM|MV_20|THRO_DR|HAS_1D2|CARRY_OBJ)
			    ,(0x8L|CAUSE_CRIT|FEAR)
			    ,(UNDEAD|EVIL|NO_INFRA|IM_FROST|IM_POISON|
			      HURT_LIGHT|CHARM_SLEEP),(DARKNESS),(NONE8)
			    ,325,30,20,50,11,'W',{12,10},{6,6,191,0},30,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Killer slicer beetle"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,200,30,14,60,11,'K',{22,10},{48,48,0,0},30,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Ghost"		    ,(MV_ATT_NORM|MV_20|HAS_1D2|CARRY_OBJ|CARRY_GOLD|
			     HAS_60|THRO_WALL|PICK_UP|MV_INVIS)
			    ,(0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS)
			    ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|
			     IM_POISON),(NONE8),(NONE8)
			    ,350,10,20,30,12,'G',{13,8},{99,192,184,0},31,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Death watch beetle"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,190,30,16,60,11,'K',{25,12},{47,67,0,0},31,3
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Ogre shaman"		    ,(MV_ATT_NORM|THRO_DR|HAS_90|CARRY_OBJ)
			    ,(0x5L|TELE|HOLD_PERSON|CAUSE_SERIOUS|
			     FEAR|MONSTER|FIRE_BOLT),(EVIL|GIANT)
			    ,(TRAP_CREATE),(NONE8)
			    ,250,30,20,55,11,'O',{14,10},{19,19,19,0},32,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Nexus quylthulg"	    ,(MV_INVIS),(0x1L|BLINK),(CHARM_SLEEP)
			    ,(TELE_AWAY),(NONE8)
			    ,300,0,10,1,11,'Q',{10,12},{0,0,0,0},32,1
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Shelob, Spider of Darkness",(MV_ATT_NORM|CARRY_OBJ|HAS_2D2|HAS_1D2)
			   ,(0x2L|FEAR|BLINDNESS|CONFUSION|SLOW|CAUSE_CRIT)
			     ,(ANIMAL|EVIL|UNIQUE|HURT_LIGHT|CHARM_SLEEP|
			     MAX_HP|GOOD|INTELLIGENT),(RAZOR|HEAL|S_SPIDER|
			     TRAP_CREATE),(NONE8)
			  ,1200,80,8,80,11,'S',{120,10},{38,167,85,167},32,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Ninja"		   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|CARRY_GOLD|HAS_1D2)
			    ,(NONE8),(EVIL|CHARM_SLEEP),(NONE8),(NONE8)
			    ,300,10,20,60,12,'p',{13,12},{152,80,80,0},32,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Memory moss"              ,(MV_ONLY_ATT),(0x6L),(CHARM_SLEEP|ANIMAL)
                            ,(FORGET),(NONE8)
			    ,150,5,30,1,11,',',{1,2},{89,89,0,0},32,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Storm giant"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|
			      CARRY_GOLD|HAS_1D2)
			    ,(0x8L|FEAR|CONFUSION|BLINK|TELE_TO)
			    ,(EVIL|GIANT|IM_LIGHTNING|IM_FROST|MAX_HP)
			    ,(LIGHT_BOLT|LIGHT_BALL),(NONE8)
			  ,1500,40,20,60,11,'P',{24,16},{215,215,215,0},32,1
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Cave troll"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|
			      CARRY_OBJ|CARRY_GOLD),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|HURT_LIGHT|GROUP)
			    ,(NONE8),(NONE8)
			    ,350,50,20,50,11,'T',{24,12},{18,7,7,7},33,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Half-troll"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_90)
			    ,(NONE8),(TROLL|EVIL|IM_POISON|GROUP)
			    ,(NONE8),(NONE8)
			    ,300,50,20,50,11,'T',{25,14},{53,53,53,36},33,2
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Mystic"		    ,(MV_ATT_NORM|HAS_1D2|CARRY_OBJ|MV_INVIS)
			    ,(0x6L),(IM_POISON|IM_ACID|MAX_HP|CHARM_SLEEP)
			    ,(HEAL|S_SPIDER),(NONE8)
			 ,500,5,30,50,12,'p',{10,35},{266,266,266,266},33,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Barrow wight"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|
			      CARRY_GOLD|HAS_60),(0x8L|CAUSE_SERIOUS|
			      HOLD_PERSON|FEAR)
			    ,(EVIL|UNDEAD|NO_INFRA|IM_FROST|IM_POISON|
			      HURT_LIGHT|CHARM_SLEEP|GROUP),(DARKNESS),(NONE8)
			    ,375,10,20,40,11,'W',{15,10},{7,7,193,0},33,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Giant skeleton troll"	    ,(MV_ATT_NORM|THRO_DR),(NONE8)
			    ,(TROLL|EVIL|UNDEAD|IM_FROST|IM_POISON|
			      NO_INFRA|CHARM_SLEEP|MAX_HP),(NONE8),(NONE8)
			    ,325,20,20,50,11,'s',{45,10},{8,8,28,28},33,1
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Chaos drake"		    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|CARRY_OBJ)
			    ,(0x6L|FEAR|CONFUSION|SLOW),(EVIL|IM_FIRE|
			     CHARM_SLEEP|MAX_HP|DRAGON),(BREATH_DI)
			    ,(NONE8)
			  ,700,30,25,100,11,'d',{50,10},{54,54,36,0},33,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Law drake"		    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|CARRY_OBJ)
			    ,(0x6L|FEAR|CONFUSION|SLOW),(IM_FROST|
			     CHARM_SLEEP|MAX_HP|DRAGON),(BREATH_SH|
			     BREATH_SD),(NONE8)
			  ,700,30,25,100,11,'d',{50,11},{54,54,36,0},33,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Balance drake"	    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|CARRY_OBJ)
			    ,(0x6L|FEAR|CONFUSION|SLOW),(IM_FIRE|
			     IM_FROST|CHARM_SLEEP|MAX_HP|DRAGON)
			    ,(BREATH_DI|BREATH_SD|BREATH_SH),(NONE8)
			  ,700,30,25,100,11,'d',{50,12},{54,54,36,0},33,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Ethereal drake"	    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|CARRY_OBJ|MV_INVIS|
			     THRO_WALL)
			    ,(0x6L|FEAR|CONFUSION|SLOW)
			    ,(EVIL|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(BREATH_LT|BREATH_DA)
			  ,700,15,25,100,11,'d',{50,9},{54,54,36,0},33,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Bert the Stone Troll"	  ,(MV_ATT_NORM|PICK_UP|THRO_DR|HAS_1D2|
			      CARRY_OBJ),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|UNIQUE|HURT_ROCK|
			      HURT_LIGHT|GOOD|IM_FROST),(NONE8),(NONE8)
			   ,2000,50,20,70,11,'T',{55,20},{23,38,33,0},33,7
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Bill the Stone Troll"	  ,(MV_ATT_NORM|PICK_UP|THRO_DR|HAS_1D2|
			      CARRY_OBJ),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|UNIQUE|HURT_ROCK|
			      HURT_LIGHT|GOOD|IM_FROST),(NONE8),(NONE8)
			  ,2000,50,20,70,11,'T',{55,20},{23,38,33,0},33,7
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Tom the Stone Troll"	 ,(MV_ATT_NORM|PICK_UP|THRO_DR|HAS_1D2|
			      CARRY_OBJ),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|UNIQUE|HURT_ROCK|
			      HURT_LIGHT|GOOD|IM_FROST),(NONE8),(NONE8)
			  ,2000,50,20,70,11,'T',{55,20},{23,38,33,0},33,7
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Shade"		    ,(MV_ATT_NORM|MV_20|CARRY_OBJ|HAS_2D2|
			     HAS_90|THRO_WALL|PICK_UP|MV_INVIS)
			    ,(0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS)
			    ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|
			     IM_POISON),(FORGET),(NONE8)
			  ,350,10,20,30,12,'G',{14,20},{99,192,184,0},33,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Spectre"		    ,(MV_ATT_NORM|MV_20|CARRY_OBJ|HAS_2D2|
			     HAS_90|THRO_WALL|PICK_UP)
			    ,(0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS)
			    ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|
			     IM_POISON),(FORGET),(NONE8)
			  ,350,10,20,30,12,'G',{14,20},{99,192,237,0},33,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Water troll"		     ,(MV_ATT_NORM|THRO_DR|HAS_60|
			      CARRY_OBJ|CARRY_GOLD),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|IM_FROST|HURT_LIGHT|
			      MAX_HP|GROUP),(NONE8),(NONE8)
			    ,420,50,20,50,11,'T',{26,14},{8,8,11,11},33,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Fire elemental"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT)
			    ,(0x6L|FIRE_BOLT)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|IM_FIRE)
			    ,(NONE8),(NONE8)
			    ,350,50,12,50,11,'E',{30,8},{103,103,0,0},33,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Astral deva"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_2D2|HAS_1D2)
			    ,(0x3L|FEAR|BLINDNESS|FIRE_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|INTELLIGENT|
			      IM_ACID|IM_LIGHTNING|CHARM_SLEEP|MAX_HP)
			    ,(HEAL|HASTE|MIND_BLAST|SUMMON),(NONE8)
			    ,400,255,30,68,12,'A',{25,18}
			    ,{21,20,21,20},33,6
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Water elemental"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT)
			    ,(0x6L|FROST_BOLT)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,325,50,12,40,11,'E',{25,8},{9,9,9,0},33,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Invisible stalker"	    ,(MV_ATT_NORM|MV_40|MV_INVIS|THRO_DR)
			    ,(NONE8),(IM_LIGHTNING|EVIL|IM_POISON|CHARM_SLEEP|
			     NO_INFRA),(NONE8),(NONE8)
			    ,300,20,20,46,13,'E',{19,12},{5,5,5,0},34,3
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Carrion crawler"	    ,(MV_ATT_NORM|MV_20),(NONE8),(ANIMAL|IM_POISON|
			     GROUP),(NONE8),(NONE8)
			    ,100,10,15,40,11,'c',{20,12},{253,253,0,0},34,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Master thief"		    ,(HAS_2D2|HAS_90|CARRY_GOLD|CARRY_OBJ
			     |THRO_DR|PICK_UP|MV_ATT_NORM),(NONE8)
			    ,(EVIL),(NONE8),(NONE8)
			  ,350,40,20,30,13,'p',{18,10},{16,17,231,232},34,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Ulfang the Black"	    ,(HAS_2D2|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(NONE8),(EVIL|UNIQUE|GOOD)
			    ,(NONE8),(NONE8)
			  ,1200,40,20,90,12,'p',{80,13},{23,23,23,23},34,5
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Lich"			    ,(MV_ATT_NORM|THRO_DR|HAS_1D2|CARRY_OBJ|
			      CARRY_GOLD)
			    ,(0x4L|BLINK|TELE_TO|CAUSE_CRIT|
			      HOLD_PERSON|BLINDNESS|MANA_DRAIN|
			      SLOW|FEAR)
			    ,(EVIL|UNDEAD|IM_FROST|IM_POISON|NO_INFRA|
			      MAX_HP|HURT_LIGHT|CHARM_SLEEP|INTELLIGENT)
			    ,(BRAIN_SMASH|TELE_AWAY),(NONE8)
			 ,800,60,20,60,11,'L',{25,12},{179,179,194,214},34,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Master vampire"	   ,(MV_ATT_NORM|THRO_DR|HAS_4D2|CARRY_OBJ|CARRY_GOLD)
			    ,(0x6L|TELE_TO|CAUSE_CRIT|HOLD_PERSON|
			      FEAR|CONFUSION)
			    ,(CHARM_SLEEP|HURT_LIGHT|EVIL|UNDEAD|IM_FROST|
			     MAX_HP|IM_POISON|NO_INFRA)
			    ,(DARKNESS|MIND_BLAST|FORGET|NETHER_BOLT),(NONE8)
			    ,750,10,20,60,11,'V',{28,12},{5,5,195,0},34,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Giant red scorpion"	    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			    ,275,40,12,50,12,'S',{18,20},{29,165,0,0},34,4
#ifdef TC_COLOR
  , RED
#endif
},

{"Earth elemental"	    ,(THRO_WALL|PICK_UP|MV_ATT_NORM)
			    ,(0x8L|ACID_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING|
			      CHARM_SLEEP|HURT_ROCK|EVIL|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,375,90,10,60,10,'E',{30,10},{22,22,22,0},34,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Air elemental"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT),(0x8L)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|IM_FIRE|IM_FROST|
			      IM_LIGHTNING|IM_ACID|IM_POISON|NO_INFRA)
			    ,(LIGHT_BOLT),(NONE8)
			    ,390,50,12,50,12,'E',{30,5},{9,89,9,0},34,2
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Hell hound"		    ,(MV_ATT_NORM|MV_20),(0x5L|BREATH_FI),
			     (ANIMAL|EVIL|MAX_HP|IM_FIRE),(NONE8),(NONE8)
			  ,600,0,25,80,12,'C',{25,16},{107,107,107,0},35,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Eog golem"		    ,(MV_ATT_NORM|CARRY_GOLD|HAS_2D2),(NONE8)
			    ,(IM_FROST|IM_FIRE|IM_LIGHTNING|IM_POISON
			      |NO_INFRA|CHARM_SLEEP),(NONE8),(NONE8)
		     ,1200,10,12,125,10,'g',{100,20},{218,218,235,235},35,4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Olog"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|
			      CARRY_OBJ|CARRY_GOLD),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|GROUP)
			    ,(NONE8),(NONE8)
			    ,400,50,20,50,11,'T',{30,14},{10,10,33,33},35,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Dagashi"		   ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|CARRY_GOLD|HAS_1D2)
			    ,(NONE8),(EVIL|CHARM_SLEEP),(NONE8),(NONE8)
			  ,500,10,20,70,12,'p',{13,25},{152,80,80,152},35,4
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Gravity hound"	    ,(MV_ATT_NORM),(0x5L)
			    ,(ANIMAL|GROUP),(NONE8)
			    ,(BREATH_GR)
			    ,500,0,30,30,11,'Z',{35,10},{39,39,39,58},35,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Acidic cytoplasm"	     ,(THRO_DR|MV_ATT_NORM|PICK_UP|HAS_4D2|
			      CARRY_GOLD|CARRY_OBJ|HAS_60|HAS_90),(NONE8)
			    ,(IM_ACID|IM_FIRE|IM_LIGHTNING|IM_POISON|IM_FROST|
			      ANIMAL|CHARM_SLEEP|MAX_HP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,36,1,12,18,12,'j',{50,8},{115,115,115,115},35,5
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Inertia hound"	    ,(MV_ATT_NORM),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(NONE8)
			    ,(BREATH_SL)
			    ,500,0,30,30,11,'Z',{35,10},{39,39,39,58},35,2
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Impact hound"		   ,(MV_ATT_NORM),(0x8L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(NONE8)
			    ,(BREATH_WA)
			    ,500,0,30,30,11,'Z',{35,10},{39,39,39,58},35,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Dread"		    ,(MV_ATT_NORM|MV_20|CARRY_OBJ|HAS_2D2|
			     HAS_60|THRO_WALL|PICK_UP|MV_INVIS)
			    ,(0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS|CONFUSION)
			    ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|
			     IM_POISON),(NETHER_BOLT),(NONE8)
			    ,600,10,20,30,12,'G',{25,20},{235,235,80,0},35,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Ooze elemental"	    ,(MV_ATT_NORM|THRO_DR)
			    ,(0x5L|ACID_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING|IM_ACID|
			      CHARM_SLEEP|EVIL|NO_INFRA),(ACID_BALL),(NONE8)
			  ,300,90,10,80,11,'E',{13,10},{115,115,115,0},35,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Smoke elemental"	    ,(MV_ATT_NORM)
			    ,(0x5L|FIRE_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING|
			      CHARM_SLEEP|EVIL),(DARKNESS),(NONE8)
			    ,375,90,10,80,12,'E',{15,10},{36,36,0,0},35,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Young black dragon"	    ,(MV_ATT_NORM|HAS_1D2|HAS_60|HAS_90|
			     CARRY_GOLD|CARRY_OBJ|THRO_DR),
			     (BREATH_A|0xBL|FEAR)
			    ,(EVIL|IM_ACID|DRAGON|MAX_HP),(NONE8),(NONE8)
			    ,620,50,20,60,11,'d',{32,8},{53,53,29,0},35,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Mumak"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL),(NONE8),(NONE8)
			 ,2100,100,20,55,11,'q',{90,10},{227,227,233,0},35,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Giant red ant lion"	    ,(MV_ATT_NORM|THRO_CREAT),(NONE8)
			    ,(ANIMAL|IM_FIRE|MAX_HP|GROUP),(NONE8),(NONE8)
			    ,350,40,14,49,11,'a',{25,8},{107,107,0,0},35,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Mature white dragon"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|CARRY_OBJ|
			      HAS_2D2),(0xAL|BREATH_FR|FEAR)
			    ,(CHARM_SLEEP|IM_FROST|EVIL|DRAGON|MAX_HP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,1000,70,20,65,11,'d',{50,8},{54,54,37,0},35,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Xorn"			    ,(MV_ATT_NORM|THRO_WALL|THRO_DR|PICK_UP)
			    ,(NONE8),(IM_FIRE|IM_FROST|IM_POISON|NO_INFRA|
			     IM_LIGHTNING|CHARM_SLEEP|HURT_ROCK|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,650,10,20,80,11,'X',{20,8},{5,5,5,5},36,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Shadow"		    ,(MV_ATT_NORM|THRO_WALL|MV_INVIS|CARRY_OBJ|
			     HAS_1D2),(0x8L|TELE_TO|SLOW),(UNDEAD|EVIL|
			     IM_FROST|IM_POISON|NO_INFRA|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,400,20,30,30,12,'G',{10,20},{200,200,184,252},
								  36,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Phantom"		    ,(MV_ATT_NORM|THRO_WALL|MV_INVIS|CARRY_OBJ|
			     HAS_1D2),(0x5L),(UNDEAD|EVIL|IM_FROST|IM_POISON|
			     NO_INFRA|CHARM_SLEEP),(FORGET),(NONE8)
			    ,400,20,30,30,12,'G',{20,25},{200,200,184,252},
								  36,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Grey wraith"		    ,(MV_ATT_NORM|THRO_DR|HAS_60|HAS_90|
			      CARRY_GOLD|CARRY_OBJ)
			    ,(0x7L|CAUSE_CRIT|HOLD_PERSON|FEAR)
			   ,(UNDEAD|EVIL|NO_INFRA|CHARM_SLEEP|IM_FROST|MAX_HP|
			     IM_POISON),(DARKNESS),(NONE8)
			    ,700,10,20,50,11,'W',{24,8},{9,9,196,0},36,1
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Young multi-hued dragon"  ,(MV_ATT_NORM|HAS_4D2|CARRY_GOLD|CARRY_OBJ|
			      THRO_DR|HAS_60|HAS_90)
			    ,(0x5L|BREATH_G|BREATH_L|BREATH_A|BREATH_FR|
			      BREATH_FI|FEAR)
			    ,(IM_FROST|IM_ACID|IM_POISON|IM_LIGHTNING|
			      IM_FIRE|EVIL|DRAGON|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1320,50,20,60,11,'d',{40,8},{55,55,38,0},36,1
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Colossus"		    ,(MV_ATT_NORM),(NONE8),(MAX_HP|CHARM_SLEEP|
			     NO_INFRA|IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING)
			    ,(NONE8),(NONE8)
		     ,850,10,12,150,10,'g',{200,15},{212,212,235,235},36,4
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Young gold dragon"	    ,(MV_ATT_NORM|HAS_2D2|HAS_60|HAS_90|
			     CARRY_GOLD|CARRY_OBJ|THRO_DR),
			     (0xBL|FEAR)
			    ,(DRAGON|MAX_HP)
			    ,(BREATH_SD),(NONE8)
			    ,950,150,20,63,11,'d',{38,8},{54,54,37,0},36,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Rogrog the Black Troll"   ,(MV_ATT_NORM|PICK_UP|THRO_DR|HAS_2D2|
			      CARRY_OBJ),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|UNIQUE|
			      GOOD|IM_FROST),(NONE8),(NONE8)
			  ,5000,50,20,70,12,'T',{55,28},{235,38,33,229},36,5
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Mature blue dragon"	    ,(HAS_2D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|BREATH_L|FEAR)
			    ,(EVIL|DRAGON|IM_LIGHTNING|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1200,70,20,75,11,'d',{49,8},{54,54,38,0},36,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Mature green dragon"	    ,(HAS_2D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|BREATH_G|FEAR)
			    ,(EVIL|DRAGON|IM_POISON|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1100,70,20,70,11,'d',{49,8},{52,52,29,0},36,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Mature bronze dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|FEAR|CONFUSION)
			    ,(DRAGON|CHARM_SLEEP|MAX_HP)
			    ,(BREATH_CO),(NONE8)
			    ,1300,150,20,70,11,'d',{55,8},{54,54,38,0},36,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Young red dragon"	    ,(MV_ATT_NORM|HAS_1D2|HAS_60|HAS_90|
			     CARRY_GOLD|CARRY_OBJ|THRO_DR),
			     (BREATH_FI|0xBL|FEAR)
			    ,(EVIL|IM_FIRE|DRAGON|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,640,50,20,63,11,'d',{36,8},{54,54,37,0},36,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Trapper"                  ,(MV_ONLY_ATT|MV_INVIS),(NONE8)
                            ,(NO_INFRA|CHARM_SLEEP|MAX_HP),(NONE8),(NONE8)
			    ,580,10,30,75,12,'.',{50,12},{20,20,265,265},36,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Bodak"		    ,(PICK_UP|MV_ATT_NORM|THRO_DR)
			    ,(0x4L|FIRE_BALL|FIRE_BOLT|S_DEMON)
			    ,(IM_POISON|IM_FIRE|CHARM_SLEEP|EVIL)
			    ,(NONE8),(NONE8)
			  ,750,90,10,68,11,'I',{35,10},{103,103,224,0},36,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Ice elemental"	    ,(PICK_UP|MV_ATT_NORM)
			    ,(0x5L|FROST_BALL)
			    ,(IM_POISON|IM_FROST|IM_LIGHTNING|
			     CHARM_SLEEP|EVIL|NO_INFRA),(ICE_BOLT),(NONE8)
			   ,650,90,10,60,11,'E',{35,10},{121,22,121,0},36,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Warlock"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(0x3L|TELE|TELE_TO|CAUSE_CRIT|HOLD_PERSON|
			   S_UNDEAD|FEAR|BLINDNESS),(EVIL|MAX_HP|INTELLIGENT)
			    ,(NETHER_BOLT|HASTE),(NONE8)
			    ,630,10,20,50,11,'p',{25,11},{15,15,0,0},36,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Lorgan, Chief of the Easterlings"
			    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
			    ,(0x4L|TELE_TO),(EVIL|MAX_HP|CHARM_SLEEP|IM_ACID|
			     IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING|GOOD|
			     UNIQUE),(SUMMON),(NONE8)
		       ,1200,10,25,100,12,'p',{50,35},{235,235,20,20},36,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Demonist"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2)
			    ,(0x2L|TELE|HOLD_PERSON|
			     S_DEMON),(EVIL|MAX_HP|INTELLIGENT)
			    ,(NONE8),(NONE8)
			    ,700,10,20,50,12,'p',{25,11},{15,15,14,0},36,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Mummified troll"	    ,(HAS_60|CARRY_GOLD|CARRY_OBJ|
			     THRO_DR|MV_ATT_NORM),(NONE8)
			    ,(UNDEAD|IM_FROST|CHARM_SLEEP|IM_POISON|
			      TROLL|EVIL|NO_INFRA|MAX_HP),(NONE8),(NONE8)
			    ,420,50,20,50,11,'M',{19,10},{15,15,0,0},37,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"The Queen Ant"	    ,(MV_ATT_NORM|CARRY_OBJ|HAS_2D2|THRO_DR)
			    ,(0x2L),(ANIMAL|MAX_HP|UNIQUE|CHARM_SLEEP|
			     GOOD),(NONE8),(S_ANT)
			 ,1000,10,30,100,12,'a',{120,12},{39,39,37,37},37,2
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Will o' the wisp"	    ,(MV_ATT_NORM|MV_40|MV_INVIS|THRO_DR|THRO_WALL)
			    ,(0x2L|BLINK|CAUSE_SERIOUS|CONFUSION|TELE)
			    ,(CHARM_SLEEP|IM_FIRE|IM_FROST|IM_POISON|
			     IM_ACID|IM_LIGHTNING|MAX_HP|INTELLIGENT)
			    ,(NONE8),(NONE8)
			    ,500,0,30,150,13,'E',{20,10},{8,8,8,8},37,4
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Magma elemental"	    ,(THRO_WALL|PICK_UP|MV_ATT_NORM)
			    ,(0x7L|FIRE_BALL)
			    ,(IM_POISON|IM_FIRE|IM_LIGHTNING|
			      CHARM_SLEEP|EVIL),(PLASMA_BOLT),(NONE8)
			    ,950,90,10,70,11,'E',{35,10},{102,22,102,0},37,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Black pudding"	    ,(THRO_DR|MV_ATT_NORM|PICK_UP|HAS_1D2|
			      CARRY_GOLD|CARRY_OBJ|HAS_60|HAS_90),(NONE8)
			    ,(IM_ACID|IM_FIRE|IM_LIGHTNING|IM_POISON|IM_FROST|
			      ANIMAL|CHARM_SLEEP|MAX_HP|NO_INFRA|GROUP)
			    ,(NONE8),(NONE8)
			    ,36,1,12,18,11,'j',{50,8},{115,115,115,115},37,5
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Iridescent beetle"	    ,(MV_ATT_NORM),(NONE8)
			    ,(ANIMAL|IM_LIGHTNING|MAX_HP),(NONE8),(NONE8)
			    ,850,30,16,60,11,'K',{32,8},{45,10,146,0},37,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Nexus vortex"		    ,(MV_ATT_NORM|MV_75),(0x6L)
			    ,(CHARM_SLEEP),(BREATH_NE),(NONE8)
			    ,800,0,100,40,12,'v',{32,10},{244,0,0,0},37,1
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Plasma vortex"	    ,(MV_ATT_NORM|MV_75),(0x6L)
			    ,(IM_FIRE|CHARM_SLEEP),(NONE8),(BREATH_PL)
			    ,800,0,100,40,12,'v',{32,10},{243,0,0,0},37,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Mature red dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|BREATH_FI|FEAR|CONFUSION)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1400,30,20,80,11,'d',{60,8},{52,56,39,0},37,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Mature gold dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|FEAR|CONFUSION)
			    ,(DRAGON|CHARM_SLEEP|MAX_HP)
			    ,(BREATH_SD),(NONE8)
			    ,1500,150,20,80,11,'d',{70,8},{52,56,39,0},37,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Crystal drake"	    ,(MV_ATT_NORM|MV_INVIS|THRO_DR|HAS_4D2|
			     CARRY_OBJ)
			    ,(0x6L|FEAR|CONFUSION|SLOW)
			    ,(EVIL|IM_FROST|CHARM_SLEEP|MAX_HP|DRAGON)
			    ,(BREATH_SH),(NONE8)
			   ,1500,30,25,100,12,'d',{50,10},{52,52,35,0},37,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Mature black dragon"	    ,(HAS_2D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|BREATH_A|FEAR)
			    ,(EVIL|DRAGON|IM_ACID|CHARM_SLEEP|MAX_HP),(NONE8)
			    ,(NONE8)
			    ,1350,30,20,55,11,'d',{58,8},{54,54,38,0},37,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Mature multi-hued dragon" ,(MV_ATT_NORM|HAS_4D2|CARRY_GOLD|CARRY_OBJ|
			      THRO_DR|HAS_60|HAS_90|HAS_2D2)
			    ,(0x5L|BREATH_G|BREATH_L|BREATH_A|BREATH_FR|
			      BREATH_FI|FEAR|CONFUSION|BLINDNESS)
			    ,(IM_FROST|IM_ACID|IM_POISON|IM_LIGHTNING|
			      IM_FIRE|EVIL|DRAGON|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1700,50,20,65,11,'d',{81,8},{56,56,39,0},38,2
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Death knight"		    ,(HAS_2D2|HAS_1D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x5L|CAUSE_CRIT|BLINDNESS|FEAR)
			    ,(EVIL|IM_FROST|NO_INFRA|MAX_HP|INTELLIGENT)
			    ,(NETHER_BOLT|SUMMON),(NONE8)
			  ,1000,10,20,100,12,'p',{30,20},{235,23,23,0},38,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Castamir the Usurper"	    ,(HAS_2D2|CARRY_OBJ|THRO_DR|PICK_UP|
			      MV_ATT_NORM),(0x2L|FIRE_BOLT|FROST_BOLT)
			    ,(EVIL|MAX_HP|UNIQUE|GOOD|INTELLIGENT)
			    ,(TRAP_CREATE|ICE_BOLT|HEAL|LIGHT_BOLT),(NONE8)
			    ,1600,40,20,90,12,'p',{80,11},{23,23,23,23},38,5
#ifdef TC_COLOR
  , RED
#endif
},

{"Time vortex"		    ,(MV_ATT_NORM|MV_75),(0x6L)
			    ,(CHARM_SLEEP),(NONE8),(BREATH_TI)
			    ,900,0,100,40,13,'v',{32,10},{244,0,0,0},38,4
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Shimmering vortex"	    ,(MV_ATT_NORM|MV_75),(0x6L)
			    ,(CHARM_SLEEP),(NONE8),(BREATH_LT)
			    ,200,0,100,30,14,'v',{6,12},{203,203,0,0},38,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Ancient blue dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x9L|BREATH_L|FEAR|BLINDNESS
			      |CONFUSION)
			    ,(EVIL|DRAGON|IM_LIGHTNING|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1500,80,20,80,12,'D',{88,8},{54,54,37,0},38,1
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Ancient bronze dragon"    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x6L|FEAR|BLINDNESS|CONFUSION)
			    ,(DRAGON|CHARM_SLEEP|MAX_HP),(BREATH_CO),(NONE8)
			    ,1700,200,20,100,12,'D',{92,8},{54,54,38,0},38,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Beholder"		    ,(MV_ATT_NORM)
			    ,(0x2L|FIRE_BOLT|FROST_BOLT|ACID_BOLT|
			      MANA_DRAIN|BLINDNESS|CONFUSION|FEAR|SLOW)
			    ,(EVIL|CHARM_SLEEP|MAX_HP|IM_POISON)
			    ,(FORGET|MIND_BLAST|DARKNESS),(NONE8)
			,6000,10,30,80,12,'e',{80,20},{223,224,225,226},38,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Emperor wight"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x6L|CAUSE_CRIT|HOLD_PERSON|FEAR)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|
			      IM_POISON|NO_INFRA|HURT_LIGHT)
			    ,(NETHER_BOLT),(NONE8)
			    ,1600,10,20,40,12,'W',{48,8},{10,10,199,0},38,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Planetar"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_2D2|HAS_1D2)
			    ,(0xBL|CONFUSION|MANA_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|INTELLIGENT|
			      IM_ACID|IM_LIGHTNING|CHARM_SLEEP|MAX_HP)
			    ,(HEAL|HASTE|SUMMON|TELE_AWAY|PLASMA_BOLT|S_ANGEL)
			    ,(NONE8)
			    ,1800,255,30,68,12,'A',{50,10}
			    ,{22,23,23,22},38,6
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Vargo, Tyrant of Fire"    ,(MV_ATT_NORM|MV_20|THRO_CREAT)
			    ,(0x4L|FIRE_BALL)
			   ,(EVIL|IM_POISON|CHARM_SLEEP|IM_FIRE|MAX_HP|UNIQUE)
			   ,(PLASMA_BOLT),(NONE8)
			,3000,50,12,50,12,'E',{60,25},{103,103,103,103},38,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Black wraith"		    ,(HAS_2D2|CARRY_OBJ|HAS_1D2|THRO_DR|MV_ATT_NORM)
			    ,(0x7L|CAUSE_CRIT|HOLD_PERSON|FEAR|BLINDNESS)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|
			      IM_POISON|HURT_LIGHT|NO_INFRA)
			    ,(NETHER_BOLT),(NONE8)
			    ,1700,10,20,55,12,'W',{50,10},{10,10,199,0},38,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Erinyes"		    ,(MV_ATT_NORM|HAS_60|CARRY_OBJ|THRO_DR)
			    ,(0x7L|BLINDNESS|CONFUSION|FIRE_BOLT)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP|IM_POISON)
			    ,(NONE8),(NONE8)
			    ,1000,80,20,50,11,'&',{18,8},{17,87,0,0},38,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Nether wraith"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM|
			      MV_INVIS|THRO_WALL)
			    ,(0x6L|CAUSE_CRIT|FEAR|BLINDNESS)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|
			      HURT_LIGHT|NO_INFRA|IM_POISON)
			    ,(NETHER_BOLT|MIND_BLAST|DARKNESS),(NONE8)
			    ,1700,10,20,55,12,'W',{60,8},{10,10,202,0},39,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Eldrak"		    ,(MV_ATT_NORM|PICK_UP|THRO_DR|HAS_60|
			      CARRY_OBJ|CARRY_GOLD),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,800,50,20,80,11,'T',{30,25},{17,17,17,0},39,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Ettin"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2),(NONE8)
			    ,(TROLL|EVIL|IM_POISON|MAX_HP|CHARM_SLEEP)
			    ,(NONE8),(NONE8)
			    ,1000,30,20,100,11,'T',{50,30},{19,19,19,0},39,3
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Waldern, King of Water"   ,(MV_ATT_NORM|MV_20|THRO_CREAT)
			    ,(0x4L|FROST_BALL)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|MAX_HP|UNIQUE
			      |NO_INFRA)
			    ,(WATER_BOLT|WATER_BALL|ICE_BOLT),(NONE8)
			    ,3250,50,12,40,12,'E',{80,25},{23,23,23,23},39,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Kavlax the Many-Headed"   ,(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|THRO_DR)
			    ,(0x4L|BREATH_FI|BREATH_FR|BREATH_L|BREATH_A)
			    ,(EVIL|DRAGON|CHARM_SLEEP|MAX_HP|UNIQUE|GOOD|
			     IM_ACID|IM_FROST|IM_FIRE|IM_LIGHTNING)
			    ,(BREATH_CO|BREATH_SD|BREATH_SH|BREATH_NE)
			    ,(BREATH_GR)
		       ,3000,30,20,85,12,'d',{130,10},{56,39,39,39},39,3
#ifdef TC_COLOR
  , ANY
#endif
},

{"Ancient white dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x9L|BREATH_FR|FEAR|BLINDNESS|CONFUSION)
			    ,(EVIL|DRAGON|IM_FROST|CHARM_SLEEP|MAX_HP|NO_INFRA)
			    ,(NONE8),(NONE8)
			    ,2500,80,20,90,12,'D',{88,8},{55,55,39,0},39,1
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Ancient green dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x9L|BREATH_G|FEAR|BLINDNESS|CONFUSION)
			    ,(EVIL|DRAGON|IM_POISON|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,2400,80,20,85,12,'D',{90,8},{54,54,38,0},39,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"7-headed hydra"	    ,(MV_ATT_NORM|CARRY_GOLD|HAS_4D2|HAS_2D2)
			    ,(0x5L|FEAR|BREATH_G)
			    ,(ANIMAL|IM_POISON),(ST_CLOUD),(NONE8)
		     ,2000,20,20,90,12,'R',{100,10},{162,162,162,144},39,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Night mare"		    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|CARRY_GOLD)
			    ,(NONE8),(UNDEAD|IM_POISON|IM_FROST|EVIL|
			     CHARM_SLEEP|NO_INFRA|MAX_HP)
			    ,(NONE8),(NONE8)
			,2900,0,30,85,12,'q',{150,10},{236,20,20,216},39,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Vampire lord"		    ,(MV_ATT_NORM|THRO_DR|HAS_4D2|CARRY_GOLD|
			     CARRY_OBJ|HAS_60)
			    ,(0x7L|CAUSE_CRIT|MANA_DRAIN|FEAR|HOLD_PERSON|
			      BLINDNESS)
			    ,(UNDEAD|EVIL|MAX_HP|CHARM_SLEEP|HURT_LIGHT|
			      IM_FROST|HURT_LIGHT|NO_INFRA|IM_POISON)
			    ,(BRAIN_SMASH|NETHER_BOLT|RAZOR|DARKNESS),(NONE8)
			    ,1800,10,20,70,12,'V',{62,25},{5,5,5,198},39,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Ancient black dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x9L|BREATH_A|FEAR|BLINDNESS|CONFUSION)
			    ,(EVIL|DRAGON|IM_ACID|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,2500,70,20,90,12,'D',{90,8},{55,55,38,0},39,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Disenchanter worm"	    ,(MULTIPLY|MV_ATT_NORM|MV_40),(NONE8),
			     (ANIMAL|HURT_LIGHT),(NONE8),(NONE8)
			    ,30,10,7,5,10,'w',{10,8},{208,0,0,0},40,3
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Rotting quylthulg"	    ,(MV_INVIS),(0x2L|S_UNDEAD|BLINK|TELE)
			    ,(ANIMAL|EVIL|MAX_HP|UNDEAD|IM_FROST|NO_INFRA|
			     CHARM_SLEEP),(NONE8),(NONE8)
			    ,1500,0,20,1,12,'Q',{20,8},{0,0,0,0},40,1
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Spirit troll"		    ,(MV_INVIS|MV_ATT_NORM|THRO_WALL|THRO_DR|
			     CARRY_OBJ|CARRY_GOLD|HAS_90),(NONE8)
			    ,(EVIL|TROLL|IM_POISON|IM_FROST|MAX_HP|UNDEAD|
			     CHARM_SLEEP|IM_LIGHTNING|NO_INFRA),(NONE8),(NONE8)
			,900,5,20,90,11,'T',{40,25},{19,18,18,0},40,3
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Lesser titan"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      CARRY_GOLD|HAS_4D2|HAS_2D2)
			    ,(0x3L|FEAR|TELE_TO)
			    ,(EVIL|GIANT|MAX_HP|INTELLIGENT)
			    ,(SUMMON|HEAL),(NONE8)
		       ,3500,15,30,80,12,'P',{35,30},{216,216,216,216},40,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"9-headed hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_4D2|HAS_2D2)
			    ,(0x4L|FEAR|FIRE_BOLT|BREATH_FI)
			    ,(ANIMAL|IM_FIRE),(NONE8),(NONE8)
		       ,3000,20,20,95,12,'R',{100,12},{106,106,106,106},40,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Enchantress"		    ,(HAS_2D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x2L|S_DRAGON|BLINDNESS)
			    ,(EVIL|CHARM_SLEEP|MAX_HP|GOOD),(NONE8),(NONE8)
			    ,2100,10,20,60,13,'p',{40,13},{15,15,16,0},40,4
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Archpriest"		    ,(HAS_2D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x2L|HOLD_PERSON|BLINDNESS|CONFUSION|
			      MONSTER|S_UNDEAD|CAUSE_CRIT)
			    ,(INTELLIGENT|EVIL|CHARM_SLEEP|MAX_HP)
			    ,(HEAL),(NONE8)
			    ,1800,10,20,60,12,'p',{40,13},{17,17,18,0},40,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Sorceror"		    ,(HAS_4D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x2L|BLINK|TELE_TO|BLINDNESS|CONFUSION|
			      MONSTER|S_UNDEAD|CAUSE_CRIT|S_DRAGON|
			      FIRE_BALL|FROST_BALL|ACID_BOLT)
			    ,(EVIL|CHARM_SLEEP|MAX_HP),(TRAP_CREATE),(NONE8)
			    ,2150,10,20,60,13,'p',{40,13},{16,16,16,0},40,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Xaren"		     ,(MV_ATT_NORM|THRO_WALL|THRO_DR|PICK_UP)
			    ,(NONE8),(IM_FIRE|IM_FROST|IM_POISON|NO_INFRA|
			     IM_LIGHTNING|CHARM_SLEEP|HURT_ROCK|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1200,10,20,80,12,'X',{40,8},{17,17,17,17},40,1
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Giant roc"                ,(MV_ATT_NORM),(NONE8),(ANIMAL|IM_LIGHTNING)
                            ,(NONE8),(NONE8)
			    ,1000,10,20,70,11,'B',{80,13},{78,78,284,0},40,3
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Uvatha the Horseman"	    ,(HAS_2D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(NONE8)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|GOOD|NO_INFRA|IM_POISON)
			    ,(NONE8),(NONE8)
			 ,7000,10,90,60,12,'W',{35,35},{235,23,23,199},40,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Minotaur"		    ,(MV_ATT_NORM),(NONE8),(EVIL),(NONE8),(NONE8)
			    ,2100,10,13,25,13,'H',{100,10}
			    ,{227,227,228,228},40,2
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Medusa, the Gorgon"	    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|HAS_1D2|CARRY_OBJ)
			    ,(0x2L|HOLD_PERSON|CAUSE_CRIT|FIRE_BOLT|FEAR)
			    ,(EVIL|IM_FIRE|CHARM_SLEEP|UNIQUE|INTELLIGENT|
			     GOOD|IM_POISON|MAX_HP|IM_ACID)
			    ,(PLASMA_BOLT|ACID_BALL),(S_REPTILE)
			,9000,5,30,100,12,'n',{40,60},{246,267,235,235},40,3
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Death drake"		    ,(MV_ATT_NORM|MV_INVIS|THRO_DR|HAS_2D2|HAS_4D2|
			     CARRY_OBJ|PICK_UP|THRO_WALL)
			    ,(0x6L|FEAR|CONFUSION|SLOW)
			    ,(EVIL|IM_FROST|CHARM_SLEEP|MAX_HP|DRAGON)
			    ,(BREATH_LD),(NONE8)
			 ,3500,30,25,100,12,'D',{105,10},{56,56,236,0},40,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Ancient red dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x6L|BREATH_FI|FEAR|BLINDNESS|CONFUSION)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			 ,2750,70,20,100,12,'D',{105,10},{56,56,40,0},40,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Ancient gold dragon"	    ,(HAS_4D2|CARRY_GOLD|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x6L|FEAR|BLINDNESS|CONFUSION)
			    ,(DRAGON|CHARM_SLEEP|MAX_HP),(BREATH_SD),(NONE8)
			 ,4000,200,20,100,12,'D',{150,10},{56,56,40,0},40,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Great crystal drake"	    ,(MV_ATT_NORM|MV_INVIS|THRO_DR|HAS_4D2|HAS_2D2|
			     CARRY_OBJ)
			    ,(0x6L|FEAR|CONFUSION|SLOW)
			    ,(EVIL|IM_FROST|CHARM_SLEEP|MAX_HP|DRAGON)
			    ,(BREATH_SH),(NONE8)
			   ,3500,30,25,100,11,'D',{50,30},{55,55,39,0},40,2
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Vrock"		    ,(MV_ATT_NORM|HAS_60|CARRY_OBJ|THRO_DR)
			    ,(0x8L|BLINDNESS|CONFUSION)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP|GROUP)
			    ,(NONE8),(NONE8)
			    ,1000,80,20,50,11,'&',{20,11},{17,78,78,0},40,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Death quasit"		    ,(HAS_4D2|HAS_2D2|HAS_90|CARRY_OBJ|MV_INVIS|
			      THRO_WALL|MV_ATT_NORM)
			  ,(0xAL|FEAR|CONFUSION|BLINDNESS|CAUSE_CRIT|S_DEMON)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|MAX_HP|DEMON|IM_FIRE
			      |INTELLIGENT),(FORGET),(NONE8)
			    ,1000,0,20,80,13,'I',{55,8},{177,58,58,0},40,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Adunaphel the Quiet"	   ,(HAS_4D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM|MV_INVIS
			     |THRO_WALL)
			    ,(0x3L|CAUSE_CRIT|HOLD_PERSON|FEAR|BLINDNESS
			      |MONSTER|FIRE_BOLT|FROST_BOLT|ACID_BOLT)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|GOOD|NO_INFRA|IM_POISON)
			    ,(FORGET|NETHER_BOLT),(NONE8)
			  ,8000,10,90,60,12,'W',{35,35},{23,23,199,0},41,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Dark elven sorceror"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x2L|BLINK|TELE_TO|BLINDNESS|CONFUSION|
			      MONSTER|S_UNDEAD|CAUSE_CRIT|S_DEMON|
			      FIRE_BALL|FROST_BALL|ACID_BOLT)
			    ,(EVIL|CHARM_SLEEP|INTELLIGENT|MAX_HP|HURT_LIGHT)
			    ,(HEAL|DARKNESS),(NONE8)
			    ,3000,10,20,70,13,'h',{40,20},{16,16,16,0},41,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Master lich"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_4D2|HAS_2D2)
			    ,(0x3L|FEAR|CONFUSION|BLINDNESS|HOLD_PERSON|
			      CAUSE_CRIT|MANA_DRAIN|TELE_TO|BLINK|S_UNDEAD)
			    ,(UNDEAD|IM_POISON|IM_FROST|EVIL|MAX_HP|
			      CHARM_SLEEP|NO_INFRA|INTELLIGENT)
			    ,(BRAIN_SMASH|RAZOR),(NONE8)
		       ,10000,50,20,80,12,'L',{42,42},{181,201,214,181},41,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Hezrou"		    ,(MV_ATT_NORM|HAS_2D2|CARRY_OBJ|THRO_DR)
			    ,(0x9L|S_DEMON|FIRE_BOLT)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP|GROUP)
			    ,(NONE8),(NONE8)
			    ,1500,80,20,40,11,'&',{20,15},{17,17,0,0},41,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Akhorahil the Blind"	    ,(HAS_4D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x3L|CAUSE_CRIT|HOLD_PERSON|FEAR|BLINDNESS
			      |MONSTER|FIRE_BOLT|FROST_BOLT)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |GOOD|IM_POISON|NO_INFRA)
			    ,(NETHER_BOLT|DARKNESS),(NONE8)
			  ,12000,10,90,70,12,'W',{35,50},{23,23,199,99},41,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Gorlim, Betrayer of Barahir",(MV_ATT_NORM|HAS_2D2|CARRY_OBJ|THRO_DR)
			    ,(0x2L|CAUSE_CRIT|MANA_BOLT)
			    ,(GOOD|UNIQUE|MAX_HP|INTELLIGENT|CHARM_SLEEP|EVIL|
			     IM_POISON|IM_FROST|IM_ACID|IM_LIGHTNING)
			    ,(WATER_BOLT),(NONE8)
		       ,7000,40,20,120,12,'p',{80,20},{218,218,230,230},41,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Solar"		    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      CARRY_GOLD|HAS_4D2|HAS_2D2|HAS_1D2)
			    ,(0x3L|FEAR|BLINDNESS|TELE_TO
			      |CAUSE_SERIOUS|MANA_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_ACID|IM_LIGHTNING|
			      GOOD|INTELLIGENT)
			    ,(S_ANGEL|RAZOR),(NONE8)
			    ,15000,255,30,140,13,'A',{120,30}
			    ,{217,217,218,218},41,6
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Glabrezu"		    ,(MV_ATT_NORM|HAS_90|CARRY_OBJ|THRO_DR)
			    ,(0x9L|S_DEMON|FIRE_BOLT)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,1750,80,20,40,11,'&',{22,15},{17,17,0,0},41,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Ren the Unclean"	    ,(HAS_4D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM|MV_INVIS)
			    ,(0x3L|CAUSE_CRIT|HOLD_PERSON|FEAR|BLINDNESS
			      |MONSTER|FIRE_BALL|FIRE_BOLT)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|IM_FIRE|GOOD|IM_POISON|NO_INFRA)
			    ,(NETHER_BOLT),(NONE8)
			  ,13000,10,90,70,12,'W',{35,50},{23,23,199,99},41,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Nalfeshnee"		    ,(MV_ATT_NORM|HAS_1D2|CARRY_OBJ|THRO_DR)
			    ,(0x9L|BLINDNESS|CONFUSION|BREATH_FI|S_DEMON)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,2000,80,20,50,11,'&',{30,15},{17,17,17,0},42,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Undead beholder"	    ,(MV_ATT_NORM)
			    ,(0x2L|S_UNDEAD|SLOW|MANA_DRAIN|MANA_BOLT)
			    ,(UNDEAD|EVIL|CHARM_SLEEP|MAX_HP|IM_POISON
			     |IM_FIRE|IM_LIGHTNING|IM_ACID|IM_FROST|NO_INFRA)
			    ,(FORGET|MIND_BLAST|RAZOR|BRAIN_SMASH),(NONE8)
		     ,6500,10,30,100,12,'e',{90,30},{223,224,225,226},42,4
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Dread"		    ,(MV_ATT_NORM|MV_20|CARRY_OBJ|
			     HAS_60|THRO_WALL|PICK_UP|MV_INVIS)
			    ,(0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS|CONFUSION)
			    ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|GROUP|
			     IM_POISON),(NETHER_BOLT),(NONE8)
			    ,600,10,20,30,12,'G',{25,20},{235,235,80,0},43,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Mumak"		    ,(MV_ATT_NORM),(NONE8),(ANIMAL|GROUP)
			    ,(NONE8),(NONE8)
			 ,2100,100,20,55,11,'q',{90,10},{227,227,233,0},43,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Ancient multi-hued dragon",(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|
			      THRO_DR|HAS_60|HAS_90|HAS_2D2|HAS_1D2)
			    ,(0x5L|BREATH_G|BREATH_L|BREATH_A|BREATH_FR|
			      BREATH_FI|FEAR|CONFUSION|BLINDNESS)
			    ,(IM_FROST|IM_ACID|IM_POISON|IM_LIGHTNING|
			      IM_FIRE|EVIL|DRAGON|CHARM_SLEEP|MAX_HP)
			    ,(NONE8),(NONE8)
			  ,13000,70,20,100,12,'D',{52,40},{57,57,42,0},43,1
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Ethereal dragon"	   ,(MV_ATT_NORM|THRO_DR|HAS_60|HAS_90|HAS_4D2|
			    HAS_2D2|HAS_1D2|CARRY_OBJ|MV_INVIS|THRO_WALL)
			   ,(0x5L|CONFUSION|BLINDNESS)
			   ,(DRAGON|CHARM_SLEEP|MAX_HP)
			   ,(BREATH_CO),(BREATH_LT|BREATH_DA)
			,11000,15,25,100,12,'D',{52,40},{57,57,42,0},43,2
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Ji Indur Dawndeath"	   ,(HAS_4D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM|MV_INVIS)
			    ,(0x3L|FIRE_BALL|CAUSE_CRIT|HOLD_PERSON|
			      FEAR|BLINDNESS|S_UNDEAD)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|IM_FIRE|GOOD|IM_POISON|NO_INFRA)
			    ,(NETHER_BALL)
			    ,(NONE8)
			  ,12000,10,90,70,12,'W',{35,50},{23,23,199,0},43,4
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Marilith"		    ,(MV_ATT_NORM|HAS_1D2|CARRY_OBJ|THRO_DR)
			    ,(0x9L|CAUSE_SERIOUS|BLINDNESS|S_DEMON)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP)
			    ,(NONE8),(NONE8)
			    ,5000,80,20,75,12,'&',{40,15},{19,19,19,19},43,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Quaker, Master of Earth"  ,(THRO_WALL|PICK_UP|MV_ATT_NORM),(0x6L|ACID_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_LIGHTNING|
			      CHARM_SLEEP|HURT_ROCK|EVIL|MAX_HP|NO_INFRA|
			      DESTRUCT|UNIQUE),(ACID_BALL),(NONE8)
			,6000,90,10,97,11,'E',{90,20},{212,235,235,235},43,4
#ifdef TC_COLOR
  , BROWN
#endif
},

{"Lesser balrog"		    ,(MV_ATT_NORM|HAS_2D2|HAS_1D2|
			      CARRY_OBJ|THRO_DR)
			   ,(0x4L|CONFUSION|BLINDNESS|S_DEMON|BREATH_FI)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP)
			    ,(NONE8),(NONE8)
			,8000,80,20,50,12,'&',{60,30},{101,22,101,23},44,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Ariel, Queen of Air"	    ,(MV_ATT_NORM|MV_20|THRO_CREAT),(0x5L|FROST_BALL)
			    ,(EVIL|IM_POISON|CHARM_SLEEP|IM_FIRE|IM_FROST|
			      IM_LIGHTNING|IM_ACID|IM_POISON|MAX_HP|UNIQUE|
			      NO_INFRA),(LIGHT_BALL|LIGHT_BOLT),(NONE8)
			,8000,50,12,50,13,'E',{60,45},{22,89,22,89},44,4
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"11-headed hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_4D2|HAS_2D2)
			    ,(0x4L|FEAR|FIRE_BOLT|FIRE_BALL|BREATH_FI)
			    ,(ANIMAL|IM_FIRE),(PLASMA_BOLT),(NONE8)
		      ,6000,20,20,100,12,'R',{100,18},{107,107,107,107},44,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Patriarch"		    ,(HAS_4D2|CARRY_OBJ|HAS_90|THRO_DR|MV_ATT_NORM)
			    ,(0x2L|HOLD_PERSON|BLINDNESS|S_UNDEAD)
			    ,(EVIL|CHARM_SLEEP|MAX_HP|INTELLIGENT)
			    ,(HEAL|BRAIN_SMASH|RAZOR|SUMMON),(NONE8)
			,5000,10,20,60,12,'p',{40,20},{17,17,18,0},44,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Dreadmaster"		   ,(MV_ATT_NORM|MV_20|HAS_4D2|CARRY_OBJ|HAS_1D2|
			     THRO_WALL|PICK_UP|MV_INVIS)
			   ,(0x9L|HOLD_PERSON|MANA_DRAIN|BLINDNESS
			     |CONFUSION|S_UNDEAD)
			   ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|MAX_HP|
			     INTELLIGENT|IM_POISON)
			   ,(NETHER_BOLT|RAZOR|TELE_LEV),(NONE8)
			,8000,10,20,100,12,'G',{60,20},{235,235,80,80},44,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Master mystic"	   ,(MV_ATT_NORM|HAS_2D2|HAS_1D2|CARRY_OBJ|MV_INVIS)
			   ,(0x3L),(IM_FROST|IM_FIRE|IM_POISON|IM_LIGHTNING|
			    IM_ACID|MAX_HP|CHARM_SLEEP)
			   ,(HEAL|S_SPIDER),(NONE8)
			,6000,5,30,60,13,'p',{20,55},{266,266,264,265},44,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

/*
 * Unique monsters have no word before them e.g Tiamat etc.. hits you
 * not The Tiamat etc.. hits you or A Tiamat etc... hits you,
 * But DONT use capital letters at the beginning unless it is a name
 * e.g. You hit a Balrog...
 * NOT You hit A Balrog...
 */

{"Drolem"		    ,(MV_ATT_NORM|THRO_DR),(0x6L|BLINDNESS|CONFUSION|
			     SLOW|BREATH_G)
			    ,(DRAGON|CHARM_SLEEP|IM_FROST|IM_FIRE|
			     IM_POISON|IM_LIGHTNING|MAX_HP|NO_INFRA)
			    ,(MISSILE),(NONE8)
		      ,12000,30,25,130,12,'g',{100,30},{48,48,238,238},44,3
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Scatha the Worm"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x3L|BREATH_FR|CAUSE_CRIT|CONFUSION)
			    ,(EVIL|DRAGON|IM_FROST|CHARM_SLEEP|MAX_HP|NO_INFRA|
			      UNIQUE|GOOD),(NONE8),(NONE8)
			,17000,70,20,130,12,'D',{150,12},{56,56,56,276},44,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Dwar, Dog Lord of Waw"    ,(HAS_4D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x3L|FIRE_BALL|CAUSE_CRIT|HOLD_PERSON|
			      FEAR|BLINDNESS|S_UNDEAD)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|IM_FIRE|GOOD|INTELLIGENT|IM_POISON|
			     NO_INFRA),(NETHER_BALL|SUMMON|S_HOUND),(NONE8)
			 ,13000,10,90,90,12,'W',{40,50},{23,23,199,99},44,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Smaug the Golden"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x3L|BREATH_FI|CAUSE_CRIT|CONFUSION)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP|
			      UNIQUE|GOOD),(NONE8),(NONE8)
			 ,19000,70,20,100,12,'D',{150,13},{56,56,56,276},45,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Dracolich"		    ,(MV_ATT_NORM|THRO_DR|HAS_2D2|HAS_4D2|
			     CARRY_OBJ|PICK_UP)
			    ,(0x6L|FEAR|CONFUSION|MANA_BOLT|BREATH_FR)
			    ,(EVIL|IM_FROST|CHARM_SLEEP|UNDEAD|
			    MAX_HP|DRAGON|IM_POISON|NO_INFRA),(NONE8),(NONE8)
			,18000,30,25,120,12,'D',{70,50},{57,57,236,236},46,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Greater titan"	    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      CARRY_GOLD|HAS_4D2|HAS_2D2)
			    ,(0x3L|TELE_TO)
			    ,(EVIL|GIANT|MAX_HP|INTELLIGENT)
			    ,(SUMMON|HEAL),(NONE8)
		      ,13500,15,30,125,12,'P',{75,50},{269,269,269,269},46,3
#ifdef TC_COLOR
  , MAGENTA
#endif
},

{"Dracolisk"		    ,(MV_ATT_NORM|THRO_DR|HAS_4D2|CARRY_OBJ)
			    ,(0x6L|HOLD_PERSON|FEAR|BREATH_FI)
			    ,(ANIMAL|EVIL|CHARM_SLEEP|MAX_HP|DRAGON|IM_FIRE|
			     IM_ACID),(BREATH_NE),(NONE8)
			,14000,30,25,120,12,'H',{70,50},{39,39,48,146},46,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Death mold"		    ,(MV_ONLY_ATT|THRO_DR),(NONE8),(IM_FIRE|IM_POISON|
			     IM_FROST|IM_ACID|IM_LIGHTNING|ANIMAL|EVIL)
			    ,(NONE8),(NONE8)
		      ,1000,0,200,60,14,'m',{200,10},{257,257,257,202},47,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Itangast the Fire Drake"  ,(HAS_4D2|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x3L|BREATH_FI|CAUSE_CRIT|CONFUSION)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP|
			      UNIQUE|GOOD),(NONE8),(NONE8)
		       ,20000,70,20,100,12,'D',{150,15},{56,56,276,277},47,4
#ifdef TC_COLOR
  , RED
#endif
},

{"Glaurung, Father of the Dragons"
			    ,(HAS_4D2|CARRY_OBJ|HAS_90|HAS_2D2|
			      HAS_60|MV_ATT_NORM)
			    ,(0x5L|BREATH_FI|CAUSE_CRIT|CONFUSION|S_DRAGON)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP|
			      UNIQUE|GOOD),(NONE8),(NONE8)
		      ,25000,70,20,120,12,'D',{110,25},{272,272,279,279},48,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Muar, the Balrog",	     (MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
			      CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
			    ,(0x4L|FEAR|S_UNDEAD|BREATH_FI|CONFUSION|S_DEMON)
			    ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP|
			      UNIQUE|GOOD),(NONE8),(NONE8)
		       ,30000,80,20,100,12,'&',{50,60},{104,78,214,0},50,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Nightwing"		    ,(MV_ATT_NORM|HAS_2D2|CARRY_OBJ|THRO_DR)
			    ,(0x4L|FEAR|S_UNDEAD|BLINDNESS|MANA_BOLT)
			    ,(EVIL|UNDEAD|IM_FROST|IM_POISON|CHARM_SLEEP|
			     INTELLIGENT|GOOD|NO_INFRA)
			    ,(BRAIN_SMASH|RAZOR|NETHER_BALL|NETHER_BOLT)
			    ,(NONE8)
		      ,6000,10,20,120,12,'W',{60,30},{172,172,230,230},50,4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Nether hound"		    ,(MV_ATT_NORM|THRO_DR),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(BREATH_LD)
			    ,(NONE8)
		       ,5000,0,30,100,12,'Z',{60,10},{39,39,39,58},51,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Time hound"		    ,(MV_ATT_NORM|THRO_DR),(0x8L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(NONE8)
			    ,(BREATH_TI)
			    ,5000,0,30,100,13,'Z',{60,10},{39,39,39,58},51,4
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Plasma hound"		    ,(MV_ATT_NORM|THRO_DR),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP|IM_FIRE),(NONE8)
			    ,(BREATH_PL)
			    ,5000,0,30,100,12,'Z',{60,10},{39,39,39,58},51,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Demonic quylthulg"	    ,(MV_INVIS),(0x2L|S_DEMON|BLINK|TELE)
			    ,(MAX_HP|CHARM_SLEEP|ANIMAL|EVIL),(NONE8),(NONE8)
			    ,3000,0,20,1,12,'Q',{60,8},{0,0,0,0},51,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Great storm wyrm"	    ,(HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x6L|BREATH_L|FEAR|BLINDNESS
			      |CONFUSION)
			    ,(EVIL|DRAGON|IM_LIGHTNING|CHARM_SLEEP
			     |MAX_HP|GOOD),(NONE8),(NONE8)
			  ,17000,80,30,150,12,'D',{50,60},{57,57,57,277},51,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Baphomet the Minotaur Lord" ,(HAS_4D2|HAS_1D2|CARRY_OBJ|MV_ATT_NORM)
			    ,(0x6L|SLOW|MANA_BOLT),(EVIL|CHARM_SLEEP|UNIQUE|
			     MAX_HP|GOOD|IM_POISON|IM_FIRE),(PLASMA_BOLT|
			     MISSILE|LIGHT_BALL),(BREATH_WA)
		      ,18000,30,30,120,13,'H',{70,50},{282,282,212,212},51,4
#ifdef TC_COLOR
  , CYAN
#endif
},

{"Harowen the Black Hand"   ,(HAS_4D2|HAS_1D2|CARRY_OBJ|MV_ATT_NORM|PICK_UP)
			    ,(0x6L),(CHARM_SLEEP|MAX_HP|UNIQUE|
			     GOOD|IM_POISON),(TRAP_CREATE),(NONE8)
			,20000,0,40,90,14,'p',{50,50},{258,259,260,261},52,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Hoarmurath of Dir",	(HAS_4D2|HAS_2D2|CARRY_OBJ|THRO_DR|MV_ATT_NORM)
			    ,(0x3L|FROST_BALL|CAUSE_CRIT|HOLD_PERSON
			      |FROST_BOLT|FEAR|BLINDNESS|S_UNDEAD)
			    ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			      |HURT_LIGHT|GOOD|INTELLIGENT|IM_POISON|
			     NO_INFRA),(RAZOR|NETHER_BALL|MIND_BLAST),(NONE8)
		       ,40000U,10,90,100,12,'W',{50,50},{212,23,199,99},52,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Grand master mystic"	  ,(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|MV_INVIS)
			  ,(0x2L),(IM_FROST|IM_FIRE|IM_POISON|IM_LIGHTNING|
			   IM_ACID|MAX_HP|CHARM_SLEEP)
			  ,(HEAL|S_HOUND|S_SPIDER|S_REPTILE),(NONE8)
		      ,15000,5,30,80,13,'p',{40,55},{263,266,264,265},53,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Khamul the Easterling", (HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_1D2|THRO_DR|
			  MV_ATT_NORM)
			,(0x2L|MANA_BOLT|FIRE_BALL|CAUSE_CRIT|HOLD_PERSON|
			 FEAR|BLINDNESS|S_UNDEAD|FROST_BALL)
		       ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP|UNIQUE
			 |HURT_LIGHT|IM_FIRE|IM_POISON|IM_ACID|GOOD
			 |INTELLIGENT|NO_INFRA)
			,(NETHER_BALL|TELE_LEV|RAZOR),(NONE8)
		      ,50000U,10,90,100,12,'W',{70,50},{212,23,199,199},53,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Astral hound"	   ,(MV_ATT_NORM|THRO_DR|MV_INVIS|THRO_WALL)
			   ,(0x5L),(ANIMAL|GROUP|CHARM_SLEEP)
			   ,(BREATH_LD),(NONE8)
		       ,5000,0,30,100,12,'Z',{60,15},{39,39,39,58},54,3
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Great ice wyrm"	   ,(HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x6L|BREATH_FR|FEAR|BLINDNESS
			      |CONFUSION)
			    ,(EVIL|DRAGON|IM_FROST|CHARM_SLEEP|NO_INFRA
			     |MAX_HP|GOOD),(NONE8),(NONE8)
			,20000,80,30,170,12,'D',{50,60},{57,57,271,277},54,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"The Phoenix"          ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_2D2)
                        ,(0x3L|FIRE_BOLT|FIRE_BALL|BREATH_FI)
			,(ANIMAL|CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|IM_POISON|
			 IM_ACID|GOOD|UNIQUE|MAX_HP),(PLASMA_BOLT)
			,(BREATH_LT|BREATH_PL)
 		      ,40000U,0,60,130,12,'B',{36,100},{251,251,220,220},54,3
#ifdef TC_COLOR
  , RED
#endif
},

{"Nightcrawler"		,(MV_ATT_NORM|HAS_2D2|CARRY_OBJ|HAS_1D2|THRO_DR)
			,(0x4L|FEAR|S_UNDEAD|BLINDNESS|MANA_BOLT)
			,(EVIL|UNDEAD|IM_FROST|IM_POISON|IM_FIRE|CHARM_SLEEP|
			 INTELLIGENT|GOOD|NO_INFRA),(BRAIN_SMASH|NETHER_BALL|
			 NETHER_BOLT|BREATH_LD),(NONE8)
		      ,8000,10,20,160,12,'W',{80,60},{254,254,255,255},54,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Hand druj"		,(MV_ONLY_ATT)
			,(0x1L|FEAR|BLINDNESS|CONFUSION|CAUSE_CRIT)
			,(EVIL|NO_INFRA|IM_FROST|CHARM_SLEEP|MAX_HP|UNDEAD
			  |INTELLIGENT|IM_POISON)
			,(DARKNESS|FORGET|TELE_AWAY),(NONE8)
			,12000,10,20,110,13,'s',{30,20},{0,0,0,0},55,4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Eye druj"		,(MV_ONLY_ATT)
			,(0x1L|S_UNDEAD|MANA_BOLT)
			,(EVIL|UNDEAD|CHARM_SLEEP|NO_INFRA|MAX_HP|IM_FROST
			  |IM_POISON|IM_FIRE|INTELLIGENT)
			,(NETHER_BOLT|NETHER_BALL),(NONE8)
			,24000,10,20,90,13,'s',{40,25},{246,246,0,0},55,4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Skull druj"		,(MV_ONLY_ATT)
			,(0x1L|S_UNDEAD|SLOW)
			,(EVIL|UNDEAD|CHARM_SLEEP|NO_INFRA|MAX_HP|IM_FROST
			  |IM_POISON|IM_FIRE|INTELLIGENT)
			,(MIND_BLAST|TRAP_CREATE|NETHER_BOLT|PLASMA_BOLT
			  |BRAIN_SMASH|RAZOR|WATER_BALL),(NONE8)
			,25000,10,20,120,13,'s',{50,27},{247,236,248,249},55,
									4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Chaos vortex"		     ,(MV_ATT_NORM|MV_75),(0x6L)
			    ,(CHARM_SLEEP),(BREATH_CH),(NONE8)
			    ,4000,0,100,80,14,'v',{32,20},{0,0,0,0},55,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Aether vortex"	    ,(MV_ATT_NORM|MV_75),(0x6L|BREATH_FI|BREATH_FR|
			    BREATH_G|BREATH_A|BREATH_L)
			    ,(CHARM_SLEEP|IM_FIRE|IM_FROST|IM_ACID|IM_POISON|
			      IM_LIGHTNING)
			    ,(BREATH_SH|BREATH_SD|BREATH_CH|BREATH_CO
			      |BREATH_LD|BREATH_NE)
			    ,(BREATH_TI|BREATH_WA|BREATH_SL|BREATH_LT
			      |BREATH_DA|BREATH_PL|BREATH_GR)
		       ,4500,0,100,40,13,'v',{32,20},{242,239,240,241},55,2
#ifdef TC_COLOR
  , ANY
#endif
},

{"The Lernean Hydra"	    ,(MV_ATT_NORM|THRO_DR|CARRY_GOLD|HAS_4D2|HAS_2D2|
			     HAS_1D2|THRO_CREAT)
			    ,(0x3L|FEAR|FIRE_BALL|FIRE_BOLT|
			     BREATH_FI|BREATH_G)
			    ,(CHARM_SLEEP|IM_FIRE|IM_POISON|UNIQUE|ANIMAL|
			     INTELLIGENT|MAX_HP)
			    ,(PLASMA_BOLT|ST_CLOUD),(S_REPTILE)
		    ,20000,20,20,140,12,'R',{100,43},{250,250,251,251},55,2
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Thuringwethil"	    ,(MV_ATT_NORM|THRO_DR|HAS_4D2|HAS_2D2|HAS_1D2|
			     CARRY_OBJ|HAS_60|HAS_90)
			    ,(0x3L|CAUSE_CRIT|MANA_DRAIN|FEAR|HOLD_PERSON|
			     BLINDNESS)
			    ,(UNDEAD|EVIL|MAX_HP|CHARM_SLEEP|INTELLIGENT|
			     IM_FROST|HURT_LIGHT|NO_INFRA|UNIQUE|GOOD|
			    IM_POISON),(NETHER_BALL|RAZOR|BRAIN_SMASH),(NONE8)
		      ,23000,10,20,145,13,'V',{100,40},{48,216,216,198},55,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Great hell wyrm"	    ,(HAS_1D2|HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x6L|BREATH_FI|FEAR|BLINDNESS
			      |CONFUSION)
			    ,(EVIL|DRAGON|IM_FIRE|CHARM_SLEEP|MAX_HP|GOOD)
			    ,(NONE8),(NONE8)
			,23000,40,40,170,12,'D',{90,60},{57,57,271,277},55,2
#ifdef TC_COLOR
  , RED
#endif
},

{"Dragonic quylthulg"	     ,(MV_INVIS)
			    ,(0x2L|S_DRAGON|BLINK|TELE),(MAX_HP|ANIMAL|EVIL|
			     CHARM_SLEEP),(NONE8),(NONE8)
			    ,5500,0,20,1,12,'Q',{90,8},{0,0,0,0},55,3
#ifdef TC_COLOR
  , LIGHTGREEN
#endif
},

{"Fundin Bluecloak"	    ,(HAS_4D2|CARRY_OBJ|HAS_1D2|MV_ATT_NORM|THRO_DR)
			    ,(0x4L|CAUSE_CRIT|FEAR|BLINDNESS|CONFUSION)
			    ,(CHARM_SLEEP|MAX_HP|GOOD|IM_POISON|IM_FROST|
			     IM_FIRE|IM_ACID|IM_LIGHTNING|UNIQUE),(FORGET|
			     RAZOR|HEAL|SUMMON|BRAIN_SMASH),(NONE8)
		     ,20000,10,25,195,13,'h',{100,48},{212,218,218,218},56,2
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Uriel, Angel of Fire"	    ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_4D2|HAS_2D2|HAS_1D2)
			    ,(0x2L|BLINDNESS|TELE_TO|BREATH_FI
			      |FIRE_BALL|FIRE_BOLT|MANA_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_ACID|IM_LIGHTNING|
			      GOOD|INTELLIGENT|MAX_HP|UNIQUE)
			    ,(S_ANGEL),(NONE8)
			    ,25000,10,40,160,13,'A',{220,25}
			    ,{220,103,212,212},56,3
#ifdef TC_COLOR
  , WHITE
#endif
},


{"Azriel, Angel of Death"   ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_4D2|HAS_2D2|HAS_1D2)
			    ,(0x2L|BLINDNESS|TELE_TO|MANA_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_ACID|IM_LIGHTNING|
			      GOOD|INTELLIGENT|MAX_HP|UNIQUE)
			    ,(S_ANGEL|BREATH_LD|NETHER_BOLT|NETHER_BALL)
			    ,(NONE8)
			    ,30000,10,40,170,13,'A',{240,25}
			    ,{202,260,212,212},57,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Ancalagon the Black"	    ,(HAS_4D2|CARRY_OBJ|HAS_90|HAS_2D2|THRO_DR|
			      HAS_60|MV_ATT_NORM)
			    ,(0x2L|BREATH_A|FEAR|BLINDNESS|CONFUSION|S_DRAGON)
			    ,(EVIL|DRAGON|IM_FIRE|IM_ACID|
			      UNIQUE|CHARM_SLEEP|MAX_HP|GOOD)
			    ,(NONE8),(S_ANCIENTD)
		      ,30000,70,20,125,12,'D',{110,70},{273,274,275,281},58,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Nightwalker"		    ,(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|THRO_DR)
			    ,(0x4L|FEAR|S_UNDEAD|BLINDNESS|MANA_BOLT)
			    ,(EVIL|UNDEAD|IM_FROST|IM_POISON|IM_LIGHTNING|GOOD
			     |IM_FIRE|CHARM_SLEEP|INTELLIGENT|NO_INFRA)
			    ,(BRAIN_SMASH|NETHER_BALL|NETHER_BOLT),(NONE8)
		      ,15000,10,20,175,13,'W',{50,65},{256,256,257,257},59,4
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Gabriel, the Messenger"   ,(MV_ATT_NORM|THRO_DR|PICK_UP|CARRY_OBJ|
			      HAS_4D2|HAS_2D2|HAS_1D2)
			    ,(0x2L|BLINDNESS|TELE_TO|MANA_BOLT)
			    ,(IM_POISON|IM_FIRE|IM_FROST|IM_ACID|IM_LIGHTNING|
			      GOOD|INTELLIGENT|MAX_HP|UNIQUE)
			    ,(S_ANGEL),(NONE8)
			    ,35000L,10,40,180,13,'A',{140,55}
			    ,{230,103,212,212},59,3
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Saruman of Many Colours"  ,(MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
			      CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
			    ,(0x2L|FEAR|BLINDNESS|S_UNDEAD|S_DEMON|S_DRAGON
			     |CONFUSION|TELE|FIRE_BALL|FROST_BALL)
			    ,(EVIL|CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|IM_FROST
			      |MAX_HP|UNIQUE|GOOD|IM_POISON|INTELLIGENT)
			     ,(RAZOR|WATER_BALL|ACID_BALL|TELE_AWAY|FORGET|
			       ICE_BOLT|MIND_BLAST|TRAP_CREATE|HEAL|HASTE)
			     ,(NONE8)
			     ,35000U,0,100,100,12
			     ,'p',{100,50},{230,230,23,23},60,1
#ifdef TC_COLOR
  , ANY
#endif
},

{"Dreadlord"		  ,(MV_ATT_NORM|MV_20|HAS_4D2|CARRY_OBJ|HAS_2D2|
			   HAS_60|THRO_WALL|PICK_UP|MV_INVIS|HAS_1D2)
			   ,(0x4L|HOLD_PERSON|MANA_DRAIN|BLINDNESS|
			    S_UNDEAD|CONFUSION)
			   ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|MAX_HP|
			    IM_POISON),(NETHER_BALL),(NONE8)
		      ,20000,10,20,150,12,'G',{100,27},{235,235,81,81},62,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"The Cat Lord"           ,(MV_ATT_NORM|MV_INVIS|THRO_DR|CARRY_OBJ|HAS_4D2)
                          ,(0x3L|TELE_TO),(CHARM_SLEEP|MAX_HP|GOOD|UNIQUE|
			   IM_FIRE|IM_FROST|IM_POISON),(NONE8),(NONE8)
		      ,30000,0,100,200,13,'f',{80,60},{269,181,260,265},64,3
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Jabberwock"		    ,(CARRY_OBJ|HAS_90|HAS_60|MV_ATT_NORM)
			    ,(0x5L)
			    ,(ANIMAL|MAX_HP),(BREATH_CH|RAZOR)
			    ,(NONE8)
			   ,19000,255,35,125,13,'J',{80,40},{212,212,212,212},
			     65,4
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Chaos hound"		    ,(MV_ATT_NORM),(0x5L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP),(BREATH_CH)
			    ,(NONE8)
			  ,10000,0,30,100,12,'Z',{60,30},{39,39,39,58},65,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Great wyrm of chaos"	    ,(HAS_1D2|HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x3L|FEAR|BLINDNESS|CONFUSION|
			     S_DRAGON),(EVIL|DRAGON|CHARM_SLEEP|MAX_HP|GOOD)
			    ,(BREATH_CH|BREATH_DI),(NONE8)
		       ,29000,20,40,170,12,'D',{65,70},{273,273,274,280},67,2
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Great wyrm of law"	    ,(HAS_1D2|HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x3L|FEAR|BLINDNESS|CONFUSION|
			     S_DRAGON),(DRAGON|CHARM_SLEEP|MAX_HP|GOOD)
			    ,(BREATH_SH|BREATH_SD),(NONE8)
		      ,29000,255,40,170,12,'D',{70,65},{273,273,274,280},67,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Great wyrm of balance"    ,(HAS_1D2|HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_90|HAS_60|
			      MV_ATT_NORM),(0x3L|FEAR|BLINDNESS|S_DRAGON
			      |CONFUSION)
			    ,(DRAGON|CHARM_SLEEP|MAX_HP|GOOD)
			    ,(BREATH_SH|BREATH_SD|BREATH_CH|BREATH_DI),
			     (S_ANCIENTD)
		      ,31000,255,40,170,12,'D',{70,70},{273,273,274,280},67,4
#ifdef TC_COLOR
  , LIGHTGRAY
#endif
},

{"Tselakus, the Dreadlord" ,(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|HAS_2D2|
			   THRO_WALL|MV_INVIS|HAS_1D2)
			   ,(0x3L|HOLD_PERSON|BLINDNESS|CONFUSION)
			   ,(UNDEAD|EVIL|IM_FROST|NO_INFRA|CHARM_SLEEP|MAX_HP|
			    IM_POISON|GOOD|UNIQUE)
			   ,(NETHER_BALL),(S_GUNDEAD|DARK_STORM|S_WRAITH)
		      ,35000U,10,20,150,13,'G',{100,67},{81,81,212,212},68,2
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Tiamat, Celestial Dragon of Evil",(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|
			      THRO_DR|HAS_60|HAS_90|HAS_2D2|HAS_1D2)
			    ,(0x2L|BREATH_G|BREATH_L|BREATH_A|BREATH_FR|
			      BREATH_FI|FEAR|CONFUSION|BLINDNESS)
			    ,(IM_FROST|IM_ACID|IM_POISON|IM_LIGHTNING|
			      IM_FIRE|EVIL|DRAGON|CHARM_SLEEP|MAX_HP|
			      UNIQUE|SPECIAL),(NONE8),(S_ANCIENTD)
		   ,45000U,70,20,125,13,'D',{100,100},{274,275,275,281},70,4
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Black reaver"		    ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_1D2|HAS_2D2)
			    ,(0x3L|CONFUSION|BLINDNESS|HOLD_PERSON|
			      CAUSE_CRIT|MANA_DRAIN|TELE_TO|S_UNDEAD)
			    ,(UNDEAD|IM_POISON|IM_FROST|EVIL|MAX_HP|GOOD|
			      CHARM_SLEEP|NO_INFRA|INTELLIGENT|BREAK_WALL)
			    ,(BRAIN_SMASH|RAZOR|NETHER_BALL),(MANA_STORM)
		      ,23000,50,20,170,12,'L',{60,60},{230,230,81,81},71,3
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Master quylthulg"	    ,(MV_INVIS)
			,(0x2L|MONSTER|S_UNDEAD|S_DRAGON),(CHARM_SLEEP|
			  ANIMAL|MAX_HP|EVIL),(SUMMON),(S_GUNDEAD|S_ANCIENTD)
		       ,12000,0,20,1,12,'Q',{100,20},{0,0,0,0},71,3
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Greater dragonic quylthulg" ,(MV_INVIS)
			    ,(0x2L|BLINK|TELE_TO),(ANIMAL|EVIL|MAX_HP|
			     CHARM_SLEEP) ,(NONE8),(S_ANCIENTD)
			,10500,0,20,1,12,'Q',{100,14},{0,0,0,0},71,3
#ifdef TC_COLOR
  , MULTI
#endif
},

{"Greater rotting quylthulg",(MV_INVIS)
			    ,(0x2L|BLINK|TELE_TO),(ANIMAL|EVIL|UNDEAD|NO_INFRA|
			     CHARM_SLEEP|MAX_HP|IM_FROST),(NONE8),(S_GUNDEAD)
			,10500,0,20,1,12,'Q',{100,14},{0,0,0,0},71,3
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Vecna, the Emperor Lich"  ,(MV_ATT_NORM|THRO_DR|CARRY_OBJ|HAS_4D2|HAS_2D2)
			    ,(0x2L|FEAR|CONFUSION|BLINDNESS|HOLD_PERSON|
			      CAUSE_CRIT|MANA_BOLT|TELE_TO|BLINK|S_UNDEAD)
			    ,(UNDEAD|IM_POISON|IM_FROST|EVIL|MAX_HP|
			      CHARM_SLEEP|NO_INFRA|UNIQUE|GOOD|INTELLIGENT)
			   ,(NETHER_BALL|TRAP_CREATE|RAZOR|SUMMON|BRAIN_SMASH)
			    ,(MANA_STORM)
		      ,30000,50,20,85,13,'L',{90,50},{181,201,214,181},72,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Omarax the Eye Tyrant"    ,(MV_ATT_NORM)
			    ,(0x2L|FIRE_BOLT|FROST_BOLT|ACID_BOLT|
			      MANA_DRAIN|BLINDNESS|CONFUSION|FEAR|SLOW)
			    ,(ANIMAL|EVIL|CHARM_SLEEP|MAX_HP|
			      IM_POISON|UNIQUE|INTELLIGENT)
			    ,(FORGET|MIND_BLAST|DARKNESS),(DARK_STORM)
		       ,16000,10,30,80,13,'e',{80,80},{223,224,225,226},73,4
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"Ungoliant, the Unlight"   ,(MV_ATT_NORM|CARRY_OBJ|HAS_4D2)
			    ,(0x3L|FEAR|BLINDNESS|CONFUSION|SLOW|BREATH_G)
			    ,(ANIMAL|EVIL|UNIQUE|HURT_LIGHT|MAX_HP|
			      CHARM_SLEEP|GOOD|INTELLIGENT|IM_POISON)
			    ,(HEAL|S_SPIDER|DARKNESS)
			    ,(DARK_STORM|BREATH_DA)
		     ,35000U,80,8,160,12,'S',{130,100},{162,162,167,167},75,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Aether hound"		    ,(MV_ATT_NORM),(0x5L|BREATH_FI|BREATH_FR|BREATH_G|
					    BREATH_A|BREATH_L)
			    ,(ANIMAL|GROUP|CHARM_SLEEP|IM_FIRE|
			      IM_FROST|IM_LIGHTNING|IM_POISON|IM_ACID)
			    ,(BREATH_CH|BREATH_SH|BREATH_SD|BREATH_CO
			      |BREATH_DI|BREATH_LD|BREATH_NE)
			    ,(BREATH_WA|BREATH_GR|BREATH_SL|
			      BREATH_PL|BREATH_TI|BREATH_LT|
			      BREATH_DA)
			,10000,0,30,100,12,'Z',{60,30},{39,39,39,58},75,2
#ifdef TC_COLOR
  , ANY
#endif
},

{"The Mouth of Sauron"	    ,(MV_ATT_NORM|MV_INVIS|THRO_DR|HAS_4D2|HAS_1D2|
			     CARRY_OBJ),(0x2L|TELE_TO|CAUSE_CRIT|FIRE_BALL|
			     HOLD_PERSON),(EVIL|MAX_HP|UNIQUE|GOOD|CHARM_SLEEP
			     |INTELLIGENT|IM_FROST|IM_FIRE|IM_LIGHTNING),
			     (TRAP_CREATE|WATER_BALL|PLASMA_BOLT|NETHER_BALL)
			    ,(MANA_STORM|DARK_STORM)
		    ,38000U,10,60,100,13,'p',{70,100},{230,230,214,214},78,3
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"The Emperor Quylthulg"    ,(MV_INVIS|CARRY_OBJ|HAS_4D2)
			    ,(0x2L),(ANIMAL|EVIL|MAX_HP|UNIQUE|CHARM_SLEEP)
			    ,(BRAIN_SMASH),(S_GUNDEAD|S_ANCIENTD)
		     ,20000,0,30,1,13,'Q',{50,100},{0,0,0,0},78,3
#ifdef TC_COLOR
  , ANY
#endif
},

{"Qlzqqlzuup, the Lord of Flesh", (MV_INVIS|CARRY_OBJ|HAS_4D2)
                            ,(0x1L|S_UNDEAD|S_DEMON|S_DRAGON|MONSTER)
			    ,(ANIMAL|EVIL|UNIQUE|MAX_HP|CHARM_SLEEP)
			    ,(SUMMON|S_ANGEL|S_SPIDER|S_HOUND)
			    ,(S_REPTILE|S_ANT|S_GUNDEAD|S_ANCIENTD|S_UNIQUE|
			      S_WRAITH)
		      ,20000,0,30,1,13,'Q',{50,100},{0,0,0,0},78,3
#ifdef TC_COLOR
  , LIGHTMAGENTA
#endif
},

{"Murazor, the Witch-King of Angmar",
                       (HAS_4D2|HAS_2D2|CARRY_OBJ|HAS_1D2|THRO_DR|MV_ATT_NORM)
                            ,(0x2L|MANA_BOLT|CAUSE_CRIT|HOLD_PERSON|
			      FEAR|BLINDNESS)
			      ,(EVIL|UNDEAD|CHARM_SLEEP|IM_FROST|MAX_HP
				|IM_POISON|HURT_LIGHT|UNIQUE|GOOD|NO_INFRA
				|INTELLIGENT)
			      ,(NETHER_BALL|BRAIN_SMASH|TELE_AWAY|SUMMON)
			      ,(S_WRAITH|S_ANCIENTD|S_GUNDEAD)
                     ,42000U,10,90,120,13,'W',{120,50},{212,23,199,199},80,3
#ifdef TC_COLOR
  , BLUE
#endif
},

{"Pazuzu, Lord of Air"     ,(MV_ATT_NORM|THRO_DR|MV_INVIS|CARRY_OBJ|HAS_4D2)
                           ,(0x3L|MANA_BOLT),(EVIL|DEMON|CHARM_SLEEP|
			    MAX_HP|IM_FROST|IM_FIRE|IM_LIGHTNING|IM_ACID|
			    IM_POISON|GOOD|UNIQUE),(LIGHT_BOLT|LIGHT_BALL|
			    MIND_BLAST),(NONE8)
		      ,30000,10,40,125,14,'B',{100,55},{284,284,284,284},82,2
#ifdef TC_COLOR
  , LIGHTBLUE
#endif
},

{"Hell hound"		    ,(MV_ATT_NORM|MV_20),(0x5L|BREATH_FI),
 			(ANIMAL|EVIL|MAX_HP|IM_FIRE|GROUP),(NONE8),(NONE8)
                       ,600,30,25,80,12,'C',{30,16},{107,107,107,0},83,4
#ifdef TC_COLOR
  , RED
#endif
},

{"Cantoras, the Skeletal Lord",
			     (MV_ATT_NORM|HAS_4D2|CARRY_OBJ|HAS_60|HAS_90|
			      THRO_DR|HAS_2D2|HAS_1D2)
			    ,(0x1L|TELE_TO|MANA_BOLT|FEAR|SLOW)
                            ,(EVIL|UNIQUE|UNDEAD|CHARM_SLEEP|IM_FROST|NO_INFRA
			      |IM_POISON|IM_FIRE|MAX_HP|INTELLIGENT|SPECIAL)
			    ,(WATER_BALL|RAZOR|BRAIN_SMASH|ICE_BOLT|
			      NETHER_BALL),(S_GUNDEAD)
			    ,45000U,80,20,120,14,'s',{150,45},{246,172,172,0},
								    84,2
#ifdef TC_COLOR
  , YELLOW
#endif
},

{"The Tarrasque"	    ,(MV_ATT_NORM|HAS_4D2|HAS_2D2|THRO_DR|CARRY_OBJ)
			    ,(0x2L|BREATH_FI)
			    ,(EVIL|CHARM_SLEEP|IM_FIRE|MAX_HP|UNIQUE|GOOD)
			    ,(BREATH_DI),(NONE8)
		     ,35000U,20,50,185,13,'R',{85,95},{212,212,214,214},84,2
#ifdef TC_COLOR
  , GREEN
#endif
},

{"Lungorthin, the Balrog of White Fire",
			     (MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
                              CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
                            ,(0x4L|FEAR|BLINDNESS|S_DEMON|
                      	      BREATH_FI|CONFUSION)
                            ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|MAX_HP|
			      UNIQUE|GOOD),(NONE8),(S_GUNDEAD)
		     ,37000U,80,20,125,13,'&',{80,95},{104,104,78,214},85,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Draugluin, Sire of all Werewolves",(MV_ATT_NORM|MV_20|PICK_UP|THRO_DR)
			    ,(0x3L|FEAR)
			    ,(ANIMAL|EVIL|UNIQUE|IM_POISON|MAX_HP|GOOD)
			    ,(SUMMON|S_HOUND),(NONE8)
		     ,40000U,90,80,90,13,'C',{100,70},{58,58,160,160},87,2
#ifdef TC_COLOR
  , WHITE
#endif
},

{"Feagwath the Undead Sorceror",(MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
                              CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
                            ,(0x3L|FEAR|BLINDNESS|S_DEMON
			      |TELE|FIRE_BALL|MANA_BOLT)
                            ,(EVIL|CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|IM_FROST
			      |MAX_HP|UNIQUE|SPECIAL|IM_POISON|INTELLIGENT
			      |NO_INFRA|UNDEAD)
			     ,(BRAIN_SMASH|RAZOR|SUMMON),(MANA_STORM|
			      S_GUNDEAD)
			     ,45000U,0,100,100,13
			     ,'L',{120,50},{230,230,23,23},90,3
#ifdef TC_COLOR
  , LIGHTCYAN
#endif
},

{"Carcharoth, the Jaws of Thirst",(MV_ATT_NORM|MV_20|PICK_UP|THRO_DR)
			    ,(0x4L|BREATH_FI|FEAR)
			    ,(ANIMAL|EVIL|UNIQUE|IM_POISON|IM_FIRE|
			      MAX_HP|GOOD|CHARM_SLEEP)
			    ,(HEAL|BRAIN_SMASH|S_HOUND),(NONE8)
		     ,40000U,10,80,110,13,'C',{150,50},{58,58,163,163},92,2
#ifdef TC_COLOR
  , BROWN 
#endif
},

{"Cerberus, Guardian of Hades",(MV_ATT_NORM|HAS_4D2|CARRY_OBJ|THRO_DR)
			    ,(0x3L|BREATH_FI),(ANIMAL|EVIL|UNIQUE|IM_FIRE|
			     MAX_HP|GOOD|CHARM_SLEEP)
			    ,(BREATH_LD|S_HOUND),(DARK_STORM)
		  ,40000U,10,50,160,13,'C',{100,100},{220,220,220,220},94,1
#ifdef TC_COLOR
  , LIGHTRED
#endif
},

{"Gothmog, the High Captain of Balrogs",
			     (MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
                              CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
                            ,(0x3L|FEAR|BLINDNESS|S_DEMON|
                              BREATH_FI|CONFUSION)
                            ,(EVIL|DEMON|CHARM_SLEEP|IM_FIRE|
			      IM_LIGHTNING|MAX_HP|UNIQUE|SPECIAL)
			    ,(NONE8),(S_GUNDEAD)
	             ,43000U,0,100,140,13,'&',{100,80},{220,220,78,214},95,1
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"Sauron, the Sorcerer",     (MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
                              CARRY_OBJ|HAS_60|HAS_90|THRO_DR)
                            ,(0x2L|FEAR|BLINDNESS|S_DEMON
                             |CONFUSION|TELE|MANA_BOLT|FIRE_BALL)
                            ,(EVIL|CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|IM_FROST
			      |MAX_HP|UNIQUE|SPECIAL|IM_POISON|QUESTOR|
			      INTELLIGENT)
			    ,(BRAIN_SMASH|NETHER_BALL|ICE_BOLT|PLASMA_BOLT|
			      WATER_BALL|TELE_LEV|FORGET|DARKNESS|SUMMON|
			      RAZOR),(MANA_STORM|S_ANCIENTD|S_GUNDEAD)
			     ,50000U,0,100,160,13
			     ,'p',{99,105},{270,270,214,214},99,1
#ifdef TC_COLOR
  , RED
#endif
},

{"Morgoth, Lord of Darkness",(MV_ATT_NORM|HAS_4D2|HAS_2D2|HAS_1D2|
                              CARRY_OBJ|HAS_60|HAS_90|THRO_DR|WINNER)
                            ,(0x3L|MANA_BOLT)
                            ,(EVIL|CHARM_SLEEP|IM_FIRE|IM_LIGHTNING|IM_FROST|
			     IM_POISON|IM_ACID|MAX_HP|UNIQUE|SPECIAL|
			     BREAK_WALL|DESTRUCT|INTELLIGENT)
			    ,(SUMMON|BRAIN_SMASH|NETHER_BALL),(S_WRAITH|
			      MANA_STORM|S_ANCIENTD|S_GUNDEAD|S_UNIQUE)
			    ,60000U,0,100,150,14
		            ,'P',{180,95},{262,262,245,214},100,10
#ifdef TC_COLOR
  , DARKGRAY
#endif
},

{"                                                                                                    " /* Players Ghost! */    ,(NONE8)
                            ,(NONE8)
			    ,(EVIL|CHARM_SLEEP|UNDEAD|UNIQUE|GOOD
			     |IM_POISON|IM_FROST|NO_INFRA),(NONE8)
                            ,(NONE8)
			    ,0,0,100,0,11
		            ,'@',{0,0},{0,0,0,0},100,1
#ifdef TC_COLOR
  , WHITE
#endif
}
};

/* ERROR: attack #35 is no longer used */
struct m_attack_type monster_attacks[N_MONS_ATTS] = {
/* 0 */	{0, 0, 0, 0},	{1, 1, 1, 2},	{1, 1, 1, 3},	{1, 1, 1, 4},
	{1, 1, 1, 5},	{1, 1, 1, 6},	{1, 1, 1, 7},	{1, 1, 1, 8},
	{1, 1, 1, 9},	{1, 1, 1, 10},	{1, 1, 1, 12},	{1, 1, 2, 2},
	{1, 1, 2, 3},	{1, 1, 2, 4},	{1, 1, 2, 5},	{1, 1, 2, 6},
	{1, 1, 2, 8},	{1, 1, 3, 4},	{1, 1, 3, 5},	{1, 1, 3, 6},
/* 20 */{1, 1, 3, 8},	{1, 1, 4, 3},	{1, 1, 4, 6},	{1, 1, 5, 5},
	{1, 2, 1, 1},	{1, 2, 1, 2},	{1, 2, 1, 3},	{1, 2, 1, 4},
	{1, 2, 1, 5},	{1, 2, 1, 6},	{1, 2, 1, 7},	{1, 2, 1, 8},
	{1, 2, 1, 10},	{1, 2, 2, 3},	{1, 2, 2, 4},	{1, 2, 2, 5},
	{1, 2, 2, 6},	{1, 2, 2, 8},	{1, 2, 2, 10},	{1, 2, 2, 12},
/* 40 */{1, 2, 2, 14},	{1, 2, 3, 4},	{1, 2, 3, 12},	{1, 2, 4, 4},
	{1, 2, 4, 5},	{1, 2, 4, 6},	{1, 2, 4, 8},	{1, 2, 5, 4},
	{1, 2, 5, 8},	{1, 3, 1, 1},	{1, 3, 1, 2},	{1, 3, 1, 3},
	{1, 3, 1, 4},	{1, 3, 1, 5},	{1, 3, 1, 8},	{1, 3, 1, 9},
	{1, 3, 1, 10},	{1, 3, 1, 12},	{1, 3, 3, 3},	{1, 4, 1, 2},
/* 60 */{1, 4, 1, 3},	{1, 4, 1, 4},	{1, 4, 2, 4},	{1, 5, 1, 2},
	{1, 5, 1, 3},	{1, 5, 1, 4},	{1, 5, 1, 5},	{1, 10, 5, 6},
	{1, 12, 1, 1},	{1, 12, 1, 2},	{1, 13, 1, 1},	{1, 13, 1, 3},
	{1, 14, 0, 0},	{1, 16, 1, 4},	{1, 16, 1, 6},	{1, 16, 1, 8},
	{1, 16, 1, 10},	{1, 16, 2, 8},	{1, 17, 8, 12},	{1, 18, 0, 0},
/* 80 */{2, 1, 3, 4},	{2, 1, 4, 6},	{2, 2, 1, 4},	{2, 2, 2, 4},
	{2, 2, 4, 4},	{2, 4, 1, 4},	{2, 4, 1, 7},	{2, 5, 1, 5},
	{2, 7, 1, 6},	{3, 1, 1, 4},	{3, 5, 1, 8},	{3, 13, 1, 4},
	{3, 7, 0, 0},	{4, 1, 1, 1},	{4, 1, 1, 4},	{4, 2, 1, 2},
	{4, 2, 1, 6},	{4, 5, 0, 0},	{4, 7, 0, 0},	{4, 10, 0, 0},
/*100 */{4, 13, 1, 6},	{5, 1, 2, 6},	{5, 1, 3, 7},	{5, 1, 4, 6},
	{5, 1, 8, 12},	{5, 2, 1, 3},	{5, 2, 3, 6},	{5, 2, 3, 12},
	{5, 5, 4, 4},	{5, 9, 3, 7},	{5, 9, 4, 5},	{5, 12, 1, 6},
	{6, 2, 1, 3},	{6, 2, 2, 8},	{6, 2, 4, 4},	{6, 5, 1, 10},
	{6, 5, 2, 3},	{6, 8, 1, 5},	{6, 9, 2, 6},	{6, 9, 3, 6},
/*120 */{7, 1, 3, 6},	{7, 2, 1, 3},	{7, 2, 1, 6},	{7, 2, 3, 6},
	{7, 2, 3, 10},	{7, 5, 1, 6},	{7, 5, 2, 3},	{7, 5, 2, 6},
	{7, 5, 4, 4},	{7, 12, 1, 4},	{8, 1, 3, 8},	{8, 2, 1, 3},
	{8, 2, 2, 6},	{8, 2, 3, 8},	{8, 2, 5, 5},	{8, 5, 5, 4},
	{9, 5, 1, 2},	{9, 5, 2, 5},	{9, 5, 2, 6},	{9, 8, 2, 4},
/*140 */{9, 12, 1, 3},	{10, 2, 1, 6},	{10, 4, 1, 1},	{10, 7, 2, 6},
	{10, 9, 1, 2},	{11, 1, 1, 2},	{11, 7, 0, 0},	{11, 13, 2, 4},
	{12, 5, 0, 0},	{13, 5, 0, 0},	{13, 19, 0, 0},	{14, 1, 1, 3},
	{14, 1, 3, 4},	{14, 2, 1, 3},	{14, 2, 1, 4},	{14, 2, 1, 5},
	{14, 2, 1, 6},	{14, 2, 1, 10},	{14, 2, 2, 4},	{14, 2, 2, 5},
/*160 */{14, 2, 2, 6},	{14, 2, 3, 4},	{14, 2, 3, 9},	{14, 2, 4, 4},
	{14, 4, 1, 2},	{14, 4, 1, 4},	{14, 4, 1, 8},	{14, 4, 2, 5},
	{14, 5, 1, 2},	{14, 5, 1, 3},	{14, 5, 2, 4},	{14, 5, 2, 6},
	{14, 5, 3, 5},	{14, 12, 1, 2},	{14, 12, 1, 4},	{14, 13, 2, 4},
	{15, 2, 1, 6},	{15, 2, 3, 6},	{15, 5, 1, 8},	{15, 5, 2, 8},
/*180 */{15, 5, 2, 10},	{15, 5, 2, 12},	{15, 12, 1, 3},	{16, 13, 1, 2},
	{17, 3, 1, 10},	{18, 5, 0, 0},	{19, 5, 5, 8},	{19, 5, 12, 8},
	{19, 5, 14, 8},	{19, 5, 15, 8},	{19, 5, 18, 8},	{19, 5, 20, 8},
	{19, 5, 22, 8},	{19, 5, 26, 8},	{19, 5, 30, 8},	{19, 5, 32, 8},
	{19, 5, 34, 8},	{19, 5, 36, 8},	{19, 5, 38, 8},	{19, 5, 42, 8},
/*200 */{19, 5, 44, 8},	{19, 5, 46, 8},	{19, 5, 52, 8},	{20, 10, 0, 0},
	{21, 1, 0, 0},	{21, 5, 0, 0},	{21, 5, 1, 6},	{21, 7, 0, 0},
	{21, 12, 1, 4},	{22, 5, 2, 3},	{22, 12, 0, 0},	{22, 15, 1, 1},
/*212 */{1, 1, 10, 10},	{23, 5, 1, 3},	{24,  5, 0, 0}, {8,   1, 3, 8},
	{3,  1, 6, 6},	{4,  7, 4, 4},	{1,   1, 8, 6},	{1,   5, 2, 5},
	{5,  1, 9,12},

/*221 */{ 4, 7, 2, 4},	{10, 7, 2, 4},	{11, 7, 2, 4},  {19, 7, 20, 8},
	{17, 7, 2, 6},  {24, 7, 2, 6},  /* Beholder */

/*227 */{ 1,20, 4, 6},  { 1,20, 2, 6},  /* Butts */

/*229 */{ 9, 9, 3, 8},  /* Spit */

/*230 */{21, 1, 6, 8},

/*231 */{12, 1, 4, 4},  {13, 1, 4, 5}, /* Master Rogue */

/*233 */{ 1,21, 4, 4},
/*234 */{ 3, 2, 2, 2},
/*235 */{ 1, 1, 6, 6},
/*236 */{19, 2,52, 8}, /* Bite for xp drain */
/*237 */{18, 3, 5, 5}, /* Claw to drain Wisdom */
/*238 */{14, 3, 3, 3}, /* Algroth poisonous claw attack */
/*239 */{5, 22, 3, 3}, /* Fire */ /* vortices */
/*240 */{6, 22, 3, 3}, /* Acid */
/*241 */{7, 22, 3, 3}, /* Cold */
/*242 */{8, 22, 5, 5}, /* Lightning */
/*243 */{5, 22, 8, 8}, /* Plasma/fire */ /* vortices */
/*244 */{1, 22, 5, 5}, /* Hit */ /* vortices */
/*245 */{25, 1,10,12}, /* Morgoths dex drain attack */
/*246 */{19, 7,30,20}, /* Eye druj drain */
/*247 */{11, 2, 4, 4}, /* Skull */
/*248 */{17, 2, 4, 4}, /* druj */
/*249 */{18, 2, 4, 4}, /* attacks */
/*250 */{14, 2, 8, 6}, /* Lernean */
/*251 */{5,  2, 12, 6}, /* Hydra */
/*252 */{18, 3, 1, 10}, /* Another drain wisdom attack */
/*253 */{11, 4, 2, 6}, /* Carrion crawler */
/*254 */{16, 4, 8, 8}, /* Nightcrawler */
/*255 */{9,  2,10,10}, /* Nightcrawler */
/*256 */{21, 1,10,10}, /* Nightwalker */
/*257 */{21, 1, 7, 7}, /* Nightwalker */
/*258 */{12, 5, 5, 5}, /* Harowen  */
/*259 */{13, 5, 5, 5}, /*   the Black Hand */
/*260 */{10, 1,10, 5}, /* Harowen  */
/*261 */{14, 1, 8, 5}, /* Harowen  */
/*262 */{ 1, 1,20,10}, /* Morgoth attacks */
/*263 */{ 1, 6,20, 2}, /* Mystic kick */
/*264 */{14, 1,20, 1}, /* Mystic poison */
/*265 */{11, 1,15, 1}, /* Mystic paralysis */
/*266 */{ 1, 6,10, 2}, /* Mystic kick */
/*267 */{11,11, 6, 6}, /* Medusa paralyse */
/*268 */{ 1,19, 0, 0}, /* Nermal insults */
/*269 */{ 3, 1,12, 12}, /* Greater titan */
/*270 */{21, 1,10, 12}, /* Sauron punch */
/*271 */{ 1, 3, 3, 12}, 
/*272 */{ 1, 3, 4, 12},
/*273 */{ 1, 3, 5, 12},
/*274 */{ 1, 3, 6, 12},
/*275 */{ 1, 3, 8, 12},
/*276 */{ 1, 2, 3, 14}, /* New claws and bites for those wimpy dragons! */
/*277 */{ 1, 2, 4, 14},
/*278 */{ 1, 2, 5, 14},
/*279 */{ 1, 2, 6, 14},
/*280 */{ 1, 2, 8, 14},
/*281 */{ 1, 2, 10,14},
/*282 */{ 1,20, 12,13},
/*283 */{ 1,23,  0, 0},
/*284 */{ 8, 1, 12,12},
};


monster_type m_list[MAX_MALLOC];
int16 m_level[MAX_MONS_LEVEL+1];

/* Blank monster values	*/
monster_type blank_monster = {0,0,0,0,0,0,0,FALSE,0,FALSE};
int16 mfptr;			/* Cur free monster ptr	*/
int16 mon_tot_mult;		/* # of repro's of creature	*/
