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
      : damage_value{}, enhanced_damage_value{}, enhanced_damage_affinity{},
        added_damage_value{} {
    enhanced_speed_value = 0;
    attack_speed_value = 0;
    attack_range_value = 0;
    crit_chance_value = 0;
    crit_multiplier_value = 0;
    armor_pierce_damage = 0;
  }

  void merge(tower_property_modifier other) {
    for (int elem_idx = 0; elem_idx < NUM_ELEM; elem_idx++) {
      damage_value[elem_idx] += other.damage_value[elem_idx];
      enhanced_damage_value[elem_idx] += other.enhanced_damage_value[elem_idx];
      enhanced_damage_affinity[elem_idx] +=
          other.enhanced_damage_affinity[elem_idx];
      added_damage_value[elem_idx] += other.added_damage_value[elem_idx];
    }
    enhanced_speed_value += other.enhanced_speed_value;
    attack_speed_value += other.attack_speed_value;
    attack_range_value += other.attack_range_value;

    armor_pierce_damage += other.armor_pierce_damage;

    crit_chance_value += other.crit_chance_value;
    crit_multiplier_value += other.crit_multiplier_value;

    // take the effects from the other modfifier
    on_attack_events.insert(std::end(on_attack_events),
                            std::begin(other.on_attack_events),
                            std::end(other.on_attack_events));
    on_hit_events.insert(std::end(on_hit_events),
                         std::begin(other.on_hit_events),
                         std::end(other.on_hit_events));
    on_death_events.insert(std::end(on_death_events),
                           std::begin(other.on_death_events),
                           std::end(other.on_death_events));
  }

  // low and high range of attack damage per damage type
  damage_type damage_value;
  std::array<float, NUM_ELEM> enhanced_damage_value;
  // for +%-enhanced damage to <X> types
  std::array<float, NUM_ELEM> enhanced_damage_affinity;
  // damage added post-modifiers per element
  std::array<float, NUM_ELEM> added_damage_value;

  // typeless post-armor damage
  float armor_pierce_damage;

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
  std::vector<event_attribute_modifier *> on_attack_events;
  std::vector<event_attribute_modifier *> on_hit_events;
  std::vector<event_attribute_modifier *> on_death_events;
};

struct tower_properties {
  using dmg_dist = tower_property_modifier::dmg_dist;
  static constexpr int NUM_ELEM = tower_property_modifier::NUM_ELEM;
  using damage_type = tower_property_modifier::damage_type;

  tower_properties() : modifier() {}

  tower_properties &operator+=(const tower_properties &rhs_modifier) {
    apply_property_modifier(rhs_modifier.modifier);
    return *this;
  }

  // NOTE: there's an optimization opportunity here to move from the modifier's
  // on-event vectors?
  void apply_property_modifier(tower_property_modifier other) {
    modifier.merge(other);
  }

  friend std::ostream &operator<<(std::ostream &out_stream,
                                  const tower_properties &props);

  tower_property_modifier modifier;
};

inline tower_properties operator+(tower_properties lhs_modifier,
                                  const tower_properties &rhs_modifier) {
  lhs_modifier += rhs_modifier;
  return lhs_modifier;
}

#endif
