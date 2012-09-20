 /* constants.h: global constants used by Angband

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* Note to the Wizard:
 *
 *       Tweaking these constants can *GREATLY* change the game.
 *       Two years of constant tuning have generated these      
 *       values.  Minor adjustments are encouraged, but you must
 *       be very careful not to unbalance the game.  Moria was  
 *       meant to be challenging, not a give away.  Many        
 *       adjustments can cause the game to act strangely, or even
 *       cause errors.
 */

/* Addendum:
 * 
 * I have greatly expanded the number of defined constants.  However, if
 * you change anything below, without understanding EXACTLY how the game
 * uses the number, the program may stop working correctly.  Modify the
 * constants at your own risk.
 */

/* Current version number of Angband: 2.6.1
 *
 * Note that 5.2 must never be used, as it was used in Angband 2.4-2.5.
 */

#define CUR_VERSION_MAJ 2
#define CUR_VERSION_MIN 6
#define PATCH_LEVEL 1

/* Basics */

#ifndef FUZZY
#define FUZZY 2
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_UCHAR       255
#define MAX_SHORT       32767           /* maximum short/long signed ints */
#define MAX_LONG        0xFFFFFFFFL

#ifndef MAXHOSTNAMELEN					/* may not be defined -b. eck */
#define MAXHOSTNAMELEN  64
#endif

/* Changing values below this line may be hazardous to your health! */

/* message line location */
#define MSG_LINE  0

/* number of messages to save in a buffer */
#define MAX_SAVE_MSG   22               /* How many messages to save -CJS- */
#define MAX_SAVE_HISCORES 500
/* How many hiscores to be saved */

/* Dungeon size parameters                                      */
#define MAX_HEIGHT  66                  /* Multiple of 11; >= 22 */
#define MAX_WIDTH  198                  /* Multiple of 33; >= 66 */
#define SCREEN_HEIGHT  22
#define SCREEN_WIDTH   66
#define QUART_HEIGHT (SCREEN_HEIGHT / 4)
#define QUART_WIDTH  (SCREEN_WIDTH / 4)

/* Dungeon generation values
 *
 * Note: The entire design of dungeon can be changed by only
 * slight adjustments here.
 */

#define DUN_TUN_RND       9     /* was 9 1/Chance of Random direction           */
#define DUN_TUN_CHG      70     /* was 70 Chance of changing direction (99 max) */
#define DUN_TUN_CON      15     /* was 15 Chance of extra tunneling             */
#define DUN_ROO_MEA      45     /* was 32 Mean of # of rooms, standard dev2     */
#define DUN_TUN_PEN      25     /* was 25 % chance of room doors                */
#define DUN_TUN_JCT       8     /* was 15 % chance of doors at tunnel junctions */
#define DUN_STR_DEN       5     /* was 5 Density of streamers                   */
#define DUN_STR_RNG       2     /* was 2 Width of streamers                     */
#define DUN_STR_MAG       3     /* was 3 Number of magma streamers              */
#define DUN_STR_MC       90     /* was 90 1/x chance of treasure per magma      */
#define DUN_STR_QUA       2     /* was 2 Number of quartz streamers             */
#define DUN_STR_QC       40     /* was 40 1/x chance of treasure per quartz     */
#define DUN_UNUSUAL      200    /* was 300 Level/x chance of unusual room       */

/* Special level constants - DGK */

#define DUN_DEST         15     /* 1/x chance of having a destroyed level */
#define SPEC_DEST        2


/* Store constants                                              */
#define MAX_OWNERS       24     /* Number of owners to choose from       */
#define MAX_STORES        8     /* Number of different stores            */
#define STORE_INVEN_MAX  24     /* Max number of discrete objs in inven  */
#define STORE_CHOICES    30     /* NUMBER of items to choose stock from  */
#define STORE_MAX_INVEN  18     /* Max diff objs in stock for auto buy   */
#define STORE_MIN_INVEN  10     /* Min diff objs in stock for auto sell  */
#define STORE_TURN_AROUND 9     /* Amount of buying and selling normally */
#define COST_ADJ         100    /* Adjust prices for buying and selling  */

#define MAX_QUESTS        4     /* only 1 defined anyway --CFT */
#define DEFINED_QUESTS    1 
#define SAURON_QUEST      0 
#define Q_PLANE          -1

/* Treasure constants                                           */
#define INVEN_ARRAY_SIZE 34     /* Size of inventory array(Do not change) */
#define MAX_OBJ_LEVEL   255     /* Maximum level of magic in dungeon      */
#define OBJ_GREAT        11     /* 1/n Chance of item being a Great Item  */

/* Number of dungeon objects */
#define MAX_DUNGEON_OBJ  423

/* Note that the following constants are all related, if you change one,
 * you must also change all succeeding ones.  Also, player_init[] and
 * store_choice[] may also have to be changed.
 */

