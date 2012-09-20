/* externs.h: declarations for global variables and initialized data

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* TURBOC wants full prototypes, or it bitches... */
#ifdef __TURBOC__
#define ARG_VOID void
#define ARG_INT int
#define ARG_INT8U int8u
#define ARG_INT16U int16u
#define ARG_INT32U int32u
#define ARG_INT8 int8
#define ARG_INT16 int16
#define ARG_INT32 int32
#define ARG_CHAR_PTR char *
#define ARG_CHAR char
#define ARG_INV_PTR inven_type *
#define ARG_INT_PTR int *
#define ARG_INT32U_PTR int32u *
#define ARG_INT8U_PTR int8u *
#define ARG_INT_FN int (*)()
#define ARG_INT_FN_PTR int (**)()
#define ARG_INT16_PTR int16 *
#define ARG_MON_PTR monster_type *
#define ARG_CTR_PTR creature_type *
#define ARG_INT32_PTR int32 * 
#define ARG_INT16U_PTR int16u *
#define ARG_UNIQ_PTR struct unique_mon *
#define ARG_VOID_PTR void *
#define ARG_COMMA ,
#else
#define ARG_VOID
#define ARG_INT
#define ARG_INT8U
#define ARG_INT16U
#define ARG_INT32U
#define ARG_INT8
#define ARG_INT16
#define ARG_INT32
#define ARG_CHAR_PTR
#define ARG_CHAR
#define ARG_INV_PTR
#define ARG_INT_PTR
#define ARG_INT32U_PTR
#define ARG_INT8U_PTR
#define ARG_INT_FN
#define ARG_INT_FN_PTR
#define ARG_INT16_PTR
#define ARG_MON_PTR
#define ARG_CTR_PTR
#define ARG_INT32_PTR
#define ARG_INT16U_PTR
#define ARG_UNIQ_PTR
#define ARG_VOID_PTR
#define ARG_COMMA
#endif

/* many systems don't define these anywhere */
#ifndef MSDOS
#if defined(USG) || defined(DGUX) || defined(atarist)
extern int sprintf();
#else
extern char *sprintf();
#endif
#endif

#ifndef MSDOS
extern int errno;
#endif

extern char *copyright[5];

extern int player_uid;
#ifdef MSDOS
extern int8u NO_SAVE;
#else
extern int NO_SAVE;
#endif

/* horrible hack: needed because compact_monster() can be called from deep
   within creatures() via place_monster() and summon_monster() */
extern int hack_monptr;

extern int16 log_index;		/* Index to log file. -CJS- */
extern vtype died_from;
extern vtype savefile;			/* The save file. -CJS- */

/* These are options, set with set_options command -CJS- */
#ifdef MSDOS
extern int8u rogue_like_commands;
extern int8u find_cut;			/* Cut corners on a run */
extern int8u find_examine;		/* Check corners on a run */
extern int8u find_prself;			/* Print yourself on a run (slower) */
extern int8u find_bound;			/* Stop run when the map shifts */
extern int8u prompt_carry_flag;		/* Prompt to pick something up */
extern int8u show_weight_flag;		/* Display weights in inventory */
extern int8u highlight_seams;		/* Highlight magma and quartz */
extern int8u find_ignore_doors;		/* Run through open doors */
extern int8u sound_beep_flag;	/* shut up bell() ! -CFT */
extern int8u no_haggle_flag; /* for those who find it tedious -CFT */
#else
extern int rogue_like_commands;
extern int find_cut;			/* Cut corners on a run */
extern int find_examine;		/* Check corners on a run */
extern int find_prself;			/* Print yourself on a run (slower) */
extern int find_bound;			/* Stop run when the map shifts */
extern int prompt_carry_flag;		/* Prompt to pick something up */
extern int show_weight_flag;		/* Display weights in inventory */
extern int highlight_seams;		/* Highlight magma and quartz */
extern int find_ignore_doors;		/* Run through open doors */
#endif

/* Unique artifact weapon flags */
#ifdef MSDOS
extern int8u
#else
extern int
#endif
	GROND, RINGIL, AEGLOS, ARUNRUTH, MORMEGIL, ANGRIST, GURTHANG,
  CALRIS, ANDURIL, STING, ORCRIST, GLAMDRING, DURIN, AULE, THUNDERFIST,
  BLOODSPIKE, DOOMCALLER, NARTHANC, NIMTHANC, DETHANC, GILETTAR, RILIA,
  BELANGIL, BALLI, LOTHARANG, FIRESTAR, ERIRIL, CUBRAGOL, BARD, COLLUIN,
  HOLCOLLETH, TOTILA, PAIN, ELVAGIL, AGLARANG, EORLINGAS, BARUKKHELED,
  WRATH, HARADEKKET, MUNDWINE, GONDRICAM, ZARCUTHRA, CARETH, FORASGIL,
  CRISDURIAN, COLANNON, HITHLOMIR, THALKETTOTH, ARVEDUI, THRANDUIL, THENGEL,
  HAMMERHAND, CELEFARN, THROR, MAEDHROS, OLORIN, ANGUIREL, OROME,
  EONWE, THEODEN, ULMO, OSONDIR, TURMIL, TIL, DEATHWREAKER, AVAVIR, TARATOL;

/* Unique artifact armour flags */
#ifdef MSDOS
extern int8u
#else
extern int
#endif
	DOR_LOMIN, NENYA, NARYA, VILYA, BELEGENNON, FEANOR, ISILDUR,
SOULKEEPER, FINGOLFIN, ANARION, POWER, PHIAL, BELEG, DAL, PAURHACH,
PAURNIMMEN, PAURAEGEN, PAURNEN, CAMMITHRIM, CAMBELEG, INGWE, CARLAMMAS,
HOLHENNETH, AEGLIN, CAMLOST, NIMLOTH, NAR, BERUTHIEL, GORLIM, ELENDIL,
THORIN, CELEBORN, THRAIN, GONDOR, THINGOL, THORONGIL, LUTHIEN, TUOR, ROHAN,
TULKAS, NECKLACE, BARAHIR, CASPANION, RAZORBACK, BLADETURNER;

/* Brand new extra effecient and kind way to add unique monsters... HOORAY!! */
extern struct unique_mon u_list[MAX_CREATURES];

