#include "Tower.hpp"

//why the hell do I have this here?
std::ostream& operator <<(std::ostream& out_stream, const tower_properties& props)
{
    for (auto it : props.damage)
        out_stream << ElementInfo::get_element_name(it.first) << " Damage: ["
        << it.second.low << ", " << it.second.high << "] \n";

    out_stream << "Speed: " << props.attack_speed << " Range: " << props.attack_range;
    return out_stream;
}

std::ostream& operator <<(std::ostream& out_stream, const Tower& t)
{
    out_stream << "Fundamental Tower -- " << t.base_attributes << std::endl;
    out_stream << "Synthesized Tower -- " << t.attack_attributes << std::endl;
	return out_stream;
}

namespace TowerGenerator
{
/*
 * The base tower will have some tier-dependant default stats and a default model (a sphere perhaps?)
 * then the user will upgrade it with various items, which will change its stats and character model.
 * Presumably its attack projectile as well. What about the element distribution?
 */
std::unique_ptr<Tower> make_fundamentaltower(const int tier)
{
    //make some base stats based on the tower tier
    tower_properties base_attributes;
    auto dmg_it = base_attributes.damage.find(Elements::CHAOS);
    if(dmg_it != base_attributes.damage.end())
        dmg_it->second = tower_properties::dmg_dist(2 * tier, 5 * tier);
    base_attributes.attack_speed = 1 * tier;
    base_attributes.attack_range = 3 * tier;

    //etc...

    auto base_tower = std::unique_ptr<Tower>(new Tower(std::move(base_attributes), tier));

    //anything else? --> make the tower model. Actually this needs a bit more thought -- where should the stuff
    //related to the display in the Model be? Should we have a central tower-model making class?
    
    return base_tower;
}

}
