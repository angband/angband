"""
One-shot generator: reads the manual_angband425_broney JSON data and emits
data.py for the Angband apworld.  Run from the repo root:

    python apworld/_gen_data.py

The names produced here must byte-match what the native game (the APCc client)
sends/receives, which is why we copy them verbatim from the manual data that was
already validated against the game's monster.txt / artifact.txt.
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
MANUAL = os.path.join(HERE, "..", "manual_angband425_broney", "data")

# Stable ID bases.  Item and location IDs live in separate namespaces in AP, so
# overlap is harmless, but distinct bases keep logs readable.
ITEM_ID_BASE = 4250000
LOCATION_ID_BASE = 4251000


def load(name):
    with open(os.path.join(MANUAL, name), encoding="utf-8") as fh:
        return json.load(fh)


def pyrepr(obj, indent=0):
    """Deterministic, readable repr for the literal dump."""
    pad = "    " * indent
    pad2 = "    " * (indent + 1)
    if isinstance(obj, dict):
        if not obj:
            return "{}"
        lines = ["{"]
        for k, v in obj.items():
            lines.append(f"{pad2}{k!r}: {pyrepr(v, indent + 1)},")
        lines.append(pad + "}")
        return "\n".join(lines)
    if isinstance(obj, (list, tuple)):
        if not obj:
            return "[]"
        # keep short lists inline
        inline = "[" + ", ".join(pyrepr(x) for x in obj) + "]"
        if len(inline) <= 96:
            return inline
        lines = ["["]
        for x in obj:
            lines.append(f"{pad2}{pyrepr(x, indent + 1)},")
        lines.append(pad + "]")
        return "\n".join(lines)
    return repr(obj)


def parse_requires(req):
    """
    Turn the manual requirement DSL into a list of (item_name, count) tuples.
    The manual only ever uses the form
        |Item A:count| AND |Item B:count| AND ...
    so we just split on AND and parse each |...| token.
    """
    if not req:
        return []
    out = []
    for tok in req.split("AND"):
        tok = tok.strip()
        if not tok:
            continue
        assert tok.startswith("|") and tok.endswith("|"), tok
        inner = tok[1:-1]
        name, _, count = inner.rpartition(":")
        out.append([name.strip(), int(count)])
    return out


# ---- Trait-mode data (Resistances option) -----------------------------------
# The "Equipment Traits" / "Full Traits" modes replace the artifact items with
# trait items that improve the unremovable Archipelago Weapon (and, in Full
# Traits, the Archipelago race).  These names are frozen: the native client's
# ap-game.c dispatch table must byte-match them.
#
# name -> (count, classification, mode) where mode is "equipment" (present in
# both trait modes) or "full" (Full Traits only).  Anything referenced by the
# gating tables below must be classified progression.
TRAIT_ITEMS = [
    # Progressive elemental resists: 1 = resist, 2 = at-will temp-resist
    # activation, 3 = immunity.
    ("Progressive Fire Resistance",      3, "progression", "equipment"),
    ("Progressive Cold Resistance",      3, "progression", "equipment"),
    ("Progressive Lightning Resistance", 3, "progression", "equipment"),
    ("Progressive Acid Resistance",      3, "progression", "equipment"),
    ("Poison Resistance",                1, "progression", "equipment"),
    ("Blindness Resistance",             1, "progression", "equipment"),
    ("Confusion Resistance",             1, "progression", "equipment"),
    ("+5 Speed",                         6, "progression", "equipment"),
    # 1 = fire brand, 2 = racial slays (orc/troll/giant/dragon), 3 = slay evil.
    ("Progressive Slay",                 3, "progression", "equipment"),
    ("Weapon To-Hit",                   12, "progression", "equipment"),
    ("Weapon Damage",                   12, "progression", "equipment"),
    # Damage dice split into two items: count (extra dice, base 1d4 -> up to 5d4)
    # and size (bigger dice, up to 5d10 with both maxed).
    ("Weapon Dice Count",                4, "progression", "equipment"),
    ("Weapon Dice",                      6, "progression", "equipment"),
    # The blade starts at 300 (30 lb), far too heavy to get blows out of; each
    # copy sheds 50 down to a floor of 20, so all 5 are needed and they gate
    # melee viability outright -- progression, one per band up to Depths 61-70.
    ("Weapon Lightness",                 5, "progression", "equipment"),
    ("Armor Class",                     10, "progression", "equipment"),
    ("Light Resistance",                 1, "useful",      "equipment"),
    ("Dark Resistance",                  1, "useful",      "equipment"),
    ("Sound Resistance",                 1, "useful",      "equipment"),
    ("Shards Resistance",                1, "useful",      "equipment"),
    ("Nexus Resistance",                 1, "useful",      "equipment"),
    ("Nether Resistance",                1, "progression", "equipment"),
    ("Chaos Resistance",                 1, "progression", "equipment"),
    ("Disenchantment Resistance",        1, "progression", "equipment"),
    ("Fear Resistance",                  1, "useful",      "equipment"),
    ("Stun Resistance",                  1, "useful",      "equipment"),
    ("Free Action",                      1, "progression", "equipment"),
    ("Hold Life",                        1, "progression", "equipment"),
    ("See Invisible",                    1, "useful",      "equipment"),
    ("Telepathy",                        1, "useful",      "equipment"),
    ("Regeneration",                     1, "useful",      "equipment"),
    ("Feather Fall",                     1, "useful",      "equipment"),
    ("Extra Blow",                       2, "useful",      "equipment"),
    ("All Sustains",                     1, "useful",      "equipment"),
    # Full Traits only: Archipelago-race improvements.
    ("Progressive Strength Boost",       5, "progression", "full"),
    ("Progressive Intelligence Boost",   5, "progression", "full"),
    ("Progressive Wisdom Boost",         5, "progression", "full"),
    ("Progressive Dexterity Boost",      5, "progression", "full"),
    ("Progressive Constitution Boost",   5, "progression", "full"),
    ("Stealth Boost",                    3, "useful",      "full"),
    ("Saving Throw Boost",               3, "useful",      "full"),
    ("Magic Device Boost",               3, "useful",      "full"),
    ("Disarming Boost",                  2, "useful",      "full"),
    ("Searching Boost",                  2, "useful",      "full"),
    ("Infravision Boost",                1, "useful",      "full"),
    ("Hit Die Boost",                    3, "useful",      "full"),
]


def trait_region_requires():
    """
    Depth-band gating for the trait modes, replacing the artifact requires.
    Roughly linear ramps: elemental resists early, speed one band at a time,
    weapon stats climbing steadily to their pool maximums by the bottom.
    Returns (equipment_requires, full_extra_requires); Full Traits gating is
    the union of the two per band.
    """
    elems = ["Progressive Fire Resistance", "Progressive Cold Resistance",
             "Progressive Lightning Resistance", "Progressive Acid Resistance"]

    def band(elem=0, speed=0, hit=0, dam=0, dice=0, dcount=0, ac=0, slay=0,
             light=0, extra=()):
        req = []
        if elem:
            req += [[e, elem] for e in elems]
        if speed:
            req.append(["+5 Speed", speed])
        if light:
            req.append(["Weapon Lightness", light])
        if hit:
            req.append(["Weapon To-Hit", hit])
        if dam:
            req.append(["Weapon Damage", dam])
        if dice:
            req.append(["Weapon Dice", dice])
        if dcount:
            req.append(["Weapon Dice Count", dcount])
        if ac:
            req.append(["Armor Class", ac])
        if slay:
            req.append(["Progressive Slay", slay])
        for name, count in extra:
            req.append([name, count])
        return req

    survival = [("Poison Resistance", 1), ("Blindness Resistance", 1),
                ("Confusion Resistance", 1)]
    equipment = {
        "Depths 21-30": band(elem=1, light=1),
        "Depths 31-40": band(elem=1, speed=1, light=2, extra=survival),
        "Depths 41-50": band(elem=2, speed=2, hit=4, dam=4, dice=2, light=3,
                             extra=survival),
        "Depths 51-60": band(elem=2, speed=3, hit=6, dam=6, dice=3, dcount=1,
                             ac=3, slay=1, light=4, extra=survival + [("Free Action", 1)]),
        "Depths 61-70": band(elem=3, speed=4, hit=8, dam=8, dice=4, dcount=2,
                             ac=4, slay=1, light=5, extra=survival + [("Free Action", 1),
                                                       ("Hold Life", 1)]),
        "Depths 71-80": band(elem=3, speed=5, hit=10, dam=10, dice=5, dcount=3,
                             ac=6, slay=2, light=5, extra=survival + [("Free Action", 1),
                                                       ("Hold Life", 1)]),
        "Depths 81-90": band(elem=3, speed=6, hit=11, dam=11, dice=6, dcount=4,
                             ac=8, slay=3, light=5, extra=survival + [
                                 ("Free Action", 1), ("Hold Life", 1),
                                 ("Nether Resistance", 1),
                                 ("Chaos Resistance", 1),
                                 ("Disenchantment Resistance", 1)]),
        "Depths 91-100": band(elem=3, speed=6, hit=12, dam=12, dice=6, dcount=4,
                              ac=10, slay=3, light=5, extra=survival + [
                                  ("Free Action", 1), ("Hold Life", 1),
                                  ("Nether Resistance", 1),
                                  ("Chaos Resistance", 1),
                                  ("Disenchantment Resistance", 1)]),
    }

    stats = ["Progressive Strength Boost", "Progressive Intelligence Boost",
             "Progressive Wisdom Boost", "Progressive Dexterity Boost",
             "Progressive Constitution Boost"]

    def stats_req(n):
        return [[s, n] for s in stats]

    full_extra = {
        "Depths 41-50": stats_req(1),
        "Depths 51-60": stats_req(2),
        "Depths 61-70": stats_req(2),
        "Depths 71-80": stats_req(3),
        "Depths 81-90": stats_req(4),
        "Depths 91-100": stats_req(5),
    }
    return equipment, full_extra


def main():
    items = load("items.json")
    locations = load("locations.json")
    regions = load("regions.json")
    game = load("game.json")

    # ---- Items -------------------------------------------------------------
    # Each distinct item NAME gets one ID.  Count/classification come alongside.
    item_name_to_id = {}
    item_meta = {}  # name -> {count, classification, is_artifact}
    next_id = ITEM_ID_BASE + 1
    for it in items:
        name = it["name"]
        assert name not in item_name_to_id, f"duplicate item {name}"
        item_name_to_id[name] = next_id
        next_id += 1
        if it.get("progression"):
            cls = "progression"
        elif it.get("useful"):
            cls = "useful"
        elif it.get("trap"):
            cls = "trap"
        else:
            cls = "filler"
        cats = it.get("category", [])
        item_meta[name] = {
            "count": it.get("count", 1),
            "classification": cls,
            "is_artifact": "Artifact" in cats,
        }

    # The dedicated filler item (from game.json) is its own item, not in the
    # itempool list above.  Give it an ID and filler classification.
    filler_name = game.get("filler_item_name", "Filler")
    if filler_name not in item_name_to_id:
        item_name_to_id[filler_name] = next_id
        next_id += 1
        item_meta[filler_name] = {
            "count": 0,            # created on demand only
            "classification": "filler",
            "is_artifact": False,
        }

    # Trait-mode items (Resistances option) get IDs appended after the frozen
    # originals so existing multiworlds keep their IDs.  They carry a trait_mode
    # tag instead of joining ITEM_TABLE's default pool (items.py assembles the
    # pool by mode).
    trait_meta = {}  # name -> {count, classification, mode}
    for name, count, cls, mode in TRAIT_ITEMS:
        assert name not in item_name_to_id, f"duplicate item {name}"
        item_name_to_id[name] = next_id
        next_id += 1
        trait_meta[name] = {
            "count": count,
            "classification": cls,
            "mode": mode,
        }

    equipment_requires, full_extra_requires = trait_region_requires()

    # ---- Regions -----------------------------------------------------------
    # Preserve insertion order; convert requires DSL to (item, count) lists.
    region_data = {}
    for rname, rinfo in regions.items():
        region_data[rname] = {
            "connects_to": list(rinfo.get("connects_to", [])),
            "requires": parse_requires(rinfo.get("requires", [])),
            "starting": bool(rinfo.get("starting", False)),
        }

    # The dungeon is a single linear descent, so each region connects to exactly
    # one deeper region.  Build that ordered chain and a "one band deeper" map.
    region_order = list(region_data.keys())
    next_region = {}
    for rname, rinfo in region_data.items():
        conn = rinfo["connects_to"]
        next_region[rname] = conn[0] if conn else None

    # The deepest region (no outgoing connections) is where Victory lives.
    deepest = [r for r, d in region_data.items() if not d["connects_to"]]
    assert len(deepest) == 1, f"expected one deepest region, got {deepest}"
    victory_region = deepest[0]

    # ---- Locations ---------------------------------------------------------
    # Unique-kill locations first (category starts with "Unique"), then the
    # artifact-pickup locations, then the accumulated "Find #X Artifacts"
    # milestones.  Stable order -> stable IDs.
    def is_artifact_loc(loc):
        return any("Artifact" in c for c in loc.get("category", []))

    # Change #3: push every artifact-pickup location one dungeon band deeper so
    # the early game is a pure unique hunt.  Artifacts already in the deepest
    # band stay put ("except for the ones that are already at the end").
    for loc in locations:
        if is_artifact_loc(loc):
            deeper = next_region.get(loc["region"])
            if deeper is not None:
                loc["region"] = deeper

    ordered = [l for l in locations if not is_artifact_loc(l)] + \
              [l for l in locations if is_artifact_loc(l)]

    location_name_to_id = {}
    location_meta = {}  # name -> {region, is_artifact, is_accumulated}
    next_loc = LOCATION_ID_BASE + 1
    for loc in ordered:
        name = loc["name"]
        assert name not in location_name_to_id, f"duplicate location {name}"
        location_name_to_id[name] = next_loc
        next_loc += 1
        location_meta[name] = {
            "region": loc["region"],
            "is_artifact": is_artifact_loc(loc),
            "is_accumulated": False,
        }

    # Change #4: the "Accumulated" artifacts-as-checks mode replaces the 136
    # per-artifact checks with 136 "Find #X Artifacts" milestone checks.  A
    # milestone fires when the player has picked up its X'th distinct artifact,
    # regardless of which artifacts those are.  We spread the milestones across
    # the dungeon bands by the rule "you are expected to have found this fraction
    # of the artifacts you *could* have found by this depth":
    #   * bands down to Depths 41-50 : 1/3   (a third)
    #   * Depths 51-70               : 1/2
    #   * Depths 71-90               : 3/4
    #   * Depths 91-100              : all remaining
    # so the milestones ramp up to the full artifact count by the bottom.
    accumulated_fraction = {
        "Depths 21-30": 1 / 3,
        "Depths 31-40": 1 / 3,
        "Depths 41-50": 1 / 3,
        "Depths 51-60": 1 / 2,
        "Depths 61-70": 1 / 2,
        "Depths 71-80": 3 / 4,
        "Depths 81-90": 3 / 4,
        "Depths 91-100": 1.0,
    }

    def find_location_name(n):
        return f"Find {n} Artifact" if n == 1 else f"Find {n} Artifacts"

    # Count artifact locations per (post-shift) region.
    arts_per_region = {r: 0 for r in region_order}
    for meta in location_meta.values():
        if meta["is_artifact"]:
            arts_per_region[meta["region"]] += 1

    total_artifacts = sum(arts_per_region.values())
    cumulative = 0
    milestone = 0  # highest "Find #X" already assigned
    for rname in region_order:
        cumulative += arts_per_region[rname]
        frac = accumulated_fraction.get(rname)
        if frac is None:
            continue  # bands with no artifacts (Town, Depths 1-20)
        if rname == victory_region:
            target = total_artifacts  # bottom band mops up all remaining
        else:
            target = int(cumulative * frac)
        while milestone < target:
            milestone += 1
            name = find_location_name(milestone)
            assert name not in location_name_to_id, f"duplicate location {name}"
            location_name_to_id[name] = next_loc
            next_loc += 1
            location_meta[name] = {
                "region": rname,
                "is_artifact": False,
                "is_accumulated": True,
            }
    assert milestone == total_artifacts, (milestone, total_artifacts)

    # ---- Emit --------------------------------------------------------------
    header = '''"""