#ifdef MSDOS
extern int16 quests[MAX_QUESTS];
#else
extern int quests[MAX_QUESTS];
#endif

/* global flags */
extern int LOAD;
#ifdef MSDOS
extern int8u good_item_flag;      /* True if an artifact has been created... */
extern int8u new_level_flag;	  /* Next level when true  */
extern int8u search_flag;	      /* Player is searching   */
extern int8u teleport_flag;	/* Handle teleport traps  */
extern int8u eof_flag;		/* Used to handle eof/HANGUP */
extern int8u player_light;      /* Player carrying light */
extern int8u find_flag;	/* Used in MORIA	      */
extern int8u free_turn_flag;	/* Used in MORIA	      */
extern int8u weapon_heavy;	/* Flag if the weapon too heavy -CJS- */
extern int8u pack_heavy;		/* Flag if the pack too heavy -CJS- */
extern char doing_inven;	/* Track inventory commands */
extern int8u screen_change;	/* Screen changes (used in inven_commands) */
extern int8u be_nasty;

extern int8u character_generated;	 /* don't save score until char gen finished */
extern int8u character_saved;	 /* prevents save on kill after save_char() */
#else
extern int good_item_flag;      /* True if an artifact has been created... */
extern int new_level_flag;	  /* Next level when true  */
extern int search_flag;	      /* Player is searching   */
extern int teleport_flag;	/* Handle teleport traps  */
extern int eof_flag;		/* Used to handle eof/HANGUP */
extern int player_light;      /* Player carrying light */
extern int find_flag;	/* Used in MORIA	      */
extern int free_turn_flag;	/* Used in MORIA	      */
extern int weapon_heavy;	/* Flag if the weapon too heavy -CJS- */
extern int pack_heavy;		/* Flag if the pack too heavy -CJS- */
extern char doing_inven;	/* Track inventory commands */
extern int screen_change;	/* Screen changes (used in inven_commands) */
extern int be_nasty;

extern int character_generated;	 /* don't save score until char gen finished */
extern int character_saved;	 /* prevents save on kill after save_char() */
#endif

extern int highscore_fd;	/* High score file descriptor */
extern int command_count;	/* Repetition of commands. -CJS- */
#ifdef MSDOS
extern int8u default_dir;		/* Use last direction in repeated commands */
extern int16u noscore;		/* Don't score this game. -CJS- */
#else
extern int default_dir;		/* Use last direction in repeated commands */
extern int16 noscore;		/* Don't score this game. -CJS- */
#endif
extern int32u randes_seed;    /* For encoding colors */
extern int32u town_seed;	    /* Seed for town genera*/
extern int16 dun_level;	/* Cur dungeon level   */
extern int16 missile_ctr;	/* Counter for missiles */
extern int msg_flag;	/* Set with first msg  */
extern vtype old_msg[MAX_SAVE_MSG];	/* Last messages -CJS- */
extern int16 last_msg;			/* Where in the array is the last */
extern int32 turn;	/* Cur trun of game    */
#ifdef MSDOS
extern int8u death;	/* True if died	      */
extern int8u wizard;	/* Wizard flag	      */
extern int8u to_be_wizard;
extern int16u panic_save; /* this is true if playing from a panic save */
extern int8u wait_for_more;
#else
extern int death;	/* True if died	      */
extern int wizard;	/* Wizard flag	      */
extern int to_be_wizard;
extern int16 panic_save; /* this is true if playing from a panic save */
extern int wait_for_more;
#endif

extern char days[7][29];
#ifdef MSDOS
extern int8u closing_flag;	/* Used for closing   */
#else
extern int closing_flag;	/* Used for closing   */
#endif

extern int16 cur_height, cur_width;	/* Cur dungeon size    */
/*  Following are calculated from max dungeon sizes		*/
extern int16 max_panel_rows, max_panel_cols;
extern int panel_row, panel_col;
extern int panel_row_min, panel_row_max;
extern int panel_col_min, panel_col_max;
extern int panel_col_prt, panel_row_prt;

/*  Following are all floor definitions				*/
#ifdef MAC
extern cave_type (*cave)[MAX_WIDTH];
#else
extern cave_type cave[MAX_HEIGHT][MAX_WIDTH];
#endif

/* Following are player variables				*/
extern player_type py;
#ifdef MACGAME
extern char *(*player_title)[MAX_PLAYER_LEVEL];
extern race_type *race;
extern background_type *background;
#else
extern char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];
extern race_type race[MAX_RACES];
extern background_type background[MAX_BACKGROUND];
#endif
extern int32u player_exp[MAX_PLAYER_LEVEL];
extern int16u player_hp[MAX_PLAYER_LEVEL];
extern int16 char_row;
extern int16 char_col;

extern char *dsp_race[MAX_RACES];	/* Short strings for races. -CJS- */
extern int8u rgold_adj[MAX_RACES][MAX_RACES];

extern class_type class[MAX_CLASS];
extern int16 class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

/* Warriors don't have spells, so there is no entry for them. */
#ifdef MACGAME
extern spell_type (*magic_spell)[63];
#else
extern spell_type magic_spell[MAX_CLASS-1][63];
#endif
extern char *spell_names[127];
extern int32u spell_learned;	/* Bit field for spells learnt -CJS- */
extern int32u spell_learned2;	/* Bit field for spells learnt -CJS- */
extern int32u spell_worked;	/* Bit field for spells tried -CJS- */
extern int32u spell_worked2;	/* Bit field for spells tried -CJS- */
extern int32u spell_forgotten;	/* Bit field for spells forgotten -JEW- */
extern int32u spell_forgotten2;	/* Bit field for spells forgotten -JEW- */
extern int8u spell_order[64];	/* remember order that spells are learned in */
extern int16u player_init[MAX_CLASS][5];
extern int16 total_winner;

/* Following are store definitions				*/
#ifdef MACGAME
extern owner_type *owners;
#else
extern owner_type owners[MAX_OWNERS];
#endif
#ifdef MAC
extern store_type *store;
#else
extern store_type store[MAX_STORES];
#endif
extern int16u store_choice[MAX_STORES][STORE_CHOICES];
#ifndef MAC
extern int (*store_buy[MAX_STORES])();
#endif

