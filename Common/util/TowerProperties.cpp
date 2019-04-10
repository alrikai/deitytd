/* TowerProperties.cpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "TowerProperties.hpp"

std::ostream &operator<<(std::ostream &out_stream,
                         const tower_properties &props) {
  const static std::array<std::string, tower_properties::NUM_ELEM>
      element_names{{"CHAOS", "WATER", "AIR", "FIRE", "EARTH"}};
  for (int eidx = 0; eidx < tower_properties::NUM_ELEM; eidx++) {
    out_stream << element_names[eidx] << " Damage: ["
               << props.modifier.damage_value[eidx].low << ", "
               << props.modifier.damage_value[eidx].high << "] \n"
               << element_names[eidx] << "\%-Enhanced Damage: ["
               << props.modifier.enhanced_damage_value[eidx] << "] \n"
               << element_names[eidx] << "\%-Element affinity: ["
               << props.modifier.enhanced_damage_affinity[eidx] << "] \n"
               << element_names[eidx] << "Added Damage: ["
               << props.modifier.added_damage_value[eidx] << "] \n";
  }
  out_stream << "Armor Piercing Damage: " << props.modifier.armor_pierce_damage
             << "\n"
             << "\%-Enhanced Speed: " << props.modifier.enhanced_speed_value
             << "\n"
             << "Speed: " << props.modifier.attack_speed_value << "\n"
             << " Range: " << props.modifier.attack_range_value << "\n"
             << "Crit: " << props.modifier.crit_chance_value
             << " Crit Multiplier: " << props.modifier.crit_multiplier_value
             << "\n";

  out_stream << "# On Attack Effects: "
             << props.modifier.on_attack_events.size() << "\n";
  out_stream << "# On Hit Effects: " << props.modifier.on_hit_events.size()
             << "\n";
  out_stream << "# On Death Effects: " << props.modifier.on_death_events.size()
             << "\n";
  /*
  for (auto& evt : props.modifier.on_attack_events) out_stream << *evt << "\n";
  out_stream << "On Hit Effects: \n";
  for (auto& evt : props.modifier.on_hit_events) out_stream << *evt << "\n";
  out_stream << "On Death Effects: \n";
  for (auto& evt : props.modifier.on_death_events) out_stream << *evt << "\n";
  */
  return out_stream;
}
//----------------------------------------------------------------------------------------
