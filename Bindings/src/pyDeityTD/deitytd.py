import _pyDTD as dtd

from typing import List, NamedTuple
from dataclasses import dataclass

#TODO: have any required python overrides here
def DTDversion():
    return "we don't really do versions..."

def modifier_factory():
    pass

class ElementRanges(NamedTuple):
    #TODO: this is an implicit dependency on the tower_properties definition
    #order is: {"CHAOS", "WATER", "AIR", "FIRE", "EARTH"}:
    chaos: dtd.range
    water: dtd.range
    air: dtd.range
    fire: dtd.range
    earth: dtd.range

class ElementTypes(NamedTuple):
    chaos: float
    water: float
    air: float
    fire: float
    earth: float

#TODO: consider ways to make constructing the tower modifiers more streamlined /
#less verbose. Is there a better way other than constructing individual modifiers
#and composing them together?
@dataclass
class TowerModifiers():
    damage_values: ElementRanges
    enhaned_damage_value: ElementTypes
    enhanced_damage_affinit: ElementTypes
    added_damage_value: ElementTypes
    armor_pierce_damage: float
    enhanced_speed_value: float
    attack_speed_value: float
    attack_range_value: float
    crit_chance_value: float
    crit_multiplier_value: float

def ModifierFactory():
    modifier_mapping = {"damage_values": lambda mod: mod.damage_values}

