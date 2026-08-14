# Angband × Archipelago

This fork turns Angband into an [Archipelago](https://archipelago.gg) (AP)
multiworld game. The game client talks to an AP server directly (no proxy); the
matching generation-side world lives in [`angband/`](angband/) and is packaged as
`angband.apworld`. The game name on both sides is **"Angband"**.

## Architecture

| Piece | Role |
|---|---|
| `src/apcc/APCc.{c,h}` | Vendored pure-C AP client (jansson + libwebsockets + glib). |
| `src/apinterface.{c,h}` | Owns **all** APCc/glib/winsock surface. Runs a dedicated service thread; exchanges events with the game thread via two `GAsyncQueue`s. Dependency-free public API (strings only). |
| `src/ap-game.{c,h}` | Game-side handlers — may use Angband headers. Registered by `ap_game_setup()` (called once from `process_player`). Talks to APCc only through `apinterface.h`. |
| `angband/` | The generation-side apworld (zipped to `angband.apworld`). |

**Threading:** `lws_service()` blocks for seconds, so all websocket servicing runs
on its own `GThread`. APCc callbacks fire there and only *enqueue* events (names
resolved on that thread, since all data-package access must stay there). The game
thread's per-turn `ap_service()` drains the queue and runs handlers cheaply.
Sends enqueue + `AP_WakeService()` to unblock the service thread promptly.

## Features

- **Uniques as checks** — killing a unique sends its location check; on connect
  the replay marks already-checked uniques dead (`max_num = 0`) so they stay dead
  across deaths/respawns. Quest monsters (Sauron/Morgoth) also get their quest
  reconciled so a respawn isn't soft-locked above a quest level.
- **Artifacts as checks** (`artifacts_as_checks` slot_data, default on) — artifacts
  spawn attributeless-but-named as location placeholders; picking one up sends the
  check and the placeholder is discarded (it never occupies a pack slot). The real
  artifact is delivered separately as an AP *item*. Checked locations are marked
  created so the placeholder stops regenerating.
- **Item grants** — received items are placed in the pack (if in the dungeon with
  room) or the home. De-duplicated per character via a saved high-water mark
  (`player->ap_items_received`). Covers specific artifacts, 13 "Progressive
  `<Category>` Artifact" lines (Nth artifact of a category, ordered weakest-first
  by spawn depth then cost), consumables, and three boons (triple gold ×3
  stacking, +5 levels, expanded shop books). AP rewards are tagged
  `ORIGIN_ARCHIPELAGO`.
- **Black Market gap-filler** — in artifacts-as-checks mode, press `$` in the Black
  Market to buy the shallowest still-unchecked artifact location (3× its cost) to
  fill RNG-heavy gaps.
- **Resistances trait modes** (`resistances` slot_data: 0 Standard, 1 Equipment
  Traits, 2 Full Traits) — in trait modes the player wields an unremovable
  **Archipelago Blade** (new kind in object.txt) and may equip only a launcher
  and light; the artifact *items* are replaced by trait items (resists,
  progressive elemental resist/attunement/immunity, slays, speed, weapon stats,
  AC…) that mutate the blade directly (mutations persist per-object in the
  savefile). Full Traits also swaps in the worst-in-everything **Archipelago
  race** (p_race.txt) whose stat/skill boosts are replay-derived
  (ap_race_boost_* hooks in player-calcs.c). Depth gating uses trait items
  instead of artifacts. Requires artifacts-as-checks ≠ Off (generation error).
- **DeathLink**, **goal on Morgoth**, and an **`AP`** indicator on the status line
  while connected.
- **Respawn / reincarnation** — a new character (retire→new, or the Borg's
  reincarnate-on-death) reconnects fresh so the item replay restocks the new life.

## Building (native Windows GDI app)

From the **MSYS2 MinGW64** shell:

```sh
cd src && mingw32-make -f Makefile.win MINGW=yes
```

Dependencies are MSYS2 pacman packages: `mingw-w64-x86_64-{jansson,libwebsockets,glib2}`.
The build copies the ~11 runtime DLLs next to `angband.exe`. Only `Makefile.win`
is AP-wired (not CMake/Makefile.std).

### Borg toggle

The automatic player ("Borg") is gated on `-DALLOW_BORG`, controlled by the
`BORG` make variable (default `yes`). Toggling requires a full rebuild (`rm`
the objects first — it spans many files):

```sh
mingw32-make -f Makefile.win MINGW=yes            # BORG on (default)
mingw32-make -f Makefile.win MINGW=yes BORG=no    # release build, no Borg
```

## Releasing

`scripts/make-release.ps1` (PowerShell) packages two artifacts into `dist/`:
`APAngband-win64.zip` (the runnable bundle) and `angband.apworld`. It reads
`angband.exe` from the **repo root**, so build first. The convention is to ship a
**non-Borg** public binary while keeping a Borg build at the root for testing:

```
clean → build BORG=no → make-release.ps1 → clean → build (BORG=yes)
```

## The apworld (`angband/`)

`apworld` best-practices layout; bulk data in `angband/data.py` is **generated**
by `angband/_gen_data.py` from `manual_angband425_broney/data/*.json` — re-run the
generator, don't hand-edit. Location/item names must byte-match what the client
sends (unique names from `monster.txt`, artifact names, the frozen item list).
The internal package folder inside the `.apworld` is `apangband/`.
