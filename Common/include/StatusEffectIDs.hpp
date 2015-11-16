#ifndef TD_STATUS_EFFECT_IDS_HPP
#define TD_STATUS_EFFECT_IDS_HPP

#include <unordered_map>

//TODO: populate this with all the various status effects
enum class STATUS_EFFECTS_IDS {
  ARMOR_REDUCE,

  FLAT_ATTACK_BOOST
};

namespace std {
    template <>
    struct hash<STATUS_EFFECTS_IDS>
    {
        size_t operator() (const STATUS_EFFECTS_IDS& id) const {
            return std::hash<int>()(static_cast<int> (id));
        }
    };
}

namespace status_helper 
{

    //returns the status precedence of the specified status effect (used when deciding what order to apply multiple statuses in)
    inline int get_status_id_precedence(const STATUS_EFFECTS_IDS id)
    {
        static const std::unordered_map<STATUS_EFFECTS_IDS, int> status_precedence {
          {STATUS_EFFECTS_IDS::ARMOR_REDUCE, 1},
          {STATUS_EFFECTS_IDS::FLAT_ATTACK_BOOST, 1}
        };

        auto seffect_id = status_precedence.find(id);
        if(seffect_id != status_precedence.end()) {
            return seffect_id->second;
        } else {
            //tried to look up a status effect that doesn't exist. This is probably exception-worthy eh?
            return -1;
        }
    }

    //returns the upoer and lower bounds for the duration of the status... should consider this to be the
    //min and max possible values at 2 (?) std deviations out from a normal distribution mean (or should I
    //just have it be a uniform distribution?)
    inline std::pair<int,int> get_status_durationrange(const STATUS_EFFECTS_IDS id)
    {
         static const std::unordered_map<STATUS_EFFECTS_IDS, std::pair<int,int>> status_durationrange {
          {STATUS_EFFECTS_IDS::ARMOR_REDUCE, {1, 5}},
          {STATUS_EFFECTS_IDS::FLAT_ATTACK_BOOST, {1, 3}}
        };

        auto seffect_id = status_durationrange.find(id);
        if(seffect_id != status_durationrange.end()) {
            return seffect_id->second;
        } else {
            //tried to look up a status effect that doesn't exist. This is probably exception-worthy eh?
            return std::make_pair(-1, -1);
        }
    }
}

#endif
