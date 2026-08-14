from collections.abc import Mapping
from typing import Any

# Base Archipelago imports are absolute; our own files are relative.
from Options import OptionError
from worlds.AutoWorld import World

from . import data, items, locations, options, regions, rules, web_world

# The world api is also documented in one continuous file here:
# https://github.com/ArchipelagoMW/Archipelago/blob/main/docs/world%20api.md


class AngbandWorld(World):
    """
    Angband is a classic roguelike dungeon crawl set in Tolkien's Middle-earth.
    Descend a hundred dungeon levels, slay the unique foes lurking there, and
    ultimately defeat Morgoth, Lord of Darkness.
    """

    game = "Angband"

    web = web_world.AngbandWebWorld()

    options_dataclass = options.AngbandOptions
    options: options.AngbandOptions  # Note: a colon, not an equals sign.

    location_name_to_id = locations.LOCATION_NAME_TO_ID
    item_name_to_id = items.ITEM_NAME_TO_ID

    # The descent starts in Town rather than the default "Menu".
    origin_region_name = "Town"

    def generate_early(self) -> None:
        # The Resistances trait modes replace the artifact items with trait
        # items, so they need the artifact/milestone locations to hold them:
        # with Artifacts As Checks off there are only the 96 unique-kill
        # locations and the pool cannot fit.
        if (int(self.options.resistances.value) != 0
                and int(self.options.artifacts_as_checks.value) == 0):
            raise OptionError(
                f"Angband ({self.player_name}): Resistances "
                f"'{self.options.resistances.current_key}' requires "
                f"Artifacts As Checks to be 'one_to_one' or 'accumulated', "
                f"not 'off'."
            )

    def create_regions(self) -> None:
        regions.create_and_connect_regions(self)
        locations.create_all_locations(self)

    def set_rules(self) -> None:
        rules.set_all_rules(self)

    def create_items(self) -> None:
        items.create_all_items(self)

    def create_item(self, name: str) -> items.AngbandItem:
        return items.create_item_with_correct_classification(self, name)

    def get_filler_item_name(self) -> str:
        return items.get_random_filler_item_name(self)

    def fill_slot_data(self) -> Mapping[str, Any]:
        # Sent to the native game on every connection.  The APCc client reads
        # "artifacts_as_checks" (to switch artifacts between checks and normal
        # drops), "resistances" (Standard / Equipment Traits / Full Traits),
        # "black_market_price_multiplier" (the artifact-location gold sink's
        # price multiplier), and "death_link".
        return self.options.as_dict(
            "artifacts_as_checks", "resistances",
            "black_market_price_multiplier", "death_link"
        )
