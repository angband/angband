/**
 * \file list-blow-methods.h
 * \brief monster race blow methods
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 20, so a flag's sequence number is its line number minus 19.
 *
 * Fields:
 * cut - whether this attack can cause cuts
 * stun - whether this attack can stun
 * miss - whether the player is notified when the attack misses
 * phys - whether the attack has physical damage
 * msg - sound/message to display
 * act - action string to append (insult and moan are NULL as they are special)
 * desc - string used in monster recall
 */

/*  symbol	cut		stun	miss	phys	msg				act							desc */
RBM(NONE,	false,	false, 	false, 	false, 	MSG_GENERIC,	"",							"")
RBM(HIT,	true,	true, 	true, 	true, 	MSG_MON_HIT,	"hits you.",				"hit")
RBM(TOUCH,	false,	false, 	true, 	false, 	MSG_MON_TOUCH,	"touches you.",				"touch")
RBM(PUNCH,	false,	true, 	true, 	true, 	MSG_MON_PUNCH,	"punches you.",				"punch")
RBM(KICK,	false,	true, 	true, 	true, 	MSG_MON_KICK,	"kicks you.",				"kick")
RBM(CLAW,	true,	false, 	true, 	true, 	MSG_MON_CLAW,	"claws you.",				"claw")
RBM(BITE,	true,	false, 	true, 	true, 	MSG_MON_BITE,	"bites you.",				"bite")
RBM(STING,	false,	false, 	true, 	true, 	MSG_MON_STING,	"stings you.",				"sting")
RBM(BUTT,	false,	true, 	true, 	true, 	MSG_MON_BUTT,	"butts you.",				"butt")
RBM(CRUSH,	false,	true, 	true, 	true, 	MSG_MON_CRUSH,	"crushes you.",				"crush")
RBM(ENGULF,	false,	false, 	true, 	false, 	MSG_MON_ENGULF,	"engulfs you.",				"engulf")
RBM(CRAWL,	false,	false, 	false, 	false, 	MSG_MON_CRAWL,	"crawls on you.",			"crawl on you")
RBM(DROOL,	false,	false, 	false, 	false, 	MSG_MON_DROOL,	"drools on you.",			"drool on you")
RBM(SPIT,	false,	false, 	false, 	false, 	MSG_MON_SPIT,	"spits on you.",			"spit")
RBM(GAZE,	false,	false, 	false, 	false, 	MSG_MON_GAZE,	"gazes at you.",			"gaze")
RBM(WAIL,	false,	false, 	false, 	false, 	MSG_MON_WAIL,	"wails at you.",			"wail")
RBM(SPORE,	false,	false, 	false, 	false, 	MSG_MON_SPORE,	"releases spores at you.",	"release spores")
RBM(BEG,	false,	false, 	false, 	false, 	MSG_MON_BEG,	"begs you for money.",		"beg")
RBM(INSULT,	false,	false, 	false, 	false, 	MSG_MON_INSULT,	NULL,						"insult")
RBM(MOAN,	false,	false, 	false, 	false, 	MSG_MON_MOAN,	NULL,						"moan")
RBM(MAX,	false, 	false, 	false, 	false, 	MSG_GENERIC,	"",							"")