#define OBJ_OPEN_DOOR           (MAX_DUNGEON_OBJ+23)
#define OBJ_CLOSED_DOOR         (MAX_DUNGEON_OBJ+24)
#define OBJ_SECRET_DOOR         (MAX_DUNGEON_OBJ+25)
#define OBJ_UP_STAIR            (MAX_DUNGEON_OBJ+26)
#define OBJ_DOWN_STAIR          (MAX_DUNGEON_OBJ+27)
#define OBJ_STORE_DOOR          (MAX_DUNGEON_OBJ+28)
#define OBJ_TRAP_LIST           (MAX_DUNGEON_OBJ+36)
#define OBJ_RUBBLE              (MAX_DUNGEON_OBJ+54)
#define OBJ_MUSH                (MAX_DUNGEON_OBJ+55)
#define OBJ_SCARE_MON           (MAX_DUNGEON_OBJ+56)
#define OBJ_GOLD_LIST           (MAX_DUNGEON_OBJ+57)
#define OBJ_NOTHING             (MAX_DUNGEON_OBJ+75)
#define OBJ_RUINED_CHEST        (MAX_DUNGEON_OBJ+76)
#define OBJ_WIZARD              (MAX_DUNGEON_OBJ+77)
/*Special start for rings amulets etc... */
#define SPECIAL_OBJ         	(MAX_DUNGEON_OBJ+79)
  /* Number of objects for universe*/
#define MAX_OBJECTS				(MAX_DUNGEON_OBJ+90)


/* was 7*64, see object_offset() in desc.c, could be MAX_OBJECTS o_o() rewritten
 * now 8*64 beacuse of Rods
 */

#define OBJECT_IDENT_SIZE 1024

#define MAX_GOLD       18       /* Number of different types of gold     */


/* with MAX_TALLOC 150, it is possible to get compacting objects during
 * level generation, although it is extremely rare
 */

#define MAX_TALLOC      400     /* Max objects per level               */
#define MIN_TRIX          1     /* Minimum t_list index used           */
#define TREAS_ROOM_ALLOC  9     /* Amount of objects for rooms         */
#define TREAS_ANY_ALLOC   3     /* Amount of objects for corridors     */
#define TREAS_GOLD_ALLOC  3     /* Amount of gold (and gems)           */


/* Magic Treasure Generation constants                  
 * Note: Number of special objects, and degree of enchantments
 *       can be adjusted here.
 */

#define OBJ_STD_ADJ     125     /* Adjust STD per level * 100        */
#define OBJ_STD_MIN       7     /* Minimum STD                       */
#define OBJ_TOWN_LEVEL    5     /* Town object generation level      */
#define OBJ_BASE_MAGIC   15     /* Base amount of magic              */
#define OBJ_BASE_MAX     70     /* Max amount of magic               */
#define OBJ_DIV_SPECIAL   4     /* magic_chance/# special magic      */
#define OBJ_DIV_CURSED   13     /* 10*magic_chance/# cursed items    */

/* Constants describing limits of certain objects */
#define OBJ_LAMP_MAX    15000   /* Maximum amount that lamp can be filled  */
#define OBJ_BOLT_RANGE     18   /* Maximum range of bolts and balls        */
#define OBJ_RUNE_PROT     550   /* Rune of protection resistance           */

/* Creature constants*/
#define MAX_CREATURES     549   /* Number of creatures defined for univ  */
#define N_MONS_ATTS       285   /* Number of monster attack types.       */

/* with MAX_MALLOC 101, it is possible to get compacting monsters messages
 * while breeding/cloning monsters
 */

#define MAX_MALLOC        600   /* Max that can be allocated                */
#define MAX_MALLOC_CHANCE 160   /* 1/x chance of new monster each round     */
#define MAX_MONS_LEVEL     99   /* Maximum level of creatures               */
#define MAX_SIGHT          20   /* Maximum dis a creature can be seen       */
#define MAX_SPELL_DIS      18   /* Maximum dis creat. spell can be cast     */
#define MAX_MON_MULT       75   /* Maximum reproductions on a level         */
#define MON_MULT_ADJ        8   /* High value slows multiplication          */
#define MON_NASTY          50   /* 1/x chance of high level creat           */
#define MIN_MALLOC_LEVEL   14   /* Minimum number of monsters/level         */
#define MIN_MALLOC_TD       4   /* Number of people on town level (day)     */
#define MIN_MALLOC_TN       8   /* Number of people on town level (night)   */
#define WIN_MON_TOT         2   /* Total number of "win" creatures          */
#define WIN_MON_APPEAR    100   /* Level where winning creatures begin      */
#define MON_SUMMON_ADJ      2   /* Adjust level of summoned creatures       */
#define MON_DRAIN_LIFE      2   /* Percent of player exp drained per hit    */
#define MAX_MON_NATTACK     4   /* Max num attacks (used in mons memory)    */
#define MIN_MONIX           2   /* Minimum index in m_list (1=py, 0=no mon) */

