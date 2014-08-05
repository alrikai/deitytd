#ifndef TD_TOWER_ATTACK_HPP
#define TD_TOWER_ATTACK_HPP

#include "util/Elements.hpp"

/*
 * define the attack types here. This will cover pretty much all attacks (meele will just be very short range)
 * this will be broken largely into 2 parts; the logic (i.e. creation, movement updates, collision detection,
 * damage application/on-hit effects, etc) and the graphics (i.e. all particle effects, animations, models, etc).
 *
 * need to define the properties for the attack (would this just be the spawning towers' attribute list + modifiers?)
 */

class TowerAttack 
{
public:
    TowerAttack(tower_properties props)
        : attack_attributes(props)
    {}
 


private:

    tower_properties attack_attributes;
};

#endif
