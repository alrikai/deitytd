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

bool Tower::add_modifier(tower_generator tower_gen, essence* modifier)
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

std::unique_ptr<TowerAttack> Tower::generate_attack(const std::string& attack_id, const uint64_t timestamp)
{
    //return std::unique_ptr<TowerAttack>(new TowerAttack( attack_id, tower_id));
   
    //set the attack parameters  
    TowerAttackParams params (base_attributes, attack_id, tower_id);

    //attack movement type -- homing updates the attack movement wrt a target,
    //while non-homing has an initial destination and moves towards it
    params.has_homing = false;
    //distance the attack can move per round (in terms of map tiles?)
    params.move_speed = 8; //GameMap::TowerTileHeight;
  
    //the game time at the point of creation
    params.origin_timestamp = timestamp;
    //starting location
    params.origin_position = position;
    return std::unique_ptr<TowerAttack>(new TowerAttack(std::move(params)));
}

namespace TowerGenerator
{
/*
 * The base tower will have some tier-dependant default stats and a default model (a sphere perhaps?)
 * then the user will upgrade it with various items, which will change its stats and character model.
 * Presumably its attack projectile as well. What about the element distribution?
 */
std::unique_ptr<Tower> make_fundamentaltower(const int tier, const std::string& tower_id, const float row, const float col)
{
    //make some base stats based on the tower tier
    tower_properties base_attributes;
    auto dmg_it = base_attributes.damage.find(Elements::CHAOS);
    if(dmg_it != base_attributes.damage.end())
        dmg_it->second = tower_properties::dmg_dist(2 * tier, 5 * tier);
    base_attributes.attack_speed = 1 * tier;
    base_attributes.attack_range = 3 * tier;

    //etc...

    auto base_tower = std::unique_ptr<Tower>(new Tower(std::move(base_attributes), tower_id, tier, row, col));

    //load a fractal mesh -- the base tower will always look the same, but the tower models will diverge as they're upgraded.
    //would it be worth sharing the base tower model and using a copy-on-write scheme for it?
    std::vector<std::vector<uint32_t>> polygon_mesh;
    std::vector<std::vector<float>> polygon_points;
    const std::string mesh_filename { TDHelpers::get_basepath() + "/data/meshfractal3d.vtk"};
    
    TowerModelUtil::load_mesh(mesh_filename, polygon_mesh, polygon_points);
    std::string t_material {"FractalTower"}; //{"BaseWhiteNoLighting"};
    auto tower_model = std::make_shared<TowerModel>(std::move(polygon_mesh), std::move(polygon_points), t_material); 
    base_tower->set_model(tower_model);

    return base_tower;
}

}