Generated by _gen_data.py from the manual_angband425_broney data.  Do not edit
by hand; re-run the generator instead.

These tables are the single source of truth shared between generation (this
apworld) and the native game's APCc client.  Every location name is exactly the
string the client sends when the corresponding unique is killed / artifact is
picked up, and every item name is exactly the string the client receives.
"""

ITEM_ID_BASE = {item_base}
LOCATION_ID_BASE = {loc_base}

# The dedicated, infinitely-repeatable filler item.
FILLER_ITEM_NAME = {filler!r}

# The region the Victory event is placed in (the deepest dungeon band).
VICTORY_REGION = {victory!r}
'''.format(item_base=ITEM_ID_BASE, loc_base=LOCATION_ID_BASE,
           filler=filler_name, victory=victory_region)

    body = []
    body.append("# name -> AP item id")
    body.append("ITEM_NAME_TO_ID = " + pyrepr(item_name_to_id))
    body.append("")
    body.append("# name -> {count, classification, is_artifact}")
    body.append("ITEM_TABLE = " + pyrepr(item_meta))
    body.append("")
    body.append("# name -> AP location id")
    body.append("LOCATION_NAME_TO_ID = " + pyrepr(location_name_to_id))
    body.append("")
    body.append("# name -> {region, is_artifact, is_accumulated}")
    body.append("LOCATION_TABLE = " + pyrepr(location_meta))
    body.append("")
    body.append("# region name -> {connects_to, requires:[[item,count]...], starting}")
    body.append("REGION_TABLE = " + pyrepr(region_data))
    body.append("")
    body.append("# Trait-mode (Resistances option) items: name -> {count,")
    body.append("# classification, mode('equipment'|'full')}.  These replace the")
    body.append("# artifact items in the pool when Resistances != Standard.")
    body.append("TRAIT_ITEM_TABLE = " + pyrepr(trait_meta))
    body.append("")
    body.append("# Trait-mode depth gating, replacing the artifact requires:")
    body.append("# region -> [[item, count]...].  Full Traits = union of both tables.")
    body.append("TRAIT_REGION_REQUIRES = " + pyrepr(equipment_requires))
    body.append("")
    body.append("FULL_TRAIT_EXTRA_REQUIRES = " + pyrepr(full_extra_requires))
    body.append("")

    out = header + "\n" + "\n".join(body)
    with open(os.path.join(HERE, "data.py"), "w", encoding="utf-8") as fh:
        fh.write(out)

    # Console summary for sanity.
    n_items = sum(m["count"] for m in item_meta.values())
    n_acc = sum(1 for m in location_meta.values() if m["is_accumulated"])
    n_art = sum(1 for m in location_meta.values() if m["is_artifact"])
    n_unique = len(location_meta) - n_art - n_acc
    print(f"items: {len(item_name_to_id)} names, {n_items} copies")
    print(f"locations: {len(location_name_to_id)} "
          f"({n_unique} unique-kill, {n_art} artifact, {n_acc} accumulated)")
    print(f"regions: {len(region_data)}; victory region: {victory_region}")
    print("wrote data.py")


if __name__ == "__main__":
    main()
