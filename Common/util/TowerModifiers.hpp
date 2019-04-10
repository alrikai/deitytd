/* TowerModifiers.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_TOWER_MODIFIERS_HPP
#define TD_TOWER_MODIFIERS_HPP

#include "AttributeModifiers.hpp"
#include "Elements.hpp"
#include "TowerProperties.hpp"

/*
 * Need to make the atomic tower attributes here -- namely, these will
 constitute the pool of
 * possible attributes to pull from when making the word combinations, e.g.

 Tower modifiers:
    +damage (constant [min, max) range)
    +%enhanced damage
    +attack speed
    +% crit chance
    +crit damage
    +range
    +fire damage
    +lightning damage
    +water/ice damage
    +earth damage

    +% physical damage
    +% magic damage
    +total tower mana
    +tower mana regen
    -spells mana cost, either a flat amount or a %
    +% luck (e.g. anything %-based that the item holder does is modified)
    +% magic find (e.g. if mob is killed, their item drop % is higher)
    +% gold find (e.g. if mob is killed, the amount of gold they yield is
 higher)
    -% move speed (e.g. if mob is hit, they move slower)
    -% physical armor (e.g. if mob is hit they take more damage)
    -% magic armor
    +damage vs mobs under %Y health
    - %target water resistance
    - %target air resistance
    - %target fire resistance
    - %target earth resistance

    Note that we also need to define an order of precedence for each of these,
 s.t. when we have each as an object to be applied to the
*/

namespace TowerModifiers {

struct flat_damage : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x1;
  struct parameter_cfg {
    float low_val;
    float high_val;

    float scale_factor;

    constexpr inline uint32_t get_ID() const { return flat_damage::ID; }
    constexpr inline const char* get_name() const { return "flat_damage"; }
  };

  //----------------------------------------------------------

  flat_damage(parameter_cfg cfg)
      : value(cfg.low_val, cfg.high_val), scale(cfg.scale_factor) {}
  flat_damage(float low_value, float high_value, float scale_f)
      : value(low_value, high_value), scale(scale_f) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    // this would give a flat +damage to all stats
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    for (int elem_idx = 0; elem_idx < tower_property_modifier::NUM_ELEM;
         elem_idx++) {
      stats_modifier.damage_value[elem_idx] += value;
    }
  }

  inline void scale_modifier(float score) override final {
    value.low += scale * score;
    value.high += scale * score;
  }

  tower_properties::dmg_dist value;
  float scale;
};

struct enhanced_damage : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x2;
  struct parameter_cfg {
    float ed_percent;
    float scale_factor;
    constexpr inline uint32_t get_ID() const { return enhanced_damage::ID; }
    constexpr inline const char* get_name() const { return "enhanced_damage"; }
  };

  //----------------------------------------------------------

  enhanced_damage(parameter_cfg cfg)
      : value(cfg.ed_percent), scale(cfg.scale_factor) {}
  // NOTE: the value for amount should be a percentage (so say, a value of 50
  // means 50%)
  enhanced_damage(float amount, float scale_factor)
      : value(amount / 100.f), scale(scale_factor) {}

  //----------------------------------------------------------

  // for if we're applying the modifier directly....
  inline void apply_modifier(tower_properties &stats) override final {
    // this would give a +X% enhanced damage to all stats
    aggregate_modifier(stats.modifier); 
  }

  // or if we are aggregating a set of modifiers to be applied later...
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    for (int elem_idx = 0; elem_idx < tower_property_modifier::NUM_ELEM;
         elem_idx++) {
      stats_modifier.enhanced_damage_value[elem_idx] += value;
    }
  }

  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  float scale;
};

struct enhanced_speed : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x3;
  struct parameter_cfg {
    float es_percent;
    float scale_factor;
    constexpr inline uint32_t get_ID() const { return enhanced_speed::ID; }
    constexpr inline const char* get_name() const { return "enhanced_speed"; }
  };

  //----------------------------------------------------------

  enhanced_speed(parameter_cfg cfg)
      : value(cfg.es_percent), scale(cfg.scale_factor) {}

  // again, the amount should be given as a percentage
  enhanced_speed(float amount, float scale_factor)
      : value(amount / 100.f), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.attack_speed_value += value;
  }
  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  float scale;
};

