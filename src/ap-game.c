/**
 * \file ap-game.c
 * \brief Angband-side Archipelago glue: wires AP events to game state.
 *
 * See ap-game.h.  This file may use the game's headers freely; it talks to the
 * APCc client only through the dependency-free apinterface.h API.
 */
#include "angband.h"
#include "cave.h"
#include "effects.h"
#include "init.h"
#include "monster.h"
#include "mon-make.h"
#include "mon-util.h"
#include "object.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-slays.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "apinterface.h"
#include "ap-game.h"

#include <stdarg.h>

/*
 * Lightweight breadcrumb log for diagnosing intermittent connect crashes.
 * Appends to "ap-debug.log" next to the executable, flushing every line so the
 * last entry survives a hard crash.  The connect replay runs many handlers in
 * one drain; the final logged line pinpoints where a crash happened.
 */
static void ap_log(const char *fmt, ...)
{
	static FILE *fp = NULL;
	static bool tried = false;
	va_list args;

	if (!fp) {
		if (tried) return;
		tried = true;
		fp = fopen("ap-debug.log", "a");
		if (!fp) return;
		fprintf(fp, "---- ap log opened ----\n");
	}
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fputc('\n', fp);
	fflush(fp);
}

/* Set while killing the player in response to a received DeathLink, so we don't
 * bounce that same death back out as another DeathLink. */
static bool ap_dying_from_deathlink = false;

/*
 * Per-artifact "its location has been checked" flags, indexed by aidx (lazily
 * allocated, z_info->a_max entries).  Populated by ap_check_confirmed -- which
 * fires for both live checks and the on-connect replay -- so it reflects the
 * server's view of which artifact locations are already found.  Drives the Black
 * Market "buy a missed location" offer (ap_find_missed_location).  Not saved: a
 * reconnect's replay rebuilds it.
 */
static bool *ap_artifact_checked = NULL;

/** Ensure the checked-artifact table is allocated.  Returns false if it can't. */
static bool ap_ensure_artifact_table(void)
{
	if (!ap_artifact_checked && z_info && z_info->a_max)
		ap_artifact_checked = mem_zalloc(z_info->a_max * sizeof(bool));
	return ap_artifact_checked != NULL;
}

/* Format helper for accumulated "Find #X Artifacts" location names (below). */
static void ap_find_artifacts_location_name(unsigned n, char *buf, size_t len);

/*
 * Highest "Find #X Artifacts" milestone the server has confirmed for this slot,
 * learned from the on-connect replay (ap_check_confirmed).  This is the only
 * record of accumulated-mode progress that survives a death: aup_info[].ap_found
 * lives in the savefile, and a dead character's savefile is replaced by the new
 * one.  ap_sync_artifact_finds() reconciles the two on every connect.
 */
static unsigned ap_find_milestone_max = 0;

/*
 * Number of distinct artifacts found for the Accumulated-mode "Find #X
 * Artifacts" milestones.  The per-artifact flag lives in aup_info[].ap_found,
 * which is saved in the artifacts savefile block and -- unlike created -- is NOT
 * cleared when a new character is born, so found artifacts persist across a
 * retire/reincarnate within one session.  Across a death or a restart they are
 * recovered from the server instead (ap_credit_found_artifacts).
 */
static int ap_artifacts_found_count(void)
{
	int i, n = 0;

	if (!aup_info || !z_info) return 0;
	for (i = 1; i < z_info->a_max; i++)
		if (aup_info[i].ap_found) n++;
	return n;
}

/*
 * Proper-named artifacts are stored quoted in artifact.txt ('Narthanc'), but the
 * Archipelago location names drop the quotes ("Narthanc").  Copy art->name into
 * buf with any wrapping single quotes stripped, so it matches the apworld.
 */
static void ap_artifact_bare_name(const struct artifact *art, char *buf, size_t len)
{
	size_t n;

	if (!len) return;
	my_strcpy(buf, art->name ? art->name : "", len);

	n = strlen(buf);
	if (n >= 2 && buf[0] == '\'' && buf[n - 1] == '\'') {
		memmove(buf, buf + 1, n - 2);
		buf[n - 2] = '\0';
	}
}

/*
 * Clean singular base-kind name ("Phial", "Bastard Sword") for an artifact's
 * base object -- the form suffix artifacts use in their apworld location name.
 * This must go through object_kind_name(), NOT raw kind->name, because kind names
 * carry article/plural markers ("& Phial~") that object_desc strips; the apworld
 * used the stripped form (it equals the artifact.txt base-object sval name).
 */
static void ap_artifact_base_name(const struct artifact *art, char *buf, size_t len)
{
	struct object_kind *kind = lookup_kind(art->tval, art->sval);

	if (kind && kind->name)
		object_kind_name(buf, len, kind, true);
	else if (len)
		buf[0] = '\0';
}

/*
 * Send the location check for an artifact.  Its apworld location name is one of
 * two forms and we can't tell which from here, so send both: the bare (de-
 * quoted) name -- "Narthanc", "Angrist" -- and base-kind + name -- "Phial of
 * Galadriel".  ap_send_check() drops whichever isn't a real location.
 */
static void ap_send_artifact_check(const struct artifact *art)
{
	char bare[120], base[120], full[120];

	if (!art || !art->name) return;

	ap_artifact_bare_name(art, bare, sizeof(bare));
	ap_send_check(bare);

	ap_artifact_base_name(art, base, sizeof(base));
	if (base[0]) {
		strnfmt(full, sizeof(full), "%s %s", base, bare);
		ap_send_check(full);
	}
}

/**
 * Does \p name match \p art's Archipelago location name?  Compares against both
 * naming forms (see ap_send_artifact_check).
 */
static bool ap_artifact_loc_matches(const struct artifact *art, const char *name)
{
	char bare[120], base[120], full[120];

	if (!art->name) return false;

	ap_artifact_bare_name(art, bare, sizeof(bare));
	if (streq(name, bare)) return true;

	ap_artifact_base_name(art, base, sizeof(base));
	if (base[0]) {
		strnfmt(full, sizeof(full), "%s %s", base, bare);
		if (streq(name, full)) return true;
	}

	return false;
}

/** Record that the artifact location named \p name (if any) is now checked. */
static void ap_mark_artifact_location_checked(const char *name)
{
	int i;

	if (!ap_ensure_artifact_table()) return;

	for (i = 1; i < z_info->a_max; i++) {
		const struct artifact *art = &a_info[i];
		if (art->name && ap_artifact_loc_matches(art, name)) {
			ap_artifact_checked[art->aidx] = true;
			/*
			 * Mirror keeping checked uniques dead: once a location is checked,
			 * suppress the dungeon's attributeless placeholder for it so we
			 * don't keep generating valueless items (re-checking is a no-op).
			 * AAC mode only -- outside it the natural artifact is the real
			 * reward and must still be allowed to spawn.
			 */
			if (ap_artifacts_as_checks())
				mark_artifact_created(art, true);
			return;
		}
	}
}

/*
 * If \p race is a quest monster (Sauron, Morgoth), mark its quest complete for
 * the current character.  A quest level generates no normal down-stairs -- the
 * way down is the magical staircase quest_check() builds on killing the quest
 * monster.  But on a respawn that monster is already dead (max_num == 0 from the
 * replay) and never spawns, so a fresh character could never finish the quest
 * and would be soft-locked above the next level (notably stuck on 99, unable to
 * reach Morgoth on 100).  Quietly completing the quest here -- setting the
 * fields, NOT calling quest_check() -- unblocks descent without triggering the
 * build-stairs message or total_winner.
 */
