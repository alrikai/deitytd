#ifndef TD_TOWER_HPP
#define TD_TOWER_HPP

#include "util/Elements.hpp"
#include "util/Types.hpp"
#include "TowerModel.hpp"
#include "TowerAttack.hpp"

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

/////////////////////////////////////////////////////////////////////
// forward declaration of mobs. Keep a ptr to the 'current' target //
/////////////////////////////////////////////////////////////////////
class Monster;

class Tower
{
public:
    enum class Tier : int { ONE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN };
   
    Tower(tower_properties&& attributes, const std::string& id, const int tier_roll, const float row, const float col)
        : base_attributes(std::move(attributes)), tower_id(id), position(col, row), current_target(nullptr)
    {
        modifiers.resize(tier_roll);
        mod_count = 0;

        tier = static_cast<Tier>(tier_roll);
    }

    virtual bool add_modifier(tower_generator tower_gen, essence* modifier); 
    
    //this is baisically a factory function for generating a given towers' attacks.
    //Tower subclasses can override to do whatever extra steps they need
    virtual std::unique_ptr<TowerAttackBase> generate_attack(const std::string& attack_id, const uint64_t timestamp);

    virtual void set_model(std::shared_ptr<TowerModel> t_model)
    {
        tower_model = t_model;
    }
    
    virtual std::shared_ptr<TowerModel> get_model() const
    {
        return tower_model;
    }
   
    virtual void apply_modifier(const std::vector<tower_properties>& modifier_list)
    {
        for (auto modifier : modifier_list)  
            base_attributes += modifier;
    }

    inline bool in_range(const float target_dist) const
    {
        //return target_dist < attack_attributes.attack_range;
        return target_dist < base_attributes.attack_range;
    }

    inline std::shared_ptr<Monster> get_target() const
    {
        return current_target;
    }

    inline void set_target (std::shared_ptr<Monster>& mob)
    {
        current_target = mob;
    }

    inline void reset_target ()
    {
        current_target.reset();
    }


    inline std::string get_id() const
    {
        return tower_id;
    }

    inline Coordinate<float> get_position() const
    {
        return position;
    }

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
    std::string tower_id; 

    //assume we'll have tower tiers be in the range [1, 11). 
    Tier tier;
    int upgrade_level;

    //how many different groups of modifiers can there be? For now, we'll assume just essences?
    //before accessing, we want to make sure we dont exceed the tier in # elements
    std::vector<std::unique_ptr<essence>> modifiers;
    int mod_count;

    uint64_t last_timestamp;
    //the tower center position
    Coordinate<float> position;

    //cache the last found target (as having to do lookups every iteration takes too long)
    std::shared_ptr<Monster> current_target;
};

namespace TowerGenerator
{
std::unique_ptr<Tower> make_fundamentaltower(const int tier, const std::string& tower_id, const float row, const float col);
}
#endif
