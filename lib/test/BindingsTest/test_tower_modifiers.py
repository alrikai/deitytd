import pyDeityTD as deitytd

import pytest

NUM_ELEMENTS = 5

@pytest.mark.parametrize("modifier_property,value", [("damage_value", deitytd.range(10, 20)), ("enhanced_damage_value", 10), ("enhanced_damage_affinity", 10), ("added_damage_value", 10)])
def test_tower_properties_simple_perelement(modifier_property,value):
    tmod = deitytd.tower_property_modifier()
    default_val = type(value)()
    for index in range(NUM_ELEMENTS):
        getattr(tmod, modifier_property)[index] = value
        tprop = deitytd.tower_properties()
        tprop.apply_property_modifier(tmod)
        assert getattr(tprop.modifier, modifier_property)[index] == value
        getattr(tmod.modifier, modifier_property)[index] = default_val 

@pytest.mark.parametrize("modifier_property,value", [("armor_pierce_damage", 20), ("enhanced_speed_value", 10), ("attack_speed_value", 10), ("attack_range_value", 10), ("crit_chance_value", 10), ("crit_multiplier_value", 10)])
def test_tower_properties_simple_properties(modifier_property,value):
    tprop = deitytd.tower_properties()
    tmod = deitytd.tower_property_modifier()
    setattr(tmod, modifier_property, value)
    tprop.apply_property_modifier(tmod)
    assert getattr(tprop.modifier, modifier_property) == value
