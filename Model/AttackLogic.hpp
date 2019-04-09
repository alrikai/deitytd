/* AttackLogic.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_ATTACK_LOGIC_HPP
#define TD_ATTACK_LOGIC_HPP

#include "Monster.hpp"
#include "Towers/TowerAttack.hpp"
#include "util/RandomUtility.hpp"

#include <algorithm>
#include <memory>

//compute the total damage values for an attack on a per-element basis, post-mitigation
void compute_mitigation(std::array<float, tower_property_modifier::NUM_ELEM>& attack, const MonsterStats& mob_stats) {
  bool has_thresh = mob_stats.thresh_armor > 0;
  for (size_t elem_idx = 0; elem_idx < attack.size(); elem_idx++) {
    if (has_thresh) {
      attack[elem_idx] = attack[elem_idx] > mob_stats.thresh_armor ? mob_stats.thresh_armor : attack[elem_idx];
    }
    attack[elem_idx] = (1 - mob_stats.percent_armor) * (attack[elem_idx] - mob_stats.flat_armor);
  }
}

//merge the per-element damage and get the final, scalar HP deduction amount
float compute_damage(const tower_properties &props, Elements ttype) {
  auto attack_roller = Randomize::UniformRoller();
  std::array<float, tower_property_modifier::NUM_ELEM> damage {};

  bool atk_crit = attack_roller.get_roll(1) < props.modifier.crit_chance_value;
  //apply base damage and %-ED
  for (int elem_idx = 0; elem_idx < tower_properties::NUM_ELEM; elem_idx++) {
    //roll the amount here, so we collapse the [low - high) range down to a float
    auto elem_dmg = props.modifier.damage_value[elem_idx];
    const auto raw_dmg = elem_dmg.low + attack_roller.get_roll(elem_dmg.high - elem_dmg.low);
    damage[elem_idx] = raw_dmg;
    damage[elem_idx] *= (1 + props.modifier.enhanced_damage_value[elem_idx]);
    //apply crit damage
    if (atk_crit) {
      damage[elem_idx] *= (1 + props.modifier.crit_multiplier_value);
    }

    //have the built-in affinity effects
    const auto atkcoeff_type = std::make_pair(
        static_cast<Elements>(elem_idx), ttype);
    const auto dmg_factor_it =
        ElementInfo::damage_coeffs.find(atkcoeff_type);

    float intrinisic_affinity_multiplier = 0;
    if (dmg_factor_it != ElementInfo::damage_coeffs.end()) {
      intrinisic_affinity_multiplier = dmg_factor_it->second;
    }
    //apply the affinity modifier
    float affinity_modifier = 1 + intrinisic_affinity_multiplier + props.modifier.enhanced_damage_affinity[static_cast<int>(ttype)];  
    damage[elem_idx] *= affinity_modifier;
    //aply flat added damage
    damage[elem_idx] += props.modifier.added_damage_value[elem_idx];
  }

  return std::accumulate(damage.begin(), damage.end(), 0);
}


// TODO: try prototyping how the logic for applying the game mechanics will
// be...
void compute_attackhit(const std::list<std::weak_ptr<Monster>> &tile_mobs,
                       std::unique_ptr<TowerAttackBase> attack) {
  // TODO: get the mob pointer to attack
  // TODO: calculate the damage to deal and state changes to apply
  // TODO: ... ???
  // TODO: apply state changes to mob and origin tower of attack
  // TODO: mark attack for removal, if mob is dead, mark for removal
  // TODO: ... ???
  // TODO: profit?

  auto origin_tower = attack->get_origin_tower();
  auto target_mob_id = origin_tower->get_target_id();

  auto mob_it = std::find_if(tile_mobs.begin(), tile_mobs.end(),
                             [target_mob_id](const std::weak_ptr<Monster> &m) {
                               if (auto tile_mob = m.lock()) {
                                 auto tilemob_name = tile_mob->get_name();
                                 return tilemob_name == target_mob_id;
                               }
                               return false;
                             });

  if (mob_it != tile_mobs.end()) {
    if (auto target_mob = mob_it->lock()) {

      //@HERE: we have the tower attack, the origin tower, and the target mob.

      auto mob_stats = target_mob->get_attributes();
      auto atk_attributes = attack->get_attack_attributes();
      float atk_dmg = compute_damage(atk_attributes, mob_stats.armor_class);
      const bool mob_alive = target_mob->recieve_damage(atk_dmg);

      std::cout << "attack " << attack->get_id() << " did " << atk_dmg
                << " damage" << " to mob " << target_mob->get_name() << std::endl;

      // TODO: generate the status updates

      // TODO: trigger the on-hit events

      // TODO: if the mob died, then handle the funeral proceedings (on-death
      // events)
      if (!mob_alive) {
        // tell the tower that it killed something... not sure what other info
        // it needs (maybe the mob id? or no?)
        origin_tower->killed_mob();
      }
    }
  } else {
    // NOTE: this shouldn't be possible, but I should check anyways for sanity's
    // sake
    std::cout << "ERROR: we collided with a non-existant mob?" << std::endl;
  }
}

#endif