/* Trap constants                                               */
#define MAX_TRAP           18   /* Number of defined traps      */

/* Descriptive constants                                        */
#define MAX_COLORS     57       /* Used with potions      */
#define MAX_MUSH       21       /* Used with mushrooms    */
#define MAX_WOODS      32       /* Used with staffs       */
#define MAX_METALS     32       /* Used with wands & rods */
#define MAX_ROCKS      42       /* Used with rings        */
#define MAX_AMULETS    16       /* Used with amulets      */
#define MAX_TITLES     45       /* Used with scrolls      */
#define MAX_SYLLABLES  158      /* Used with scrolls      */

/* Player constants                                             */
#define MAX_PLAYER_LEVEL    50  /* Maximum possible character level        */
#define MAX_EXP      99999999L  /* Maximum amount of experience -CJS-      */
#define MAX_RACES           10  /* Number of defined races                 */
#define MAX_CLASS            6  /* Number of defined classes               */
#define USE_DEVICE           3  /* x> Harder devices x< Easier devices     */
#define MAX_BACKGROUND     128  /* Number of types of histories for univ   */
#define PLAYER_FOOD_FULL 10000  /* Getting full                            */
#define PLAYER_FOOD_MAX  15000  /* Maximum food value, beyond is wasted    */
#define PLAYER_FOOD_FAINT  300  /* Character begins fainting               */
#define PLAYER_FOOD_WEAK  1000  /* Warn player that he is getting very low */
#define PLAYER_FOOD_ALERT 2000  /* Warn player that he is getting low      */
#define PLAYER_REGEN_FAINT  33  /* Regen factor*2^16 when fainting         */
#define PLAYER_REGEN_WEAK   98  /* Regen factor*2^16 when weak             */
#define PLAYER_REGEN_NORMAL 197 /* Regen factor*2^16 when full             */
#define PLAYER_REGEN_HPBASE 1442 /* Min amount hp regen*2^16               */
#define PLAYER_REGEN_MNBASE 524 /* Min amount mana regen*2^16              */
#define PLAYER_WEIGHT_CAP  130  /* "#"*(1/10 pounds) per strength point    */
#define PLAYER_EXIT_PAUSE    1  /* Pause time before player can re-roll    */
								
/* class level adjustment constants */
#define CLA_BTH         0
#define CLA_BTHB        1
#define CLA_DEVICE      2
#define CLA_DISARM      3
#define CLA_SAVE        4

/* this depends on the fact that CLA_SAVE values are all the same, if not,
 * then should add a separate column for this
 */
#define CLA_MISC_HIT    4
#define MAX_LEV_ADJ     5

/* Base to hit constants                                        */
#define BTH_PLUS_ADJ    3       /* Adjust BTH per plus-to-hit   */

/* magic numbers for players inventory array */
#define INVEN_WIELD		22		/* must be first item in equipment list */
#define INVEN_HEAD      23
#define INVEN_NECK      24
#define INVEN_BODY      25
#define INVEN_ARM       26
#define INVEN_HANDS     27
#define INVEN_RIGHT     28
#define INVEN_LEFT      29
#define INVEN_FEET      30
#define INVEN_OUTER     31
#define INVEN_LIGHT     32
#define INVEN_AUX       33

/* Attribute indexes -CJS- */

#define A_STR	0
#define A_INT	1
#define A_WIS	2
#define A_DEX	3
#define A_CON	4
#define A_CHR	5

/* some systems have a non-ANSI definition of this, so undef it first */
#undef CTRL
#define CTRL(c) ((c)&037)
#define DELETE          0x7f
#define ESCAPE        '\033'    /* ESCAPE character -CJS- */

#ifndef NULL

#ifdef __STDC__
#define NULL ((void *)0)
#else
#define NULL (char *)0
#endif /* __STDC__ */

#endif /* NULL */


/* Fval definitions: these describe the various types of dungeon floors and
 * walls, if numbers above 15 are ever used, then the test against
 * MIN_CAVE_WALL will have to be changed, also the save routines will have
 * to be changed.
 */

#define NULL_WALL       0
#define DARK_FLOOR      1
#define LIGHT_FLOOR     2
#define NT_DARK_FLOOR   3
#define NT_LIGHT_FLOOR  4
#define MAX_CAVE_ROOM   4
#define CORR_FLOOR      5
#define BLOCKED_FLOOR   6       /* a corridor space with cl/st/se door or rubble */
#define MAX_CAVE_FLOOR  6

#define MAX_OPEN_SPACE  5
#define MIN_CLOSED_SPACE 6

