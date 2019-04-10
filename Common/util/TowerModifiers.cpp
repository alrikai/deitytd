/* TowerModifiers.cpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "TowerModifiers.hpp"

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_damage::parameter_cfg &cfg) {
  stream << "flat_damage::parameter_cfg: [ " << cfg.low_val << ", "
         << cfg.high_val << " ]";
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::enhanced_damage::parameter_cfg &cfg) {
  stream << "enhanced_damage::parameter_cfg: " << cfg.ed_percent << "%";
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::enhanced_speed::parameter_cfg &cfg) {
  stream << "enhanced_speed::parameter_cfg: " << cfg.es_percent << "%";
  return stream;
}

std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_range::parameter_cfg &cfg) {
  stream << "flat_range::parameter_cfg: " << cfg.range << " units";
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_crit_chance::parameter_cfg &cfg) {
  stream << "flat_crit_chance::parameter_cfg: " << cfg.crit_percent << " %";
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_crit_multiplier::parameter_cfg &cfg) {
  stream << "flat_crit_multiplier::parameter_cfg: "
         << cfg.crit_multiplier_percent << " %";
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_type_damage::parameter_cfg &cfg) {
  static const std::string element_names[] = {"CHAOS", "WATER", "AIR", "FIRE",
                                              "EARTH"};

  stream << "flat_type_damage::parameter_cfg: [" << cfg.low_val << ", "
         << cfg.high_val << "] @ " << element_names[static_cast<int>(cfg.type)];
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::enhanced_type_damage::parameter_cfg &cfg) {
  static const std::string element_names[] = {"CHAOS", "WATER", "AIR", "FIRE",
                                              "EARTH"};

  stream << "enhanced_type_damage::parameter_cfg: " << cfg.ed_percent_amount
         << " @ " << element_names[static_cast<int>(cfg.type)];
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_damage_onhit::parameter_cfg &cfg) {
  stream << "flat_damage_onhit::parameter_cfg: inc -- " << cfg.increment_amount
         << " max -- " << cfg.cap << " duration -- " << cfg.duration;
  return stream;
}

std::ostream &
operator<<(std::ostream &stream,
           const TowerModifiers::flat_added_damage::parameter_cfg &cfg) {
  static const std::string element_names[] = {"CHAOS", "WATER", "AIR", "FIRE",
                                              "EARTH"};

  stream << "flat_added_damage::parameter_cfg: " << cfg.flat_dmg_amount << " @ "
         << element_names[static_cast<int>(cfg.type)];
  return stream;
}