static void ap_complete_quest_for(const struct monster_race *race)
{
	size_t i;

	if (!player || !player->quests) return;

	for (i = 0; i < z_info->quest_max; i++) {
		struct quest *q = &player->quests[i];

		if (q->level && q->race == race) {
			q->cur_num = q->max_num;
			q->level = 0;
		}
	}
}

/**
 * A location check was confirmed for our slot.  This fires both for live checks
 * and -- crucially -- for the replay of every previously-checked location right
 * after connecting.  Locations are named after unique monsters and artifacts;
 * for a unique we zero its max_num so it stays dead (including for a brand-new
 * character reincarnating into the same AP slot) and complete any quest it
 * gates, and for an artifact we note its location as found so the Black Market
 * won't re-offer it.
 */
/*
 * Remove any live instances of a now-permanently-dead unique from the current
 * level.  The replay marks uniques dead (max_num = 0) on the first turn after
 * connecting, but a fresh character's town level has already been generated by
 * then, so a depth-0 town unique (Farmer Maggot, Grip, Fang) may have spawned as
 * a resident before the replay ran.  Generation respects max_num, but an
 * already-placed monster isn't retroactively removed -- so reap it here.
 */
static void ap_reap_dead_unique(const struct monster_race *race)
{
	int i;

	if (!cave) return;
	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		struct monster *mon = cave_monster(cave, i);

		if (mon && mon->race == race) {
			ap_log("  reaping live '%s' (m_idx %d)", race->name, i);
			delete_monster_idx(cave, i);
		}
	}
}

/*
 * If \p name is an accumulated-mode "Find #X Artifacts" location, store X in
 * \p n_out and return true.  The digits are parsed and then the name is rebuilt
 * from them and compared, so a location that merely starts with "Find " (or one
 * with leading zeroes) can never be mistaken for a milestone.
 */
static bool ap_find_milestone_number(const char *name, unsigned *n_out)
{
	char expect[64];
	const char *p = name;
	unsigned n = 0;

	if (!name || !prefix(name, "Find ")) return false;
	p += sizeof("Find ") - 1;
	if (!isdigit((unsigned char)*p)) return false;
	for (; isdigit((unsigned char)*p); p++)
		n = n * 10 + (unsigned)(*p - '0');

	ap_find_artifacts_location_name(n, expect, sizeof(expect));
	if (!streq(name, expect)) return false;

	*n_out = n;
	return true;
}

static void ap_check_confirmed(const char *name)
{
	struct monster_race *race = lookup_monster(name);
	unsigned milestone;

	ap_log("check_confirmed: %s", name ? name : "(null)");
	if (race) {
		/* Killed for good: prevents this unique from being generated again. */
		if (rf_has(race->flags, RF_UNIQUE)) {
			race->max_num = 0;
			ap_complete_quest_for(race);
			ap_reap_dead_unique(race);
		}
		return;
	}

	/* Accumulated mode: the artifact locations are count milestones.  Remember
	 * the highest one the server has, so a fresh character can inherit the
	 * slot's progress (ap_sync_artifact_finds). */
	if (ap_find_milestone_number(name, &milestone)) {
		if (milestone > ap_find_milestone_max)
			ap_find_milestone_max = milestone;
		return;
	}

	ap_mark_artifact_location_checked(name);
}

/*** Archipelago item delivery: name -> game effect ***/

/*
 * The 23 item names below are frozen: they are the data-package contents the
 * apworld generates (see angband/data.py).  They split into:
 *   - 13 "Progressive <Category> Artifact": the Nth grant of a category yields
 *     the Nth artifact of that category ordered by spawn depth (alloc_min, then
 *     cost; see ap_art_cmp);
 *   - 2 specific named artifacts (the diggers Pick of Erebor / Mattock of Náin);
 *   - 5 consumables placed in the home;
 *   - 3 non-object boons (gold, experience, expanded shop books).
 */

/** Progressive artifact categories, in the order their items are numbered. */
enum ap_prog_cat {
	AP_CAT_LIGHT, AP_CAT_BLADED, AP_CAT_BLUNT, AP_CAT_POLEARM, AP_CAT_RANGED,
	AP_CAT_BOOTS, AP_CAT_HELM, AP_CAT_ARMOR, AP_CAT_CLOAK, AP_CAT_GLOVES,
	AP_CAT_SHIELD, AP_CAT_RING, AP_CAT_AMULET, AP_CAT_MAX
};

/** Maps each progressive item name to the object tvals that make up the category. */
static const struct {
	const char *item_name;
	int tvals[3];	/* TV_NULL (0) terminated */
} ap_prog_cats[AP_CAT_MAX] = {
	{ "Progressive Light Artifact",         { TV_LIGHT } },
	{ "Progressive Bladed Weapon Artifact", { TV_SWORD } },
	{ "Progressive Blunt Weapon Artifact",  { TV_HAFTED } },
	{ "Progressive Polearm Weapon Artifact",{ TV_POLEARM } },
	{ "Progressive Ranged Weapon Artifact", { TV_BOW } },
	{ "Progressive Boots Artifact",         { TV_BOOTS } },
	{ "Progressive Helm Artifact",          { TV_HELM, TV_CROWN } },
	{ "Progressive Armor Artifact",         { TV_SOFT_ARMOR, TV_HARD_ARMOR, TV_DRAG_ARMOR } },
	{ "Progressive Cloak Artifact",         { TV_CLOAK } },
	{ "Progressive Gloves Artifact",        { TV_GLOVES } },
	{ "Progressive Shield Artifact",        { TV_SHIELD } },
	{ "Progressive Ring Artifact",          { TV_RING } },
	{ "Progressive Amulet Artifact",        { TV_AMULET } },
};

/** Consumable items, with the object kind and stack size to place in the home. */
static const struct {
	const char *name;
	int tval;
	const char *sval;	/* kind name, as looked up by lookup_sval() */
	int qty;
} ap_consumables[] = {
	{ "2 Potions of Augmentation", TV_POTION, "Augmentation",              2 },
	{ "Rod of Recall",             TV_ROD,    "Recall",                    1 },
	{ "Scroll of Acquirement",     TV_SCROLL, "Acquirement",               1 },
	{ "Scroll of Deep Descent",    TV_SCROLL, "Deep Descent",              1 },
	{ "Piece of Elvish Waybread",  TV_FOOD,   "Piece of Elvish Waybread",  1 },
};

/*
 * Transient, game-thread-only count of how many of each progressive category
 * we have seen so far in the current item stream.  Reset at the start of every
 * replay (index 0) so the Nth-artifact mapping is recomputed deterministically
 * from the top, even across the per-character de-duplication.
 */
static int ap_prog_count[AP_CAT_MAX];

/*
 * Transient count of "Expanded Starting Shop Books" grants seen this stream:
 * the Kth grant stocks the bookseller's Kth dungeon book (in sval order).  Like
 * the progressive counters this is reset at index 0 and re-counted every replay.
 */
static int ap_books_count;

/*
 * Set when a home/inventory item this stream couldn't be placed (everything
 * full): the contiguous high-water mark stops there and the remaining items are
 * left for a later replay, so nothing is silently dropped or delivered twice.
 */
static bool ap_blocked;

/** Return the progressive category for an item name, or -1 if it isn't one. */
static int ap_prog_cat_of(const char *name)
{
	int c;

	for (c = 0; c < AP_CAT_MAX; c++)
		if (streq(name, ap_prog_cats[c].item_name)) return c;
	return -1;
}

