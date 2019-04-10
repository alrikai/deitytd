#include "ModifierConfigParser.hpp"

#include <algorithm>

//TODO: get this to be an easy way to map from string --> ID, or at least, to get from the yaml 
//"type" string to the actual type. I want to move away from using the ID directly in the yaml
//file, as that is very error-prone. Better to have user-readable strings, but then we need to have
//a nice way to go from string --> TowerModifier type. Then in all the switch statements over IDs, 
//we can just index directly

ModifierMapper::ModifierMapper() {
		//add the various modifiers -- the verbosity is a result of the get_ID and get_name methods
		//not uniformly being constexpr / static method (due to my questionable choice of mapping
		//per-element modifiers to the same ID, then having additional per-element offsets...)
    TowerModifiers::flat_damage::parameter_cfg flat_dmg;
    modifier_mapping.insert(std::make_pair(flat_dmg.get_ID(), flat_dmg.get_name()));

    TowerModifiers::enhanced_damage::parameter_cfg enhanced_damage;
    modifier_mapping.insert(std::make_pair(enhanced_damage.get_ID(), enhanced_damage.get_name()));

    TowerModifiers::enhanced_speed::parameter_cfg enhanced_speed;
    modifier_mapping.insert(std::make_pair(enhanced_speed.get_ID(), enhanced_speed.get_name()));

    TowerModifiers::flat_range::parameter_cfg flat_range;
    modifier_mapping.insert(std::make_pair(flat_range.get_ID(), flat_range.get_name()));

    TowerModifiers::flat_crit_chance::parameter_cfg flat_crit_chance;
    modifier_mapping.insert(std::make_pair(flat_crit_chance.get_ID(), flat_crit_chance.get_name()));

    TowerModifiers::flat_crit_multiplier::parameter_cfg flat_crit_multiplier;
    modifier_mapping.insert(std::make_pair(flat_crit_multiplier.get_ID(), flat_crit_multiplier.get_name()));

    modifier_mapping.insert(std::make_pair(TowerModifiers::flat_type_damage::ID + 0, std::string{TowerModifiers::flat_type_damage::NAME}));

    TowerModifiers::flat_type_damage::parameter_cfg flat_type_damage_chaos;
    flat_type_damage_chaos.type = Elements::CHAOS;
    modifier_mapping.insert(std::make_pair(flat_type_damage_chaos.get_ID(), flat_type_damage_chaos.get_name()));

    TowerModifiers::flat_type_damage::parameter_cfg flat_type_damage_water;
    flat_type_damage_water.type = Elements::WATER;
    modifier_mapping.insert(std::make_pair(flat_type_damage_water.get_ID(), flat_type_damage_water.get_name()));

    TowerModifiers::flat_type_damage::parameter_cfg flat_type_damage_air;
    flat_type_damage_air.type = Elements::AIR;
    modifier_mapping.insert(std::make_pair(flat_type_damage_air.get_ID(), flat_type_damage_air.get_name()));

    TowerModifiers::flat_type_damage::parameter_cfg flat_type_damage_fire;
    flat_type_damage_fire.type = Elements::FIRE;
    modifier_mapping.insert(std::make_pair(flat_type_damage_fire.get_ID(), flat_type_damage_fire.get_name()));

    TowerModifiers::flat_type_damage::parameter_cfg flat_type_damage_earth;
    flat_type_damage_earth.type = Elements::EARTH;
    modifier_mapping.insert(std::make_pair(flat_type_damage_earth.get_ID(), flat_type_damage_earth.get_name()));

    modifier_mapping.insert(std::make_pair(TowerModifiers::enhanced_type_damage::ID + 0, std::string{TowerModifiers::enhanced_type_damage::NAME}));

    TowerModifiers::enhanced_type_damage::parameter_cfg enhanced_type_damage_chaos;
    enhanced_type_damage_chaos.type = Elements::CHAOS;
    modifier_mapping.insert(std::make_pair(enhanced_type_damage_chaos.get_ID(), enhanced_type_damage_chaos.get_name()));

    TowerModifiers::enhanced_type_damage::parameter_cfg enhanced_type_damage_water;
    enhanced_type_damage_water.type = Elements::WATER;
    modifier_mapping.insert(std::make_pair(enhanced_type_damage_water.get_ID(), enhanced_type_damage_water.get_name()));

    TowerModifiers::enhanced_type_damage::parameter_cfg enhanced_type_damage_air;
    enhanced_type_damage_air.type = Elements::AIR;
    modifier_mapping.insert(std::make_pair(enhanced_type_damage_air.get_ID(), enhanced_type_damage_air.get_name()));

    TowerModifiers::enhanced_type_damage::parameter_cfg enhanced_type_damage_fire;
    enhanced_type_damage_fire.type = Elements::FIRE;
    modifier_mapping.insert(std::make_pair(enhanced_type_damage_fire.get_ID(), enhanced_type_damage_fire.get_name()));

    TowerModifiers::enhanced_type_damage::parameter_cfg enhanced_type_damage_earth;
    enhanced_type_damage_earth.type = Elements::EARTH;
    modifier_mapping.insert(std::make_pair(enhanced_type_damage_earth.get_ID(), enhanced_type_damage_earth.get_name()));

    TowerModifiers::flat_damage_onhit::parameter_cfg flat_damage_onhit;
    modifier_mapping.insert(std::make_pair(flat_damage_onhit.get_ID(), flat_damage_onhit.get_name()));

    TowerModifiers::flat_added_damage::parameter_cfg flat_added_damage_chaos;
    flat_added_damage_chaos.type = Elements::CHAOS;
    modifier_mapping.insert(std::make_pair(flat_added_damage_chaos.get_ID(), flat_added_damage_chaos.get_name()));

    TowerModifiers::flat_added_damage::parameter_cfg flat_added_damage_water;
    flat_added_damage_water.type = Elements::WATER;
    modifier_mapping.insert(std::make_pair(flat_added_damage_water.get_ID(), flat_added_damage_water.get_name()));

    TowerModifiers::flat_added_damage::parameter_cfg flat_added_damage_air;
    flat_added_damage_air.type = Elements::AIR;
    modifier_mapping.insert(std::make_pair(flat_added_damage_air.get_ID(), flat_added_damage_air.get_name()));

    TowerModifiers::flat_added_damage::parameter_cfg flat_added_damage_fire;
    flat_added_damage_fire.type = Elements::FIRE;
    modifier_mapping.insert(std::make_pair(flat_added_damage_fire.get_ID(), flat_added_damage_fire.get_name()));

    TowerModifiers::flat_added_damage::parameter_cfg flat_added_damage_earth;
    flat_added_damage_earth.type = Elements::EARTH;
    modifier_mapping.insert(std::make_pair(flat_added_damage_earth.get_ID(), flat_added_damage_earth.get_name()));
}

