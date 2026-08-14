from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import Location

from . import data, items

if TYPE_CHECKING:
    from .world import AngbandWorld

# Every location needs a unique integer ID, present in this lookup even if it
# doesn't exist under some option combination.  Generated in data.py so the names
# match exactly the strings the native client sends (unique-monster names and
# artifact names).
LOCATION_NAME_TO_ID = data.LOCATION_NAME_TO_ID


class AngbandLocation(Location):
    game = "Angband"


def get_location_names_with_ids(location_names: list[str]) -> dict[str, int | None]:
    # Helper mirroring APQuest's: turn a list of names into the {name: id} subset
    # that region.add_locations wants, avoiding copy-paste id typos.
    return {name: LOCATION_NAME_TO_ID[name] for name in location_names}


def create_all_locations(world: AngbandWorld) -> None:
    create_regular_locations(world)
    create_events(world)


def create_regular_locations(world: AngbandWorld) -> None:
    # artifacts_as_checks is a 3-way choice: 0 = Off, 1 = One to One (per-artifact
    # location checks), 2 = Accumulated ("Find #X Artifacts" milestone checks).
    mode = int(world.options.artifacts_as_checks.value)

    # Bucket the location names by their region.  Unique-kill locations always
    # exist; the two artifact location kinds are mutually exclusive and only one
    # (or neither, in Off mode) is created.
    by_region: dict[str, list[str]] = {}
    for name, meta in data.LOCATION_TABLE.items():
        if meta["is_artifact"] and mode != 1:
            continue
        if meta["is_accumulated"] and mode != 2:
            continue
        by_region.setdefault(meta["region"], []).append(name)

    for region_name, names in by_region.items():
        region = world.get_region(region_name)
        region.add_locations(get_location_names_with_ids(names), AngbandLocation)


def create_events(world: AngbandWorld) -> None:
    # Victory in Angband is slaying Morgoth in the deepest dungeon band.  We model
    # it the standard way: an event location holding a locked "Victory" event item,
    # placed in the deepest region so it is only reachable once that region's
    # descent requirements are met.  rules.py ties completion to this item.
    victory_region = world.get_region(data.VICTORY_REGION)
    victory_region.add_event(
        "Slay Morgoth", "Victory", location_type=AngbandLocation, item_type=items.AngbandItem
    )