/**
 * qsort comparator ordering artifacts weakest-first for progressive grants:
 * native spawn depth (alloc_min) first, ties broken by cost, then aidx for a
 * stable/deterministic result.  NB: NOT art->level -- that is the activation
 * *difficulty*, which is meaningless for non-activating artifacts and unrelated
 * to power even for activating ones (e.g. Narya's low-level fire bolt vs its
 * deep, powerful ring), so it put deep rings like Narya/Nenya before shallow
 * ones like the Ring of Barahir.
 */
static int ap_art_cmp(const void *a, const void *b)
{
	const struct artifact *x = *(const struct artifact * const *)a;
	const struct artifact *y = *(const struct artifact * const *)b;

	if (x->alloc_min != y->alloc_min) return x->alloc_min - y->alloc_min;
	if (x->cost != y->cost) return x->cost - y->cost;
	return (int)x->aidx - (int)y->aidx;
}

/** The \p n-th (1-based) artifact of progressive category \p cat, or NULL. */
static const struct artifact *ap_nth_cat_artifact(int cat, int n)
{
	const struct artifact **list;
	const struct artifact *result = NULL;
	int count = 0, i, t;

	if (cat < 0 || cat >= AP_CAT_MAX || n < 1) return NULL;

	list = mem_zalloc(z_info->a_max * sizeof(*list));
	for (i = 0; i < z_info->a_max; i++) {
		const struct artifact *art = &a_info[i];

		if (!art->name) continue;
		for (t = 0; t < 3 && ap_prog_cats[cat].tvals[t]; t++) {
			if (art->tval == ap_prog_cats[cat].tvals[t]) {
				list[count++] = art;
				break;
			}
		}
	}

	sort(list, count, sizeof(*list), ap_art_cmp);
	if (n <= count) result = list[n - 1];
	mem_free(list);
	return result;
}

/** Set up an object's "known" twin the way stores do, ready to be carried. */
static void ap_object_know(struct object *obj)
{
	obj->known = object_new();
	obj->known->notice |= OBJ_NOTICE_ASSESSED;
	object_set_base_known(player, obj);
	obj->known->notice |= OBJ_NOTICE_ASSESSED;
	/*
	 * Mark the known twin as the artifact, exactly as object_touch() does on a
	 * normal pickup: object_is_known_artifact() (hence object_desc showing the
	 * artifact's name) keys off obj->known->artifact, not obj->artifact.  NULL
	 * for non-artifacts, so this is a no-op for ordinary granted items.
	 */
	obj->known->artifact = obj->artifact;
	player_know_object(player, obj);
	/*
	 * Auto-identify granted consumables: make the flavour aware so a delivered
	 * potion/scroll/rod (Augmentation, Acquirement, Deep Descent, Recall, ...)
	 * is immediately recognised instead of arriving as an unknown flavour.
	 */
	if (obj->kind->flavor)
		object_flavor_aware(player, obj);
	/*
	 * Tag AP rewards with a dedicated origin (persists through drop/pickup and
	 * save/load).  For artifacts this lets pickup tell the granted reward apart
	 * from the dungeon's attributeless location-check copy, so re-picking-up a
	 * granted artifact does NOT falsely send its location check.
	 */
	obj->origin = ORIGIN_ARCHIPELAGO;
}

/** Build a known object of \p kind in a stack of \p qty. */
static struct object *ap_make_kind_object(struct object_kind *kind, int qty)
{
	struct object *obj;

	if (!kind) return NULL;

	obj = object_new();
	object_prep(obj, kind, 0, RANDOMISE);
	obj->number = MAX(1, qty);
	ap_object_know(obj);
	return obj;
}

/** Build a fully-realised artifact object (the real reward, not the check). */
static struct object *ap_make_artifact_object(const struct artifact *art)
{
	struct object_kind *kind;
	struct object *obj;

	if (!art) return NULL;
	kind = lookup_kind(art->tval, art->sval);
	if (!kind) return NULL;

	obj = object_new();
	object_prep(obj, kind, art->alloc_min, MAXIMISE);
	obj->artifact = art;
	copy_artifact_data(obj, art);
	/*
	 * Deliberately NOT mark_artifact_created(): in artifacts-as-checks mode
	 * the attributeless natural copy must still be allowed to spawn so it can
	 * be picked up as the location check.  This granted copy is the separate,
	 * real reward.
	 */
	ap_object_know(obj);
	return obj;
}

/**
 * Turn an Archipelago item name into a game object to place in the home, or
 * NULL if the name isn't a home-delivered item (a boon, or unmapped).  \p prog_n
 * is the 1-based progressive index for "Progressive <Category> Artifact" names.
 */
static struct object *ap_make_item(const char *name, int prog_n)
{
	const struct artifact *art;
	int cat = ap_prog_cat_of(name);
	size_t i;

	/* Progressive artifact: the Nth of its category by depth. */
	if (cat >= 0)
		return ap_make_artifact_object(ap_nth_cat_artifact(cat, prog_n));

	/* Consumables and the filler. */
	for (i = 0; i < N_ELEMENTS(ap_consumables); i++) {
		if (streq(name, ap_consumables[i].name)) {
			int tval = ap_consumables[i].tval;
			int sval = lookup_sval(tval, ap_consumables[i].sval);
			struct object_kind *kind = (sval >= 0) ?
				lookup_kind(tval, sval) : NULL;
			return ap_make_kind_object(kind, ap_consumables[i].qty);
		}
	}

	/* Any name that is exactly an artifact (the specific diggers). */
	art = lookup_artifact_name(name);
	if (art && art->name && streq(art->name, name))
		return ap_make_artifact_object(art);

	return NULL;
}

/**
 * Place a granted object: into the pack if the player is down in the dungeon
 * and there is room, otherwise into the home.  Returns false only if it could
 * go nowhere (the home was full too), leaving the caller to retry it later.
 */
static bool ap_place_object(struct object *obj, const char *name)
{
	struct store *home = &stores[f_info[FEAT_HOME].shopnum - 1];

	if (player->depth > 0 && inven_carry_okay(obj)) {
		inven_carry(player, obj, true, false);
		msg("AP: '%s' arrives in your pack.", name);
		return true;
	}

	if (store_check_num(home, obj)) {
		home_carry(obj);
		msg("AP: '%s' delivered to your home.", name);
		return true;
	}

	return false;
}

/**
 * Deliver a single granted home/inventory item.  Returns false if the item is
 * real but couldn't be placed anywhere (so the caller should not advance past
 * it); true otherwise (placed, or nothing to place).
 */
static bool ap_deliver_item(const char *name, int prog_n)
{
	struct object *obj = ap_make_item(name, prog_n);

	if (!obj) {
		msg("AP: '%s' has no delivery mapping.", name);
		return true;
	}

	if (ap_place_object(obj, name))
		return true;

	/* No room anywhere: drop this copy; the replay remakes it next connect. */
	if (obj->known) object_delete(NULL, NULL, &obj->known);
	obj->known = NULL;
	object_delete(NULL, NULL, &obj);
	return false;
}

/**
 * Triple-gold boon: multiply the current purse by three.  Multiplicative and
 * stacking, so successive grants give 3x, 9x, 27x, ... -- applied once per
 * character (gated by the high-water mark), so a fresh or respawned character
 * gets the full multiplier while a reconnecting one is not multiplied again.
 */
static void ap_boon_triple_gold(void)
{
	int32_t cap = (int32_t)((1UL << 31) - 1);
	int64_t total = (int64_t)player->au * 3;

	player->au = (total > cap) ? cap : (int32_t)total;
	player->upkeep->redraw |= PR_GOLD;
	msg("AP: Your gold is tripled!");
}

