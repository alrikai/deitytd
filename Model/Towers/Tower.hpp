#ifndef TD_TOWER_HPP
#define TD_TOWER_HPP

#include "util/Elements.hpp"
#include "TowerModel.hpp"

#include "Essences.hpp"
#include "EssenceSynthesis.hpp"

#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

//temporarily put this in here...
template <typename T>
struct TowerInformation
{
    static std::vector<std::pair<T,T>> TowerTierCoefficients;
};
//tower creation coefficients; arranged as [mean, variance]
template <typename T>
std::vector<std::pair<T,T>> TowerInformation<T>::TowerTierCoefficients { {20,5}, {30,5}, {40,5}, {50,5}, {60,5}, {70,5}, {80,5}, {85,5} };


class Tower
{
public:
    enum class Tier : int { ONE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN };
   
    Tower(tower_properties&& attributes, const int tier_roll)
        : base_attributes(std::move(attributes))
    {
        modifiers.resize(tier_roll);
        mod_count = 0;

        tier = static_cast<Tier>(tier_roll);
    }

    virtual void set_model(std::shared_ptr<TowerModel> t_model)
    {
        tower_model = t_model;
    }
    
    virtual std::shared_ptr<TowerModel> get_model() const
    {
        return tower_model;
    }
   
    virtual void attack()
    {
        //will need to have some projectile type to generate, and then I need to figure out if we want seeking or non-seeking
        //attacks, etc. for now dont have anything...
        
    }

    virtual void apply_modifier(const std::vector<tower_properties>& modifier_list)
    {
        for (auto modifier : modifier_list)  
            base_attributes += modifier;
    }


    virtual bool add_modifier(tower_generator tower_gen, essence* modifier)
    {
        if(mod_count >= get_tier(tier))
        {
            //TODO: also want to generate an error message for the backend error queue
            std::cout << "Too many modifiers for the tier!" << std::endl;
            return false;
        }

        //modifiers.at(mod_count++) = std::move(modifier);
        modifiers.at(mod_count++) = std::unique_ptr<essence>(modifier); 

        tower_properties working_properties;
        //re-compute the tower modifier attribute contribution
        switch(mod_count)
        {
            case 1:
                modifiers.at(0)->apply_modifier(working_properties);
                break;
            case 2:
                working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get());
                break;
            case 3:
                working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(), 
                        modifiers.at(2).get());
                break;
            case 4:
                working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                        modifiers.at(2).get(), modifiers.at(3).get());
                break;
                /*
            case 5:
                working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                        modifiers.at(2).get(), modifiers.at(3).get(), modifiers.at(4).get());                
                break;
            case 6:
                working_properties = tower_gen.combine(modifiers.at(0).get(), modifiers.at(1).get(),
                        modifiers.at(2).get(), modifiers.at(3).get(), modifiers.at(4).get(), modifiers.at(5).get());                
                break;
                */
            default:
                std::cout << "Problem @ Tower Modification -- " << mod_count << " #essences" << std::endl;
        }

        attack_attributes = working_properties + base_attributes;
        std::cout << "Modified Tower: " << attack_attributes << std::endl;
        return true;
    }

    //TODO: decide how to handle the targetting -- will the projectiles follow the mob, or go to a location,
    //or should we have a mix of both?
    //bool set_target()

	friend std::ostream& operator <<(std::ostream& out_stream, const Tower& t);   

protected:

    template <typename EType>
    auto get_tier(EType t)
        -> typename std::underlying_type<EType>::type
    {
        return static_cast<typename std::underlying_type<EType>::type>(t);
    }

    const static int MAX_UPGRADE_LEVEL = 3;


    //the baseline, fundamental (and immutable) attributes
    tower_properties base_attributes;
    //the current, subject-to-change attributes
    tower_properties attack_attributes;
    double cost;

    //should we keep the tower model here? Will probably want to pull this out into its own class soon
    std::shared_ptr<TowerModel> tower_model;

    //assume we'll have tower tiers be in the range [1, 11). 
    Tier tier;
    int upgrade_level;

    //how many different groups of modifiers can there be? For now, we'll assume just essences?
    //before accessing, we want to make sure we dont exceed the tier in # elements
    std::vector<std::unique_ptr<essence>> modifiers;
    int mod_count;

    double last_timestamp;
};

namespace TowerGenerator
{
std::unique_ptr<Tower> make_fundamentaltower(const int tier, const std::string& tower_id);
}
#endif