/* Following are treasure arrays	and variables			*/
#ifdef MACGAME
extern treasure_type *object_list;
#else
extern treasure_type object_list[MAX_OBJECTS];
#endif
extern int8u object_ident[OBJECT_IDENT_SIZE];
extern int16 t_level[MAX_OBJ_LEVEL+1];
extern inven_type t_list[MAX_TALLOC];
extern inven_type inventory[INVEN_ARRAY_SIZE];
extern char *special_names[SN_ARRAY_SIZE];
extern int16 sorted_objects[MAX_DUNGEON_OBJ];
extern int16 inven_ctr;		/* Total different obj's	*/
extern int16 inven_weight;	/* Cur carried weight	*/
extern int16 equip_ctr;	/* Cur equipment ctr	*/
extern int16 tcptr;	/* Cur treasure heap ptr	*/

/* Following are creature arrays and variables			*/
#ifdef MACGAME
extern creature_type *c_list;
#else
extern creature_type c_list[MAX_CREATURES];
#endif
extern describe_mon_type desc_list[MAX_CREATURES];
extern monster_type m_list[MAX_MALLOC];
extern int16 m_level[MAX_MONS_LEVEL+1];
extern m_attack_type monster_attacks[N_MONS_ATTS];
#ifdef MAC
extern recall_type *c_recall;
#else
extern recall_type c_recall[MAX_CREATURES];	/* Monster memories. -CJS- */
#endif
extern monster_type blank_monster;	/* Blank monster values	*/
extern int16 mfptr;	/* Cur free monster ptr	*/
extern int16 mon_tot_mult;	/* # of repro's of creature	*/

/* Following are arrays for descriptive pieces			*/
#ifdef MACGAME
extern char **colors;
extern char **mushrooms;
extern char **woods;
extern char **metals;
extern char **rocks;
extern char **amulets;
extern char **syllables;
#else
extern char *colors[MAX_COLORS];
extern char *mushrooms[MAX_MUSH];
extern char *woods[MAX_WOODS];
extern char *metals[MAX_METALS];
extern char *rocks[MAX_ROCKS];
extern char *amulets[MAX_AMULETS];
extern char *syllables[MAX_SYLLABLES];
#ifdef TC_COLOR
extern int8u tccolors[MAX_COLORS];
extern int8u tcmushrooms[MAX_MUSH];
extern int8u tcwoods[MAX_WOODS];
extern int8u tcmetals[MAX_METALS];
extern int8u tcrocks[MAX_ROCKS];
extern int8u tcamulets[MAX_AMULETS];
#endif
#endif

extern int8u blows_table[11][12];

extern int16u normal_table[NORMAL_TABLE_SIZE];

/* Initialized data which had to be moved from some other file */
/* Since these get modified, macrsrc.c must be able to access them */
/* Otherwise, game cannot be made restartable */
/* dungeon.c */
extern char last_command;  /* Memory of previous command. */
/* moria1.c */
/* Track if temporary light about player.  */
#ifdef MSDOS
extern int8u light_flag;
#else
extern int8u light_flag;
#endif

#ifdef MSDOS
extern int8u	floorsym, wallsym;
extern int	ansi, saveprompt;
extern char	moriatop[], moriasav[];
#endif

/* function return values */
/* only extern functions declared here, static functions declared inside
   the file that defines them */
#if defined(LINT_ARGS)
/* these prototypes can be used by MSC for type checking of arguments
   WARNING: note that this only works for MSC because it is NOT, I repeat,
   NOT an ANSI C compliant compiler, correct compilers, e.g. Gnu C, will give
   error messages if you use these prototypes */

/* create.c */
void create_character(void);

/* creature.c */
void update_mon(int);
int movement_rate(int16);
int multiply_monster(int, int, int, int);
void creatures(int);

/* death.c */
void exit_game(void);

/* desc.c */
int is_a_vowel(char);
void magic_init(void);
void known1(char *);
int known1_p(inven_type *);
void known2(char *);
int known2_p*(inven_type *);
void clear_known2(inven_type *);
void clear_empty(inven_type *);
void store_bought(inven_type *);
int store_bought_p(inven_type *);
void sample(struct inven_type *);
void identify(int *);
void unmagic_name(char *);
void objdes(char *, struct inven_type *, int);
void scribe_object(void);
void add_inscribe(char *, char *);
void inscribe(char *, char *);
void invcopy(inven_type *, int);
void desc_charges(int);
void desc_remain(int);

/* dungeon.c */
void dungeon(void);

/* eat.c */
void eat(void);

/* files.c */
void read_times(void);
void helpfile(char *);
void print_objects(void);
#ifdef MAC
int file_character(void)
#else
int file_character(char *);
#endif


/* generate.c */
void generate_cave(void);

/* help.c */
void ident_char(void);

/* io.c */
#ifdef SIGTSTP
int suspend(void);
#endif
void init_curses(void);
void moriaterm(void);
void put_buffer(char *, int, int);
void put_qio(void);
void restore_term(void);
void shell_out(void);
char inkey(void);
void flush(void);
void erase_line(int, int);
void clear_screen(void);
void clear_from(int);
void print(char, int, int);
void move_cursor_relative(int, int);
void count_msg_print(char *);
void prt(char *, int, int);
void move_cursor(int, int);
void msg_print(char *);
int get_check(char *);
int get_com(char *, char *);
int get_string(char *, int, int, int);
void pause_line(int);
void pause_exit(int, int);
void save_screen(void);
void restore_screen(void);
void bell(void);
void screen_map(void);

/* magic.c */
void cast(void);

/* main.c */
int main(int, char **);

/* misc1.c */
void init_seeds(int32u);
void set_seed(int32u);
void reset_seed(void);
int check_time(void);
int randint(int);
int randnor(int, int);
int bit_pos(int32u *);
int in_bounds(int, int);
void panel_bounds(void);
int get_panel(int, int, int);
int panel_contains(int, int);
int distance(int, int, int, int);
int next_to_wall(int, int);
int next_to_corr(int, int);
int damroll(int, int);
int pdamroll(char *);
int los(int, int, int, int);
unsigned char loc_symbol(int, int);
int test_light(int, int);
void prt_map(void);
void add_food(int);
int popm(void);
int max_hp(char *);
void place_monster(int, int, int, int);
void place_win_monster(void);
int get_mons_num(int);
void alloc_monster(int, int, int);
int summon_monster(int * ,int *, int);
int summon_undead(int *, int *);
int popt(void);
void pusht(int8u);
int magik(int);
int m_bonus(int, int, int);
void magic_treasure(int, int);
void set_options(void);

