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

#include <memory>

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

  auto attack_roller = Randomize::UniformRoller();
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
      std::cout << "got mob " << target_mob->get_name() << std::endl;

      // TODO: compute the samage and status effect changes
      // TODO: apply said status effects
      //... but for now, we'll just start with damage

      // TODO: move this part off into another file. Should have a bunch of
      // (generic) functions for computing the state changes as a result of an
      // attakc
      auto mob_stats = target_mob->get_attributes();
      auto atk_attributes = attack->get_attack_attributes();
      float atk_dmg = 0.f;

      for (int elem_idx = 0; elem_idx < tower_property_modifier::NUM_ELEM;
           elem_idx++) {

        const auto atkcoeff_type = std::make_pair(
            static_cast<Elements>(elem_idx), mob_stats.armor_class);
        const auto dmg_factor_it =
            ElementInfo::damage_coeffs.find(atkcoeff_type);

        if (dmg_factor_it != ElementInfo::damage_coeffs.end()) {
          const auto raw_dmg =
              atk_attributes.damage[elem_idx].low +
              attack_roller.get_roll(atk_attributes.damage[elem_idx].high -
                                     atk_attributes.damage[elem_idx].low);
          atk_dmg += dmg_factor_it->second * raw_dmg;
        }
      }
      std::cout << "attack " << attack->get_id() << " did " << atk_dmg
                << " damage" << std::endl;

      const bool mob_alive = target_mob->recieve_damage(atk_dmg);

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