#define TMP1_WALL       8
#define TMP2_WALL       9

#define MIN_CAVE_WALL   12
#define GRANITE_WALL    12
#define MAGMA_WALL      13
#define QUARTZ_WALL     14
#define BOUNDARY_WALL   15

/* Column for stats    */
#define STAT_COLUMN     0

/* Class spell types */
#define NONE            0
#define MAGE            1
#define PRIEST          2

/* offsets to spell names in spell_names[] array */
#define SPELL_OFFSET    0
#define PRAYER_OFFSET   63

/* definitions for the psuedo-normal distribution generation */
#define NORMAL_TABLE_SIZE       256
#define NORMAL_TABLE_SD         64  /* the standard deviation for the table */

/* definitions for the player's status field */
#define PY_HUNGRY       0x00000001L
#define PY_WEAK         0x00000002L
#define PY_BLIND        0x00000004L
#define PY_CONFUSED     0x00000008L
#define PY_FEAR         0x00000010L
#define PY_POISONED     0x00000020L
#define PY_FAST         0x00000040L
#define PY_SLOW         0x00000080L
#define PY_SEARCH       0x00000100L
#define PY_REST         0x00000200L
#define PY_STUDY        0x00000400L

#define PY_INVULN       0x00001000L
#define PY_HERO         0x00002000L
#define PY_SHERO        0x00004000L
#define PY_BLESSED      0x00008000L
#define PY_DET_INV      0x00010000L
#define PY_TIM_INFRA    0x00020000L
#define PY_SPEED        0x00040000L
#define PY_STR_WGT      0x00080000L
#define PY_PARALYSED    0x00100000L
#define PY_REPEAT       0x00200000L
#define PY_ARMOR        0x00400000L

#define PY_STATS        0x3F000000L
#define PY_STR          0x01000000L /* these 6 stat flags must be adjacent */
#define PY_INT          0x02000000L
#define PY_WIS          0x04000000L
#define PY_DEX          0x08000000L
#define PY_CON          0x10000000L
#define PY_CHR          0x20000000L

#define PY_HP           0x40000000L
#define PY_MANA         0x80000000L

/* definitions for objects that can be worn */
#define TR_STATS        0x0000003FL /* the stats must be the low 6 bits */
#define TR_STR          0x00000001L
#define TR_INT          0x00000002L
#define TR_WIS          0x00000004L
#define TR_DEX          0x00000008L
#define TR_CON          0x00000010L
#define TR_CHR          0x00000020L
#define TR_SEARCH       0x00000040L
#define TR_SLOW_DIGEST  0x00000080L
#define TR_STEALTH      0x00000100L
#define TR_AGGRAVATE    0x00000200L
#define TR_TELEPORT     0x00000400L
#define TR_REGEN        0x00000800L
#define TR_SPEED        0x00001000L

#define TR_EGO_WEAPON   0x0007E000L
#define TR_SLAY_DRAGON  0x00002000L
#define TR_SLAY_ANIMAL  0x00004000L
#define TR_SLAY_EVIL    0x00008000L
#define TR_SLAY_UNDEAD  0x00010000L
#define TR_FROST_BRAND  0x00020000L
#define TR_FLAME_TONGUE 0x00040000L

#define TR_RES_FIRE     0x00080000L
#define TR_RES_ACID     0x00100000L
#define TR_RES_COLD     0x00200000L
#define TR_SUST_STAT    0x00400000L
#define TR_FREE_ACT     0x00800000L
#define TR_SEE_INVIS    0x01000000L
#define TR_RES_LIGHT    0x02000000L
#define TR_FFALL        0x04000000L
#define TR_SLAY_X_DRAGON        0x08000000L
#define TR_POISON       0x10000000L
#define TR_TUNNEL       0x20000000L
#define TR_INFRA        0x40000000L
#define TR_CURSED       0x80000000L

/* flags for flags2 */
#define TR_SLAY_DEMON   0x00000001L
#define TR_SLAY_TROLL   0x00000002L
#define TR_SLAY_GIANT   0x00000004L
#define TR_HOLD_LIFE    0x00000008L
#define TR_SLAY_ORC     0x00000010L
#define TR_TELEPATHY    0x00000020L
#define TR_IM_FIRE      0x00000040L
#define TR_IM_COLD      0x00000080L
#define TR_IM_ACID      0x00000100L
#define TR_IM_LIGHT     0x00000200L
#define TR_LIGHT        0x00000400L
#define TR_ACTIVATE     0x00000800L
#define TR_LIGHTNING    0x00001000L
#define TR_IMPACT       0x00002000L
#define TR_IM_POISON    0x00004000L
#define TR_RES_CONF     0x00008000L
#define TR_RES_SOUND    0x00010000L
#define TR_RES_LT       0x00020000L
#define TR_RES_DARK     0x00040000L
#define TR_RES_CHAOS    0x00080000L
#define TR_RES_DISENCHANT       0x00100000L
#define TR_RES_SHARDS   0x00200000L
#define TR_RES_NEXUS    0x00400000L
#define TR_RES_BLIND    0x00800000L
#define TR_RES_NETHER   0x01000000L
#define TR_ARTIFACT     0x02000000L /* means "is an artifact" -CFT */
#define TR_BLESS_BLADE  0x04000000L /* priests use w/o penalty -DGK*/
#define TR_ATTACK_SPD   0x08000000L /* extra attacks/round -DGK */
#define TR_RES_FEAR     0x10000000L

