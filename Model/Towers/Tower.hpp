/* Tower.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_TOWER_HPP
#define TD_TOWER_HPP

#include "TowerAttack.hpp"
#include "TowerModel.hpp"
#include "util/Elements.hpp"
#include "util/Types.hpp"

//#include "TowerProperties.hpp"
#include "shared/common_information.hpp"
#include "Combinations/TowerCombiner.hpp"
#include "util/AttributeModifiers.hpp"

//#include "Essences.hpp"
//#include "EssenceSynthesis.hpp"
//#include "StatusEffectIDs.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

// temporarily put this in here...
template <typename T> struct TowerInformation {
  static std::vector<std::pair<T, T>> TowerTierCoefficients;
};
// tower creation coefficients; arranged as [mean, variance]
template <typename T>
std::vector<std::pair<T, T>> TowerInformation<T>::TowerTierCoefficients{
    {20, 5}, {30, 5}, {40, 5}, {50, 5}, {60, 5}, {70, 5}, {80, 5}, {85, 5}};

/////////////////////////////////////////////////////////////////////
// forward declaration of mobs. Keep a ptr to the 'current' target //
/////////////////////////////////////////////////////////////////////
class Monster;

class Tower {
public:
  enum class Tier : int {
    ONE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN
  };

  Tower(tower_properties &&attributes, const uint32_t tID,
        const std::string &name, const int tier_roll, const float row,
        const float col)
      : base_attributes(std::move(attributes)), ID(tID), tower_name(name),
        position(col, row), current_target(nullptr) {
    // modifiers.resize(tier_roll);
    // mod_count = 0;
    num_kills = 0;
    tier = static_cast<Tier>(tier_roll);
  }
  virtual ~Tower() {}

  // when the tower hits some threshold for experience, have it gain
  // semi-randomized new abilities virtual bool add_modifier(tower_generator
  // tower_gen, essence* modifier); virtual bool add_modifier(const
  // TowerCombiner& tower_combiner, tower_property_modifier* modifier);
  virtual bool add_modifier(tower_property_modifier &&modifier);

  // this is baisically a factory function for generating a given towers'
  // attacks. Tower subclasses can override to do whatever extra steps they need
  virtual std::unique_ptr<TowerAttackBase>
  generate_attack(const std::string &attack_id, const uint64_t timestamp);

  virtual void set_model(std::shared_ptr<TowerModel> t_model) {
    tower_model = t_model;
  }

  virtual std::shared_ptr<TowerModel> get_model() const { return tower_model; }

  virtual bool set_properties(tower_properties &&props) {
    base_attributes += props;
    // Q: anything else we need to change? or is this it?
    std::cout << "NOTE: changed tower " << get_name() << " properties to: \n"
              << base_attributes << std::endl;

    // Q: is this return entirely vestigial?
    return true;
  }

	/*
  virtual void
  apply_modifier(const std::vector<tower_properties> &modifier_list) {
    for (auto modifier : modifier_list) {
      base_attributes += modifier;
    }
  }
	*/

  // NOTE: we would also trigger any on-kill effects here (if the tower has
  // them)
  virtual void killed_mob() { num_kills += 1; }

  inline bool in_range(const float target_dist) const {
    // return target_dist < attack_attributes.attack_range;
    return target_dist < base_attributes.modifier.attack_range_value;
  }

  inline std::shared_ptr<Monster> get_target() const { return current_target; }

  inline std::string get_target_id() const {
    return current_target->get_name();
  }

  inline void set_target(std::shared_ptr<Monster> &mob) {
    current_target = mob;
  }

  inline void reset_target() { current_target.reset(); }

  inline uint32_t get_id() const { return ID; }

  // returns the (self-reported) tower infomrnation
  inline CommonTowerInformation get_common_info() const {
    CommonTowerInformation info;
    info.base_tower_props = base_attributes;
    info.tier = get_tier(tier);
    // hmmm haven't implemented this part yet have I? --> use a placeholder for
    // now
    info.num_wordslots = info.tier;
    info.tower_name = tower_name;
    return info;
  }

	tower_properties compute_attack_damage() const;

  inline std::string get_name() const { return tower_name; }

  inline Coordinate<float> get_position() const { return position; }

  std::string get_stats() const;

  friend std::ostream &operator<<(std::ostream &out_stream, const Tower &t);

protected:
  template <typename EType>
  auto get_tier(EType t) const -> typename std::underlying_type<EType>::type {
    return static_cast<typename std::underlying_type<EType>::type>(t);
  }

  // void generate_statuseffects();

  const static int MAX_UPGRADE_LEVEL = 3;

  // the baseline, fundamental attributes. These represent the permenent
  // attributes (so only permentant changes will be written to this one)
  tower_properties base_attributes;
  tower_property_modifier enhancements;
  // the current, subject-to-change attributes
  tower_properties attack_attributes;
  double cost;

  // should we keep the tower model here? Will probably want to pull this out
  // into its own class soon
  std::shared_ptr<TowerModel> tower_model;
  const uint32_t ID;
  std::string tower_name;

  // assume we'll have tower tiers be in the range [1, 11).
  Tier tier;
  int upgrade_level;

  // how many different groups of modifiers can there be? For now, we'll assume
  // just essences? before accessing, we want to make sure we dont exceed the
  // tier in # elements std::vector<std::unique_ptr<essence>> modifiers; int
  // mod_count;

  // TODO: need some infrastructure for keeping track of the characters / slots
  // used
  //...

  uint64_t last_timestamp;
  // the tower center position
  Coordinate<float> position;

  // cache the last found target (as having to do lookups every iteration takes
  // too long)
  std::shared_ptr<Monster> current_target;
  uint32_t num_kills;

  std::vector<tower_attribute_modifier *> status_effects;
#if 0
    //when the tower attacks, it should spawn zero or more status effects, that apply to the tower and/or the monster.
    //The tower is the one that should know what statuses to spawn; the towerattack will just carry them along, as when
    //the attack collides, the status effects will be applied and modify the tower/monster/game state.
    //
    //NOTE: will need to change around how I'm doing this part, as I have a circular dependancy wrt the Tower and 
    //the status effect. Can't use an incomplete type if I store it in a vector. 
    //COULD just store the enum types, but then I would have to store each statuses' data as well.
    //--> need to put more thought into the design...
    friend class StatusEffect;
    //the per-status effect metadata
    struct statuseffect_metadata
    {
        statuseffect_metadata (STATUS_EFFECTS_IDS sid, const int duration, const int precedence) 
            : id(sid), metadata(duration, precedence)
        {}
        
        STATUS_EFFECTS_IDS id;
        StatusData metadata;
    };
	std::vector<statuseffect_metadata> status_effects; 

    //the tower-wide status parameters
    StatusParameters status_params;
#endif
};

namespace TowerGenerator {
std::unique_ptr<Tower> make_fundamentaltower(const uint32_t ID, const int tier,
                                             const std::string &tower_id,
                                             const float row, const float col);
}
#endif
