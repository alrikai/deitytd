/* Tower.cpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "Tower.hpp"

//#include "StatusEffects.hpp"
#include "util/RandomUtility.hpp"

#include <iomanip>
#include <sstream>

class TowerStatusEffect;

std::ostream &operator<<(std::ostream &out_stream, const Tower &t) {
  out_stream << "Fundamental Tower -- " << t.base_attributes << std::endl;
  out_stream << "Synthesized Tower -- " << t.attack_attributes << std::endl;
  return out_stream;
}

//----------------------------------------------------------------------------

namespace {
template <typename T>
std::string to_string_wprecision(const T val, const uint32_t ndec = 2) {
  std::ostringstream strout;
  strout << std::setprecision(ndec) << val;
  strout << std::fixed;
  return strout.str();
}
} // namespace

// TODO: need to consolidate this and the ostream printing, especially since I'm
// using floating point #'s for everything so I need to use string streams for
// the string conversions T_T
std::string Tower::get_stats() const {
  std::ostringstream ostr;
  ostr <<"Base Stats:\n" << base_attributes;
  ostr << "\n\nCurrent Stats:\n" << attack_attributes;
  return ostr.str();
}


tower_properties Tower::compute_attack_damage() const {
  auto attributes = base_attributes;
  tower_property_modifier status_modifier = enhancements;
  // generate the tower effects for the attack
  for (auto effect_it : status_effects) {
    effect_it->aggregate_modifier(status_modifier);
  }
  attributes.apply_property_modifier(std::move(status_modifier));
	return attributes;
}

/*
 * upon attacking, the tower needs to tally all of the long-term and short-term
 * effects and modifiers that it has, and compute the attack attributes for the
 * tower at that given snapshot in time. It'll then spawn an attack object
 * that'll have the attack attributes and any event-based functors (e.g. on-hit
 * or on-death effects). Or rather, maybe we should just have a callback functor
 * that'll be called by the attack object that's a Tower member method?
 */
std::unique_ptr<TowerAttackBase>
Tower::generate_attack(const std::string &attack_id, const uint64_t timestamp) {
  // NOTE: we assume that the tower has a target if it is generating attacks
  // (NOTE: will not work if we allow say, an 'attack ground' option for splash
  // towers)
  const auto mob_id = get_target()->get_name();
  // reset the (per-cycle) attack atttributes
  attack_attributes = compute_attack_damage();
  std::cout << "Tower (temporary) properties " << attack_attributes
            << std::endl;

  // set the attack parameters
  TowerAttackParams params(attack_attributes, this, attack_id, mob_id);
  // distance the attack can move per round (normalized)
  params.move_speed = 0.15;

  // the game time at the point of creation
  params.origin_timestamp = timestamp;
  // starting location
  params.origin_position = position;

  // attack movement type -- homing updates the attack movement wrt a target,
  // while non-homing has an initial destination and moves towards it
  bool has_homing = true;
  // TODO: make a proper factory for generating the appropriate TowerAttacks
  if (has_homing) {
    return std::unique_ptr<TowerAttackBase>(
        new TowerAttack<HomingAttackMovement>(
            std::move(params), HomingAttackMovement(current_target)));
  } else {
    return std::unique_ptr<TowerAttackBase>(
        new TowerAttack<FixedAttackMovement>(std::move(params),
                                             FixedAttackMovement()));
  }
}

bool Tower::add_modifier(tower_property_modifier &&modifier) {

	enhancements.merge(std::move(modifier));
	//base_attributes.apply_property_modifier(modifier);

  // NOTE: what do we do here?
  std::cout << "@Tower::add_modifier with " << typeid(modifier).name()
            << std::endl;
  return true;
}
/*
bool Tower::add_modifier(const TowerCombiner& tower_combiner,
tower_property_modifier* modifier)
{

    //NOTE: what do we do here?
    std::cout << "@Tower::add_modifier with " << typeid(*modifier).name() <<
std::endl; return true;
}
*/

namespace TowerGenerator {
/*
 * The base tower will have some tier-dependant default stats and a default
 * model (a sphere perhaps?) then the user will upgrade it with various items,
 * which will change its stats and character model. Presumably its attack
 * projectile as well. What about the element distribution?
 */
std::unique_ptr<Tower> make_fundamentaltower(const uint32_t ID, const int tier,
                                             const std::string &tower_name,
                                             const float row, const float col) {
  // make some base stats based on the tower tier -- 
  // TODO: this should be in a yaml file
  tower_properties base_attributes;

  base_attributes.modifier.damage_value[static_cast<int>(Elements::CHAOS)] =
      tower_properties::dmg_dist(2 * tier, 5 * tier);
  base_attributes.modifier.attack_speed_value = 1 * tier;
  base_attributes.modifier.attack_range_value = 3 * tier;
  base_attributes.modifier.crit_multiplier_value = 50;
  // etc...

  auto base_tower = std::unique_ptr<Tower>(
      new Tower(std::move(base_attributes), ID, tower_name, tier, row, col));

  // load a fractal mesh -- the base tower will always look the same, but the
  // tower models will diverge as they're upgraded. would it be worth sharing the
  // base tower model and using a copy-on-write scheme for it?
  std::vector<std::vector<uint32_t>> polygon_mesh;
  std::vector<std::vector<float>> polygon_points;
  const std::string mesh_filename{TDHelpers::get_basepath() +
                                  "/data/meshfractal3d.vtk"};

  TowerModelUtil::load_mesh(mesh_filename, polygon_mesh, polygon_points);
  std::string t_material{"FractalTower"}; //{"BaseWhiteNoLighting"};
  auto tower_model = std::make_shared<TowerModel>(
      std::move(polygon_mesh), std::move(polygon_points), t_material);
  base_tower->set_model(tower_model);

  return base_tower;
}

} // namespace TowerGenerator

