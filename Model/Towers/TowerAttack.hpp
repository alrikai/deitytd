#ifndef TD_TOWER_ATTACK_HPP
#define TD_TOWER_ATTACK_HPP

#include "util/Elements.hpp"
#include "util/Types.hpp"

#include <cmath>

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

//since all of these are set initially by the caller, seems nicer to put them into a struct
//rather than have a constructor with a dozen inputs
struct TowerAttackParams
{
    TowerAttackParams(tower_properties atk_props, const std::string& atk_id, const std::string& t_id)
        : attack_attributes(atk_props), id(atk_id), origin_id(t_id)
    {}

    tower_properties attack_attributes;
    const std::string id;
    const std::string origin_id;

    //attack movement type -- homing updates the attack movement wrt a target,
    //while non-homing has an initial destination and moves towards it
    bool has_homing;
    double move_speed;

    //the game time at the point of creation
    uint64_t origin_timestamp;
    //starting location
    Coordinate<float> origin_position;
    Coordinate<float> target_position;
};

class TowerAttack 
{
public:
    explicit TowerAttack(TowerAttackParams&& attack_params)
        : params(std::move(attack_params))
    {
        //these parameters will change per-iteration
        current_position = params.origin_position;
        timestamp = params.origin_timestamp;

        //flag indicating whether the attack has hit something
        has_hit_target = false;
    }

    inline std::string get_origin_id() const
    {
        return params.origin_id;
    }

    inline std::string get_id() const
    {
        return params.id;
    }

    //NOTE: it is assumed we are using normalized coordinates
    inline bool in_bounds() const
    {
        return current_position.row >= 0.0f && current_position.row <= 1.0f && current_position.col >= 0.0f && current_position.col <= 1.0f; 
    }

    inline Coordinate<float> get_position() const
    {
        return current_position;
    }
   
    Coordinate<float> move_update(const uint64_t time)
    {
        //move speed dictates the maximum L2 distance the attack can move in a turn.
        //We need to move the attack position along this trajectory, and see if the
        //target is within the attack range

        //distance to move based on last move time
        //float ms = (time - timestamp) * params.move_speed;
        float ms = 0.1;
        timestamp = time;        

        float nx_factor = (params.target_position.col - current_position.col);
        float ny_factor = (params.target_position.row - current_position.row);
        float target_dist = std::sqrt(nx_factor*nx_factor + ny_factor*ny_factor);

        if(target_dist < ms)
        {
            //need to take care for over-shooting -- for now we can do something a bit less difficult...
            current_position = params.target_position;
            has_hit_target = true;
        }
        else
        {
            float dist_mag = ms / target_dist;
            current_position.col += nx_factor * dist_mag;
            current_position.row += ny_factor * dist_mag;
        }
        return current_position;
    }

    void set_target(Coordinate<float> target)
    {
        params.target_position = target;
    }

    bool hit_target()
    {
        return has_hit_target; 
    }

private:
    TowerAttackParams params;

    //the game time at the point of creation
    uint64_t timestamp;
    //current location
    Coordinate<float> current_position;

    bool has_hit_target;

    //how to handle the targeting?
};

#endif
