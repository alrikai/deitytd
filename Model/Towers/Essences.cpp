#include "Essences.hpp"

/*
 *  NOTE: these values are arbitrary and should really be ready fron config files if anything
 */

void aphrodite::apply_modifier(tower_properties& properties)
{
	properties.attack_range += 10;
};

void apollo::apply_modifier(tower_properties& properties)
{
	properties.attack_range += 5;
	properties.attack_speed += 5;
};

void ares::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::CHAOS)].damage_range += tower_properties::dmg_dist(5, 5);
    properties.damage[static_cast<int>(Elements::EARTH)].damage_range += tower_properties::dmg_dist(5, 5);
	//properties.damage.find(Elements::EARTH)->second += tower_properties::dmg_dist(5, 5);
	//properties.damage.find(Elements::CHAOS)->second += tower_properties::dmg_dist(5, 5);
};

void artemis::apply_modifier(tower_properties& properties)
{
    properties.crit_multiplier += 100;
};

void athena::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::AIR)].damage_range += tower_properties::dmg_dist(5, 5);
	//properties.damage.find(Elements::AIR)->second += tower_properties::dmg_dist(5, 5);
	properties.attack_range += 5;
};

void demeter::apply_modifier(tower_properties& properties)
{
    properties.crit_chance += 2;
};

void dionysus::apply_modifier(tower_properties& properties)
{
    properties.attack_speed += 5;
    properties.crit_chance += 1;
};

void hades::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::EARTH)].damage_range += tower_properties::dmg_dist(10, 10);
	//properties.damage.find(Elements::EARTH)->second += tower_properties::dmg_dist(10, 10);
};

void hephaestus::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::FIRE)].damage_range += tower_properties::dmg_dist(10, 10);
	//properties.damage.find(Elements::FIRE)->second += tower_properties::dmg_dist(10, 10);
};

void hera::apply_modifier(tower_properties& properties)
{
    properties.crit_chance += 1;
    properties.crit_multiplier += 50;
};

void hermes::apply_modifier(tower_properties& properties)
{
    properties.attack_speed += 10;
};

void hestia::apply_modifier(tower_properties& properties)
{
    properties.attack_speed += 5;
    properties.crit_multiplier += 50;
};

void poseidon::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::WATER)].damage_range += tower_properties::dmg_dist(10, 10);
	//properties.damage.find(Elements::WATER)->second += tower_properties::dmg_dist(10, 10);
};

void zeus::apply_modifier(tower_properties& properties)
{
    properties.damage[static_cast<int>(Elements::CHAOS)].damage_range += tower_properties::dmg_dist(10, 10);
	//properties.damage.find(Elements::CHAOS)->second += tower_properties::dmg_dist(10, 10);
};