/** Experience boon: jump the character up \p levels from wherever it is now. */
static void ap_boon_extra_levels(int levels)
{
	int old_lev = player->lev;
	int target = old_lev + levels;
	int32_t need;

	if (target > PY_MAX_LEVEL) target = PY_MAX_LEVEL;
	if (target < 2 || target <= old_lev) return;

	/* Experience needed to sit at the start of the target level. */
	need = (int32_t)(player_exp[target - 2] * player->expfact / 100L);
	if (need > player->exp)
		player_exp_gain(player, need - player->exp);

	msg("AP: +%d level(s) of experience (now level %d).",
		player->lev - old_lev, player->lev);
}

/** The \p n-th (1-based) dungeon spellbook of a book \p tval, in sval order. */
static struct object_kind *ap_nth_dungeon_book(int tval, int n)
{
	struct object_base *base = &kb_info[tval];
	int sval, rank = 0;

	for (sval = 1; sval <= base->num_svals; sval++) {
		struct object_kind *kind = lookup_kind(tval, sval);
		const struct class_book *book;

		if (!kind) continue;
		book = object_kind_to_book(kind);
		if (!book || !book->dungeon) continue;
		if (++rank == n) return kind;
	}

	return NULL;
}

/**
 * Append \p kind to a store's always-stock list (idempotent) and put one in the
 * stock right now, so it shows up without waiting for the next day's turnover.
 * Returns true if the kind was newly added.
 */
static bool ap_store_always_add(struct store *s, struct object_kind *kind)
{
	struct object *obj;
	size_t i;

	if (!s || !kind) return false;

	for (i = 0; i < s->always_num; i++)
		if (s->always_table[i] == kind) return false;	/* already stocked */

	if (!s->always_num) {
		s->always_size = 8;
		s->always_table = mem_zalloc(s->always_size * sizeof(*s->always_table));
	} else if (s->always_num >= s->always_size) {
		s->always_size += 8;
		s->always_table = mem_realloc(s->always_table,
			s->always_size * sizeof(*s->always_table));
	}
	s->always_table[s->always_num++] = kind;

	obj = ap_make_kind_object(kind, 1);
	if (obj && !store_carry(s, obj)) {
		object_delete(NULL, NULL, &obj->known);
		obj->known = NULL;
		object_delete(NULL, NULL, &obj);
	}
	return true;
}

/**
 * Expanded-shop-books boon: each grant adds one more tier of dungeon books to
 * the town bookseller -- the \p n-th dungeon book (sval order) of every realm.
 * The bookseller's stock is rebuilt from store.txt every launch, so this is
 * re-applied (idempotently) for every received copy on every connect/replay.
 */
static void ap_boon_expanded_books(int n)
{
	static const int book_tvals[] = {
		TV_MAGIC_BOOK, TV_PRAYER_BOOK, TV_NATURE_BOOK, TV_SHADOW_BOOK
	};
	struct store *book = &stores[f_info[FEAT_STORE_BOOK].shopnum - 1];
	bool any = false;
	size_t k;

	for (k = 0; k < N_ELEMENTS(book_tvals); k++) {
		struct object_kind *kind = ap_nth_dungeon_book(book_tvals[k], n);
		if (kind && ap_store_always_add(book, kind)) any = true;
	}

	if (any) msg("AP: The bookseller stocks deeper spellbooks (tier %d).", n);
}

/**
 * Boons that touch state rebuilt from scratch every launch (the town
 * bookseller).  Must be re-applied on every replay, not just first receipt;
 * each is idempotent.  Returns true if \p name was such a boon.
 */
static bool ap_apply_replay_boon(const char *name)
{
	if (streq(name, "Expanded Starting Shop Books")) {
		ap_boon_expanded_books(++ap_books_count);
		return true;
	}
	return false;
}

/**
 * Boons that mutate saved player state (gold, experience) and so must be
 * applied exactly once per character.  Returns true if \p name was such a boon.
 */
static bool ap_apply_oneshot_boon(const char *name)
{
	if (streq(name, "Triple Starting Gold")) {
		ap_boon_triple_gold();
		return true;
	}
	if (streq(name, "+5 levels of Experience")) {
		ap_boon_extra_levels(5);
		return true;
	}
	return false;
}

/*** Resistances trait modes: the Archipelago Weapon and trait items ***/

/*
 * In the Equipment/Full Traits modes (slot_data "resistances" 1/2) the player
 * wields an unremovable Archipelago Blade and the multiworld grants trait items
 * that improve it.  Weapon field mutations (resists, flags, modifiers, dice,
 * to-hit/dam/AC, weight) persist in the savefile per object, so they are applied
 * once per character, gated by the item high-water mark -- a respawned character
 * gets a fresh blade and the full replay rebuilds it.  The elemental Attunement
 * activation and the Full-Traits race boosts are NOT persistent (obj->effect is
 * re-derived from the kind on load; the race struct is shared), so they are
 * recomputed from the replay on every connect.
 */

/* Progressive elemental resist categories, in fixed order. */
enum ap_trait_elem { AP_TE_FIRE, AP_TE_COLD, AP_TE_ELEC, AP_TE_ACID, AP_TE_MAX };

static const struct {
	const char *item_name;
	int elem;           /* ELEM_* the resist applies to */
	int tmd;            /* TMD_OPP_* for the attunement activation */
} ap_trait_elems[AP_TE_MAX] = {
	{ "Progressive Fire Resistance",      ELEM_FIRE, TMD_OPP_FIRE },
	{ "Progressive Cold Resistance",      ELEM_COLD, TMD_OPP_COLD },
	{ "Progressive Lightning Resistance", ELEM_ELEC, TMD_OPP_ELEC },
	{ "Progressive Acid Resistance",      ELEM_ACID, TMD_OPP_ACID },
};

/* Single-item resistances: name -> ELEM_*. */
static const struct { const char *name; int elem; } ap_trait_resists[] = {
	{ "Poison Resistance",         ELEM_POIS },
	{ "Light Resistance",          ELEM_LIGHT },
	{ "Dark Resistance",           ELEM_DARK },
	{ "Sound Resistance",          ELEM_SOUND },
	{ "Shards Resistance",         ELEM_SHARD },
	{ "Nexus Resistance",          ELEM_NEXUS },
	{ "Nether Resistance",         ELEM_NETHER },
	{ "Chaos Resistance",          ELEM_CHAOS },
	{ "Disenchantment Resistance", ELEM_DISEN },
};

/* Single-item object flags: name -> OF_* list (0-terminated). */
static const struct { const char *name; int flags[6]; } ap_trait_flags[] = {
	{ "Blindness Resistance", { OF_PROT_BLIND } },
	{ "Confusion Resistance", { OF_PROT_CONF } },
	{ "Fear Resistance",      { OF_PROT_FEAR } },
	{ "Stun Resistance",      { OF_PROT_STUN } },
	{ "Free Action",          { OF_FREE_ACT } },
	{ "Hold Life",            { OF_HOLD_LIFE } },
	{ "See Invisible",        { OF_SEE_INVIS } },
	{ "Telepathy",            { OF_TELEPATHY } },
	{ "Regeneration",         { OF_REGEN } },
	{ "Feather Fall",         { OF_FEATHER } },
	{ "All Sustains",         { OF_SUST_STR, OF_SUST_INT, OF_SUST_WIS,
	                            OF_SUST_DEX, OF_SUST_CON } },
};

