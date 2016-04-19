/**
 * \file list-player-timed.h
 * \brief timed player properties
 *
 * Adjusting these flags will break savefiles. Flags below start from 0
 * on line 25, so a flag's sequence number is its line number minus 24.
 *
 * Note that CUT and STUN need special treatment
 *
 * Fields:
 * symbol - the effect name
 * description - the effect description
 * on_begin - the message on beginning the effect
 * on_end - the message on ending the effect
 * on_increase - the message on the effect increasing
 * on_decrease - the message on the effect decreasing
 * flag_redraw - the things to be redrawn when the effect is set
 * flag_update - the things to be updated when the effect is set
 * msg - the message type for this effect
 * code - determines what flag type makes the effect fail: 1 means object flag,
 *        2 means resist, 3 means vulnerabiity
 * fail - the actual flag that causes the failure
 */

/* symbol		description					on_begin									on_end											on_increase									on_decrease									flag_redraw							flag_update										msg				code fail */
TMD(FAST,		"haste",					"You feel yourself moving faster!",			"You feel yourself slow down.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SPEED,		0,	0)
TMD(SLOW,		"slowness",					"You feel yourself moving slower!",			"You feel yourself speed up.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SLOW,		TMD_FAIL_FLAG_OBJECT,	OF_FREE_ACT)
TMD(BLIND,		"blindness",				"You are blind.",							"You blink and your eyes clear.",				NULL,										NULL,										PR_MAP,								PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS,	MSG_BLIND,		TMD_FAIL_FLAG_OBJECT,	OF_PROT_BLIND ) 
TMD(PARALYZED,	"paralysis",				"You are paralysed!",						"You can move again.",							NULL,										NULL,										0,									0,												MSG_PARALYZED,	TMD_FAIL_FLAG_OBJECT,	OF_FREE_ACT)
TMD(CONFUSED,	"confusion",				"You are confused!",						"You are no longer confused.",					"You are more confused!",					"You feel a little less confused.",			0,									PU_BONUS,												MSG_CONFUSED,	TMD_FAIL_FLAG_OBJECT,	OF_PROT_CONF)
TMD(AFRAID,		"fear",						"You are terrified!",						"You feel bolder now.",							"You are more scared!",						"You feel a little less scared.",			0,									PU_BONUS,										MSG_AFRAID,		TMD_FAIL_FLAG_OBJECT,	OF_PROT_FEAR)
TMD(IMAGE,		"hallucination",			"You feel drugged!",						"You can see clearly again.",					"You feel more drugged!",					"You feel less drugged.",					PR_MAP | PR_MONLIST | PR_ITEMLIST,	PU_BONUS,												MSG_DRUGGED,	TMD_FAIL_FLAG_RESIST,	ELEM_CHAOS)
TMD(POISONED,	"poisoning",				"You are poisoned!",						"You are no longer poisoned.",					"You are more poisoned!",					"You are less poisoned.",					0,									PU_BONUS,												MSG_POISONED,	TMD_FAIL_FLAG_RESIST,	ELEM_POIS)
TMD(CUT,		"wounds",					NULL,										NULL,											NULL,										NULL,										0,									0,												0,				0,	0)
TMD(STUN,		"stunning",					NULL,										NULL,											NULL,										NULL,										0,									0,												0,				TMD_FAIL_FLAG_OBJECT,	OF_PROT_STUN)
TMD(PROTEVIL,	"protection from evil",		"You feel safe from evil!",					"You no longer feel safe from evil.",			"You feel even safer from evil!",			"You feel less safe from evil.",			0,									0,												MSG_PROT_EVIL,	0,	0)
TMD(INVULN,		"invulnerability",			"You feel invulnerable!",					"You feel vulnerable once more.",				NULL,										NULL,										0,									PU_BONUS,										MSG_INVULN,		0,	0)
TMD(HERO,		"heroism",					"You feel like a hero!",					"You no longer feel heroic.",					"You feel more like a hero!",				"You feel less heroic.",					0,									PU_BONUS,										MSG_HERO,		0,	0)
TMD(SHERO,		"berserk rage",				"You feel like a killing machine!",			"You no longer feel berserk.",					"You feel even more berserk!",				"You feel less berserk.",					0,									PU_BONUS,										MSG_BERSERK,	0,	0)
TMD(SHIELD,		"mystic shield",			"A mystic shield forms around your body!",	"Your mystic shield crumbles away.",			"The mystic shield strengthens.",			"The mystic shield weakens.",				0,									PU_BONUS,										MSG_SHIELD,		0,	0)
TMD(BLESSED,	"your AC and to-hit bonus",	"You feel righteous!",						"The prayer has expired.",						"You feel more righteous!",					"You feel less righteous.",					0,									PU_BONUS,										MSG_BLESSED,	0,	0)
TMD(SINVIS,		"see invisible",			"Your eyes feel very sensitive!",			"Your eyes no longer feel so sensitive.",		"Your eyes feel more sensitive!",			"Your eyes feel less sensitive.",			0,									(PU_BONUS | PU_MONSTERS),						MSG_SEE_INVIS,	0,	0)
TMD(SINFRA,		"enhanced infravision",		"Your eyes begin to tingle!",				"Your eyes stop tingling.",						"Your eyes' tingling intensifies.",			"Your eyes tingle less.",					0,									(PU_BONUS | PU_MONSTERS),						MSG_INFRARED,	0,	0)
TMD(OPP_ACID,	"acid resistance",			"You feel resistant to acid!",				"You are no longer resistant to acid.",			"You feel more resistant to acid!",			"You feel less resistant to acid.",			PR_STATUS,							PU_BONUS,												MSG_RES_ACID,	TMD_FAIL_FLAG_VULN,	ELEM_ACID)
TMD(OPP_ELEC,	"electricity resistance",	"You feel resistant to electricity!",		"You are no longer resistant to electricity.",	"You feel more resistant to electricity!",	"You feel less resistant to electricity.",	PR_STATUS,							PU_BONUS,												MSG_RES_ELEC,	TMD_FAIL_FLAG_VULN,	ELEM_ELEC)
TMD(OPP_FIRE,	"fire resistance",			"You feel resistant to fire!",				"You are no longer resistant to fire.",			"You feel more resistant to fire!",			"You feel less resistant to fire.",			PR_STATUS,							PU_BONUS,												MSG_RES_FIRE,	TMD_FAIL_FLAG_VULN,	ELEM_FIRE)
TMD(OPP_COLD,	"cold resistance",			"You feel resistant to cold!",				"You are no longer resistant to cold.",			"You feel more resistant to cold!",			"You feel less resistant to cold.",			PR_STATUS,							PU_BONUS,												MSG_RES_COLD,	TMD_FAIL_FLAG_VULN,	ELEM_COLD)
TMD(OPP_POIS,	"poison resistance",		"You feel resistant to poison!",			"You are no longer resistant to poison.",		"You feel more resistant to poison!",		"You feel less resistant to poison.",		0,									PU_BONUS,												MSG_RES_POIS,	0,	0)
TMD(OPP_CONF,	"confusion resistance",		"You feel resistant to confusion!",			"You are no longer resistant to confusion.",	"You feel more resistant to confusion!",	"You feel less resistant to confusion.",	PR_STATUS,							PU_BONUS,										0,				0,	0)
TMD(AMNESIA,	"amnesia",					"You feel your memories fade.",				"Your memories come flooding back.",			NULL,										NULL,										0,									PU_BONUS,												MSG_GENERIC,	0,	0)
TMD(TELEPATHY,	"telepathy",				"Your mind expands.",						"Your horizons are once more limited.",			"Your mind expands further.",				NULL,										0,									PU_BONUS,										MSG_GENERIC,	0,	0)
TMD(STONESKIN,	"stone skin",				"Your skin turns to stone.",				"A fleshy shade returns to your skin.",			NULL,										NULL,										0,									PU_BONUS,										MSG_GENERIC,	0,	0)
TMD(TERROR,		"terror",					"You feel the need to run away, and fast!",	"The urge to run dissipates.",					NULL,										NULL,										0,									PU_BONUS,										MSG_AFRAID,		0,	0)
TMD(SPRINT,		"sprinting",				"You start sprinting.",						"You suddenly stop sprinting.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SPEED,		0,	0)
TMD(BOLD,		"fearlessness",				"You feel bold.",							"You no longer feel bold.",						"You feel even bolder!",					"You feel less bold.",						0,									PU_BONUS,										MSG_BOLD,		0,	0)
TMD(SCRAMBLE,   "scrambled",                "Your body starts to scramble...",          "Your body reasserts its true nature.",         "You are more scrambled!",                  "You are less scrambled.",                  PU_BONUS,                           PU_BONUS,                                       MSG_SCRAMBLE,   2,  ELEM_NEXUS)
