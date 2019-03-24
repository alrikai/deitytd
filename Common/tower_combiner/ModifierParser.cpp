/* ModifierParser.cpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ModifierParser.hpp"

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               flat_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.low_val = mod_attributes["value_low"].as<float>();
  modifier_cfg.high_val = mod_attributes["value_high"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               enhanced_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.ed_percent = mod_attributes["percent_edamage"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               enhanced_speed::parameter_cfg &modifier_cfg) {
  modifier_cfg.es_percent = mod_attributes["percent_espeed"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               flat_range::parameter_cfg &modifier_cfg) {
  modifier_cfg.range = mod_attributes["value_range"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               flat_crit_chance::parameter_cfg &modifier_cfg) {
  modifier_cfg.crit_percent = mod_attributes["percent_ecrit"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    flat_crit_multiplier::parameter_cfg &modifier_cfg) {
  modifier_cfg.crit_multiplier_percent =
      mod_attributes["value_critmult"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               flat_type_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.type =
      static_cast<Elements>(mod_attributes["dmg_type"].as<int>());
  modifier_cfg.flat_dmg_amount = mod_attributes["dmg_amount"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    enhanced_type_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.type =
      static_cast<Elements>(mod_attributes["dmg_type"].as<int>());
  modifier_cfg.ed_percent_amount = mod_attributes["dmg_percent"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

void parse_modifier_parameters(const YAML::Node &mod_attributes,
                               flat_damage_onhit::parameter_cfg &modifier_cfg) {
  modifier_cfg.increment_amount = mod_attributes["dmg_inc"].as<float>();
  modifier_cfg.cap = mod_attributes["max_dmg"].as<float>();
  modifier_cfg.duration = mod_attributes["dmg_duration"].as<float>();
  modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
}

/*
void parse_modifier_parameters(const YAML::Node& mod_attributes,
::parameter_cfg& modifier_cfg)
{
}
std::ostream& operator<< (std::ostream& stream, const ::parameter_cfg& cfg)
{
}
*/
