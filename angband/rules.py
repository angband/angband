from __future__ import annotations

from typing import TYPE_CHECKING

from worlds.generic.Rules import set_rule

from . import data
from .regions import entrance_name

if TYPE_CHECKING:
    from .world import AngbandWorld


def set_all_rules(world: AngbandWorld) -> None:
    set_all_entrance_rules(world)
    set_completion_condition(world)


def region_requires(world: AngbandWorld, region: str) -> list[list]:
    """The [[item, count]...] needed to enter a region, per the active mode."""
    resistances = int(world.options.resistances.value)
    if resistances == 0:
        # Standard: the "Progressive ... Artifact" gating from the region table.
        return data.REGION_TABLE[region]["requires"]
    # Trait modes: trait-item gating; Full Traits adds the stat-boost ramps.
    requires = list(data.TRAIT_REGION_REQUIRES.get(region, []))
    if resistances == 2:
        requires += data.FULL_TRAIT_EXTRA_REQUIRES.get(region, [])
    return requires


def set_all_entrance_rules(world: AngbandWorld) -> None:
    # Each deeper dungeon band lists the items required to descend into it
    # (progressive artifacts in Standard, trait items in the trait modes).  We
    # put that requirement on the single entrance leading into the band.  When
    # artifacts aren't checks, the artifact items don't exist, so Standard mode
    # leaves the descent ungated (the dungeon is a straight unique hunt); the
    # trait modes require AAC to be on (enforced in world.generate_early).
    if (int(world.options.artifacts_as_checks.value) == 0
            and int(world.options.resistances.value) == 0):
        return

    for src, info in data.REGION_TABLE.items():
        for dst in info["connects_to"]:
            requires = region_requires(world, dst)
            if not requires:
                continue
            entrance = world.get_entrance(entrance_name(src, dst))
            # Bind the requirement list as a default arg so the lambda captures
            # this specific entrance's needs rather than the loop variable.
            set_rule(
                entrance,
                lambda state, reqs=tuple((n, c) for n, c in requires): all(
                    state.has(name, world.player, count) for name, count in reqs
                ),
            )


def set_completion_condition(world: AngbandWorld) -> None:
    # The goal is the Victory event placed in the deepest band (see
    # locations.create_events).  Reaching it already implies satisfying every
    # descent requirement, so this is all we need.
    world.multiworld.completion_condition[world.player] = (
        lambda state: state.has("Victory", world.player)
    )