struct flat_range : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x4;
  struct parameter_cfg {
    float range;
    float scale_factor;
    constexpr inline uint32_t get_ID() const { return flat_range::ID; }
    constexpr inline const char* get_name() const { return "flat_range"; }
  };

  //----------------------------------------------------------

  flat_range(parameter_cfg cfg) : value(cfg.range), scale(cfg.scale_factor) {}
  flat_range(float amount, float scale_factor)
      : value(amount), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.attack_range_value += value;
  }
  inline void scale_modifier(float score) override final {
    value += score * scale;
  }

  float value;
  float scale;
};

struct flat_crit_chance : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x5;
  struct parameter_cfg {
    float crit_percent;
    float scale_factor;
    constexpr inline uint32_t get_ID() const { return flat_crit_chance::ID; }
    constexpr inline const char* get_name() const { return "flat_crit_chance"; }
  };

  //----------------------------------------------------------

  flat_crit_chance(parameter_cfg cfg)
      : value(cfg.crit_percent), scale(cfg.scale_factor) {}
  flat_crit_chance(float amount, float scale_factor)
      : value(amount), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.crit_chance_value += value;
  }
  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  float scale;
};

struct flat_crit_multiplier : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x6;
  struct parameter_cfg {
    float crit_multiplier_percent;
    float scale_factor;
    constexpr inline uint32_t get_ID() const { return flat_crit_multiplier::ID; }
    constexpr inline const char* get_name() const { return "flat_crit_multiplier"; }
  };

  //----------------------------------------------------------

  flat_crit_multiplier(parameter_cfg cfg)
      : value(cfg.crit_multiplier_percent), scale(cfg.scale_factor) {}
  flat_crit_multiplier(float amount, float scale_factor)
      : value(amount), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.crit_multiplier_value += value;
  }
  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  float scale;
};

// can give a flat +damage for one type of damage
struct flat_type_damage : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x7;
	static constexpr uint32_t FD_OFFSET = 0x00FF0000;
  static constexpr uint32_t CHAOS_ID = FD_OFFSET + 0x0;
  static constexpr uint32_t WATER_ID = FD_OFFSET + 0x1;
  static constexpr uint32_t AIR_ID = FD_OFFSET + 0x2;
  static constexpr uint32_t FIRE_ID = FD_OFFSET + 0x3;
  static constexpr uint32_t EARTH_ID = FD_OFFSET + 0x4;
  static constexpr const char* NAME = "flat_type_damage";

  struct parameter_cfg {
    float low_val;
    float high_val;
    Elements type;
    float scale_factor;

    inline uint32_t get_ID() const {
      return flat_type_damage::ID + static_cast<int>(type) +
             flat_type_damage::CHAOS_ID;
    }
    inline const char* get_name() const { 
			std::string type {"flat_type_damage_"}; 
		  auto typecode = get_ID() - ID;
			switch(typecode) {
        case CHAOS_ID:
					type += "chaos";
					break;
				case WATER_ID:
					type += "water";
					break;
				case AIR_ID:
					type += "air";
					break;
				case FIRE_ID:
					type += "fire";
					break;
				case EARTH_ID:
					type += "earth";
					break;
				default:
					std::string err_msg {"Invalid flat_type_damage typecode " + std::to_string(typecode)};
					throw std::runtime_error(err_msg);
			}
		  return type.c_str();
		}
  };

  //----------------------------------------------------------

  flat_type_damage(parameter_cfg cfg)
      : value(cfg.low_val, cfg.high_val), type(cfg.type), scale(cfg.scale_factor) {
    std::cout << "setting type " << static_cast<int>(type) << " -- flat dmg ["
              << value.low << ", " << value.high << "]" << std::endl;
  }

  flat_type_damage(float low_v, float high_v, Elements type, float scale_factor)
      : value(low_v, high_v), type(type), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.damage_value[static_cast<int>(type)] += value;
  }
  inline void scale_modifier(float score) override final {
    value.low += scale * score;
    value.high += scale * score;
  }

  tower_properties::dmg_dist value;
  Elements type;
  float scale;
};

