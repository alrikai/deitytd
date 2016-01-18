#ifndef ATTRIBUTE_MODIFIERS_HPP
#define ATTRIBUTE_MODIFIERS_HPP

#include "TowerProperties.hpp"
#include <vector>

class Monster;
class Tower;

struct tower_attribute_modifier
{
    static constexpr uint32_t ID = 0xFFFFFF00;

    virtual ~tower_attribute_modifier() {}
    //this is how the modifier will be added to the tower -- i.e. its tower_property 
    //object will be passed in, and the attribute modifier will add itself to the 
    //appropriate tower modifier lists, etc. as needed
    inline virtual void apply_modifier (tower_properties& stats) {}
    inline virtual void aggregate_modifier (tower_property_modifier& stats_modifier) {}
    inline virtual void scale_modifier(float score) {}
};

//handles the stat enhancement type modifiers
struct stat_attribute_modifier : tower_attribute_modifier
{
    static constexpr uint32_t ID = 0xFFFFFF01;
    virtual ~stat_attribute_modifier() {}
    inline virtual void apply_modifier (tower_properties& stats) override {}
    inline virtual void aggregate_modifier (tower_property_modifier& stats_modifier) override {}
    inline virtual void scale_modifier(float score) override {}
};

//handles the on-hit, on-death events
struct event_attribute_modifier : tower_attribute_modifier
{
    static constexpr uint32_t ID = 0xFFFFFF02;
    virtual ~event_attribute_modifier() {}
    inline virtual void apply_modifier (tower_properties& stats) override {}
    inline virtual void aggregate_modifier (tower_property_modifier& stats_modifier) override {}
    inline virtual void scale_modifier(float score) override {}

    virtual void apply_on_event(Monster* mob, Tower* tower, float time_delta) {}
};

struct aura_attribute_modifier : tower_attribute_modifier
{
    static constexpr uint32_t ID = 0xFFFFFF03;
    virtual ~aura_attribute_modifier() {}
    inline virtual void apply_modifier (tower_properties& stats) override {}
    inline virtual void aggregate_modifier (tower_property_modifier& stats_modifier) override {}
    inline virtual void scale_modifier(float score) override {} 

    //should take a collection of tower objects within the aura range to modify
    virtual void apply_aura(std::vector<Tower*>& tower) {}

    //should take a collection of monster objects within the aura range to modify
    virtual void apply_aura(std::vector<Monster*>& tower) {}
};


#endif
