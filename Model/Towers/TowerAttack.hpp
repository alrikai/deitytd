#ifndef TD_TOWER_ATTACK_HPP
#define TD_TOWER_ATTACK_HPP

#include "util/Elements.hpp"
/*
 * define the attack types here. This will cover pretty much all attacks (meele will just be very short range)
 * this will be broken largely into 2 parts; the logic (i.e. creation, movement updates, collision detection,
 * damage application/on-hit effects, etc) and the graphics (i.e. all particle effects, animations, models, etc).
 *
 * need to define the properties for the attack (would this just be the spawning towers' attribute list + modifiers?)
 * Also, how will we handle the targetting?
 * Will we set a target in the beginning, then see if it hits? Or will we have targetted/homing attacks?
 * Or will we have the attack movement speed be so fast that they will almost always hit? 
 *
 */

class TowerAttack 
{
public:
    TowerAttack(tower_properties props, const std::string& attack_id, const std::string tower_id, const uint64_t tstamp)
        : attack_attributes(props), id(attack_id), origin_id(tower_id), timestamp(tstamp)
    {}

    inline std::string get_origin_id() const
    {
        return origin_id;
    }

    inline std::string get_id() const
    {
        return id;
    }

    /*
    std::tuple<float, float> move_update(const uint64_t time)
    {
        //for now....... 
        row_position
        return 
    }
    */
private:

    //some normalized value for controlling how fast the attack moves
    double move_speed;

    tower_properties attack_attributes;
    const std::string id;
    const std::string origin_id;

    //the game time at the point of creation
    uint64_t timestamp;
    //current location
    float row_position;
    float col_position;
};

#endif