/* definitions for chests */
#define CH_LOCKED       0x00000001L
#define CH_TRAPPED      0x000001F0L
#define CH_LOSE_STR     0x00000010L
#define CH_POISON       0x00000020L
#define CH_PARALYSED    0x00000040L
#define CH_EXPLODE      0x00000080L
#define CH_SUMMON       0x00000100L

/* definitions for creatures, cmove field */
#define CM_ALL_MV_FLAGS 0x0000001FL
#define CM_ATTACK_ONLY  0x00000001L
#define CM_MOVE_NORMAL  0x00000002L

#define CM_RANDOM_MOVE  0x0000001CL
#define CM_20_RANDOM    0x00000004L
#define CM_40_RANDOM    0x00000008L
#define CM_75_RANDOM    0x00000010L

#define CM_SPECIAL      0x003F0000L
#define CM_INVISIBLE    0x00010000L
#define CM_OPEN_DOOR    0x00020000L
#define CM_PHASE        0x00040000L
#define CM_EATS_OTHER   0x00080000L
#define CM_PICKS_UP     0x00100000L
#define CM_MULTIPLY     0x00200000L

#define CM_CARRY_OBJ    0x01000000L
#define CM_CARRY_GOLD   0x02000000L
#define CM_TREASURE     0x7C000000L
#define CM_TR_SHIFT     26              /* used for recall of treasure */
#define CM_60_RANDOM    0x04000000L
#define CM_90_RANDOM    0x08000000L
#define CM_1D2_OBJ      0x10000000L
#define CM_2D2_OBJ      0x20000000L
#define CM_4D2_OBJ      0x40000000L
#define CM_WIN          0x80000000L

/* creature spell definitions */
#define CS_FREQ         0x0000000FL
#define CS_SPELLS       0xFF07FFF0L
#define CS_TEL_SHORT    0x00000010L
#define CS_TEL_LONG     0x00000020L
#define CS_TEL_TO       0x00000040L
#define CS_LGHT_WND     0x00000080L
#define CS_SER_WND      0x00000100L
#define CS_HOLD_PER     0x00000200L
#define CS_BLIND        0x00000400L
#define CS_CONFUSE      0x00000800L
#define CS_FEAR         0x00001000L
#define CS_SUMMON_MON   0x00002000L
#define CS_SUMMON_UND   0x00004000L
#define CS_SLOW_PER     0x00008000L
#define CS_DRAIN_MANA   0x00010000L

#define CS_INT1         0x0006FC30L     /* was 0x80060020L -DGK */
#define CS_INT2         0x71027200L     /* was 0x51023400L -DGK */
#define CS_INT3         0x0000F900L     /* was 0x00000000L -DGK */
#define CS_BREATHE      0x00F80000L
#define CS_BREATHE2     0x8000003FL
#define CS_BREATHE3     0x0000007FL
#define CS_BR_LIGHT     0x00080000L
#define CS_BR_GAS       0x00100000L
#define CS_BR_ACID      0x00200000L
#define CS_BR_FROST     0x00400000L
#define CS_BR_FIRE      0x00800000L

/* creature defense flags */
#define CD_DRAGON       0x0001
#define CD_ANIMAL       0x0002
#define CD_EVIL         0x0004
#define CD_UNDEAD       0x0008
#define CD_WEAKNESS     0x03F0
#define CD_FROST        0x0010
#define CD_FIRE         0x0020
#define CD_POISON       0x0040
#define CD_ACID         0x0080
#define CD_LIGHT        0x0100
#define CD_STONE        0x0200

#define CD_NO_SLEEP     0x1000
#define CD_INFRA        0x2000
#define CD_MAX_HP       0x4000
#define CD_ORC          0x8000


/* inventory stacking subvals
 * these never stack:
 */

#define ITEM_NEVER_STACK_MIN    0
#define ITEM_NEVER_STACK_MAX    63

/* these items always stack with others of same subval, always treated as
 * single objects, must be power of 2
 */

#define ITEM_SINGLE_STACK_MIN   64
#define ITEM_SINGLE_STACK_MAX   192     /* see NOTE below */


