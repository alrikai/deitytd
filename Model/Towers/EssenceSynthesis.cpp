
#include "EssenceSynthesis.hpp"

/*
 *  just make whatever combinations here. Should have some sort of unqiue features.
 *  Also, consider how to make this easier to adjust. Perhaps it should get its 
 *  coefficient values from a text file or something?
 */
namespace combination_rules{

tower_properties combine_essences(aphrodite* aph_essence, apollo* apo_essence)
{
    tower_properties properties;

    //need to think about what kind of damage types to have for these
    properties.damage.find(Elements::WATER)->second = tower_properties::dmg_dist (1.7f,2.0f);
    properties.attack_range = 1.9;
    properties.attack_speed = 1.8;

    return properties;
}

tower_properties combine_essences(athena* ath_essence, artemis* art_essence)
{
    tower_properties properties;

    properties.damage.find(Elements::AIR)->second = tower_properties::dmg_dist (1.9f, 3.0f);
    properties.attack_range = 1.0;
    properties.attack_speed = 0.5;

    return properties;
}

tower_properties combine_essences(aphrodite* aph_essence, apollo* apo_essence, ares* are_essence)
{
    tower_properties properties;

    properties.damage.find(Elements::FIRE)->second = tower_properties::dmg_dist (17.0f,20.0f);
    properties.attack_range = 19;
    properties.attack_speed = 18;

    return properties;
}

tower_properties combine_essences(athena* ath_essence, artemis* art_essence, demeter* dem_essence)
{
    tower_properties properties;

    properties.damage.find(Elements::EARTH)->second = tower_properties::dmg_dist (19.0f,30.0f);
    properties.attack_range = 10;
    properties.attack_speed = 5;

    return properties;

}

}