/* Full-Traits race boosts: name -> which array/how much. */
static const struct { const char *name; int stat; } ap_trait_stats[] = {
	{ "Progressive Strength Boost",     STAT_STR },
	{ "Progressive Intelligence Boost", STAT_INT },
	{ "Progressive Wisdom Boost",       STAT_WIS },
	{ "Progressive Dexterity Boost",    STAT_DEX },
	{ "Progressive Constitution Boost", STAT_CON },
};

/* Transient trait counters, reset at replay index 0 (like ap_prog_count). */
static int ap_trait_elem_count[AP_TE_MAX];
static int ap_trait_slay_count;

/* Full-Traits race boosts, replay-derived (reset at index 0). */
static int ap_race_stat_adj[STAT_MAX];
static int ap_race_skill_adj[SKILL_MAX];
static int ap_race_infra_adj;
static int ap_race_hitdie_adj;

int ap_race_boost_stat(int stat)
{
	return (stat >= 0 && stat < STAT_MAX) ? ap_race_stat_adj[stat] : 0;
}

int ap_race_boost_skill(int skill)
{
	return (skill >= 0 && skill < SKILL_MAX) ? ap_race_skill_adj[skill] : 0;
}

int ap_race_boost_infra(void)
{
	return ap_race_infra_adj;
}

int ap_race_boost_hitdie(void)
{
	return ap_race_hitdie_adj;
}

/*
 * The Elemental Attunement activation: a C-owned effect chain of TIMED_INC
 * entries, relinked as elements reach progressive level 2.  Attached to the
 * blade's obj->effect (the kind has none); duration is effectively "until you
 * leave the level" and there is no recharge time, so it can be re-invoked at
 * will.  Rebuilt on every connect/replay -- obj->effect is re-derived from the
 * kind on savefile load, so the attachment never persists (nor needs to).
 */
static struct effect ap_attune_effects[AP_TE_MAX];
static bool ap_attune_init_done;

static void ap_attune_init(void)
{
	int i;

	if (ap_attune_init_done) return;
	for (i = 0; i < AP_TE_MAX; i++) {
		struct effect *e = &ap_attune_effects[i];

		memset(e, 0, sizeof(*e));
		e->index = EF_TIMED_INC;
		e->subtype = ap_trait_elems[i].tmd;
		e->dice = dice_new();
		dice_parse_string(e->dice, "5000");
	}
	ap_attune_init_done = true;
}

/** The wielded Archipelago Blade, or NULL. */
static struct object *ap_trait_weapon(void)
{
	struct object *obj = equipped_item_by_slot_name(player, "weapon");

	if (obj && obj->origin == ORIGIN_ARCHIPELAGO && of_has(obj->flags, OF_STICKY))
		return obj;
	return NULL;
}

/** Relink the attunement chain to the unlocked elements and (re)attach it. */
static void ap_attune_refresh(void)
{
	struct object *weapon = ap_trait_weapon();
	struct effect *head = NULL, **tail = &head;
	int i;

	if (!weapon) return;
	ap_attune_init();

	for (i = 0; i < AP_TE_MAX; i++) {
		if (ap_trait_elem_count[i] >= 2) {
			*tail = &ap_attune_effects[i];
			tail = &ap_attune_effects[i].next;
		}
	}
	*tail = NULL;

	weapon->effect = head;
	if (weapon->known) weapon->known->effect = head;
}

/** Look up a slay (or, with \p brand true, a brand) by its gamedata code. */
static int ap_slay_index(const char *code, bool brand)
{
	int i, max = brand ? z_info->brand_max : z_info->slay_max;

	for (i = 1; i < max; i++) {
		const char *c = brand ? brands[i].code : slays[i].code;
		if (c && streq(c, code)) return i;
	}
	return -1;
}

static void ap_weapon_add_slay(struct object *weapon, const char *code,
		bool brand)
{
	int idx = ap_slay_index(code, brand);

	if (idx < 0) return;
	if (brand)
		append_brand(&weapon->brands, idx);
	else
		append_slay(&weapon->slays, idx);
	if (weapon->known) {
		if (brand)
			append_brand(&weapon->known->brands, idx);
		else
			append_slay(&weapon->known->slays, idx);
	}
}

/** Note a change to the player's kit/race and get everything recalculated. */
static void ap_trait_update(void)
{
	if (!player || !player->upkeep) return;
	player->upkeep->update |= (PU_BONUS | PU_HP | PU_INVEN);
	player->upkeep->redraw |= (PR_MISC | PR_STATS | PR_SPEED | PR_ARMOR);
}

/*
 * Reveal every rune the blade currently has so its granted traits show up as
 * known (on the character sheet and when inspecting the weapon), rather than as
 * unidentified runes.  Learning a rune is global, so on the first grant the
 * player gets a "you have learned the rune of ..." message and it stays known.
 */
static void ap_weapon_learn(struct object *weapon)
{
	int guard = 0;

	if (!weapon || !player) return;
	while (!object_runes_known(weapon) && guard++ < 256)
		object_learn_unknown_rune(player, weapon);
	if (weapon->known) weapon->known->notice |= OBJ_NOTICE_ASSESSED;
	player_know_object(player, weapon);
}

/**
 * Make sure the trait-mode kit is in place: the Archipelago Blade wielded (and
 * unremovable), everything except a launcher and a light unequipped, and -- in
 * Full Traits -- the player switched to the Archipelago race.  Called on
 * connect, and lazily before applying a weapon trait.  Idempotent.
 */
static void ap_ensure_trait_weapon(void)
{
	struct object *obj;
	struct object_kind *kind;
	int sval, i;

	if (!ap_trait_mode() || !player || !player->body.slots) return;

	ap_log("ensure_trait_weapon: mode=%d depth=%d", ap_resistances_mode(),
		player->depth);

	/* Full Traits: force the Archipelago race (birth predates slot_data). */
	if (ap_resistances_mode() == 2 && player->race
			&& !streq(player->race->name, "Archipelago")) {
		struct player_race *r;

		for (r = races; r; r = r->next) {
			if (streq(r->name, "Archipelago")) {
				player->race = r;
				msg("AP: You are remade as an Archipelago being.");
				ap_trait_update();
				break;
			}
		}
	}

	/*
	 * Free the melee-weapon slot for the Archipelago Blade (any other
	 * equipment -- armour, launcher, light -- is fine to keep on).
	 */
	for (i = 0; i < player->body.count; i++) {
		obj = slot_object(player, i);
		if (!obj) continue;
		if (!slot_type_is(player, i, EQUIP_WEAPON)) continue;
		if (obj->origin == ORIGIN_ARCHIPELAGO
				&& of_has(obj->flags, OF_STICKY)) continue;
		inven_takeoff(obj);
		ap_trait_update();
	}

	if (ap_trait_weapon()) {
		ap_attune_refresh();
		return;
	}

	/* Forge and wield the blade. */
	sval = lookup_sval(TV_SWORD, "Archipelago Blade");
	kind = (sval >= 0) ? lookup_kind(TV_SWORD, sval) : NULL;
	if (!kind) {
		msg("AP: Archipelago Blade kind is missing from object.txt!");
		return;
	}

	ap_log("  forging Archipelago Blade");
	obj = object_new();
	object_prep(obj, kind, 0, MINIMISE);
	of_on(obj->flags, OF_STICKY);
	ap_object_know(obj);	/* sets ORIGIN_ARCHIPELAGO + the known twin */
	inven_carry(player, obj, false, false);
	inven_wield(obj, wield_slot(obj));
	msg("AP: An Archipelago Blade binds itself to your hands.");
	ap_attune_refresh();
	ap_trait_update();
	ap_log("  blade wielded");
}

