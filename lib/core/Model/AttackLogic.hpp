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
#include "Towers/Tower.hpp"
#include "Towers/TowerAttack.hpp"

#include <memory>

// compute the total damage values for an attack on a per-element basis,
// post-mitigation
void compute_mitigation(
    std::array<float, tower_property_modifier::NUM_ELEM> &attack,
    const MonsterStats &mob_stats);

// merge the per-element damage and get the final, scalar HP deduction amount
float compute_damage(const tower_properties &props,
                     const MonsterStats &mob_stats);

// TODO: try prototyping how the logic for applying the game mechanics will
// be...
void compute_attackhit(const std::list<std::weak_ptr<Monster>> &tile_mobs,
                       std::unique_ptr<TowerAttackBase> attack);

#endif
