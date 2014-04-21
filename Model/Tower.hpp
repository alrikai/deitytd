#ifndef TOWER_HPP
#define TOWER_HPP

#include "Elements.hpp"
#include "RandomUtility.hpp"

#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>

//const std::map<ElementTypes::ElementIndex,double> ElementTypes::ElementCoefficients;

//temporarily put this in here...
template <typename T>
struct TowerInformation
{
    static std::vector<std::pair<T,T>> TowerTierCoefficients;
};
//tower creation coefficients; arranged as [mean, variance]
template <typename T>
std::vector<std::pair<T,T>> TowerInformation<T>::TowerTierCoefficients { {20,5}, {30,5}, {40,5}, {50,5}, {60,5}, {70,5}, {80,5}, {85,5} };




//will have to put this in its own file later...
template <typename T>
class Tower
{
public:
    enum class Tier { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN };
    
    Tower(const int tier_roll, std::unique_ptr<ElementTypes::Water<T>> w_info, std::unique_ptr<ElementTypes::Air<T>> a_info, 
          std::unique_ptr<ElementTypes::Fire<T>> f_info, std::unique_ptr<ElementTypes::Earth<T>> e_info)
        : water_info(std::move(w_info)), air_info(std::move(a_info)), fire_info(std::move(f_info)), earth_info(std::move(e_info))
    {
        set_tier(tier_roll);
        make_tower_stats();
    }
    
    double get_damage() const { return damage; }
    double get_speed() const { return speed; }
    double get_range() const { return range; }
    double get_cost() const { return cost; }

    ElementTypes::Water<T>* get_water_info() const { return water_info.get(); }
    ElementTypes::Air<T>* get_air_info() const { return air_info.get(); }
    ElementTypes::Fire<T>* get_fire_info() const { return fire_info.get(); }
    ElementTypes::Earth<T>* get_earth_info() const { return earth_info.get(); }

protected:
    //determine the tower stats based on the element information
    void make_tower_stats()
    {
        //find the damage -- each coefficient  
        double dmg_weight = 0;
        dmg_weight += ElementTypes::Water<T>::DMGCoeff * water_info->get_weight();
        dmg_weight += ElementTypes::Air<T>::DMGCoeff * air_info->get_weight();        
        dmg_weight += ElementTypes::Fire<T>::DMGCoeff * fire_info->get_weight();
        dmg_weight += ElementTypes::Earth<T>::DMGCoeff * earth_info->get_weight();

        double spd_weight = 0;
        spd_weight += ElementTypes::Water<T>::SPDCoeff  * water_info->get_weight();       
        spd_weight += ElementTypes::Air<T>::SPDCoeff  * air_info->get_weight();       
        spd_weight += ElementTypes::Fire<T>::SPDCoeff * fire_info->get_weight();
        spd_weight += ElementTypes::Earth<T>::SPDCoeff * earth_info->get_weight();   

        double rng_weight = 0;
        rng_weight += ElementTypes::Water<T>::RNGCoeff * water_info->get_weight();        
        rng_weight += ElementTypes::Air<T>::RNGCoeff * air_info->get_weight();        
        rng_weight += ElementTypes::Fire<T>::RNGCoeff * fire_info->get_weight();
        rng_weight += ElementTypes::Earth<T>::RNGCoeff * earth_info->get_weight();
        
        double cost_weight = 0;
        cost_weight += ElementTypes::Water<T>::CSTCoeff * water_info->get_weight();        
        cost_weight += ElementTypes::Air<T>::CSTCoeff * air_info->get_weight();        
        cost_weight += ElementTypes::Fire<T>::CSTCoeff * fire_info->get_weight();
        cost_weight += ElementTypes::Earth<T>::CSTCoeff * earth_info->get_weight();

        //TODO: have some function for converting the weights + tier to stats
        damage = dmg_weight;
        speed = spd_weight;
        range = rng_weight;
        cost = cost_weight;

    }
    