uint32_t ModifierMapper::modifier_name_to_ID(std::string mod_type) {
		std::transform(mod_type.begin(), mod_type.end(), mod_type.begin(), ::tolower);
		for (auto it = modifier_mapping.begin(); it != modifier_mapping.end(); it++) {
      if (it->second == mod_type) {
        return it->first;
			}
		}
		std::string err_msg {"Invalid Modifier type string -- " + mod_type};
		throw std::runtime_error(err_msg); 
}


tower_property_modifier ModifierMapper::parse_tower_config(const std::string& tower_cfg) {
    tower_property_modifier tower_modifier;
	  YAML::Node cfg_root = YAML::LoadFile(tower_cfg);
	  if (cfg_root.IsNull()) {
		  std::ostringstream ostr;
		  ostr << "ERROR -- config yaml file " << tower_cfg << " not found";
		  throw std::runtime_error(ostr.str());
	  }

	  YAML::Node tower_node = cfg_root["TowerAttributes"];
	  assert(tower_node.size() == 3);

	  auto tower_name = tower_node["name"].as<std::string>();
	  auto tower_tier = tower_node["tier"].as<uint32_t>();
	  YAML::Node tower_attributes = tower_node["attributes"];
	  const size_t num_attributes = tower_attributes.size();
	  for (size_t attr_idx = 0; attr_idx < num_attributes; attr_idx++) {
		  YAML::Node value = tower_attributes[attr_idx];
			const uint32_t ID = modifier_name_to_ID(value["type"].as<std::string>());

		  auto mod_attributes = value["value"];
		  switch (ID) {
		    case TowerModifiers::flat_damage::ID: {
		      TowerModifiers::flat_damage::parameter_cfg cfg;
			    parse_modifier_parameters(mod_attributes, cfg);
			    TowerModifiers::flat_damage tmod (cfg);
			    tmod.aggregate_modifier(tower_modifier);
			    break;
				}
				case TowerModifiers::enhanced_damage::ID: {
					TowerModifiers::enhanced_damage::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::enhanced_damage tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::enhanced_speed::ID: {
					TowerModifiers::enhanced_speed::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::enhanced_speed tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::flat_range::ID: {
					TowerModifiers::flat_range::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::flat_range tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::flat_crit_chance::ID: {
					TowerModifiers::flat_crit_chance::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::flat_crit_chance tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::flat_crit_multiplier::ID: {
					TowerModifiers::flat_crit_multiplier::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::flat_crit_multiplier tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::flat_type_damage::ID: {
					TowerModifiers::flat_type_damage::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::flat_type_damage tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::enhanced_type_damage::ID: {
					TowerModifiers::enhanced_type_damage::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::enhanced_type_damage tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				case TowerModifiers::flat_damage_onhit::ID: {
					TowerModifiers::flat_damage_onhit::parameter_cfg cfg;
					parse_modifier_parameters(mod_attributes, cfg);
					TowerModifiers::flat_damage_onhit tmod (cfg);
					tmod.aggregate_modifier(tower_modifier);
					break;
				}
				default: {
					std::cerr << "Invalid ID " << ID << std::endl;
					break;
				}
		  }
	  }
	  return tower_modifier;
  }



