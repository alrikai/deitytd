/* TowerProperties.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TOWER_PROPERTIES_HPP
#define TOWER_PROPERTIES_HPP

#include "Elements.hpp"
#include <vector>

struct event_attribute_modifier;

struct tower_property_modifier {
  using dmg_dist = range<float>;
  static constexpr int NUM_ELEM = 5;
  using damage_type = std::array<dmg_dist, NUM_ELEM>;

  tower_property_modifier()
      : damage_value{{dmg_dist(0, 0), dmg_dist(0, 0), dmg_dist(0, 0),
                      dmg_dist(0, 0), dmg_dist(0, 0)}} {
    for (int elem_idx = 0; elem_idx < NUM_ELEM; elem_idx++) {
      enhanced_damage_value[elem_idx] = 0.f;
    }
    enhanced_speed_value = 0.f;
    attack_speed_value = 0.f;
    attack_range_value = 0.f;

    crit_chance_value = 0.f;
    crit_multiplier_value = 50.f;
  }

  // low and high range of attack damage per damage type
  damage_type damage_value;
  float enhanced_damage_value[NUM_ELEM];

  float enhanced_speed_value;

  // measured in attacks per second
  float attack_speed_value;

  // attack radius. not sure of the units quite yet
  float attack_range_value;

  // critical hit stats
  float crit_chance_value;
  float crit_multiplier_value;

  // eventually: will have mana amount, mana regen rate

  // event lists for on-hit and on-death effects. When these events are
  // triggered, will loop through these event lists and fire them off
  std::vector<event_attribute_modifier *> on_hit_events;
  std::vector<event_attribute_modifier *> on_death_events;
};

struct tower_properties {
  using dmg_dist = tower_property_modifier::dmg_dist;
  static constexpr int NUM_ELEM = tower_property_modifier::NUM_ELEM;
  using damage_type = tower_property_modifier::damage_type;

  tower_properties()
      : damage{{dmg_dist(0, 0), dmg_dist(0, 0), dmg_dist(0, 0), dmg_dist(0, 0),
                dmg_dist(0, 0)}} {
    attack_speed = 0.f;
    attack_range = 0.f;

    crit_chance = 0.f;
    crit_multiplier = 50.f;
  }

  tower_properties &operator+=(const tower_properties &rhs_modifier) {
    auto this_it = damage.begin();
    auto rhs_it = rhs_modifier.damage.begin();
    for (int eidx = 0; eidx < NUM_ELEM; eidx++, this_it++, rhs_it++) {
      *this_it += *rhs_it;
    }

    attack_speed += rhs_modifier.attack_speed;
    attack_range += rhs_modifier.attack_range;

    crit_chance += rhs_modifier.crit_chance;
    crit_multiplier += rhs_modifier.crit_multiplier;

    // also take the RHS properties' events
    on_hit_events.insert(std::end(on_hit_events),
                         std::begin(rhs_modifier.on_hit_events),
                         std::end(rhs_modifier.on_hit_events));
    on_death_events.insert(std::end(on_death_events),
                           std::begin(rhs_modifier.on_death_events),
                           std::end(rhs_modifier.on_death_events));

    return *this;
  }

  // NOTE: there's an optimization oppotunity here to move from the modifier's
  // on-event vectors?
  void apply_property_modifier(tower_property_modifier modifier) {
    // NOTE: the order that we do the combination matters here

    // apply the +damage first, then the +%enhanced damage
    for (int dmg_idx = 0; dmg_idx < NUM_ELEM; dmg_idx++) {
      damage[dmg_idx] += modifier.damage_value[dmg_idx];
      damage[dmg_idx] +=
          damage[dmg_idx] * modifier.enhanced_damage_value[dmg_idx];
    }

    attack_speed += modifier.attack_speed_value;
    attack_speed += attack_speed * modifier.enhanced_speed_value;

    attack_range += modifier.attack_range_value;
    crit_chance += modifier.crit_chance_value;
    crit_multiplier += modifier.crit_multiplier_value;

    on_hit_events.insert(std::end(on_hit_events),
                         std::begin(modifier.on_hit_events),
                         std::end(modifier.on_hit_events));
    on_death_events.insert(std::end(on_death_events),
                           std::begin(modifier.on_death_events),
                           std::end(modifier.on_death_events));
  }

  // low and high range of attack damage per damage type
  damage_type damage;

  // measured in attacks per second
  float attack_speed;

  // attack radius. not sure of the units quite yet
  float attack_range;

  // critical hit stats
  float crit_chance;
  float crit_multiplier;

  // eventually: will have mana amount, mana regen rate

  // event lists for on-hit and on-death effects. When these events are
  // triggered, will loop through these event lists and fire them off
  std::vector<event_attribute_modifier *> on_hit_events;
  std::vector<event_attribute_modifier *> on_death_events;

  friend std::ostream &operator<<(std::ostream &out_stream,
                                  const tower_properties &props);
};

inline tower_properties operator+(tower_properties lhs_modifier,
                                  const tower_properties &rhs_modifier) {
  lhs_modifier += rhs_modifier;
  return lhs_modifier;
}

#endif
