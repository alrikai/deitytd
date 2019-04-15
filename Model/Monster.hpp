/* Monster.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include "MapTile.hpp"
#include "ModelUtils.hpp"
#include "util/Elements.hpp"
#include "util/MonsterProperties.hpp"
#include "util/TowerProperties.hpp"
#include "util/Types.hpp"
//#include "StatusEffects.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <list>
#include <memory>
#include <tuple>

/*
 * The monster class -- just a placeholder for now
 */
class Monster {
public:
  Monster(const CharacterModels::ModelIDs mob_id, const std::string &mob_name,
          const MonsterStats &stats)
      : Monster(mob_id, mob_name, stats, 0, 0) {}
  Monster(const CharacterModels::ModelIDs mob_id, const std::string &mob_name,
          const MonsterStats &stats, float starting_col, float starting_row)
      : current_position(starting_col, starting_row), id(mob_id),
        attributes(stats), monster_name(mob_name) {
    current_tile = nullptr;
    destination_tile = nullptr;

    // placeholder model -- TODO: make some sort of factory arrangement for
    // making the different mobs
    id = CharacterModels::ModelIDs::ogre_S;
  }

  inline MonsterStats get_attributes() const { return attributes; }

  inline Coordinate<float> get_position() const { return current_position; }

  inline std::string get_name() const { return monster_name; }

  inline CharacterModels::ModelIDs get_mobid() const { return id; }

  inline bool is_alive() const { return attributes.health > 0; }

  inline bool recieve_damage(const float atk_dmg) {
    attributes.health -= atk_dmg;

    std::cout << "mob " << get_name() << " has " << attributes.health
              << " health" << std::endl;
    return is_alive();
  }

  // NOTE: this would be useful if we have some tower ability to teleport mobs
  // around, or if we have map effects like wormholes.
  void set_position(Coordinate<float> position) {
    current_position = position;

    // TODO: remake the tower path... needs to have the pathfinder to do so
    // however
    if (current_position.row >= 0.f && current_position.row < 1.0f &&
        current_position.col >= 0.f && current_position.col < 1.0f) {
    }
  }

  void set_path(std::list<const MapTile *> mob_path) {
    path = std::move(mob_path);
    current_tile = path.front();
    path.pop_front();

    // get the 1st destination on the mob path
    destination_tile = path.front();
    path.pop_front();
    dest_position = Coordinate<float>(destination_tile->tile_center.col,
                                      destination_tile->tile_center.row);
    std::cout << "NOTE: mob has " << path.size() << " #steps" << std::endl;
  }

  // returns the next position of the mob, and whether the mob it at its
  // destination or not (TRUE: is at destination, FALSE: not yet at destination)
  std::tuple<Coordinate<float>, bool> move_update(const uint64_t time) {
    timestamp = time;

    float nx_factor = (dest_position.col - current_position.col);
    float ny_factor = (dest_position.row - current_position.row);
    float target_dist =
        std::sqrt(nx_factor * nx_factor + ny_factor * ny_factor);

    bool hit_destination = false;
    // check if we reached the current destination
    if (target_dist <= attributes.speed) {
      // the distance to travel in the next step
      const auto distance_left = attributes.speed - target_dist;
      current_position = dest_position;

      // get the next destination
      if (path.size() > 0) {
        auto dest_tile = path.front();
        path.pop_front();
        dest_position.col = dest_tile->tile_center.col;
        dest_position.row = dest_tile->tile_center.row;

        // TODO: move distance_left units along the new trajectory -- how best
        // to handle this part? it's possible that the mob moves very fast, and
        // we cover multiple destinations in one cycle... need to loop?
        migrate_mob();

        // update the tile info
        current_tile = destination_tile;
        destination_tile = dest_tile;
      } else {
        std::cout << "NOTE: mob " << monster_name << " at destination"
                  << std::endl;
        hit_destination = true;
        migrate_mob();
      }
    } else {
      float dist_mag = attributes.speed / target_dist;
      current_position.col += nx_factor * dist_mag;
      current_position.row += ny_factor * dist_mag;
    }
    return std::make_tuple(current_position, hit_destination);
  }

private:
  // move the monster to a new tile, remove from the old tile
  inline void migrate_mob() {
    // notify the tiles that the mob is migrating (both the tile it's leaving
    // and the tile it's entering)
    auto mob_wpit = std::find_if(current_tile->resident_mobs.begin(),
                                 current_tile->resident_mobs.end(),
                                 [this](const std::weak_ptr<Monster> &m) {
                                   if (auto other_mob = m.lock()) {
                                     auto other_name = other_mob->get_name();
                                     return other_name == this->get_name();
                                   }
                                   return false;
                                 });
    // move add the mob to the new tile and remove it from the old one
    if (mob_wpit != destination_tile->resident_mobs.end()) {
      if (auto mobp = mob_wpit->lock()) {
        destination_tile->resident_mobs.push_back(mobp);
      }
      current_tile->resident_mobs.erase(mob_wpit);
    } else {
      std::cout << "ERROR: mob " << monster_name
                << " doesn't exist in current tile..." << std::endl;
    }
  }