/* these items stack with others only if have same subval and same p1,
 * they are treated as a group for wielding, etc.
 */

#define ITEM_GROUP_MIN          192
#define ITEM_GROUP_MAX          255

/* NOTE: items with subval 192 are treated as single objects, but only stack
 * with others of same subval if have the same p1 value, only used for
 * torches
 */


/* id's used for object description, stored in object_ident */
#define OD_TRIED        0x1
#define OD_KNOWN1       0x2

/* id's used for item description, stored in i_ptr->ident */
#define ID_MAGIK        0x1
#define ID_DAMD         0x2
#define ID_EMPTY        0x4
#define ID_KNOWN2       0x8
#define ID_STOREBOUGHT  0x10
#define ID_SHOW_HITDAM  0x20
#define ID_NOSHOW_P1    0x40    /* don't show (+x) even if p1 != 0 -CWS   */
#define ID_NOSHOW_TYPE  0x80    /* don't show (+x of yyy), just (+x) -CWS */

/* indexes into the special name table */
#define SN_NULL                 0
#define SN_R                    1
#define SN_RA                   2
#define SN_RF                   3
#define SN_RC                   4
#define SN_RL                   5
#define SN_HA                   6
#define SN_DF                   7
#define SN_SA                   8
#define SN_SD                   9
#define SN_SE                   10
#define SN_SU                   11
#define SN_FT                   12
#define SN_FB                   13
#define SN_FREE_ACTION          14
#define SN_SLAYING              15
#define SN_CLUMSINESS           16
#define SN_WEAKNESS             17
#define SN_SLOW_DESCENT         18
#define SN_SPEED                19
#define SN_STEALTH              20
#define SN_SLOWNESS             21
#define SN_NOISE                22
#define SN_GREAT_MASS           23
#define SN_INTELLIGENCE         24
#define SN_WISDOM               25
#define SN_INFRAVISION          26
#define SN_MIGHT                27
#define SN_LORDLINESS           28
#define SN_MAGI                 29
#define SN_BEAUTY               30
#define SN_SEEING               31
#define SN_REGENERATION         32
#define SN_STUPIDITY            33
#define SN_DULLNESS             34
#define SN_BLINDNESS            35
#define SN_TIMIDNESS            36
#define SN_TELEPORTATION        37
#define SN_UGLINESS             38
#define SN_PROTECTION           39
#define SN_IRRITATION           40
#define SN_VULNERABILITY        41
#define SN_ENVELOPING           42
#define SN_FIRE                 43
#define SN_SLAY_EVIL            44
#define SN_DRAGON_SLAYING       45
#define SN_EMPTY                46
#define SN_LOCKED               47
#define SN_POISON_NEEDLE        48
#define SN_GAS_TRAP             49
#define SN_EXPLOSION_DEVICE     50
#define SN_SUMMONING_RUNES      51
#define SN_MULTIPLE_TRAPS       52
#define SN_DISARMED             53
#define SN_UNLOCKED             54
#define SN_SLAY_ANIMAL          55
#define SN_GROND                56
#define SN_RINGIL               57
#define SN_AEGLOS               58
#define SN_ARUNRUTH             59
#define SN_MORMEGIL             60
#define SN_MORGUL               61
#define SN_ANGRIST              62
#define SN_GURTHANG             63
#define SN_CALRIS               64
#define SN_ACCURACY             65
#define SN_ANDURIL              66
#define SN_SO                   67
#define SN_POWER                68
#define SN_DURIN                69
#define SN_AULE                 70
#define SN_WEST                 71
#define SN_BLESS_BLADE          72
#define SN_SDEM                 73
#define SN_ST                   74
#define SN_BLOODSPIKE           75
#define SN_THUNDERFIST          76
#define SN_WOUNDING             77
#define SN_ORCRIST              78
#define SN_GLAMDRING            79
#define SN_STING                80
#define SN_LIGHT                81
#define SN_AGILITY              82
#define SN_BACKBITING           83
#define SN_DOOMCALLER           84
#define SN_SG                   85
#define SN_TELEPATHY            86
#define SN_DRAGONKIND           87
#define SN_NENYA                88
#define SN_NARYA                89
#define SN_VILYA                90
#define SN_AMAN                 91
#define SN_BELEGENNON           92
#define SN_FEANOR               93
#define SN_ANARION              94
#define SN_ISILDUR              95
#define SN_FINGOLFIN            96
#define SN_ELVENKIND            97
#define SN_SOULKEEPER           98
#define SN_DOR_LOMIN            99
#define SN_MORGOTH             100
#define SN_BELEG               101
#define SN_DAL                 102
#define SN_PAURHACH            103
#define SN_PAURNIMMEN          104
#define SN_PAURAEGEN           105
#define SN_CAMMITHRIM          106
#define SN_CAMBELEG            107
#define SN_HOLHENNETH          108
#define SN_PAURNEN             109
#define SN_AEGLIN              110
#define SN_CAMLOST             111
#define SN_NIMLOTH             112
#define SN_NAR                 113
#define SN_BERUTHIEL           114
#define SN_GORLIM              115
#define SN_NARTHANC            116
#define SN_NIMTHANC            117
#define SN_DETHANC             118
#define SN_GILETTAR            119
#define SN_RILIA               120
#define SN_BELANGIL            121
#define SN_BALLI               122
#define SN_LOTHARANG           123
#define SN_FIRESTAR            124
#define SN_ERIRIL              125
#define SN_CUBRAGOL            126
#define SN_BARD                127
#define SN_COLLUIN             128
#define SN_HOLCOLLETH          129
#define SN_TOTILA              130
#define SN_PAIN                131
#define SN_ELVAGIL             132
#define SN_AGLARANG            133
#define SN_ROHAN               134
#define SN_EORLINGAS           135
#define SN_BARUKKHELED         136
#define SN_WRATH               137
#define SN_HARADEKKET          138
#define SN_MUNDWINE            139
#define SN_GONDRICAM           140
#define SN_ZARCUTHRA           141
#define SN_CARETH              142
#define SN_FORASGIL            143
#define SN_CRISDURIAN          144
#define SN_COLANNON            145
#define SN_HITHLOMIR           146
#define SN_THALKETTOTH         147
#define SN_ARVEDUI             148
#define SN_THRANDUIL           149
#define SN_THENGEL             150
#define SN_HAMMERHAND          151
#define SN_CELEGORM            152
#define SN_THROR               153
#define SN_MAEDHROS            154
#define SN_OLORIN              155
#define SN_ANGUIREL            156
#define SN_THORIN              157
#define SN_CELEBORN            158
#define SN_OROME               159
#define SN_EONWE               160
#define SN_GONDOR              161
#define SN_THEODEN             162
#define SN_THINGOL             163
#define SN_THORONGIL           164
#define SN_LUTHIEN             165
#define SN_TUOR                166
#define SN_ULMO                167
#define SN_OSONDIR             168
#define SN_TURMIL              169
#define SN_CASPANION           170
#define SN_TIL                 171
#define SN_DEATHWREAKER        172
#define SN_AVAVIR              173
#define SN_TARATOL             174
#define SN_RAZORBACK           175
#define SN_BLADETURNER         176
#define SN_SHATTERED           177
#define SN_BLASTED             178
#define SN_ATTACKS             179
#define SN_ARRAY_SIZE          180 /* must be at end of this list */


