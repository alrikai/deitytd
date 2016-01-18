#include "TowerProperties.hpp"

std::ostream& operator <<(std::ostream& out_stream, const tower_properties& props)
{
    const static std::array<std::string, tower_properties::NUM_ELEM> element_names {
        {"CHAOS", "WATER", "AIR", "FIRE", "EARTH"}
    };
    for (int eidx = 0; eidx < tower_properties::NUM_ELEM; eidx++) {
        out_stream << element_names[eidx] << " Damage: ["
        << props.damage[eidx].low << ", " << props.damage[eidx].high << "] \n";
    }
    out_stream << "Speed: " << props.attack_speed << " Range: " << props.attack_range << "\n";
    out_stream << "Crit: " << props.crit_chance << " Crit Multiplier: " << props.crit_multiplier;
    return out_stream;
}
//----------------------------------------------------------------------------------------