  // normalized positions wrt the map
  Coordinate<float> current_position;
  Coordinate<float> dest_position;

  // the game time at the point of creation
  uint64_t timestamp;

  // NOTE: we don't 'own' these, the game map owns these, we just have pointers
  // to them (and we know that the game map will outlive any monsters)
  std::list<const MapTile *> path;

  const MapTile *destination_tile;
  const MapTile *current_tile;

  // the character model ID
  CharacterModels::ModelIDs id;
  std::string monster_name;
  MonsterStats attributes;
};

template <typename MonsterT, class... MonsterArgs>
Monster *make_monster(MonsterArgs... args) {
  return new MonsterT(std::forward<MonsterArgs>(args)...);
}

inline std::tuple<std::string, CharacterModels::ModelIDs, MonsterStats>
parse_monster_info(const std::string &mob_cfg) {
  YAML::Node cfg_root = YAML::LoadFile(mob_cfg);
  if (cfg_root.IsNull()) {
    std::ostringstream ostr;
    ostr << "ERROR -- config yaml file " << mob_cfg << " not found";
    throw std::runtime_error(ostr.str());
  }

  YAML::Node mob_node = cfg_root["MonsterAttributes"];
  const std::string base_name = mob_node["name"].as<std::string>();
  const std::string model = mob_node["model"].as<std::string>();
  const CharacterModels::ModelIDs mob_model_id =
      CharacterModels::to_modelid(model);

  YAML::Node mob_attributes = mob_node["attributes"];
  const auto health = mob_attributes["health"].as<float>();
  const auto speed = mob_attributes["speed"].as<float>();
  const auto armor_flat = mob_attributes["flat_def"].as<float>();
  const auto armor_percent = mob_attributes["percent_def"].as<float>();
  const auto armor_thresh = mob_attributes["threshold_def"].as<float>();
  const auto mob_estr = mob_attributes["element_ID"].as<std::string>();
  const auto element_type = ElementInfo::get_element_type(mob_estr);

  MonsterStats stats(health, speed, element_type, armor_flat, armor_percent,
                     armor_thresh);
  return std::make_tuple(base_name, mob_model_id, stats);
}

inline std::vector<std::shared_ptr<Monster>>
parse_monster(const std::string &mob_cfg, const std::string name_prefix,
              size_t num_mobs = 1) {
  auto minfo = parse_monster_info(mob_cfg);
  std::vector<std::shared_ptr<Monster>> mobs;
  for (size_t idx = 0; idx < num_mobs; idx++) {
    std::string mobname =
        name_prefix + std::get<0>(minfo) + std::to_string(idx);
    mobs.emplace_back(std::make_shared<Monster>(
        std::get<1>(minfo), std::move(mobname), std::move(std::get<2>(minfo))));
  }
  return mobs;
}

// TODO: have some more detailed factory scheme for creating the different
// monsters... need to wait on the creation of these other monster types however

#endif