/* defines for treasure type values (tval) */

#define TV_NEVER        -1 /* used by find_range() for non-search */
#define TV_NOTHING       0
#define TV_MISC          1
#define TV_CHEST         2
#define TV_SPIKE         3

/* min tval for wearable items, all items between TV_MIN_WEAR and TV_MAX_WEAR
 * use the same flag bits, see the TR_* defines
 */

#define TV_MIN_WEAR     10

/* items tested for enchantments, i.e. the MAGIK inscription, see the
 * enchanted() procedure
 */

#define TV_MIN_ENCHANT  10
#define TV_SLING_AMMO   10
#define TV_BOLT         11
#define TV_ARROW        12
#define TV_LIGHT        15
#define TV_BOW          20
#define TV_HAFTED       21
#define TV_POLEARM      22
#define TV_SWORD        23
#define TV_DIGGING      25
#define TV_BOOTS        30
#define TV_GLOVES       31
#define TV_CLOAK        32
#define TV_HELM         33
#define TV_SHIELD       34
#define TV_HARD_ARMOR   35
#define TV_SOFT_ARMOR   36
/* max tval that uses the TR_* flags */
#define TV_MAX_ENCHANT  39
#define TV_AMULET       40
#define TV_RING         45
/* max tval for wearable items */
#define TV_MAX_WEAR     50
#define TV_STAFF        55
#define TV_WAND         65
#define TV_ROD          66
#define TV_SCROLL1      70
#define TV_SCROLL2      71
#define TV_POTION1      75
#define TV_POTION2      76
#define TV_FLASK        77
#define TV_FOOD         80
#define TV_MAGIC_BOOK   90
#define TV_PRAYER_BOOK  91
/* objects with tval above this are never picked up by monsters */
#define TV_MAX_OBJECT   99
#define TV_GOLD         100
/* objects with higher tvals can not be picked up */
#define TV_MAX_PICK_UP  100
#define TV_INVIS_TRAP   101
/* objects between TV_MIN_VISIBLE and TV_MAX_VISIBLE are always visible,
   i.e. the cave fm flag is set when they are present */
#define TV_MIN_VISIBLE  102
#define TV_VIS_TRAP     102
#define TV_RUBBLE       103
/* following objects are never deleted when trying to create another one
   during level generation */
