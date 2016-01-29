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

struct MonsterStats
{
    /*
     * TODO: determine which stats are needed for the monster type
     *
     */
    float health;
    float speed;
    //currently using the same elements as the attacks; consider if we want a seperate armor type system
    Elements armor_class;
    float armor_amount;
};

#endif