/* misc2.c */
void place_trap(int, int, int);
void place_rubble(int, int);
void place_gold(int, int);
int get_obj_num(int);
void place_object(int, int);
void alloc_object(int (*)(), int, int);
void random_object(int, int, int);
void cnv_stat(int16u, char *);
void prt_stat(int);
void prt_field(char *, int, int);
int stat_adj(int);
int chr_adj(void);
int con_adj(void);
char *title_string(void);
void prt_title(void);
void prt_level(void);
void prt_cmana(void);
void prt_mhp(void);
void prt_chp(void);
void prt_pac(void);
void prt_gold(void);
void prt_depth(void);
void prt_hunger(void);
void prt_blind(void);
void prt_confused(void);
void prt_afraid(void);
void prt_poisoned(void);
void prt_state(void);
void prt_speed(void);
void prt_study(void);
void prt_winner(void);
int16u modify_stat(int, int16);
void set_use_stat(int);
int inc_stat(int);
int dec_stat(int);
int res_stat(int);
void bst_stat(int, int);
int tohit_adj(void);
int toac_adj(void);
int todis_adj(void);
int todam_adj(void);
void prt_stat_block(void);
void draw_cave(void);
void put_character(void);
void put_stats(void);
char *likert(int, int);
void put_misc1(void);
void put_misc2(void);
void put_misc3(void);
void display_char(void);
void get_name(void);
void change_name(void);
void inven_destroy(int);
void take_one_item(struct inven_type *, struct inven_type *);
void inven_drop(int, int);
int inven_damage(int (*)(), int);
int weight_limit(void);
int inven_check_num(void);
int inven_check_weight(struct inven_type *);
void check_strength(void);
int inven_carry(struct inven_type *);
int spell_chance(int);
void print_spells(int *, int, int, int);
int get_spell(int *, int, int *, int *, char *, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);
void prt_experience(void);
void calc_hitpoints(void);
void insert_str(char *, char *, char *);
void insert_lnum(char *, char *, int32, int);
int enter_wiz_mode(void);
int attack_blows(int, int *);
int tot_dam(struct inven_type *, int, int);
int critical_blow(int, int, int, int);
int mmove(int, int *, int *);
int player_saves(void);
int find_range(int, int, int *, int *);
void teleport(int);
void check_view(void);

/* monsters.c */

/* moria1.c */
void change_speed(int);
void py_bonuses(struct inven_type *, int);
void calc_bonuses(void);
int show_inven(int, int, int, int);
char *describe_use(int);
int show_equip(int, int);
void takeoff(int, int);
int verify(char *, int);
void inven_command(char);
int get_item(int *, char *, int, int);
int no_light(void);
int get_dir(char *, int *);
int get_alldir(char *, int *);
void move_rec(int, int, int, int);
void light_room(int, int);
void lite_spot(int, int);
void move_light(int, int, int, int);
void disturb(int, int);
void search_on(void);
void search_off(void);
void rest(void);
void rest_off(void);
int test_hit(int, int, int, int, int);
void take_hit(int, char *);
void change_trap(int, int);
void search(int, int, int);
void find_init(void);
void find_run(void);
void end_find(void);
void area_affect(int, int, int);
int minus_ac(int32u);
void corrode_gas(char *);
void poison_gas(int, char *);
void fire_dam(int, char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, char *);

/* moria2.c */
int cast_spell(char * ,int, int *, int *);
void delete_monster(int);
void fix1_delete_monster(int);
void fix2_delete_monster(int);
int delete_object(int, int);
int32u monster_death(int, int, int32u);
int mon_take_hit(int, int);
void move_char(int, int);
void openobject(void);
void closeobject(void);
int twall(int, int, int, int);
void tunnel(int);
void disarm_trap(void);
void look(void);
void throw_object(void);
void bash(void);

#ifdef MSDOS
/* ms_misc.c */
char *getlogin(void);
#ifdef __TURBOC__
void sleep(unsigned);
#else
unsigned int sleep(int );
#endif
void error(char *, ...);
void warn(char *, ...);
void msdos_init(void);
void msdos_raw(void);
void msdos_noraw(void);
int bios_getch(void);
int msdos_getch(void);
void bios_clear(void);
void msdos_intro(void);
void bios_clear(void);
#endif

/* potions.c */
void quaff(void);

/* prayer.c */
void pray(void);

/* recall.c */
int bool_roff_recall(int);
int roff_recall(int);

/* rnd.c */
int32u get_rnd_seed(void);
void set_rnd_seed(int32u);
int32 rnd(void);

/* save.c */
#ifdef MAC
int save_char(int);
#else
int save_char(void);
#endif
int _save_char(char *);
int get_char(int *);

/* scrolls.c */
void read_scroll(void);

/* sets.c */
int set_room(int);
int set_corr(int);
int set_floor(int);
int set_corrodes(inven_type *);
int set_flammable(inven_type *);
int set_frost_destroy(inven_type *);
int set_acid_affect(inven_type *);
int set_lightning_destroy(inven_type *);
int set_null(inven_type *);
int set_acid_destroy(inven_type *);
int set_fire_destroy(inven_type *);
int general_store(int);
int armory(int);
int weaponsmith(int);
int temple(int);
int alchemist(int);
int magic_shop(int);
#ifdef MAC
int store_buy(int, int);
#endif

/* signals.c */
void nosignals(void);
void signals(void);
void init_signals(void);
void ignore_signals(void);
void default_signals(void);
void restore_signals(void);

