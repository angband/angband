/**
 * \file ap-game.h
 * \brief Angband-side Archipelago glue: wires AP events to game state.
 *
 * Unlike apinterface.c (which owns the APCc/glib/winsock dependencies), this
 * lives on the normal Angband side and may freely use the game's headers.  It
 * registers handlers with apinterface so that AP item/location events act on the
 * world (e.g. keeping killed uniques dead, stocking the home).
 */
#ifndef AP_GAME_H
#define AP_GAME_H

struct object;
struct artifact;

/** Register the Archipelago game-side handlers.  Idempotent; call at game start. */
void ap_game_setup(void);

/**
 * Find the artifact whose Archipelago location the Black Market should offer to
 * "buy": the shallowest-spawning (ties broken by cheapest) artifact whose
 * location is still unchecked and whose spawn depth the player has already
 * reached.  Returns NULL when none qualify (or not in artifacts-as-checks mode /
 * not connected).  On success, *price_out (if non-NULL) gets the gold price
 * (3x the artifact's value).  The artifact name is deliberately not exposed to
 * the shop UI -- callers show only the price.
 */
const struct artifact *ap_find_missed_location(int *price_out);

/**
 * Complete a Black Market "missed location" purchase for \p art: send its
 * location check (the server then releases whatever item sits there through the
 * normal grant path) and mark it found locally so it isn't re-offered.  Does not
 * grant the artifact itself.  The caller is responsible for charging the player.
 */
void ap_buy_missed_location(const struct artifact *art);

/**
 * Report that the player picked up \p obj.  When artifacts-as-checks mode is on
 * and \p obj is a dungeon location-check placeholder (an attributeless artifact,
 * not an AP-granted reward), this sends the matching location check and returns
 * true to signal the caller that the valueless placeholder should be discarded
 * rather than carried.  Returns false (and does nothing) otherwise.
 */
bool ap_game_item_picked_up(const struct object *obj);

/**
 * Report that the player has died, broadcasting a DeathLink to other players
 * (unless this death was itself caused by a received DeathLink).
 */
void ap_game_player_died(void);

/** Report that the player has won (killed Morgoth): completes the AP goal. */
void ap_game_player_won(void);

/**
 * Reset Archipelago state for a brand-new life (e.g. Borg reincarnation): clear
 * the per-character item high-water mark and drop the connection so the next
 * ap_service() reconnects from scratch.  The fresh connection re-runs the item
 * replay for the new character (restocking the home, re-applying boons) while
 * the server keeps already-checked locations (killed uniques stay dead).
 */
void ap_game_reset_for_new_life(void);

/**
 * Full-Traits (Resistances mode 2) race boosts, rebuilt from the item replay on
 * every connect.  player-calcs.c adds these on top of the race/class values:
 * per-stat adjustment, per-skill adjustment, infravision, and bonus hitpoints
 * per level (the "Hit Die Boost" items).  All return 0 outside Full Traits.
 */
int ap_race_boost_stat(int stat);
int ap_race_boost_skill(int skill);
int ap_race_boost_infra(void);
int ap_race_boost_hitdie(void);

#endif /* AP_GAME_H */
