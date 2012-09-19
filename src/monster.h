#define NONE8       	0x00000000
#define MV_ONLY_ATT    	0x00000001
#define MV_ATT_NORM 	0x00000002
#define MV_20		0x00000004
#define MV_40		0x00000008
#define MV_75		0x00000010
#define MV_INVIS	0x00010000
#define THRO_DR		0x00020000
#define THRO_WALL	0x00040000
#define THRO_CREAT	0x00080000
#define PICK_UP		0x00100000
#define MULTIPLY        0x00200000
#define CARRY_OBJ   	0x01000000
#define CARRY_GOLD	0x02000000
#define HAS_60		0x04000000
#define HAS_90		0x08000000
#define HAS_1D2		0x10000000
#define HAS_2D2		0x20000000
#define HAS_4D2		0x40000000
#define WINNER		0x80000000

#define BLINK		0x000010L
#define TELE		0x000020L
#define TELE_TO	 	0x000040L
#define CAUSE_LIGHT	0x000080L
#define CAUSE_SERIOUS	0x000100L
#define HOLD_PERSON	0x000200L
#define BLINDNESS	0x000400L
#define CONFUSION	0x000800L
#define FEAR		0x001000L
#define MONSTER		0x002000L
#define S_UNDEAD	0x004000L
#define SLOW		0x008000L
#define MANA_DRAIN	0x010000L
#define S_DEMON		0x020000L
#define S_DRAGON	0x040000L
#define BREATH_L	0x080000L
#define BREATH_G	0x100000L
#define BREATH_A	0x200000L
#define BREATH_FR	0x400000L
#define BREATH_FI	0x800000L
#define FIRE_BOLT      0x1000000L
#define FROST_BOLT     0x2000000L
#define ACID_BOLT    0x04000000L
#define MAG_MISS     0x08000000L
#define CAUSE_CRIT   0x10000000L
#define FIRE_BALL    0x20000000L
#define FROST_BALL   0x40000000L
#define MANA_BOLT    0x80000000L

/* spells 2 :-> */
#define BREATH_CH    0x00000001L /* Chaos */
#define BREATH_SH    0x00000002L /* Shards */
#define BREATH_SD    0x00000004L /* Sound */
#define BREATH_CO    0x00000008L /* Confusion */
#define BREATH_DI    0x00000010L /* Disenchantment */
#define BREATH_LD    0x00000020L /* Life Draining */
#define LIGHT_BOLT   0x00000040L
#define LIGHT_BALL   0x00000080L
#define ACID_BALL    0x00000100L
#define TRAP_CREATE  0x00000200L
#define RAZOR        0x00000400L
#define MIND_BLAST   0x00000800L
#define TELE_AWAY    0x00001000L
#define HEAL         0x00002000L
#define HASTE        0x00004000L
#define MISSILE      0x00008000L
#define PLASMA_BOLT  0x00010000L
#define SUMMON       0x00020000L
#define NETHER_BOLT  0x00040000L
#define ICE_BOLT     0x00080000L
#define DARKNESS     0x00100000L
#define FORGET       0x00200000L
#define BRAIN_SMASH  0x00400000L
#define ST_CLOUD     0x00800000L
#define TELE_LEV     0x01000000L
#define WATER_BOLT   0x02000000L
#define WATER_BALL   0x04000000L
#define NETHER_BALL  0x08000000L
#define S_ANGEL      0x10000000L
#define S_SPIDER     0x20000000L
#define S_HOUND      0x40000000L
#define BREATH_NE    0x80000000L /* Nexus */

/* spells 3:-> */
#define BREATH_WA    0x00000001L /* Wall Building */
#define BREATH_SL    0x00000002L /* Slowness */
#define BREATH_LT    0x00000004L /* Light */
#define BREATH_TI    0x00000008L /* Time */
#define BREATH_GR    0x00000010L /* Gravity */
#define BREATH_DA    0x00000020L /* Darkness */
#define BREATH_PL    0x00000040L /* Plasma */
#define ARROW        0x00000080L /* fires an arrow */
#define S_WRAITH     0x00000100L /* Summon ringwraiths */
#define DARK_STORM   0x00000200L /* Big darkness breath */
#define MANA_STORM   0x00000400L /* Mana storm */
#define S_REPTILE    0x00000800L /* Summon reptiles */
#define S_ANT	     0x00001000L /* Summon ants/ant lions */
#define S_UNIQUE     0x00002000L /* Summon uniques */
#define S_GUNDEAD    0x00004000L /* Summon greater undead */
#define S_ANCIENTD   0x00008000L /* Summon ancient dragon */

#define ANIMAL		0x00000001
#define EVIL		0x00000002
#define ORC		0x00000004
#define UNDEAD		0x00000008
#define DRAGON		0x00000010
#define DEMON		0x00000020
#define HURT_LIGHT	0x00000040
#define	HURT_ROCK	0x00000080
#define CHARM_SLEEP	0x00000100
#define NO_INFRA	0x00000200
#define MAX_HP		0x00000400
#define IM_FROST	0x00000800
#define IM_FIRE		0x00001000
#define IM_LIGHTNING	0x00002000
#define IM_POISON	0x00004000
#define IM_ACID		0x00008000
#define TROLL		0x00010000
#define GIANT		0x00020000
#define SPECIAL         0x00040000
#define GROUP           0x00080000
#define GOOD            0x00100000
#define BREAK_WALL      0x00200000
#define DESTRUCT        0x00400000
#define QUESTOR         0x00800000 /* killing it completes a quest */
#define INTELLIGENT     0x01000000 /* casts spells intelligently */
#define UNIQUE          0x80000000 /* unique monster */