/**
 * Apply one trait item.  \p is_new is false when the grant is at/under the
 * high-water mark (a replayed duplicate): persistent weapon mutations are then
 * skipped, but counters and the replay-derived state (attunement, race boosts)
 * still run so they are rebuilt identically every replay.
 * Returns true if \p name was a trait item.
 */
static bool ap_apply_trait_item(const char *name, bool is_new)
{
	struct object *weapon;
	size_t i;
	int e;

	if (!ap_trait_mode()) return false;

	/* Progressive elemental resists: count always, mutate only when new. */
	for (e = 0; e < AP_TE_MAX; e++) {
		if (!streq(name, ap_trait_elems[e].item_name)) continue;
		ap_trait_elem_count[e]++;
		ap_ensure_trait_weapon();
		weapon = ap_trait_weapon();
		if (weapon && is_new) {
			int lvl = ap_trait_elem_count[e];
			int elem = ap_trait_elems[e].elem;

			if (lvl == 1 && weapon->el_info[elem].res_level < 1)
				weapon->el_info[elem].res_level = 1;
			else if (lvl >= 3)
				weapon->el_info[elem].res_level = 3;
			if (weapon->known)
				weapon->known->el_info[elem].res_level =
					weapon->el_info[elem].res_level;
			msg("AP: Your blade attunes further to %s (rank %d).",
				name, lvl);
			ap_weapon_learn(weapon);
			ap_trait_update();
		}
		ap_attune_refresh();
		return true;
	}

	/* Progressive slays: 1 fire brand, 2 racial slays, 3 slay evil. */
	if (streq(name, "Progressive Slay")) {
		int n = ++ap_trait_slay_count;

		ap_ensure_trait_weapon();
		weapon = ap_trait_weapon();
		if (weapon && is_new) {
			if (n == 1) {
				ap_weapon_add_slay(weapon, "FIRE_2", true);
				msg("AP: Your blade burns with elemental fire.");
			} else if (n == 2) {
				ap_weapon_add_slay(weapon, "ORC_3", false);
				ap_weapon_add_slay(weapon, "TROLL_3", false);
				ap_weapon_add_slay(weapon, "GIANT_3", false);
				ap_weapon_add_slay(weapon, "DRAGON_3", false);
				msg("AP: Your blade thirsts for monstrous blood.");
			} else if (n >= 3) {
				ap_weapon_add_slay(weapon, "EVIL_2", false);
				msg("AP: Your blade blazes against all evil.");
			}
			ap_weapon_learn(weapon);
			ap_trait_update();
		}
		return true;
	}

	/* Everything below mutates persistent state only: skip stale replays. */

	for (i = 0; i < N_ELEMENTS(ap_trait_resists); i++) {
		if (!streq(name, ap_trait_resists[i].name)) continue;
		if (!is_new) return true;
		ap_ensure_trait_weapon();
		weapon = ap_trait_weapon();
		if (weapon) {
			int elem = ap_trait_resists[i].elem;

			if (weapon->el_info[elem].res_level < 1)
				weapon->el_info[elem].res_level = 1;
			if (weapon->known)
				weapon->known->el_info[elem].res_level =
					weapon->el_info[elem].res_level;
			msg("AP: Your blade shields you. (%s)", name);
			ap_weapon_learn(weapon);
			ap_trait_update();
		}
		return true;
	}

	for (i = 0; i < N_ELEMENTS(ap_trait_flags); i++) {
		if (!streq(name, ap_trait_flags[i].name)) continue;
		if (!is_new) return true;
		ap_ensure_trait_weapon();
		weapon = ap_trait_weapon();
		if (weapon) {
			int f;

			for (f = 0; f < 6 && ap_trait_flags[i].flags[f]; f++) {
				of_on(weapon->flags, ap_trait_flags[i].flags[f]);
				if (weapon->known)
					of_on(weapon->known->flags,
						ap_trait_flags[i].flags[f]);
			}
			msg("AP: Your blade grants you %s.", name);
			ap_weapon_learn(weapon);
			ap_trait_update();
		}
		return true;
	}

	if (streq(name, "+5 Speed") || streq(name, "Extra Blow")
			|| streq(name, "Weapon To-Hit") || streq(name, "Weapon Damage")
			|| streq(name, "Weapon Dice") || streq(name, "Weapon Dice Count")
			|| streq(name, "Weapon Lightness")
			|| streq(name, "Armor Class")) {
		if (!is_new) return true;
		ap_ensure_trait_weapon();
		weapon = ap_trait_weapon();
		if (weapon) {
			if (streq(name, "+5 Speed")) {
				weapon->modifiers[OBJ_MOD_SPEED] += 5;
			} else if (streq(name, "Extra Blow")) {
				weapon->modifiers[OBJ_MOD_BLOWS] += 1;
			} else if (streq(name, "Weapon To-Hit")) {
				weapon->to_h += 2;
			} else if (streq(name, "Weapon Damage")) {
				weapon->to_d += 2;
			} else if (streq(name, "Weapon Dice")) {
				weapon->ds += 1;
			} else if (streq(name, "Weapon Dice Count")) {
				weapon->dd += 1;
			} else if (streq(name, "Weapon Lightness")) {
				weapon->weight = MAX(weapon->weight - 50, 20);
			} else { /* Armor Class */
				weapon->to_a += 10;
			}
			if (weapon->known) {
				weapon->known->modifiers[OBJ_MOD_SPEED] =
					weapon->modifiers[OBJ_MOD_SPEED];
				weapon->known->modifiers[OBJ_MOD_BLOWS] =
					weapon->modifiers[OBJ_MOD_BLOWS];
				weapon->known->to_h = weapon->to_h;
				weapon->known->to_d = weapon->to_d;
				weapon->known->to_a = weapon->to_a;
				weapon->known->dd = weapon->dd;
				weapon->known->ds = weapon->ds;
				weapon->known->weight = weapon->weight;
			}
			msg("AP: Your blade improves. (%s)", name);
			ap_weapon_learn(weapon);
			ap_trait_update();
		}
		return true;
	}

	/* Full-Traits race boosts: replay-derived, applied on every occurrence. */
	for (i = 0; i < N_ELEMENTS(ap_trait_stats); i++) {
		if (!streq(name, ap_trait_stats[i].name)) continue;
		ap_race_stat_adj[ap_trait_stats[i].stat]++;
		if (is_new) msg("AP: Your being improves. (%s)", name);
		ap_trait_update();
		return true;
	}
	if (streq(name, "Stealth Boost")) {
		ap_race_skill_adj[SKILL_STEALTH] += 1;
	} else if (streq(name, "Saving Throw Boost")) {
		ap_race_skill_adj[SKILL_SAVE] += 5;
	} else if (streq(name, "Magic Device Boost")) {
		ap_race_skill_adj[SKILL_DEVICE] += 5;
	} else if (streq(name, "Disarming Boost")) {
		ap_race_skill_adj[SKILL_DISARM_PHYS] += 10;
		ap_race_skill_adj[SKILL_DISARM_MAGIC] += 10;
	} else if (streq(name, "Searching Boost")) {
		ap_race_skill_adj[SKILL_SEARCH] += 5;
	} else if (streq(name, "Infravision Boost")) {
		ap_race_infra_adj += 4;
	} else if (streq(name, "Hit Die Boost")) {
		ap_race_hitdie_adj += 1;
	} else {
		return false;
	}
	if (is_new) msg("AP: Your being improves. (%s)", name);
	ap_trait_update();
	return true;
}

