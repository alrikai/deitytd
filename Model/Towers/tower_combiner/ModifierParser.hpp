/* ModifierParser.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_MODIFIER_PARSER_HPP
#define TD_MODIFIER_PARSER_HPP

#include "util/TowerModifiers.hpp"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <iostream>

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_damage::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_damage::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new flat_damage (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, enhanced_damage::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (enhanced_damage::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new enhanced_damage (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, enhanced_speed::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (enhanced_speed::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new enhanced_speed (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_range::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_range::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new flat_range (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_crit_chance::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_crit_chance::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new flat_crit_chance (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_crit_multiplier::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_crit_multiplier::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new flat_crit_multiplier (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}
//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_type_damage::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_type_damage::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        //NOTE: this approach works, at least
        float amount = modifier_cfg.flat_dmg_amount;
        auto type = modifier_cfg.type;
        auto scale = modifier_cfg.scale_factor;

        auto modifier_obj = new flat_type_damage (amount, type, scale); //(std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}
//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, enhanced_type_damage::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (enhanced_type_damage::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        float amount = modifier_cfg.ed_percent_amount;
        auto type = modifier_cfg.type;
        auto scale = modifier_cfg.scale_factor;

        auto modifier_obj = new enhanced_type_damage (amount, type, scale); //(std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}

//-------------------------------------------------------------------------------------------------------------

void parse_modifier_parameters(const YAML::Node& mod_attributes, flat_damage_onhit::parameter_cfg& modifier_cfg);

template <typename mod_generator_t>
mod_generator_t make_modifier_fn (flat_damage_onhit::parameter_cfg modifier_cfg)
{
    mod_generator_t modifier_fn = [modifier_cfg](float level)
    {
        auto modifier_obj = new flat_damage_onhit (std::move(modifier_cfg)); 
        modifier_obj->scale_modifier(level);
        return modifier_obj;
    };
    return modifier_fn;
}


#endif


