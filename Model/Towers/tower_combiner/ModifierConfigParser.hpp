/* ModifierConfigParser.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */




#ifndef MODIFIER_CONFIG_PARSER_HPP
#define MODIFIER_CONFIG_PARSER_HPP

#include "ModifierParser.hpp"
#include <yaml-cpp/yaml.h>

#include <string>

template <typename modifierfactory_t, typename modifier_t>
void parse_modifier(modifierfactory_t& attribute_fact, YAML::Node& mod_attributes, std::string&& modifier_name)
{
    typename modifier_t::parameter_cfg cfg;
    parse_modifier_parameters (mod_attributes, cfg);
    std::cout << "ID " << modifier_t::ID << " cfg: " << cfg << std::endl;
 
    //TODO: need to make a unique ID for the flat_type_damage_{} and enhanced_type_damage_{} types
    uint32_t modifier_ID = cfg.get_ID();   
    //make the modifier generator function
    auto mod_fn = make_modifier_fn<typename modifierfactory_t::product_generator_t> (std::move(cfg));

    /*
    auto mod_attr = *mod_fn(1.f);
    std::cout << typeid(mod_attr).name() << std::endl;
    tower_properties props;
    mod_attr.apply_modifier(props);
    std::cout << modifier_name << ": " << props << std::endl;
    */

    //add to modifier factory  
    attribute_fact.register_product (modifier_ID, mod_fn);
}

template <typename modifierfactory_t>
void get_modifier_configs(modifierfactory_t& attribute_fact, const std::string& config_file)
{
	static constexpr int NUM_MODIFIERS = 17;
	std::array<std::string, NUM_MODIFIERS> modifier_names = {
		{"flat_damage", "enhanced_damage", "enhanced_speed", "flat_range", "flat_crit_chance", "flat_crit_multiplier", 
         "flat_type_damage_chaos", "flat_type_damage_water", "flat_type_damage_air", "flat_type_damage_fire", "flat_type_damage_earth", 
		 "enhanced_type_damage_chaos", "enhanced_type_damage_water", "enhanced_type_damage_air", "enhanced_type_damage_fire", "enhanced_type_damage_earth", 
		 "flat_damage_onhit"}};

    YAML::Node cfg_root = YAML::LoadFile(config_file);
	if(cfg_root.IsNull()) {
        std::cerr << "ERROR -- config yaml file " << config_file << " not found" << std::endl;
		return;
	}

	YAML::Node modifier_node = cfg_root ["attributemodifiers"];
	std::cout << modifier_node.size() << " #modifiers" << std::endl;

    for (auto mod_it = modifier_node.begin(); mod_it != modifier_node.end(); mod_it++) {

        YAML::Node key = mod_it->first;
        assert(key.Type() == YAML::NodeType::Scalar);
        YAML::Node value = mod_it->second;
        assert(value.Type() == YAML::NodeType::Map);
        
        auto modifier_name = key.as<std::string>();
        const uint32_t ID = value["ID"].as<uint32_t>();
        auto mod_attributes = value["attributes"];
        
        //std::cout << modifier_name << " ID: " << ID << " -- attributes: " << mod_attributes << std::endl; 

        switch (ID) {
            case flat_damage::ID:
            {
                parse_modifier<modifierfactory_t, flat_damage>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case enhanced_damage::ID:
            {
                parse_modifier<modifierfactory_t, enhanced_damage>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case enhanced_speed::ID:
            {
                parse_modifier<modifierfactory_t, enhanced_speed>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case flat_range::ID:
            {
                parse_modifier<modifierfactory_t, flat_range>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case flat_crit_chance::ID:
            {
                parse_modifier<modifierfactory_t, flat_crit_chance>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case flat_crit_multiplier::ID:
            {
                parse_modifier<modifierfactory_t, flat_crit_multiplier>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case flat_type_damage::ID:
            {
                parse_modifier<modifierfactory_t, flat_type_damage>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }
            case enhanced_type_damage::ID:
            {
                parse_modifier<modifierfactory_t, enhanced_type_damage>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }        
            case flat_damage_onhit::ID:
            {
                parse_modifier<modifierfactory_t, flat_damage_onhit>(attribute_fact, mod_attributes, std::move(modifier_name));
                break;
            }       
            default:
            {
                std::cerr << "Invalid ID " << ID << std::endl;
                break;
            }
        }
    }
}
#endif
