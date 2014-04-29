/* list-player-timed.h - timed player properties
 *
 * Adjusting these flags will break savefiles. Flags below start from 0
 * on line 10, so a flag's sequence number is its line number minus 10.
 *
 * Note that CUT and STUN need special treatment
 */

/* symbol		on_begin									on_end											on_increase									on_decrease									flag_redraw							flag_update										msg				code fail */
TMD(FAST,		"You feel yourself moving faster!",			"You feel yourself slow down.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SPEED,		0,	0)
TMD(SLOW,		"You feel yourself moving slower!",			"You feel yourself speed up.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SLOW,		1,	OF_FREE_ACT)
TMD(BLIND,		"You are blind.",							"You blink and your eyes clear.",				NULL,										NULL,										PR_MAP,								PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS,	MSG_BLIND,		1,	OF_PROT_BLIND ) 
TMD(PARALYZED,	"You are paralysed!",						"You can move again.",							NULL,										NULL,										0,									0,												MSG_PARALYZED,	1,	OF_FREE_ACT)
TMD(CONFUSED,	"You are confused!",						"You are no longer confused.",					"You are more confused!",					"You feel a little less confused.",			0,									0,												MSG_CONFUSED,	1,	OF_PROT_CONF)
TMD(AFRAID,		"You are terrified!",						"You feel bolder now.",							"You are more scared!",						"You feel a little less scared.",			0,									PU_BONUS,										MSG_AFRAID,		1,	OF_PROT_FEAR)
TMD(IMAGE,		"You feel drugged!",						"You can see clearly again.",					"You feel more drugged!",					"You feel less drugged.",					PR_MAP | PR_MONLIST | PR_ITEMLIST,	0,												MSG_DRUGGED,	2,	ELEM_CHAOS)
TMD(POISONED,	"You are poisoned!",						"You are no longer poisoned.",					"You are more poisoned!",					"You are less poisoned.",					0,									0,												MSG_POISONED,	2,	ELEM_POIS)
TMD(CUT,		NULL,										NULL,											NULL,										NULL,										0,									0,												0,				0,	0)
TMD(STUN,		NULL,										NULL,											NULL,										NULL,										0,									0,												0,				1,	OF_PROT_STUN)
TMD(PROTEVIL,	"You feel safe from evil!",					"You no longer feel safe from evil.",			"You feel even safer from evil!",			"You feel less safe from evil.",			0,									0,												MSG_PROT_EVIL,	0,	0)
TMD(INVULN,		"You feel invulnerable!",					"You feel vulnerable once more.",				NULL,										NULL,										0,									PU_BONUS,										MSG_INVULN,		0,	0)
TMD(HERO,		"You feel like a hero!",					"You no longer feel heroic.",					"You feel more like a hero!",				"You feel less heroic.",					0,									PU_BONUS,										MSG_HERO,		0,	0)
TMD(SHERO,		"You feel like a killing machine!",			"You no longer feel berserk.",					"You feel even more berserk!",				"You feel less berserk.",					0,									PU_BONUS,										MSG_BERSERK,	0,	0)
TMD(SHIELD,		"A mystic shield forms around your body!",	"Your mystic shield crumbles away.",			"The mystic shield strengthens.",			"The mystic shield weakens.",				0,									PU_BONUS,										MSG_SHIELD,		0,	0)
TMD(BLESSED,	"You feel righteous!",						"The prayer has expired.",						"You feel more righteous!",					"You feel less righteous.",					0,									PU_BONUS,										MSG_BLESSED,	0,	0)
TMD(SINVIS,		"Your eyes feel very sensitive!",			"Your eyes no longer feel so sensitive.",		"Your eyes feel more sensitive!",			"Your eyes feel less sensitive.",			0,									(PU_BONUS | PU_MONSTERS),						MSG_SEE_INVIS,	0,	0)
TMD(SINFRA,		"Your eyes begin to tingle!",				"Your eyes stop tingling.",						"Your eyes' tingling intensifies.",			"Your eyes tingle less.",					0,									(PU_BONUS | PU_MONSTERS),						MSG_INFRARED,	0,	0)
TMD(OPP_ACID,	"You feel resistant to acid!",				"You are no longer resistant to acid.",			"You feel more resistant to acid!",			"You feel less resistant to acid.",			PR_STATUS,							0,												MSG_RES_ACID,	3,	ELEM_ACID)
TMD(OPP_ELEC,	"You feel resistant to electricity!",		"You are no longer resistant to electricity.",	"You feel more resistant to electricity!",	"You feel less resistant to electricity.",	PR_STATUS,							0,												MSG_RES_ELEC,	3,	ELEM_ELEC)
TMD(OPP_FIRE,	"You feel resistant to fire!",				"You are no longer resistant to fire.",			"You feel more resistant to fire!",			"You feel less resistant to fire.",			PR_STATUS,							0,												MSG_RES_FIRE,	3,	ELEM_FIRE)
TMD(OPP_COLD,	"You feel resistant to cold!",				"You are no longer resistant to cold.",			"You feel more resistant to cold!",			"You feel less resistant to cold.",			PR_STATUS,							0,												MSG_RES_COLD,	3,	ELEM_COLD)
TMD(OPP_POIS,	"You feel resistant to poison!",			"You are no longer resistant to poison.",		"You feel more resistant to poison!",		"You feel less resistant to poison.",		0,									0,												MSG_RES_POIS,	0,	0)
TMD(OPP_CONF,	"You feel resistant to confusion!",			"You are no longer resistant to confusion.",	"You feel more resistant to confusion!",	"You feel less resistant to confusion.",	PR_STATUS,							PU_BONUS,										0,				0,	0)
TMD(AMNESIA,	"You feel your memories fade.",				"Your memories come flooding back.",			NULL,										NULL,										0,									0,												MSG_GENERIC,	0,	0)
TMD(TELEPATHY,	"Your mind expands.",						"Your horizons are once more limited.",			"Your mind expands further.",				NULL,										0,									PU_BONUS,										MSG_GENERIC,	0,	0)
TMD(STONESKIN,	"Your skin turns to stone.",				"A fleshy shade returns to your skin.",			NULL,										NULL,										0,									PU_BONUS,										MSG_GENERIC,	0,	0)
TMD(TERROR,		"You feel the need to run away, and fast!",	"The urge to run dissipates.",					NULL,										NULL,										0,									PU_BONUS,										MSG_AFRAID,		0,	0)
TMD(SPRINT,		"You start sprinting.",						"You suddenly stop sprinting.",					NULL,										NULL,										0,									PU_BONUS,										MSG_SPEED,		0,	0)
TMD(BOLD,		"You feel bold.",							"You no longer feel bold.",						"You feel even bolder!",					"You feel less bold.",						0,									PU_BONUS,										MSG_BOLD,		0,	0)
