/* z-msg.h */

#ifndef INCLUDED_Z_MSG_H
#define INCLUDED_Z_MSG_H

#include "h-basic.h"

/*
 * The "message memorization" package.
 *
 * Each call to "message_add(s)" will add a new "most recent" message
 * to the "message recall list", using the contents of the string "s".
 *
 * The number of memorized messages is available as "message_num()".
 *
 * Old messages can be retrieved by "message_str(age)", where the "age"
 * of the most recently memorized message is zero, and the oldest "age"
 * which is available is "message_num() - 1".  Messages outside this
 * range are returned as the empty string.
 */

errr messages_init(void);
void messages_free(void);
u16b messages_num(void);	

const char *message_str(u16b age);
u16b message_count(u16b age);
u16b message_type(u16b age);
byte message_color(u16b age);

byte message_type_color(u16b type);
errr message_color_define(u16b type, byte color);
void message_add(const char *str, u16b type);




/*** Message constants ***/

#define MSG_GENERIC          0
#define MSG_HIT              1
#define MSG_MISS             2
#define MSG_FLEE             3
#define MSG_DROP             4
#define MSG_KILL             5
#define MSG_LEVEL            6
#define MSG_DEATH            7
#define MSG_STUDY            8
#define MSG_TELEPORT         9
#define MSG_SHOOT           10
#define MSG_QUAFF           11
#define MSG_ZAP_ROD         12
#define MSG_WALK            13
#define MSG_TPOTHER         14
#define MSG_HITWALL         15
#define MSG_EAT             16
#define MSG_STORE1          17
#define MSG_STORE2          18
#define MSG_STORE3          19
#define MSG_STORE4          20
#define MSG_DIG             21
#define MSG_OPENDOOR        22
#define MSG_SHUTDOOR        23
#define MSG_TPLEVEL         24
#define MSG_BELL            25
#define MSG_NOTHING_TO_OPEN 26
#define MSG_LOCKPICK_FAIL   27
#define MSG_STAIRS_DOWN     28 
#define MSG_HITPOINT_WARN   29
#define MSG_ACT_ARTIFACT    30 
#define MSG_USE_STAFF       31 
#define MSG_DESTROY         32 
#define MSG_MON_HIT         33 
#define MSG_MON_TOUCH       34 
#define MSG_MON_PUNCH       35 
#define MSG_MON_KICK        36 
#define MSG_MON_CLAW        37 
#define MSG_MON_BITE        38 
#define MSG_MON_STING       39 
#define MSG_MON_BUTT        40 
#define MSG_MON_CRUSH       41 
#define MSG_MON_ENGULF      42 
#define MSG_MON_CRAWL       43 
#define MSG_MON_DROOL       44 
#define MSG_MON_SPIT        45 
#define MSG_MON_GAZE        46 
#define MSG_MON_WAIL        47 
#define MSG_MON_SPORE       48 
#define MSG_MON_BEG         49 
#define MSG_MON_INSULT      50 
#define MSG_MON_MOAN        51 
#define MSG_RECOVER         52 
#define MSG_BLIND           53 
#define MSG_CONFUSED        54 
#define MSG_POISONED        55 
#define MSG_AFRAID          56 
#define MSG_PARALYZED       57 
#define MSG_DRUGGED         58 
#define MSG_SPEED           59 
#define MSG_SLOW            60 
#define MSG_SHIELD          61 
#define MSG_BLESSED         62 
#define MSG_HERO            63 
#define MSG_BERSERK         64 
#define MSG_PROT_EVIL       65 
#define MSG_INVULN          66 
#define MSG_SEE_INVIS       67 
#define MSG_INFRARED        68 
#define MSG_RES_ACID        69 
#define MSG_RES_ELEC        70 
#define MSG_RES_FIRE        71 
#define MSG_RES_COLD        72 
#define MSG_RES_POIS        73 
#define MSG_STUN            74 
#define MSG_CUT             75 
#define MSG_STAIRS_UP       76 
#define MSG_STORE_ENTER     77 
#define MSG_STORE_LEAVE     78 
#define MSG_STORE_HOME      79 
#define MSG_MONEY1          80 
#define MSG_MONEY2          81 
#define MSG_MONEY3          82 
#define MSG_SHOOT_HIT       83 
#define MSG_STORE5          84 
#define MSG_LOCKPICK        85 
#define MSG_DISARM          86 
#define MSG_IDENT_BAD       87 
#define MSG_IDENT_EGO       88 
#define MSG_IDENT_ART       89 
#define MSG_BR_ELEMENTS     90
#define MSG_BR_FROST        91
#define MSG_BR_ELEC         92
#define MSG_BR_ACID         93
#define MSG_BR_GAS          94
#define MSG_BR_FIRE         95
#define MSG_BR_CONF         96
#define MSG_BR_DISENCHANT   97
#define MSG_BR_CHAOS        98
#define MSG_BR_SHARDS       99
#define MSG_BR_SOUND        100
#define MSG_BR_LIGHT        101
#define MSG_BR_DARK         102
#define MSG_BR_NETHER       103
#define MSG_BR_NEXUS        104
#define MSG_BR_TIME         105
#define MSG_BR_INERTIA      106
#define MSG_BR_GRAVITY      107
#define MSG_BR_PLASMA       108
#define MSG_BR_FORCE        109
#define MSG_SUM_MONSTER     110
#define MSG_SUM_ANGEL       111
#define MSG_SUM_UNDEAD      112
#define MSG_SUM_ANIMAL      113
#define MSG_SUM_SPIDER      114
#define MSG_SUM_HOUND       115
#define MSG_SUM_HYDRA       116
#define MSG_SUM_DEMON       117
#define MSG_SUM_DRAGON      118
#define MSG_SUM_HI_UNDEAD   119
#define MSG_SUM_HI_DRAGON   120
#define MSG_SUM_HI_DEMON    121
#define MSG_SUM_WRAITH      122
#define MSG_SUM_UNIQUE      123
#define MSG_WIELD           124
#define MSG_CURSED          125
#define MSG_PSEUDOID        126
#define MSG_HUNGRY          127
#define MSG_NOTICE          128
#define MSG_AMBIENT_DAY     129
#define MSG_AMBIENT_NITE    130
#define MSG_AMBIENT_DNG1    131
#define MSG_AMBIENT_DNG2    132
#define MSG_AMBIENT_DNG3    133
#define MSG_AMBIENT_DNG4    134
#define MSG_AMBIENT_DNG5    135
#define MSG_CREATE_TRAP     136
#define MSG_SHRIEK          137
#define MSG_CAST_FEAR       138
#define MSG_HIT_GOOD        139
#define MSG_HIT_GREAT       140
#define MSG_HIT_SUPERB      141
#define MSG_HIT_HI_GREAT    142
#define MSG_HIT_HI_SUPERB   143
#define MSG_SPELL           144
#define MSG_PRAYER          145
#define MSG_KILL_UNIQUE     146
#define MSG_KILL_KING       147
#define MSG_DRAIN_STAT      148
#define MSG_MULTIPLY        149

#define MSG_MAX             150

/*
 * Hack -- maximum known sounds
 *
 * Should be the same as MSG_MAX for compatibility reasons.
 */
#define SOUND_MAX MSG_MAX


#endif /* !INCLUDED_Z_MSG_H */
