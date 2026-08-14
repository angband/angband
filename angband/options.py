from dataclasses import dataclass

from Options import Choice, DeathLink, OptionGroup, PerGameCommonOptions, Range

# Angband's player-facing options.  These end up in the template yaml and on the
# WebHost options page.  See the Options API doc:
# https://github.com/ArchipelagoMW/Archipelago/blob/main/docs/options%20api.md


class ArtifactsAsChecks(Choice):
    """
    Choose how artifacts interact with Archipelago.

    "One to One" (the default and the way the randomizer is balanced): every
    artifact is its own location check.  Picking up an artifact sends that
    artifact's check and the real artifacts are instead handed out by the
    multiworld.  Descending past the early dungeon also requires having received
    enough "Progressive ... Artifact" items, so the artifacts gate progression.

    "Accumulated": artifacts are still handed out by the multiworld and still gate
    descent exactly as in One to One, but the location checks are milestones -
    "Find #X Artifacts" - that fire on your X'th distinct artifact pickup no matter
    which artifacts you find.  The milestones are spread deeper the further you
    descend (see the docs), reaching every artifact by the bottom.

    "Off": artifacts spawn and behave normally, only unique-monster kills are
    checks, and the dungeon has no artifact gating.

    The connected game reads this from slot_data (key "artifacts_as_checks":
    0 = Off, 1 = One to One, 2 = Accumulated).
    """

    display_name = "Artifacts As Checks"
    option_off = 0
    option_one_to_one = 1
    option_accumulated = 2
    default = 1

    # Keep older yamls that used the boolean toggle (true/on, false/off) working.
    aliases = {"true": 1, "on": 1, "false": 0}


class Resistances(Choice):
    """
    Choose how the character's power curve works.

    "Standard": normal Angband play.  Your equipment is whatever you find, and
    (if artifacts are checks) the multiworld hands out artifacts.

    "Equipment Traits": you cannot use artifacts and cannot equip anything except
    a ranged weapon and a light source.  Instead you wield an unremovable
    Archipelago Weapon, and the multiworld items are traits that improve it:
    every resistance, protections, to-hit/damage/dice/lightness, armor class,
    speed, extra blows and three progressive slays.  The fire/cold/lightning/acid
    resists are progressive (resist, then an at-will temporary-resist activation,
    then immunity).  Dungeon depth is gated on these traits instead of artifacts.

    "Full Traits": Equipment Traits, and additionally your character is switched
    to the Archipelago race - worst-in-everything - whose stats and skills are
    improved by additional multiworld trait items.

    Trait modes replace the artifact items in the pool, so they require
    Artifacts As Checks to be On (One to One or Accumulated).

    The connected game reads this from slot_data (key "resistances": 0 =
    Standard, 1 = Equipment Traits, 2 = Full Traits).
    """

    display_name = "Resistances"
    option_standard = 0
    option_equipment_traits = 1
    option_full_traits = 2
    default = 0


class BlackMarketPriceMultiplier(Range):
    """
    Price multiplier for the Black Market's "buy a missed artifact location"
    gold sink.

    While artifacts are checks (One to One or Accumulated), pressing $ in the
    Black Market lets you spend gold to send an artifact location check you would
    otherwise have to hunt for.  The cost is this multiplier times the artifact's
    base value; a higher multiplier makes the gold sink more expensive.

    The connected game reads this from slot_data (key
    "black_market_price_multiplier").  Has no effect when Artifacts As Checks is
    Off.
    """

    display_name = "Black Market Price Multiplier"
    range_start = 1
    range_end = 5
    default = 3


# DeathLink is a standard Archipelago option; we reuse the built-in class so it
# behaves consistently with every other game.  The native client supports it.
# (DeathLink defaults to off; players opt in from their yaml.)


@dataclass
class AngbandOptions(PerGameCommonOptions):
    artifacts_as_checks: ArtifactsAsChecks
    resistances: Resistances
    black_market_price_multiplier: BlackMarketPriceMultiplier
    death_link: DeathLink


option_groups = [
    OptionGroup(
        "Gameplay Options",
        [ArtifactsAsChecks, Resistances, BlackMarketPriceMultiplier, DeathLink],
    ),
]
