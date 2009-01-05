#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

/*** Variables ***/

/** Maximum number of scroll titles generated */
#define MAX_TITLES     50

/** The titles of scrolls, ordered by sval. */
extern char scroll_adj[MAX_TITLES][16];


/*** Constants ***/

/**
 * Modes for object_desc().
 */
typedef enum
{
	ODESC_BASE = 0,   /*!< Only describe the base name */
	ODESC_COMBAT = 1, /*!< Also show combat bonuses */
	ODESC_FULL = 3,   /*!< Show entire description */
	ODESC_STORE = 4   /*!< Also show {squelch} marker */
} odesc_detail_t;


/**
 * Pseudo-ID markers.
 */
typedef enum
{
	INSCRIP_NULL = 0,            /*!< No pseudo-ID status */
	INSCRIP_TERRIBLE = 1,        /*!< Cursed artifact */
	INSCRIP_WORTHLESS = 2,       /*!< Worthless item */
	INSCRIP_CURSED = 3,          /*!< Cursed normal item */
	INSCRIP_STRANGE = 4,         /*!< Item that has mixed combat bonuses */
	INSCRIP_AVERAGE = 5,         /*!< Item with no interesting features */
	INSCRIP_MAGICAL = 6,         /*!< Item with combat bonuses */
	INSCRIP_EXCELLENT = 7,       /*!< Ego-item */
	INSCRIP_SPECIAL = 8,         /*!< Artifact */
	INSCRIP_UNCURSED = 9,        /*!< Item previous cursed, now uncursed */
	INSCRIP_INDESTRUCTIBLE = 10, /*!< Artifact that was tried to be destroyed */

	INSCRIP_MAX                  /*!< Maximum number of pseudo-ID markers */
} obj_pseudo_t;



/*** Functions ***/

/* obj-desc.c */
void object_kind_name(char *buf, size_t max, int k_idx, bool easy_know);
size_t object_desc(char *buf, size_t max, const object_type *o_ptr, bool prefix, odesc_detail_t mode);
void object_desc_spoil(char *buf, size_t max, const object_type *o_ptr, int pref, odesc_detail_t mode);

/* obj-info.c */
void object_info_header(const object_type *o_ptr);
bool object_info_known(const object_type *o_ptr);
bool object_info_chardump(const object_type *o_ptr);
bool object_info_full(const object_type *o_ptr);
bool object_info_store(const object_type *o_ptr);

/* obj-make.c */
void free_obj_alloc(void);
bool init_obj_alloc(void);
s16b get_obj_num(int level);
void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great);
bool make_object(object_type *j_ptr, int lev, bool good, bool great);
void make_gold(object_type *j_ptr, int lev, int coin_type);
int object_pseudo_heavy(const object_type *o_ptr);
int object_pseudo_light(const object_type *o_ptr);

/* obj-ui.c */
void display_inven(void);
void display_equip(void);
void show_inven(void);
void show_equip(void);
void show_floor(const int *floor_list, int floor_num, bool gold);
bool verify_item(cptr prompt, int item);
bool get_item(int *cp, cptr pmt, cptr str, int mode);

/* obj-util.c */
void flavor_init(void);
void reset_visuals(bool unused);
void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
void object_flags_known(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
char index_to_label(int i);
s16b label_to_inven(int c);
s16b label_to_equip(int c);
bool wearable_p(const object_type *o_ptr);
s16b wield_slot(const object_type *o_ptr);
const char *mention_use(int slot);
cptr describe_use(int i);
bool item_tester_okay(const object_type *o_ptr);
int scan_floor(int *items, int max_size, int y, int x, int mode);
void excise_object_idx(int o_idx);
void delete_object_idx(int o_idx);
void delete_object(int y, int x);
void compact_objects(int size);
void wipe_o_list(void);
s16b o_pop(void);
object_type *get_first_object(int y, int x);
object_type *get_next_object(const object_type *o_ptr);
void object_known(object_type *o_ptr);
void object_aware(object_type *o_ptr);
void object_tried(object_type *o_ptr);
void object_id_on_wield(object_type *o_ptr);
bool is_blessed(const object_type *o_ptr);
s32b object_value(const object_type *o_ptr, int qty);
bool object_similar(const object_type *o_ptr, const object_type *j_ptr);
void object_absorb(object_type *o_ptr, const object_type *j_ptr);
void object_wipe(object_type *o_ptr);
void object_copy(object_type *o_ptr, const object_type *j_ptr);
void object_prep(object_type *o_ptr, int k_idx);
s16b floor_carry(int y, int x, object_type *j_ptr);
void drop_near(object_type *j_ptr, int chance, int y, int x);
void acquirement(int y1, int x1, int level, int num, bool great);
void inven_item_charges(int item);
void inven_item_describe(int item);
void inven_item_increase(int item, int num);
void inven_item_optimize(int item);
void floor_item_charges(int item);
void floor_item_describe(int item);
void floor_item_increase(int item, int num);
void floor_item_optimize(int item);
bool inven_carry_okay(const object_type *o_ptr);
bool inven_stack_okay(const object_type *o_ptr);
s16b inven_carry(object_type *o_ptr);
s16b inven_takeoff(int item, int amt);
void inven_drop(int item, int amt);
void combine_pack(void);
void reorder_pack(void);
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
void reduce_charges(object_type *o_ptr, int amt);
unsigned check_for_inscrip(const object_type *o_ptr, const char *inscrip);
int lookup_kind(int tval, int sval);
bool lookup_reverse(s16b k_idx, int *tval, int *sval);
int lookup_name(int tval, const char *name);
int lookup_sval(int tval, const char *name);
int tval_find_idx(const char *name);
const char *tval_find_name(int tval);

#endif /* !INCLUDED_OBJECT_H */