    void set_tier(const int tier_roll)
    {
        if(tier_roll < 10)
        {
            tier = Tier::ONE;
        }
        else if(tier_roll >= 10 && tier_roll < 20)
        {
            tier = Tier::TWO;
        }
        else if(tier_roll >= 20 && tier_roll < 30)
        {
            tier = Tier::THREE;
        }
        else if(tier_roll >= 30 && tier_roll < 40)
        {
            tier = Tier::FOUR;
        }
        else if(tier_roll >= 40 && tier_roll < 50)
        {
            tier = Tier::FIVE;
        }
        else if(tier_roll >= 50 && tier_roll < 60)
        {
            tier = Tier::SIX;
        }
        else if(tier_roll >= 60 && tier_roll < 70)
        {
            tier = Tier::SEVEN;
        }
        else if(tier_roll >= 70 && tier_roll < 80)
        {
            tier = Tier::EIGHT;
        }
        else if(tier_roll >= 80 && tier_roll < 90)
        {
            tier = Tier::NINE;
        }
        else
        {
            tier = Tier::TEN; 
        } 
    }

    std::unique_ptr<ElementTypes::Water<T>> water_info;
    std::unique_ptr<ElementTypes::Air<T>> air_info;
    std::unique_ptr<ElementTypes::Fire<T>> fire_info;
    std::unique_ptr<ElementTypes::Earth<T>> earth_info;

    double damage;
    double speed;
    double range;
    double cost;

    //assume we'll have tower tiers be in the range [1, 11). 
    Tier tier;
};


/*
 *  Want to have a pool of element types (i.e. WAFE). Then the user will set the weights for 
 *  certain attributes (e.g. can start with this just being the elements, but it'd be better
 *  if it were some other attributes (e.g. strength/agility/intelligence/???) that would be
 *  mapped to the elements in a slightly randomized manner). The higher the weight the more
 *  expensive it would cost.
 *  Then much like DnD attack rolls, you'd have some RNGs that would roll, then weight the 
 *  rolls with the user-input weights to determine the "amount" of element for that tower.
 *  This will determine the base stats for the tower.
 *
 *  Then also have a pool of different spells/abilities which the towers would be able to get
 *  based on the element distributions that they rolled. i.e. could have "slows movement speed
 *  by X% on attack" if water > Y and earth > Z. Have various combinations like this with certain
 *  preconditions. Any amount over the minimum precondition will strengthen the ability.
 *
 *  If the tower qualifies for multiple abilities, can either let the user choose (this would 
 *  probably be best) which one(s) to take, OR could choose for the user (might be easier). 
 *
 */
namespace ElementMaker
{
    typedef double WeightType;
    Tower<WeightType>* MakeTower(const double water_weight, const double air_weight, const double fire_weight, const double earth_weight)
    {

        const int tier_idx = 0;
        auto tier_roller = Randomize::GaussianRoller(TowerInformation<double>::TowerTierCoefficients.at(tier_idx).first, TowerInformation<double>::TowerTierCoefficients.at(tier_idx).second);
        auto element_roller = Randomize::UniformRoller();

        int tower_tier = tier_roller.roll_tower_tier();
        
        double water_roll = element_roller.get_roll(water_weight);
        double air_roll = element_roller.get_roll(air_weight);
        double fire_roll = element_roller.get_roll(fire_weight);
        double earth_roll = element_roller.get_roll(earth_weight);

        /*
         *  Now we have the tower tier and element rolls. From here, we need to create the tower with
         *  the given values. Use the tier & element rolls to find the tower stats. How should we do this?
         *
         */

        using WaterType = ElementTypes::Water<WeightType>;
        using AirType = ElementTypes::Air<WeightType>;
        using FireType = ElementTypes::Fire<WeightType>;
        using EarthType = ElementTypes::Earth<WeightType>;

        auto water_info = std::unique_ptr<WaterType>(new WaterType(water_roll));
        auto air_info = std::unique_ptr<AirType>(new AirType(air_roll));
        auto fire_info = std::unique_ptr<FireType>(new FireType(fire_roll));
        auto earth_info = std::unique_ptr<EarthType>(new EarthType(earth_roll));
    
        Tower<WeightType>* new_tower = new Tower<WeightType>(tower_tier, std::move(water_info), std::move(air_info), std::move(fire_info), std::move(earth_info));
        return new_tower;
    }
}

#endif