#define TV_MIN_DOORS    104
#define TV_OPEN_DOOR    104
#define TV_CLOSED_DOOR  105
#define TV_UP_STAIR     107
#define TV_DOWN_STAIR   108
#define TV_SECRET_DOOR  109
#define TV_STORE_DOOR   110
#define TV_MAX_VISIBLE  110

/* spell types used by get_flags(), breathe(), fire_bolt() and fire_ball() */
#define GF_MAGIC_MISSILE 0
#define GF_LIGHTNING    1
#define GF_POISON_GAS   2
#define GF_ACID         3
#define GF_FROST        4
#define GF_FIRE         5
#define GF_HOLY_ORB     6
#define GF_ARROW        7
#define GF_PLASMA       8
#define GF_NETHER       9
#define GF_WATER        10
#define GF_CHAOS        11
#define GF_SHARDS       12
#define GF_SOUND        13
#define GF_CONFUSION    14
#define GF_DISENCHANT   15
#define GF_NEXUS        16
#define GF_FORCE        17
#define GF_INERTIA      18
#define GF_LIGHT        19
#define GF_DARK         20
#define GF_TIME         21
#define GF_GRAVITY      22
#define GF_MANA         23
#define GF_METEOR       24
#define GF_ICE          25

#define WD_LT           1L
#define WD_LT_BLTS      2L
#define WD_FT_BLTS      3L
#define WD_FR_BLTS      4L
#define WD_ST_MUD       5L
#define WD_POLY         6L
#define WD_HEAL_MN      7L
#define WD_HAST_MN      8L
#define WD_SLOW_MN      9L
#define WD_CONF_MN      10L
#define WD_SLEE_MN      11L
#define WD_DRAIN        12L
#define WD_TR_DEST      13L
#define WD_MAG_MIS      14L
#define WD_FEAR_MN      15L
#define WD_CLONE        16L
#define WD_TELE         17L
#define WD_DISARM       18L
#define WD_LT_BALL      19L
#define WD_CD_BALL      20L
#define WD_FR_BALL      21L
#define WD_ST_CLD       22L
#define WD_AC_BALL      23L
#define WD_WONDER       24L
#define WD_DRG_FIRE     25L
#define WD_DRG_FRST     26L
#define WD_DRG_BREA     27L
#define WD_AC_BLTS      28L
#define WD_ANHIL        29L

#define RD_LT           1L
#define RD_LT_BLTS      2L
#define RD_FT_BLTS      3L
#define RD_FR_BLTS      4L
#define RD_ST_MUD       5L
#define RD_POLY         6L
#define RD_SLOW_MN      7L
#define RD_CONF_MN      8L
#define RD_SLEE_MN      9L
#define RD_DRAIN        10L
#define RD_TR_DEST      11L
#define RD_MAG_MIS      12L
#define RD_TELE         13L
#define RD_DISARM       14L
#define RD_LT_BALL      15L
#define RD_CD_BALL      16L
#define RD_FR_BALL      17L
#define RD_ST_CLD       18L
#define RD_AC_BALL      19L
#define RD_AC_BLTS      20L
#define RD_ANHIL        21L
#define RD_MAPPING      22L
#define RD_IDENT        23L
#define RD_CURE         24L
#define RD_HEAL         25L
#define RD_DETECT       26L
#define RD_RESTORE      27L
#define RD_SPEED        28L
#define RD_ILLUME       29L
#define RD_PROBE        30L
#define RD_RECALL       31L
#define RD_TRAP_LOC     32L
#define RD_MK_WALL      33L

#define ST_LIGHT        1L
#define ST_DR_LC        2L
#define ST_TRP_LC       3L
#define ST_TRE_LC       4L
#define ST_OBJ_LC       5L
#define ST_TELE         6L
#define ST_EARTH        7L
#define ST_SUMMON       8L
#define ST_DEST         9L
#define ST_STAR         10L
#define ST_HAST_MN      11L
#define ST_SLOW_MN      12L
#define ST_SLEE_MN      13L
#define ST_CURE_LT      14L
#define ST_DET_INV      15L
#define ST_SPEED        16L
#define ST_SLOW         17L
#define ST_REMOVE       18L
#define ST_DET_EVI      19L
#define ST_CURING       20L
#define ST_DSP_EVI      21L
#define ST_DARK         22L
#define ST_GENOCIDE     23L
#define ST_POWER        24L
#define ST_MAGI         25L
#define ST_HOLYNESS     26L
#define ST_IDENTIFY     27L
#define ST_SURROUND     28L
#define ST_HEALING      29L
#define ST_PROBE        30L

/* bit flags used in my revamped enchant() code -CFT */
#define ENCH_TOHIT   1
#define ENCH_TODAM   2
#define ENCH_TOAC    4