#if 0
/*
 * this one still needs some work; I want to have the towers 'choose' their status effects
 * i.e. the types that they'll have, how long they'll last, what magnitude they'll have, etc.
 * This should have an element of randomness, weighted by the tower's tier, among other things
 *
 * Make a proof-of-concept for now, and refine/rebalance as we go, I guess
 */
void Tower::generate_statuseffects()
{
    //TODO: put this... somewhere else, I guess. giving these as defaults for testing, but we'll want some
    //semi-randomized scheme for assigning these effects to towers (or if we have 'uniques', some file that
    //specifies what towers get what effects.

    int num_effects = static_cast<int> (tier);
    Randomize::UniformRoller effect_dice;
    //TODO: this is the #total possible effects... note that this should
    //A. be defined elsewhere
    //B. be a bit more intelligent; i.e. if the effects themselves are tiered, we don't want even distributions, etc.
    static constexpr int num_total_effects = 2;

    //TODO: how to handle repeats? i.e. if we get the same status twice, what do we do? 
    //1. we could just NOT allow that to happen,
    //2. we could have them stack
    //3. we could have 1 override the other(s)
    //4. some combination of the above... i.e. take the max of the duration(s) and the max of the magnitude(s)
    //... I think having them stack would be the more interesting mechanic, although balance would be tricky
    for (int se_idx = 0; se_idx < num_effects; ++se_idx) {
        //TODO: for now, just randomly select from the possible statuses...
        const int rolled_idx = static_cast<int>(effect_dice.get_roll(num_total_effects));
        auto effect_id = static_cast<STATUS_EFFECTS_IDS>(rolled_idx); //STATUS_EFFECTS_IDS::ARMOR_REDUCE;
        auto effect_precedence = status_helper::get_status_id_precedence(effect_id);
        //status_helper::get_status_durationrange
        auto effect_duration = 1;
        
        //@HERE: would need to check for duplicates in the status_effects vector. Since we won't (ever?)
        //exceed ~10 effects in a tower, we'll just brute force the search
       
        //TODO: this method doesn't mesh very well with the centralized parameter approach; namely, since we are treating
        //the status effects homogeneously here, there's not a good way to access their corresponding parameter in the 
        //status_params structure. How to rectify this? --> could change the status_params to be an aggregation of each
        //status effect's parameters, s.t. we make each status effect's parameters individually, then assign them into 
        //the aggregate parameter afterwards. 
        bool new_effect = true;
        for (auto effect_it : status_effects) {
            if (effect_id == effect_it.id) {
                new_effect = false;
                std::cout << "NOTE: stacking effect " << (int)effect_id << " from D: " << effect_it.metadata.life_ticks << std::endl;
             
                //TODO: this is more or less a placeholder for the 'real' merging (see the 1-up TODO comment)
                effect_it.metadata.life_ticks += effect_duration;
                
                std::cout << "... to D: " << effect_it.metadata.life_ticks << std::endl;
            }
        }

        if(new_effect) {
            status_effects.push_back(statuseffect_metadata(effect_id, effect_duration, effect_precedence));
        }
    }
    //status_effects.push_back(statuseffect_metadata(STATUS_EFFECTS_IDS::FLAT_ATTACK_BOOST, 1, 1);

    //the next question; how to store the parameters for the status effects?
    //TODO: will we ever have these change? i.e. if we have some meta-modifiers, like 1.5x all +dmg modifiers for N seconds....
 	//TODO: have the status effect parameter generation be more... intelligent (either load from find or randomize 
	//the effects based on the tower tier. Actually the randomization part will be quite involved)
    //... the specific statuses' parameters (same thing for the status metadata).
	//NOTE: the status effect parameters will be general for the tower, but the status metadata will be PER 
	//status (since they'll have different lifespans and precedence levels)
    //
	status_params.armor_reduce_amount = 1;
    status_params.flat_attack_amount = 10;
}

bool Tower::add_modifier(tower_generator tower_gen, essence* modifier)
{
    if(mod_count >= get_tier(tier))
    {
        //TODO: also want to generate an error message for the backend error queue
        std::cout << "Too many modifiers for the tier!" << std::endl;
        return false;
    }

    //modifiers.at(mod_count++) = std::move(modifier);
    modifiers.at(mod_count++) = std::unique_ptr<essence>(modifier); 

    tower_properties working_properties;
    //re-compute the tower modifier attribute contribution
    switch(mod_count)
    {
        case 1:
            modifiers.at(0)->apply_modifier(working_properties);
            break;
        case 2:
            working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get());
            break;
        case 3:
            working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(), 
                    modifiers.at(2).get());
            break;
        case 4:
            working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                    modifiers.at(2).get(), modifiers.at(3).get());
            break;
            /*
        case 5:
            working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                    modifiers.at(2).get(), modifiers.at(3).get(), modifiers.at(4).get());                
            break;
        case 6:
            working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                    modifiers.at(2).get(), modifiers.at(3).get(), modifiers.at(4).get(), modifiers.at(5).get());                
            break;
            */
        default:
            std::cout << "Problem @ Tower Modification -- " << mod_count << " #essences" << std::endl;
    }

    attack_attributes = working_properties + base_attributes;
    std::cout << "Modified Tower: " << attack_attributes << std::endl;
    return true;
}

#endif