/* spells.c */
void monster_name(char *, struct monster_type *, struct creature_type *);
void lower_monster_name(char *, struct monster_type *, struct creature_type *);
int sleep_monsters1(int, int);
int detect_treasure(void);
int detect_object(void);
int detect_trap(void);
int detect_sdoor(void);
int detect_invisible(void);
int light_area(int, int);
int unlight_area(int, int);
void map_area(void);
int ident_spell(void);
int aggravate_monster(int);
int trap_creation(void);
int door_creation(void);
int td_destroy(void);
int detect_monsters(void);
void light_line(int, int, int);
void starlite(int, int);
int disarm_all(int, int, int);
void get_flags(int, int32u *, int32u *, int (**)());
void fire_bolt(int, int, int, int, int, char *);
void fire_ball(int, int, int, int, int, char *);
void breath(int, int, int, int, char *, int);
int recharge(int);
int hp_monster(int, int, int, int);
int drain_life(int, int, int);
int speed_monster(int, int, int, int);
int confuse_monster(int, int, int);
int sleep_monster(int, int, int);
int wall_to_mud(int, int, int);
int td_destroy2(int, int, int);
int poly_monster(int, int, int);
int build_wall(int, int, int);
int clone_monster(int, int, int);
void teleport_away(int, int);
void teleport_to(int, int);
int teleport_monster(int, int, int);
int mass_genocide(void);
int genocide(void);
int speed_monsters(int);
int sleep_monsters2(void);
int mass_poly(void);
int detect_evil(void);
int hp_player(int);
int cure_confusion(void);
int cure_blindness(void);
int cure_poison(void);
int remove_fear(void);
void earthquake(void);
int protect_evil(void);
void create_food(void);
int dispel_creature(int, int);
int turn_undead(void);
void warding_glyph(void);
void lose_str(void);
void lose_int(void);
void lose_wis(void);
void lose_dex(void);
void lose_con(void);
void lose_chr(void);
void lose_exp(int32);
int slow_poison(void);
void bless(int);
void detect_inv2(int);
void destroy_area(int, int);
int enchant(int16 *);
int remove_curse(void);
int restore_level(void);
void self_knowledge(void);

/* staffs.c */
void use(void);

/* store1.c */
int32 item_value(struct inven_type *);
int32 sell_price(int, int32 *, int32 *, struct inven_type *);
int store_check_num(int);
void store_carry(int, int *, struct inven_type *);
void store_destroy(int, int, int);
void store_init(void);
void store_maint(void);
int noneedtobargain(int, int32);
void updatebargain(int, int32, int32);

/* store2.c */
void enter_store(int);

/* treasur1.c */

/* treasur2.c */

#ifdef unix
/* unix.c */
int check_input(int);
#if 0
int system_cmd(char *);
#endif
void user_name(char *);
int tilde(char *, char *);
FILE *tfopen(char *, char *);
int topen(char *, int, int);
#endif

/* variable.c */

/* wands.c */
void aim(void);

/* wizard.c */
void wizard_light(void);
void change_character(void);
void wizard_create(void);
void artifact_check(void);

#else

/* create.c */
void create_character(ARG_VOID);
void rerate(ARG_VOID);

/* creature.c */
void update_mon(ARG_INT);
int movement_rate(ARG_INT16);
void creatures(ARG_INT);
int multiply_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);

/* death.c */
void exit_game(ARG_VOID);
void display_scores(ARG_INT ARG_COMMA ARG_INT);
int delete_entry(ARG_INT);

/* desc.c */
int is_a_vowel(ARG_CHAR);
void magic_init(ARG_VOID);
void known1(ARG_INV_PTR);
int known1_p(ARG_INV_PTR);
void known2(ARG_INV_PTR);
int known2_p(ARG_INV_PTR);
void clear_known2(ARG_INV_PTR);
void clear_empty(ARG_INV_PTR);
void store_bought(ARG_INV_PTR);
int store_bought_p(ARG_INV_PTR);
void sample(ARG_INV_PTR);
void identify(ARG_INT_PTR);
void unmagic_name(ARG_INV_PTR);
void objdes(ARG_CHAR_PTR ARG_COMMA ARG_INV_PTR ARG_COMMA ARG_INT);
/* void scribe_object();  apperently moved to misc2.c -CFT */
/* void add_inscribe(); apperently moved to misc2.c -CFT */
/* void inscribe(); apperently moved to misc2.c -CFT */
void invcopy(ARG_INV_PTR ARG_COMMA ARG_INT);
void desc_charges(ARG_INT);
void desc_remain(ARG_INT);
int16 object_offset(ARG_INV_PTR);

/* dungeon.c */
void dungeon(ARG_VOID);
int is_quest(ARG_INT);
int special_check(ARG_INV_PTR);
char *value_check(ARG_INV_PTR);
int ruin_stat(ARG_INT);
int special_check(ARG_INV_PTR);

/* eat.c */
void eat(ARG_VOID);

/* files.c */
void read_times(ARG_VOID);
void helpfile(ARG_CHAR_PTR);
void print_objects(ARG_VOID);
int file_character(ARG_CHAR_PTR);
int init_scorefile(ARG_VOID);

/* generate.c */
void generate_cave(ARG_VOID);

/* help.c */
void ident_char(ARG_VOID);

#ifndef USING_TCIO
/* io.c */
#ifdef SIGTSTP
int suspend();
#endif
void init_curses();
void moriaterm();
void put_buffer();
void put_qio();
void restore_term();
void shell_out();
char inkey();
void flush();
void erase_line();
void clear_screen();
void clear_from();
void print();
void move_cursor_relative();
void count_msg_print();
void prt();
void move_cursor();
void msg_print();
int get_check();
int get_com();
int get_string();
void pause_line();
void pause_exit();
void save_screen();
void restore_screen();
void bell();
void screen_map();
#endif

/* magic.c */
void cast(ARG_VOID);

/* main.c */
int main();

