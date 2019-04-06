/* TowerAttack.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_TOWER_ATTACK_HPP
#define TD_TOWER_ATTACK_HPP

#include "Monster.hpp"
#include "util/Elements.hpp"
#include "util/Types.hpp"

#include <cmath>

/*
 * define the attack types here. This will cover pretty much all attacks (meele
 * will just be very short range) this will be broken largely into 2 parts; the
 * logic (i.e. creation, movement updates, collision detection, damage
 * application/on-hit effects, etc) and the graphics (i.e. all particle effects,
 * animations, models, etc).
 *
 * need to define the properties for the attack (would this just be the spawning
 * towers' attribute list + modifiers?) Also, how will we handle the targetting?
 * Will we set a target in the beginning, then see if it hits? Or will we have
 * targetted/homing attacks? Or will we have the attack movement speed be so
 * fast that they will almost always hit?
 *
 */

// since all of these are set initially by the caller, seems nicer to put them
// into a struct rather than have a constructor with a dozen inputs

class Tower;

struct TowerAttackParams {
  // TODO: Have the tower attack have a pointer to the tower that it originated
  // from, s.t. it can update the tower status (e.g. on-hit effects) directly
  // (otherwise we would need to queue all the status updates and apply them
  // later, matching it based on the origin_id, which seems needlessly
  // convoluted).
  TowerAttackParams(tower_properties atk_props, Tower *origin,
                    const std::string &atk_id, const std::string &m_id)
      : attack_attributes(atk_props), origin_tower(origin), id(atk_id),
        mob_id(m_id) {}

  tower_properties attack_attributes;
  Tower *origin_tower;
  const std::string id;
  const std::string mob_id;

  double move_speed;

  // the game time at the point of creation
  uint64_t origin_timestamp;
  // starting location
  Coordinate<float> origin_position;
  Coordinate<float> target_position;
};

class TowerAttackBase {
public:
  explicit TowerAttackBase(TowerAttackParams &&attack_params)
      : params(std::move(attack_params)) {
    // these parameters will change per-iteration
    current_position = params.origin_position;
    timestamp = params.origin_timestamp;

    // flag indicating whether the attack has hit something
    has_hit_target = false;
  }

  virtual ~TowerAttackBase() {}

  inline Tower *get_origin_tower() const { return params.origin_tower; }

  /*
  inline std::string get_origin_id() const
  {
      return params.origin_tower->get_id();
  }
  */

  inline tower_properties get_attack_attributes() const {
    return params.attack_attributes;
  }

  inline std::string get_target_id() const { return params.mob_id; }

  inline std::string get_id() const { return params.id; }

  // NOTE: it is assumed we are using normalized coordinates
  inline bool in_bounds() const {
    return current_position.row >= 0.0f && current_position.row <= 1.0f &&
           current_position.col >= 0.0f && current_position.col <= 1.0f;
  }

  inline Coordinate<float> get_position() const { return current_position; }

  inline void set_target(Coordinate<float> target) {
    params.target_position = target;
  }

  bool hit_target() { return has_hit_target; }

  virtual Coordinate<float> move_update(const uint64_t time) = 0;

protected:
  TowerAttackParams params;

  // the game time at the point of creation
  uint64_t timestamp;
  // current location
  Coordinate<float> current_position;

  bool has_hit_target;

  // how to handle the targeting?
};

// NOTE: can have various policies here for governing how the attack behaves.
// so far, AttackT will be the attack type, i.e. linear, homing, splash, random,
// etc.
template <typename AttackT> class TowerAttack : public TowerAttackBase {
public:
  explicit TowerAttack(TowerAttackParams &&attack_params,
                       AttackT &&towerattack_type)
      : TowerAttackBase(std::move(attack_params)),
        attack_type(std::move(towerattack_type)) {}

  virtual Coordinate<float> move_update(const uint64_t time) override {
    has_hit_target = attack_type(params, current_position, time);
    return current_position;
  }

private:
  AttackT attack_type;
};

struct FixedAttackMovement {
  bool operator()(TowerAttackParams &params,
                  Coordinate<float> &current_position, const uint64_t time) {
    float nx_factor = (params.target_position.col - current_position.col);
    float ny_factor = (params.target_position.row - current_position.row);
    float target_dist =
        std::sqrt(nx_factor * nx_factor + ny_factor * ny_factor);

    bool hit_target = target_dist < params.move_speed;
    if (hit_target) {
      // need to take care for over-shooting -- for now we can do something a
      // bit less difficult...
      current_position = params.target_position;
    } else {
      float dist_mag = params.move_speed / target_dist;
      current_position.col += nx_factor * dist_mag;
      current_position.row += ny_factor * dist_mag;
    }

    return hit_target;
  }
};

struct HomingAttackMovement {
  HomingAttackMovement(std::shared_ptr<Monster> &target_mob)
      : target(target_mob) {}

  bool operator()(TowerAttackParams &params,
                  Coordinate<float> &current_position, const uint64_t time) {
    if (auto atk_target = target.lock()) {
      auto target_pos = atk_target->get_position();
      params.target_position = target_pos;
    } else {
      // TODO: figure out what do we do if the mob is gone / no longer
      // available?
      //
      // if it's just out of range of the tower, then the attack should persist
      // if the mob is dead, then the attack is no longer really relevant....
      // although the situation gets complicated if we have splash damage, etc.
      // Not sure what to do in that case actually. Maybe have it detonate at the
      // mob's location of death? (i.e. it becomes a FixedAttackMovement object)
      std::cout << "NOTE: The attack's mob is gone/reset" << std::endl;
    }

    return mover(params, current_position, time);
  }

  // NOTE: having it target a monster might be too restrictive -- would be nice
  // to have something more generic than a Monster (i.e. if we had some type like
  // 'Moveable' which governed anything that could be moved around the map).... I
  // guess this will work for now though?
  std::weak_ptr<Monster> target;
  FixedAttackMovement mover;
};

#endif