/**
 * An item was granted to our slot, with a stable per-slot sequence index.
 *
 * De-duplication is per character via a high-water mark in the save file: a
 * fresh character starts at 0 and receives the full replay (restocking its empty
 * home), while the same character reconnecting has its mark past the replay and
 * adds nothing.  This is why we can't use APCc's server-side notify flag.
 *
 * Counting runs for *every* occurrence regardless of the high-water mark: the
 * progressive-category counters (so the Nth-artifact mapping stays correct
 * across skipped items), the expanded-books tier counter, and re-applying boons
 * over launch-rebuilt state (the bookseller).  State that is saved -- home and
 * pack contents, gold, experience -- is applied once, gated by the mark.
 *
 * If a home/inventory item can't be placed (everything full), the mark is left
 * at that item and the rest of the stream is held back (ap_blocked) so the mark
 * stays contiguous; a later replay (after space frees up) retries from there.
 */
static void ap_item_granted(const char *item_name, uint64_t index)
{
	int prog_n = 0, cat;

	if (!player) return;

	ap_log("item_granted[%llu] '%s' (mark %u)", (unsigned long long)index,
		item_name ? item_name : "(null)", (unsigned)player->ap_items_received);

	/* A fresh replay restarts at index 0: recompute all transient counters. */
	if (index == 0) {
		memset(ap_prog_count, 0, sizeof(ap_prog_count));
		ap_books_count = 0;
		ap_blocked = false;
		memset(ap_trait_elem_count, 0, sizeof(ap_trait_elem_count));
		ap_trait_slay_count = 0;
		memset(ap_race_stat_adj, 0, sizeof(ap_race_stat_adj));
		memset(ap_race_skill_adj, 0, sizeof(ap_race_skill_adj));
		ap_race_infra_adj = 0;
		ap_race_hitdie_adj = 0;
	}

	/* Advance the category counter even for items we won't re-deliver. */
	cat = ap_prog_cat_of(item_name);
	if (cat >= 0) prog_n = ++ap_prog_count[cat];

	/* Re-applied every replay (idempotent). */
	ap_apply_replay_boon(item_name);

	/*
	 * Trait items (Resistances modes) run for every occurrence too: their
	 * counters and replay-derived state (attunement, race boosts) must be
	 * rebuilt identically on every replay, while their persistent weapon
	 * mutations are applied only when the grant is new (past the mark) --
	 * ap_apply_trait_item handles the split via is_new.
	 */
	if (ap_apply_trait_item(item_name,
			index >= player->ap_items_received && !ap_blocked)) {
		if (index >= player->ap_items_received && !ap_blocked)
			player->ap_items_received = (uint32_t)(index + 1);
		return;
	}

	/* Once-per-character below: skip anything at/under the high-water mark. */
	if (index < player->ap_items_received) return;

	/* A prior item this stream is waiting for space: hold the mark contiguous. */
	if (ap_blocked) return;

	if (ap_apply_oneshot_boon(item_name)) {
		player->ap_items_received = (uint32_t)(index + 1);
	} else if (ap_deliver_item(item_name, prog_n)) {
		player->ap_items_received = (uint32_t)(index + 1);
	} else {
		/* No room anywhere: stop here and retry this item on a later connect. */
		ap_blocked = true;
	}
}

/** A DeathLink arrived: kill the player (without bouncing it back out). */
static void ap_deathlink_received(void)
{
	if (!player || player->is_dead) return;

	ap_dying_from_deathlink = true;
	take_hit(player, 32000, "an Archipelago death");
	ap_dying_from_deathlink = false;
}

void ap_game_player_died(void)
{
	/* Don't echo a received DeathLink back to the multiworld. */
	if (ap_dying_from_deathlink) return;
	ap_send_deathlink();
}

void ap_game_player_won(void)
{
	ap_send_victory();
}

/*
 * Pick the artifact whose cost sets the price of the next Black Market location
 * buy, and report that price.  The selection differs by mode:
 *   - One to One: the shallowest (ties: cheapest) still-unchecked artifact the
 *     player is deep enough for -- buying sends that specific location's check.
 *   - Accumulated: order the depth-eligible artifacts weakest-first and take the
 *     one at the player's current find count (i.e. the artifact backing the next
 *     "Find #X Artifacts" milestone).  Buying advances the milestone.  Returns
 *     NULL once the count has reached the number of artifacts reachable at this
 *     depth, so the sink still requires descending.
 * The price is the slot's Black Market multiplier times the artifact's value.
 */
const struct artifact *ap_find_missed_location(int *price_out)
{
	const struct artifact *best = NULL;
	int mode = ap_artifacts_mode();
	int i;

	if ((mode != 1 && mode != 2) || !ap_is_connected()) return NULL;
	if (!ap_ensure_artifact_table() || !player) return NULL;

	for (i = 1; i < z_info->a_max; i++) {
		const struct artifact *art = &a_info[i];
		/*
		 * "Already done" differs by mode: One to One tracks per-location checks
		 * (ap_artifact_checked); Accumulated tracks distinct finds
		 * (aup_info[].ap_found).  Either way we offer the shallowest (ties:
		 * cheapest) depth-eligible artifact that isn't done yet.
		 */
		bool done = (mode == 2)
			? (aup_info && aup_info[art->aidx].ap_found)
			: ap_artifact_checked[art->aidx];

		if (!art->name) continue;
		if (done) continue;
		if (art->alloc_prob <= 0) continue;               /* not a normal drop
		                                                   * (story artifacts like
		                                                   * Grond aren't AP
		                                                   * locations) */
		if (player->max_depth < art->alloc_min) continue; /* not yet deep enough */
		if (art->cost <= 0) continue;                     /* need a price */

		/* Lowest spawn depth first; break ties by the cheaper artifact. */
		if (!best
				|| art->alloc_min < best->alloc_min
				|| (art->alloc_min == best->alloc_min && art->cost < best->cost))
			best = art;
	}

	if (!best) return NULL;
	if (price_out) *price_out = ap_black_market_multiplier() * best->cost;
	return best;
}

void ap_buy_missed_location(const struct artifact *art)
{
	if (!art) return;

	if (ap_artifacts_accumulated()) {
		/*
		 * Accumulated: buying marks this specific artifact found (exactly as a
		 * real pickup would) and sends the milestone for the new distinct-find
		 * count.  ap_find_missed_location only ever offers a not-yet-found
		 * artifact, so this always advances by one.
		 */
		char name[64];

		if (!aup_info) return;
		aup_info[art->aidx].ap_found = true;
		mark_artifact_created(art, true);	/* banked: stop it generating */
		ap_find_artifacts_location_name((unsigned)ap_artifacts_found_count(),
			name, sizeof(name));
		ap_send_check(name);
		return;
	}

	/* One to One: mark it found locally so we don't re-offer it before the
	 * server's confirmation (and replay) comes back and sets the same flag. */
	if (ap_ensure_artifact_table())
		ap_artifact_checked[art->aidx] = true;

	/*
	 * Send the location check.  Whatever item sits at this location is released
	 * by the server through the normal grant path -- we do not hand out the
	 * artifact here.
	 */
	ap_send_artifact_check(art);
}

