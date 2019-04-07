#include "ModifierConfigParser.hpp"

#include <algorithm>

//TODO: get this to be an easy way to map from string --> ID, or at least, to get from the yaml 
//"type" string to the actual type. I want to move away from using the ID directly in the yaml
//file, as that is very error-prone. Better to have user-readable strings, but then we need to have
//a nice way to go from string --> TowerModifier type. Then in all the switch statements over IDs, 
//we can just index directly
const std::array<std::string, TowerPropertyParser::NUM_MODIFIERS>& get_modifier_names {
  static std::set<std::pair<int, std::string>> modifier_mapping = {
      {TowerModifiers::flat_damage::ID, "flat_damage", },
			{"enhanced_damage", },
			{"enhanced_speed", },
			{"flat_range", },
			{"flat_crit_chance", 
			{"flat_crit_multiplier", 
			{"flat_type_damage_chaos",
       "flat_type_damage_water", "flat_type_damage_air",
       "flat_type_damage_fire", "flat_type_damage_earth",
       "enhanced_type_damage_chaos", "enhanced_type_damage_water",
       "enhanced_type_damage_air", "enhanced_type_damage_fire",
       "enhanced_type_damage_earth", "flat_damage_onhit"}};


uint32_t TowerPropertyParser::modifier_name_to_ID(std::string mod_type) {

  

  std::transform(mod_type.begin(), mod_type.end(), mod_type.begin(), ::tolower);
	for (size_t id = 0; id < TowerPropertyParser::NUM_MODIFIERS; id++) {
    if (mod_type == TowerPropertyParser::modifier_names[id]) {
      return id;
		}
	}

	switch (mod_type) {
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

tower_property_modifier TowerPropertyParser::parse_tower_config(const std::string& tower_cfg) {
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