/* misc1.c */
void set_options(ARG_VOID); /* apperently moved from moria1.c -CFT */
void panel_bounds(ARG_VOID); /* apperently moved from moria1.c -CFT */
int get_panel(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
	 /* apperently moved from moria1.c -CFT */
int panel_contains(ARG_INT ARG_COMMA ARG_INT);
	 /* apperently moved from moria1.c -CFT */
void init_seeds(ARG_INT32U);
void set_seed(ARG_INT32U);
void reset_seed(ARG_VOID);
int check_time(ARG_VOID);
int randint(ARG_INT);
int randnor(ARG_INT ARG_COMMA ARG_INT);
int bit_pos(ARG_INT32U_PTR);
int in_bounds(ARG_INT ARG_COMMA ARG_INT);
int distance(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int next_to_walls(ARG_INT ARG_COMMA ARG_INT);
int next_to_corr(ARG_INT ARG_COMMA ARG_INT);
int damroll(ARG_INT ARG_COMMA ARG_INT);
int pdamroll(ARG_INT8U_PTR);
int los(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
unsigned char loc_symbol(ARG_INT ARG_COMMA ARG_INT);
int test_light(ARG_INT ARG_COMMA ARG_INT);
void prt_map(ARG_VOID);
void add_food(ARG_INT);
int popm(ARG_VOID);
int max_hp(ARG_INT8U_PTR);
void place_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT
		ARG_COMMA ARG_INT);
void place_win_monster(ARG_VOID);
int get_mons_num(ARG_INT);
void alloc_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int summon_monster(ARG_INT_PTR ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INT);
int summon_undead(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int popt(ARG_VOID);
void pusht(ARG_INT8U);
int magik(ARG_INT);
int m_bonus(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void magic_treasure(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT
		ARG_COMMA ARG_INT);
void set_ghost(ARG_CTR_PTR ARG_COMMA ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int test_place(ARG_INT ARG_COMMA ARG_INT);
void place_group(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int unique_armour(ARG_INV_PTR);
int unique_weapon(ARG_INV_PTR);
int get_nmons_num(ARG_INT);
int place_ghost(ARG_VOID);
int summon_demon(ARG_INT ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_dragon(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_angel(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_spider(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_hound(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_wraith(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_gundead(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_reptile(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_ant(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_unique(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_jabberwock(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int summon_ancientd(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int lose_all_info(ARG_VOID);

/* misc2.c */
void scribe_object(ARG_VOID);  /* apperently moved from desc.c -CFT */
void check_strength(ARG_VOID); /* apparently moved from moria1.c -CFT */
void add_inscribe(ARG_INV_PTR ARG_COMMA ARG_INT8U);
	 /* apperently moved from desc.c -CFT */
void inscribe(ARG_INV_PTR ARG_COMMA ARG_CHAR_PTR);
	  /* apperently moved from desc.c -CFT */
void place_trap(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void place_rubble(ARG_INT ARG_COMMA ARG_INT);
void place_gold(ARG_INT ARG_COMMA ARG_INT);
int get_obj_num(ARG_INT ARG_COMMA ARG_INT);
void place_object(ARG_INT ARG_COMMA ARG_INT);
void alloc_object(ARG_INT_FN ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void random_object(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void cnv_stat(ARG_INT16U ARG_COMMA ARG_CHAR_PTR);
void prt_stat(ARG_INT);
void prt_field(ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int stat_adj(ARG_INT);
int chr_adj(ARG_VOID);
int con_adj(ARG_VOID);
char *title_string(ARG_VOID);
void prt_title(ARG_VOID);
void prt_level(ARG_VOID);
void prt_cmana(ARG_VOID);
void prt_mhp(ARG_VOID);
void prt_chp(ARG_VOID);
void prt_pac(ARG_VOID);
void prt_gold(ARG_VOID);
void prt_depth(ARG_VOID);
void prt_hunger(ARG_VOID);
void prt_blind(ARG_VOID);
void prt_confused(ARG_VOID);
void prt_afraid(ARG_VOID);
void prt_poisoned(ARG_VOID);
void prt_state(ARG_VOID);
void prt_speed(ARG_VOID);
void prt_study(ARG_VOID);
void prt_winner(ARG_VOID);
int16u modify_stat(ARG_INT ARG_COMMA ARG_INT16U);
void set_use_stat(ARG_INT);
int inc_stat(ARG_INT);
int dec_stat(ARG_INT);
int res_stat(ARG_INT);
void bst_stat(ARG_INT ARG_COMMA ARG_INT);
int tohit_adj(ARG_VOID);
int toac_adj(ARG_VOID);
int todis_adj(ARG_VOID);
int todam_adj(ARG_VOID);
void prt_stat_block(ARG_VOID);
void draw_cave(ARG_VOID);
void put_character(ARG_VOID);
void put_stats(ARG_VOID);
char *likert(ARG_INT ARG_COMMA ARG_INT);
void put_misc1(ARG_VOID);
void put_misc2(ARG_VOID);
void put_misc3(ARG_VOID);
void display_char(ARG_VOID);
void get_name(ARG_VOID);
void change_name(ARG_VOID);
void inven_destroy(ARG_INT);
void take_one_item(ARG_INV_PTR ARG_COMMA ARG_INV_PTR);
void inven_drop(ARG_INT ARG_COMMA ARG_INT);
int inven_damage(ARG_INT_FN ARG_COMMA ARG_INT);
int weight_limit(ARG_VOID);
int inven_check_num(ARG_INV_PTR);
int inven_check_weight(ARG_INV_PTR);
void check_strength(ARG_VOID);
int inven_carry(ARG_INV_PTR);
int spell_chance(ARG_INT);
void print_spells(ARG_INT_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT
		ARG_COMMA ARG_INT);
int get_spell(ARG_INT_PTR ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT_PTR ARG_COMMA ARG_INT_PTR ARG_COMMA
		ARG_CHAR_PTR ARG_COMMA ARG_INT);
void calc_spells(ARG_INT);
void gain_spells(ARG_VOID);
void calc_mana(ARG_INT);
void prt_experience(ARG_VOID);
void calc_hitpoints(ARG_VOID);
void insert_str(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR ARG_COMMA
		ARG_CHAR_PTR);
void insert_lnum(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR ARG_COMMA
		ARG_INT32 ARG_COMMA ARG_INT);
int enter_wiz_mode(ARG_VOID);
int attack_blows(ARG_INT ARG_COMMA ARG_INT_PTR);
int tot_dam(ARG_INV_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int critical_blow(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT);
int mmove(ARG_INT ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
int player_saves(ARG_VOID);
int find_range(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT_PTR ARG_COMMA
		ARG_INT_PTR);
void teleport(ARG_INT);
void check_view(ARG_VOID);
int special_place_object(ARG_INT ARG_COMMA ARG_INT);
void place_special(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT32U);
void prt_cut(ARG_VOID);
void prt_stun(ARG_VOID);
void cut_player(ARG_INT);
void stun_player(ARG_INT);
void special_random_object(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);


/* monsters.c */

/* moria1.c */
void change_speed(ARG_INT);
void py_bonuses(ARG_INV_PTR ARG_COMMA ARG_INT);
void calc_bonuses(ARG_VOID);
int show_inven(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT ARG_COMMA ARG_INT_FN);
char *describe_use(ARG_INT);
int show_equip(ARG_INT ARG_COMMA ARG_INT);
void takeoff(ARG_INT ARG_COMMA ARG_INT);
/* void check_strength(); apparently moved to misc2.c -CFT */
int verify(ARG_CHAR_PTR ARG_COMMA ARG_INT);
void inven_command(ARG_CHAR);
int get_item(ARG_INT_PTR ARG_COMMA ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT ARG_COMMA ARG_INT_FN);
/* void panel_bounds(); apperently moved to misc1.c -CFT */
/* int get_panel(); apperently moved to misc1.c -CFT */
/* int panel_contains(); apperently moved to misc1.c -CFT */
int no_light(ARG_VOID);
int get_dir(ARG_CHAR_PTR ARG_COMMA ARG_INT_PTR);
int get_alldir(ARG_CHAR_PTR ARG_COMMA ARG_INT_PTR);
void move_rec(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void light_room(ARG_INT ARG_COMMA ARG_INT);
void lite_spot(ARG_INT ARG_COMMA ARG_INT);
void move_light(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void disturb(ARG_INT ARG_COMMA ARG_INT);
void search_on(ARG_VOID);
void search_off(ARG_VOID);
void rest(ARG_VOID);
void rest_off(ARG_VOID);
int test_hit(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT ARG_COMMA ARG_INT);
void take_hit(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void change_trap(ARG_INT ARG_COMMA ARG_INT);
void search(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
/* void set_options(); apperently moved to misc1.c -CFT */
void find_init(ARG_INT);
void find_run(ARG_VOID);
void end_find(ARG_VOID);
void area_affect(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int minus_ac(ARG_INT32U);
void corrode_gas(ARG_CHAR_PTR);
void poison_gas(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void fire_dam(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void cold_dam(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void light_dam(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void acid_dam(ARG_INT ARG_COMMA ARG_CHAR_PTR);
void darken_room(ARG_INT ARG_COMMA ARG_INT);

/* moria2.c */
int cast_spell(ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT_PTR
		ARG_COMMA ARG_INT_PTR);
void delete_monster(ARG_INT);
void fix1_delete_monster(ARG_INT);
void fix2_delete_monster(ARG_INT);
int delete_object(ARG_INT ARG_COMMA ARG_INT);
int32u monster_death(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT32U ARG_COMMA ARG_INT32U ARG_COMMA ARG_INT32U);
int mon_take_hit(ARG_INT ARG_COMMA ARG_INT);
void move_char(ARG_INT ARG_COMMA ARG_INT);
void openobject(ARG_VOID);
void closeobject(ARG_VOID);
int twall(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void tunnel(ARG_INT);
void disarm_trap(ARG_VOID);
void look(ARG_VOID);
void throw_object(ARG_VOID);
void bash(ARG_VOID);
void delete_unique(ARG_VOID);
void carry(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void special_random_object(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void check_unique(ARG_MON_PTR);


#ifdef MSDOS
/* ms_misc.c */
char *getlogin(ARG_VOID);
void msdos_init(ARG_VOID);
void user_name(ARG_CHAR_PTR);
#ifdef __TURBOC__
void sleep(unsigned);
#else
unsigned int sleep(ARG_INT );
#endif
void error(ARG_CHAR_PTR ARG_COMMA ...);
void warn(ARG_CHAR_PTR ARG_COMMA ...);
void msdos_init(ARG_VOID);
void msdos_raw(ARG_VOID);
void msdos_noraw(ARG_VOID);
int bios_getch(ARG_VOID);
int msdos_getch(ARG_VOID);
void bios_clear(ARG_VOID);
/* void msdos_intro(void); old code... -CFT */
void bios_clear(ARG_VOID);
#endif

/* potions.c */
void quaff(ARG_VOID);

/* prayer.c */
void pray(ARG_VOID);

/* recall.c */
int bool_roff_recall(ARG_INT);
int roff_recall(ARG_INT);

/* rnd.c */
int32u get_rnd_seed(ARG_VOID);
void set_rnd_seed(ARG_INT32U);
int32 rnd(ARG_VOID);

/* rods.c */
void activate_rod(ARG_VOID);

/* save.c */
#ifdef MAC
int save_char(ARG_INT);
#else
int save_char(ARG_VOID);
#endif
int _save_char(ARG_CHAR_PTR);
int get_char(ARG_INT_PTR);

/* scrolls.c */
void read_scroll(ARG_VOID);

/* sets.c */
int set_room(ARG_INT);
int set_corr(ARG_INT);
int set_floor(ARG_INT);
int set_corrodes(ARG_INV_PTR);
int set_flammable(ARG_INV_PTR);
int set_frost_destroy(ARG_INV_PTR);
int set_acid_affect(ARG_INV_PTR);
int set_lightning_destroy(ARG_INV_PTR);
int set_null(ARG_INV_PTR);
int set_acid_destroy(ARG_INV_PTR);
int set_fire_destroy(ARG_INV_PTR);
int general_store(ARG_INT);
int armory(ARG_INT);
int weaponsmith(ARG_INT);
int temple(ARG_INT);
int alchemist(ARG_INT);
int magic_shop(ARG_INT);
#ifdef MAC
int store_buy(ARG_INT ARG_COMMA ARG_INT);
#endif

/* signals.c */
void nosignals(ARG_VOID);
void signals(ARG_VOID);
void init_signals(ARG_VOID);
void ignore_signals(ARG_VOID);
void default_signals(ARG_VOID);
void restore_signals(ARG_VOID);

/* spells.c */
void monster_name(ARG_CHAR_PTR ARG_COMMA ARG_MON_PTR ARG_COMMA ARG_CTR_PTR);
void lower_monster_name(ARG_CHAR_PTR ARG_COMMA ARG_MON_PTR ARG_COMMA ARG_CTR_PTR);
int sleep_monsters1(ARG_INT ARG_COMMA ARG_INT);
int detect_treasure(ARG_VOID);
int detect_object(ARG_VOID);
int detect_trap(ARG_VOID);
int detect_sdoor(ARG_VOID);
int detect_invisible(ARG_VOID);
int light_area(ARG_INT ARG_COMMA ARG_INT);
int unlight_area(ARG_INT ARG_COMMA ARG_INT);
void map_area(ARG_VOID);
int ident_spell(ARG_VOID);
int aggravate_monster(ARG_INT);
int trap_creation(ARG_VOID);
int door_creation(ARG_VOID);
int td_destroy(ARG_VOID);
int detect_monsters(ARG_VOID);
void light_line(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void starlite(ARG_INT ARG_COMMA ARG_INT);
int disarm_all(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void get_flags(ARG_INT ARG_COMMA ARG_INT32U_PTR ARG_COMMA ARG_INT32U_PTR ARG_COMMA ARG_INT_FN_PTR);
void fire_bolt(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_CHAR_PTR);
void fire_ball(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_CHAR_PTR);
void breath(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_CHAR_PTR ARG_COMMA ARG_INT);
int recharge(ARG_INT);
int hp_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int drain_life(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int speed_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int confuse_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int sleep_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int wall_to_mud(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int td_destroy2(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int poly_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int build_wall(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int clone_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void teleport_away(ARG_INT ARG_COMMA ARG_INT);
void teleport_to(ARG_INT ARG_COMMA ARG_INT);
int teleport_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
int mass_genocide(ARG_INT);
int genocide(ARG_INT);
int speed_monsters(ARG_INT);
int sleep_monsters2(ARG_VOID);
int mass_poly(ARG_VOID);
int detect_evil(ARG_VOID);
int hp_player(ARG_INT);
int cure_confusion(ARG_VOID);
int cure_blindness(ARG_VOID);
int cure_poison(ARG_VOID);
int remove_fear(ARG_VOID);
void earthquake(ARG_VOID);
int protect_evil(ARG_VOID);
void create_food(ARG_VOID);
int dispel_creature(ARG_INT32U ARG_COMMA ARG_INT);
int turn_undead(ARG_VOID);
void warding_glyph(ARG_VOID);
void lose_str(ARG_VOID);
void lose_int(ARG_VOID);
void lose_wis(ARG_VOID);
void lose_dex(ARG_VOID);
void lose_con(ARG_VOID);
void lose_chr(ARG_VOID);
void lose_exp(ARG_INT32);
int slow_poison(ARG_VOID);
void bless(ARG_INT);
void detect_inv2(ARG_INT);
void destroy_area(ARG_INT ARG_COMMA ARG_INT);
int enchant(ARG_INT16_PTR);
int remove_curse(ARG_VOID);
int restore_level(ARG_VOID);
int probing(ARG_VOID);
int detection(ARG_VOID);
void starball(ARG_INT ARG_COMMA ARG_INT);
int bolt(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_CHAR_PTR ARG_COMMA ARG_MON_PTR ARG_COMMA ARG_INT);
int chaos(ARG_MON_PTR);
int stair_creation(ARG_VOID);
void tele_level(ARG_VOID);
int detect_enchantment(ARG_VOID);
void identify_pack(ARG_VOID);
int banish_creature(ARG_INT32U ARG_COMMA ARG_INT);
int remove_all_curse(ARG_VOID);
char *pain_message(ARG_INT ARG_COMMA ARG_INT);
void self_knowledge(ARG_VOID);

/* staffs.c */
void use(ARG_VOID);

/* store1.c */
int32 item_value(ARG_INV_PTR);
int32 sell_price(ARG_INT ARG_COMMA ARG_INT32_PTR ARG_COMMA ARG_INT32_PTR ARG_COMMA ARG_INV_PTR);
int store_check_num(ARG_INV_PTR ARG_COMMA ARG_INT);
void store_carry(ARG_INT ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INV_PTR);
void store_destroy(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void store_init(ARG_VOID);
void store_maint(ARG_VOID);
int noneedtobargain(ARG_INT ARG_COMMA ARG_INT32);
void updatebargain(ARG_INT ARG_COMMA ARG_INT32 ARG_COMMA ARG_INT32);

/* store2.c */
void enter_store(ARG_INT);

#ifdef USING_TCIO
/* tcio.c */
#ifdef SIGTSTP
int suspend();
#endif
void init_curses(ARG_VOID);
/* void moriaterm(); old -CFT */
void put_buffer(ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void put_qio(ARG_VOID);
void restore_term(ARG_VOID);
void shell_out(ARG_VOID);
char inkey(ARG_VOID);
void flush(ARG_VOID);
void erase_line(ARG_INT ARG_COMMA ARG_INT);
void clear_screen(ARG_VOID);
void clear_from(ARG_INT);
void print(ARG_CHAR ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void move_cursor_relative(ARG_INT ARG_COMMA ARG_INT);
void count_msg_print(ARG_CHAR_PTR);
void prt(ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void move_cursor(ARG_INT ARG_COMMA ARG_INT);
void msg_print(ARG_CHAR_PTR);
int get_check(ARG_CHAR_PTR);
int get_com(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR);
int get_string(ARG_CHAR_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
void pause_line(ARG_INT);
void pause_exit(ARG_INT ARG_COMMA ARG_INT);
void save_screen(ARG_VOID);
void restore_screen(ARG_VOID);
void bell(ARG_VOID);
void screen_map(ARG_VOID);
#endif

/* treasur1.c */

/* treasur2.c */

/* undef.c */
void init_files(ARG_VOID);
int _new_log(ARG_VOID);

#ifdef unix
/* unix.c */
int check_input();
#if 0
int system_cmd();
#endif
void user_name();
int tilde();
/* only declare this if stdio.h has been previously included, which will
 be true if stdin is defined */
#ifdef stdin
FILE *tfopen();
#endif
int topen();
#endif

/* variable.c */

/* wands.c */
void aim(ARG_VOID);

/* wizard.c */
int is_wizard(ARG_INT);
void wizard_light(ARG_INT);
void change_character(ARG_VOID);
void wizard_create(ARG_VOID);
void artifact_check(ARG_VOID);

#endif

#ifdef MSDOS /* need to define delay() function somewhere : acts like
		usleep(), but with millisecs not usecs -CFT */
void delay(unsigned);

/* misc. prototypes... */
int abs(ARG_INT);
int close(ARG_INT);
void exit(ARG_INT);
#ifdef __TURBOC__
#define atoi(s) ((int) atol(s)) /* stolen form stdlib.h -CFT */
long atol(const char *);
int open(const char *, int, ...);
int sprintf(char *, const char *, ...);
#ifdef TC_COLOR
void textcolor(int);
#endif
#endif
#endif

#ifdef unix
/* call functions which expand tilde before calling open/fopen */
#define open topen
#define fopen tfopen
#endif
