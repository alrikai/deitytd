/* ModifierParser.cpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ModifierParser.hpp"
#include "Common/util/Elements.hpp"

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.low_val = mod_attributes["value_low"].as<float>();
  modifier_cfg.high_val = mod_attributes["value_high"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::enhanced_damage::parameter_cfg &modifier_cfg) {
  modifier_cfg.ed_percent = mod_attributes["percent_edamage"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::enhanced_speed::parameter_cfg &modifier_cfg) {
  modifier_cfg.es_percent = mod_attributes["percent_espeed"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_range::parameter_cfg &modifier_cfg) {
  modifier_cfg.range = mod_attributes["value_range"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_crit_chance::parameter_cfg &modifier_cfg) {
  modifier_cfg.crit_percent = mod_attributes["percent_ecrit"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_crit_multiplier::parameter_cfg &modifier_cfg) {
  modifier_cfg.crit_multiplier_percent =
      mod_attributes["value_critmult"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_type_damage::parameter_cfg &modifier_cfg) {
  auto elem_name = mod_attributes["elem"].as<std::string>();
  modifier_cfg.type = ElementInfo::get_element_type(elem_name);
  modifier_cfg.low_val = mod_attributes["value_low"].as<float>();
  modifier_cfg.high_val = mod_attributes["value_high"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::enhanced_type_damage::parameter_cfg &modifier_cfg) {
  auto elem_name = mod_attributes["elem"].as<std::string>();
  modifier_cfg.type = ElementInfo::get_element_type(elem_name);
  modifier_cfg.ed_percent_amount =
      mod_attributes["percent_edamage"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
}

void parse_modifier_parameters(
    const YAML::Node &mod_attributes,
    TowerModifiers::flat_damage_onhit::parameter_cfg &modifier_cfg) {
  modifier_cfg.increment_amount = mod_attributes["dmg_inc"].as<float>();
  modifier_cfg.cap = mod_attributes["max_dmg"].as<float>();
  modifier_cfg.duration = mod_attributes["dmg_duration"].as<float>();
  if (mod_attributes["scale_factor"]) {
    modifier_cfg.scale_factor = mod_attributes["scale_factor"].as<float>();
  } else {
    modifier_cfg.scale_factor = 1.f;
  }
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
