/* MonsterProperties.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_MONSTER_STATS_HPP
#define TD_MONSTER_STATS_HPP

#include "Elements.hpp"

struct MonsterStats {
  /*
   * TODO: determine which stats are needed for the monster type
   *
   */

  MonsterStats(float health, float speed, Elements armor, float flat_def, float percent_def, float thresh_def) 
      : health(health), speed(speed), armor_class(armor), flat_armor(flat_def), percent_armor(percent_def), thresh_armor(thresh_def)
  {}

  float health;
  float speed;
  // currently using the same elements as the attacks; consider if we want a
  // seperate armor type system
  Elements armor_class;
  float flat_armor;
  float percent_armor;
  float thresh_armor;
};

#endif