// can give a flat +damage for one type of damage
struct enhanced_type_damage : stat_attribute_modifier {
  static constexpr uint32_t ID = 0x8;
	static constexpr uint32_t ED_OFFSET = 0x00FE0000;
  static constexpr uint32_t CHAOS_ID = ED_OFFSET + 0x0;
  static constexpr uint32_t WATER_ID = ED_OFFSET + 0x1;
  static constexpr uint32_t AIR_ID = ED_OFFSET + 0x2;
  static constexpr uint32_t FIRE_ID = ED_OFFSET + 0x3;
  static constexpr uint32_t EARTH_ID = ED_OFFSET + 0x4;
  static constexpr const char* NAME = "enhanced_type_damage";

  struct parameter_cfg {
    float ed_percent_amount;
    Elements type;
    float scale_factor;

    inline uint32_t get_ID() const {
      return enhanced_type_damage::ID + static_cast<int>(type) +
             enhanced_type_damage::CHAOS_ID;
    }
    inline const char* get_name() const { 
			std::string type {"enhanced_type_damage_"}; 
		  auto typecode = get_ID() - ID;
			switch(typecode) {
        case CHAOS_ID:
					type += "chaos";
					break;
				case WATER_ID:
					type += "water";
					break;
				case AIR_ID:
					type += "air";
					break;
				case FIRE_ID:
					type += "fire";
					break;
				case EARTH_ID:
					type += "earth";
					break;
				default:
					std::string err_msg {"Invalid enhanced_type_damage_ typecode " + std::to_string(typecode)};
					throw std::runtime_error(err_msg);
			}
		  return type.c_str();
		}
  };

  //----------------------------------------------------------

  enhanced_type_damage(parameter_cfg cfg)
      : value(cfg.ed_percent_amount), type(cfg.type), scale(cfg.scale_factor) {}
  enhanced_type_damage(float amount, Elements type, float scale_factor)
      : value(amount / 100.f), type(type), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.enhanced_damage_value[static_cast<int>(type)] += value;
  }
  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  Elements type;
  float scale;
};

// NOTE: we also need a decay period for this i.e. gives +N damage per hit
// up to +M damage total, where each bonus application lasts for X sec
//
struct flat_damage_onhit : event_attribute_modifier {
  static constexpr uint32_t ID = 0x9;
  struct parameter_cfg {
    float increment_amount;
    float cap;
    float duration;
    float scale_factor;

    constexpr inline uint32_t get_ID() const { return flat_damage_onhit::ID; }
    constexpr inline const char* get_name() const { return "flat_damage_onhit"; }
  };

  //----------------------------------------------------------

  flat_damage_onhit(parameter_cfg cfg)
      : increment_value(cfg.increment_amount), max_amount(cfg.cap),
        duration(cfg.duration), scale(cfg.scale_factor) {
    max_num_applications = max_amount / increment_value;
    // have none active at the start
    timer_ticks.resize(max_num_applications, 0.f);
    oldest_application = 0;
  }

  flat_damage_onhit(float increment_amount, float cap, float duration,
                    float scale_factor)
      : increment_value(increment_amount), max_amount(cap), duration(duration),
        scale(scale_factor) {
    max_num_applications = max_amount / increment_value;
    // have none active at the start
    timer_ticks.resize(max_num_applications, 0.f);
    oldest_application = 0;
  }

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    // err.... is this the right way to do this?
    stats.modifier.on_hit_events.push_back(this);
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.on_hit_events.push_back(this);
  }
  inline void scale_modifier(float score) override final {
    increment_value += scale * score;
    max_amount = max_num_applications * increment_value;
  }

  //@HERE -- need to cycle through the active application of it, and overwrite
  //the oldest with the new application. Also need to decrease the existing ones
  // by the time delta and de-activate any that are <= 0
  void apply_on_event(Monster *mob, Tower *tower,
                      float time_delta) override final {
    timer_ticks[oldest_application] = duration + time_delta;
    oldest_application = (oldest_application + 1) % max_num_applications;

    float additional_damage = 0.f;
    // update time delta state
    for (int application_idx = 0; application_idx < max_num_applications;
         application_idx++) {
      timer_ticks[oldest_application] -= time_delta;
      if (timer_ticks[oldest_application] > 0) {
        additional_damage += increment_value;
      }
    }

    // TODO: apply the modifier... in this case we just need the tower's
    // tower_properties, but for others... who knows? will be +additional_damage
    // as flat damage in this case
    //
    std::cout << "will add +" << additional_damage << " damage" << std::endl;
  }

  float increment_value;
  float max_amount;
  float duration;
  float scale;

  int max_num_applications;
  std::vector<float> timer_ticks;
  int oldest_application;
};

