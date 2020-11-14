/* ModifierConfigParser.hpp -- part of the DietyTD Model subsystem
 * implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef MODIFIER_CONFIG_PARSER_HPP
#define MODIFIER_CONFIG_PARSER_HPP

#include "ModifierParser.hpp"
#include "util/TowerModifiers.hpp"
#include <yaml-cpp/yaml.h>

#include <string>
#include <cassert>

struct ModifierMapper {
  ModifierMapper();
  tower_property_modifier parse_tower_config(const std::string &tower_cfg);
  uint32_t modifier_name_to_ID(std::string mod_type);

  std::set<std::pair<int, std::string>> modifier_mapping;
};

template <typename modifierfactory_t> class AttributeMapper {
public:
  explicit AttributeMapper(const std::string &config_file)
      : attribute_factory() {
    parse_modifiers(config_file);
  }

  const modifierfactory_t *get_factory() const { return &attribute_factory; }

private:
  void parse_modifiers(const std::string &config_file);

  template <typename modifier_t>
  void parse_attribute_modifier(YAML::Node &mod_attributes,
                                std::string &&modifier_name);

  modifierfactory_t attribute_factory;
  ModifierMapper mapper;
};

template <typename modifierfactory_t>
void AttributeMapper<modifierfactory_t>::parse_modifiers(
    const std::string &config_file) {
  YAML::Node cfg_root = YAML::LoadFile(config_file);
  if (cfg_root.IsNull()) {
    std::cerr << "ERROR -- config yaml file " << config_file << " not found"
              << std::endl;
    return;
  }

  YAML::Node modifier_node = cfg_root["attributemodifiers"];
  std::cout << modifier_node.size() << " #modifiers" << std::endl;

  for (auto mod_it = modifier_node.begin(); mod_it != modifier_node.end();
       mod_it++) {

    YAML::Node key = mod_it->first;
    assert(key.Type() == YAML::NodeType::Scalar);
    YAML::Node value = mod_it->second;
    assert(value.Type() == YAML::NodeType::Map);

    auto modifier_name = key.as<std::string>();
    const uint32_t ID = mapper.modifier_name_to_ID(modifier_name);
    auto mod_attributes = value["attributes"];

    // std::cout << modifier_name << " ID: " << ID << " -- attributes: " <<
    // mod_attributes << std::endl;

    switch (ID) {
    case TowerModifiers::flat_damage::ID: {
      parse_attribute_modifier<TowerModifiers::flat_damage>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::enhanced_damage::ID: {
      parse_attribute_modifier<TowerModifiers::enhanced_damage>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::enhanced_speed::ID: {
      parse_attribute_modifier<TowerModifiers::enhanced_speed>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::flat_range::ID: {
      parse_attribute_modifier<TowerModifiers::flat_range>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::flat_crit_chance::ID: {
      parse_attribute_modifier<TowerModifiers::flat_crit_chance>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::flat_crit_multiplier::ID: {
      parse_attribute_modifier<TowerModifiers::flat_crit_multiplier>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::flat_type_damage::ID: {
      parse_attribute_modifier<TowerModifiers::flat_type_damage>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::enhanced_type_damage::ID: {
      parse_attribute_modifier<TowerModifiers::enhanced_type_damage>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    case TowerModifiers::flat_damage_onhit::ID: {
      parse_attribute_modifier<TowerModifiers::flat_damage_onhit>(
          mod_attributes, std::move(modifier_name));
      break;
    }
    default: {
      std::cerr << "Invalid ID " << ID << std::endl;
      break;
    }
    }
  }
}

template <typename modifierfactory_t>
template <typename modifier_t>
void AttributeMapper<modifierfactory_t>::parse_attribute_modifier(
    YAML::Node &mod_attributes, std::string &&modifier_name) {
  typename modifier_t::parameter_cfg cfg;
  parse_modifier_parameters(mod_attributes, cfg);
  std::cout << "ID " << modifier_t::ID << " cfg: " << cfg << std::endl;

  // make the modifier generator function
  uint32_t modifier_ID = cfg.get_ID();
  auto mod_fn =
      make_modifier_fn<typename modifierfactory_t::product_generator_t>(
          std::move(cfg));

  /*
  auto mod_attr = *mod_fn(1.f);
  std::cout << typeid(mod_attr).name() << std::endl;
  tower_properties props;
  mod_attr.apply_modifier(props);
  std::cout << modifier_name << ": " << props << std::endl;
  */

  // add to modifier factory
  attribute_factory.register_product(modifier_ID, mod_fn);
}

#endif