void ap_game_reset_for_new_life(void)
{
	/*
	 * A fresh character must receive the full item replay again, so reset the
	 * item high-water mark.  The Accumulated-mode artifact-find set is
	 * deliberately NOT reset here: aup_info[].ap_found survives a retire or
	 * reincarnate within this session (birth does not clear it), and where it
	 * does not survive -- a death that starts a new savefile, or a restart --
	 * ap_sync_artifact_finds() rebuilds it from the server on reconnect.  Either
	 * way the "Find #X Artifacts" milestones stay earned for the whole slot.
	 */
	if (player) player->ap_items_received = 0;

	/*
	 * Drop the connection; process_player's next ap_service() call sees it is
	 * down and reconnects with the (preserved) server/slotname, which re-fires
	 * the Connected event -> checked-location replay (uniques stay dead) and
	 * item replay (restock the new home, re-apply boons).
	 */
	ap_shutdown();
}

/*
 * Credit \p n artifacts to this character's find set, weakest-first, and stop
 * them spawning.  Used when the server's milestone count is ahead of the local
 * one -- i.e. a character who inherited an in-progress slot from a dead
 * predecessor.  Returns the resulting find count.
 *
 * "Find #X Artifacts" locations carry only a count, never which artifacts were
 * found, so the specific set credited here need not be the set the previous life
 * actually picked up; it is the same weakest-first order (spawn depth, then
 * cost) the Black Market offers in, so the artifacts still available to find are
 * the deeper ones, which is what matters for pacing.
 */
static int ap_credit_found_artifacts(int n)
{
	while (n-- > 0) {
		const struct artifact *best = NULL;
		int i;

		for (i = 1; i < z_info->a_max; i++) {
			const struct artifact *art = &a_info[i];

			if (!art->name) continue;
			if (aup_info[art->aidx].ap_found) continue;
			if (art->alloc_prob <= 0) continue;	/* not an AP location:
			                                     * story artifacts (Grond,
			                                     * Morgoth's crown) never drop */
			/* NB no cost filter here, unlike the Black Market offer, which
			 * needs a price: Camlost has cost 0 but is a real location, so
			 * excluding it would leave the credit pool one short of the
			 * apworld's 136 milestones. */

			if (!best
					|| art->alloc_min < best->alloc_min
					|| (art->alloc_min == best->alloc_min
						&& art->cost < best->cost))
				best = art;
		}

		if (!best) break;	/* nothing left to credit */
		ap_log("  crediting inherited find: %s", best->name);
		aup_info[best->aidx].ap_found = true;
	}

	return ap_artifacts_found_count();
}

/*
 * Reconcile accumulated-mode artifact progress with the server.  Runs on every
 * connect, after the checked-location replay has set ap_find_milestone_max.
 *
 * Two directions:
 *   - Server ahead of us: a fresh character starts with an empty find set, so
 *     credit the shortfall (above) rather than making the player re-find
 *     artifacts the slot has already banked.
 *   - Us ahead of the server: re-send every milestone up to our find count.  A
 *     single ap_send_check can be lost if the link drops between the pickup and
 *     the send, which silently skips one milestone while later ones still land;
 *     re-sending the whole run repairs any such hole.  The server de-duplicates,
 *     exactly as for the dead-unique reconciliation below.
 * Found artifacts are also marked created so they stop generating -- birth
 * clears created but not ap_found, so this is what keeps an already-banked
 * artifact from littering a new character's dungeon.
 */
static void ap_sync_artifact_finds(void)
{
	int found, i;

	if (!ap_artifacts_accumulated() || !aup_info || !z_info) return;

	found = ap_artifacts_found_count();
	ap_log("sync_artifact_finds: local=%d server=%u", found,
		ap_find_milestone_max);

	if ((unsigned)found < ap_find_milestone_max)
		found = ap_credit_found_artifacts((int)ap_find_milestone_max - found);

	for (i = 1; i < z_info->a_max; i++)
		if (aup_info[i].ap_found)
			mark_artifact_created(&a_info[i], true);

	for (i = 1; i <= found; i++) {
		char name[64];

		ap_find_artifacts_location_name((unsigned)i, name, sizeof(name));
		ap_send_check(name);
	}
}

/**
 * Authentication finished.  The checked-location replay has already run (marking
 * known-dead uniques), so reconcile the other direction: send checks for any
 * unique that is dead in this save file but that the server might not know about
 * (e.g. killed while disconnected).  The server de-duplicates already-known
 * checks, so this is safe to run on every connect.
 */
static void ap_on_connected(void)
{
	int i;

	ap_log("on_connected: begin");

	/*
	 * Echo the slot_data the server sent, so a yaml option that appears not to
	 * have taken effect can be checked against what actually arrived rather
	 * than guessed at.  Shown in-game as well as logged: these all change how
	 * the run plays, and the Black Market price in particular is otherwise only
	 * visible as a number with nothing to compare it to.
	 */
	ap_log("on_connected: slot_data artifacts=%d resistances=%d bm_mult=%d",
		ap_artifacts_mode(), ap_resistances_mode(),
		ap_black_market_multiplier());
	msg("AP: options -- artifacts %d, resistances %d, black market x%d.",
		ap_artifacts_mode(), ap_resistances_mode(),
		ap_black_market_multiplier());

	/* Trait modes: put the Archipelago Blade (and race) in place first. */
	ap_ensure_trait_weapon();

	ap_log("on_connected: reconciling dead uniques");
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];

		if (!race->name) continue;
		if (!rf_has(race->flags, RF_UNIQUE)) continue;
		if (race->max_num != 0) continue;       /* still alive */

		ap_send_check(race->name);
	}

	ap_log("on_connected: reconciling artifact finds");
	ap_sync_artifact_finds();
	ap_log("on_connected: done");
}

/*
 * Format the apworld location name for the \p n'th "Find #X Artifacts"
 * accumulated milestone.  Must byte-match angband/_gen_data.py
 * find_location_name(): singular for 1, plural otherwise.
 */
static void ap_find_artifacts_location_name(unsigned n, char *buf, size_t len)
{
	strnfmt(buf, len, "Find %u Artifact%s", n, n == 1 ? "" : "s");
}

bool ap_game_item_picked_up(const struct object *obj)
{
	if (!obj || !obj->artifact) return false;
	if (!ap_artifacts_as_checks()) return false;

	/*
	 * Only the dungeon's attributeless location-check copy should send the
	 * check; an AP-granted artifact (the reward for the check) is tagged
	 * ORIGIN_ARCHIPELAGO, so picking it back up after dropping it must not count
	 * as finding the location.  The grant never marks the artifact created, so
	 * the real location copy can still spawn and be checked.
	 */
	if (obj->origin == ORIGIN_ARCHIPELAGO) return false;

	if (ap_artifacts_accumulated()) {
		/*
		 * Accumulated mode: the checks are "Find #X Artifacts" milestones, not
		 * per-artifact.  Record this specific artifact as found (persists across
		 * lives via aup_info[].ap_found) and, if it is newly found, send the
		 * milestone for the new distinct-find count.  Re-picking an artifact
		 * already found in this or a previous life is a no-op.
		 */
		unsigned aidx = obj->artifact->aidx;

		if (aup_info && !aup_info[aidx].ap_found) {
			char name[64];

			aup_info[aidx].ap_found = true;
			ap_find_artifacts_location_name(
				(unsigned)ap_artifacts_found_count(), name, sizeof(name));
			ap_send_check(name);
		}
	} else {
		/* One to One: send this specific artifact's location check. */
		ap_send_artifact_check(obj->artifact);
	}

	/* It's a valueless placeholder; tell the caller to discard it. */
	return true;
}

void ap_game_setup(void)
{
	ap_set_check_handler(ap_check_confirmed);
	ap_set_item_handler(ap_item_granted);
	ap_set_connect_handler(ap_on_connected);
	ap_set_deathlink_handler(ap_deathlink_received);
}
