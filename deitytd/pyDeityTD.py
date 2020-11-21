import _pyDTD as dtdcore

from typing import List, NamedTuple
from dataclasses import dataclass

def modifier_factory():
    pass

class ElementRanges(NamedTuple):
    #TODO: this is an implicit dependency on the tower_properties definition
    #order is: {"CHAOS", "WATER", "AIR", "FIRE", "EARTH"}:
    chaos: dtdcore.range
    water: dtdcore.range
    air: dtdcore.range
    fire: dtdcore.range
    earth: dtdcore.range

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
class TowerModifiersProperties():
    damage_values: ElementRanges
    enhaned_damage_value: ElementTypes
    enhanced_damage_affinity: ElementTypes
    added_damage_value: ElementTypes
    armor_pierce_damage: float
    enhanced_speed_value: float
    attack_speed_value: float
    attack_range_value: float
    crit_chance_value: float
    crit_multiplier_value: float

"""
class TowerPropertyFactory():

    def __init__():
        self.modifier_factory = {}

    def construct (self, properties: List[TowerModifiersProperties]):
        tmod = dtdcore.tower_property_modifier()
        for modifier in properties:
            tmod.merge(modifier)
        tprop = dtdcore.tower_properties()
        tprop.apply_property_modifier(tmod)
        return tprop

    def generate_modifier(mod_key, value):
        modifier_values = self.modifier_factory[mod_key](value)
        return dtdcore.tower_property_modifier)(modifier_values)

    def _make_modifier():
        mod = TowerModifierProperties()



def ModifierFactory():
    modifier_mapping = {"damage_values": lambda mod: mod.damage_values}
"""