//this is flat typed damage that is added *after* enhancements are taken into account,
//whereas flat_damage is done *pre* enhancements. 
//So, we can get bigger #'s for this one, but it will not scale as well. 
struct flat_added_damage : stat_attribute_modifier {
  static constexpr uint32_t ID = 0xa;
	static constexpr uint32_t FAD_OFFSET = 0x00FD0000;
  static constexpr uint32_t CHAOS_ID = FAD_OFFSET + 0x0;
  static constexpr uint32_t WATER_ID = FAD_OFFSET + 0x1;
  static constexpr uint32_t AIR_ID = FAD_OFFSET + 0x2;
  static constexpr uint32_t FIRE_ID = FAD_OFFSET + 0x3;
  static constexpr uint32_t EARTH_ID = FAD_OFFSET + 0x4;
  struct parameter_cfg {
    float flat_dmg_amount;
    Elements type;
    float scale_factor;

    inline uint32_t get_ID() const {
      return flat_added_damage::ID + static_cast<int>(type) +
             flat_added_damage::CHAOS_ID;
    }
    inline const char* get_name() const { 
			std::string type {"enhanced_type_damage_"}; 
		  auto typecode = get_ID() - ID;
			switch(typecode) {
        case CHAOS_ID:
					type += "chaos";
					break;
				case WATER_ID:
					type += "water";
					break;
				case AIR_ID:
					type += "air";
					break;
				case FIRE_ID:
					type += "fire";
					break;
				case EARTH_ID:
					type += "earth";
					break;
				default:
					std::string err_msg {"Invalid enhanced_type_damage_ typecode " + std::to_string(typecode)};
					throw std::runtime_error(err_msg);
			}
		  return type.c_str();
		}  
	};


  //----------------------------------------------------------

  flat_added_damage(parameter_cfg cfg)
      : value(cfg.flat_dmg_amount), type(cfg.type), scale(cfg.scale_factor) {
    std::cout << "setting type " << static_cast<int>(type) << " -- flat dmg "
              << value << std::endl;
  }

  flat_added_damage(float amount, Elements type, float scale_factor)
      : value(amount), type(type), scale(scale_factor) {}

  //----------------------------------------------------------

  inline void apply_modifier(tower_properties &stats) override final {
    aggregate_modifier(stats.modifier); 
  }
  inline void
  aggregate_modifier(tower_property_modifier &stats_modifier) override final {
    stats_modifier.damage_value[static_cast<int>(type)] += value;
  }
  inline void scale_modifier(float score) override final {
    value += scale * score;
  }

  float value;
  Elements type;
  float scale;
};


/*

struct  : tower_attribute_modifier
{

    inline void apply_modifier (tower_properties& stats) override final
    {
    }
};


struct  : tower_attribute_modifier
{

    inline void apply_modifier (tower_properties& stats) override final
    {
    }
};


struct  : tower_attribute_modifier
{

    inline void apply_modifier (tower_properties& stats) override final
    {
    }
};
*/

} 

std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_damage::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::enhanced_damage::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::enhanced_speed::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_range::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_crit_chance::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_crit_multiplier::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_type_damage::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::enhanced_type_damage::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_damage_onhit::parameter_cfg &cfg);
std::ostream &operator<<(std::ostream &stream,
                         const TowerModifiers::flat_added_damage::parameter_cfg &cfg);
#endif
