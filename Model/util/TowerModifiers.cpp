#include "TowerModifiers.hpp"

std::ostream& operator<< (std::ostream& stream, const flat_damage::parameter_cfg& cfg)
{
    stream << "flat_damage::parameter_cfg: [ " << cfg.low_val << ", " << cfg.high_val << " ]";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const enhanced_damage::parameter_cfg& cfg)
{
    stream << "enhanced_damage::parameter_cfg: " << cfg.ed_percent << "%";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const enhanced_speed::parameter_cfg& cfg)
{
    stream << "enhanced_speed::parameter_cfg: " << cfg.es_percent << "%";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const flat_range::parameter_cfg& cfg)
{
    stream << "flat_range::parameter_cfg: " << cfg.range << " units";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const flat_crit_chance::parameter_cfg& cfg)
{
    stream << "flat_crit_chance::parameter_cfg: " << cfg.crit_percent << " %";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const flat_crit_multiplier::parameter_cfg& cfg)
{
    stream << "flat_crit_multiplier::parameter_cfg: " << cfg.crit_multiplier_percent << " %";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const flat_type_damage::parameter_cfg& cfg)
{
    static const std::string element_names [] = {
      "CHAOS", "WATER", "AIR", "FIRE", "EARTH"
    };

    stream << "flat_type_damage::parameter_cfg: " << cfg.flat_dmg_amount << " @ " << element_names[static_cast<int>(cfg.type)];
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const enhanced_type_damage::parameter_cfg& cfg)
{
    static const std::string element_names [] = {
      "CHAOS", "WATER", "AIR", "FIRE", "EARTH"
    };

    stream << "enhanced_type_damage::parameter_cfg: " << cfg.ed_percent_amount << " @ " << element_names[static_cast<int>(cfg.type)];
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const flat_damage_onhit::parameter_cfg& cfg)
{
    stream << "flat_damage_onhit::parameter_cfg: inc -- " << cfg.increment_amount << " max -- " << cfg.cap << " duration -- " << cfg.duration;
    return stream;
}
