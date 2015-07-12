#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include "MapTile.hpp"
#include "ModelUtils.hpp"
#include "util/Types.hpp"

#include <tuple>
#include <memory>
#include <list>
#include <iostream>


/*
 * The monster class -- just a placeholder for now
 */
class Monster
{
public:
    
    Monster(const CharacterModels::ModelIDs mob_id, const std::string& mob_name, float starting_col, float starting_row)
        : id(mob_id), monster_name(mob_name), current_position(starting_col, starting_row)
    {
      //placeholder model -- TODO: make some sort of factory arrangement for making the different mobs
      id = CharacterModels::ModelIDs::ogre_S;
      speed = 0.05f;
      current_tile = nullptr;
      destination_tile = nullptr;
    }

    inline Coordinate<float> get_position() const
    {
        return current_position;
    }

    inline std::string get_name() const 
    {
      return monster_name; 
    }

    inline CharacterModels::ModelIDs get_mobid() const 
    {
      return id; 
    }

    //NOTE: this would be useful if we have some tower ability to teleport mobs around, or if we have map effects like wormholes.
    void set_position (Coordinate<float> position) {
      current_position = position;

      //TODO: remake the tower path... needs to have the pathfinder to do so however
      if(current_position.row >= 0.f && current_position.row < 1.0f && current_position.col >= 0.f && current_position.col < 1.0f) {
        
      }
    }

    void set_path(std::list<const MapTile*> mob_path)
    {
      path = std::move(mob_path);
      current_tile = path.front();
      path.pop_front();

      //get the 1st destination on the mob path 
      destination_tile = path.front();
      path.pop_front();
      dest_position = Coordinate<float>(destination_tile->tile_center.col, destination_tile->tile_center.row);
      std::cout << "NOTE: mob has " << path.size() << " #steps" << std::endl;
    }


    //returns the next position of the mob, and whether the mob it at its destination or not
    //(TRUE: is at destination, FALSE: not yet at destination)
    std::tuple<Coordinate<float>, bool> move_update(const uint64_t time)
    {
        timestamp = time;        

        float nx_factor = (dest_position.col - current_position.col);
        float ny_factor = (dest_position.row - current_position.row);
        float target_dist = std::sqrt(nx_factor*nx_factor + ny_factor*ny_factor);

        bool hit_destination = false;
        //check if we reached the current destination
        if(target_dist <= speed)
        {
          //the distance to travel in the next step
          const auto distance_left = speed - target_dist; 
          current_position = dest_position;

          //get the next destination
          if(path.size() > 0) {
            auto dest_tile = path.front();
            path.pop_front();
            dest_position.col = dest_tile->tile_center.col;
            dest_position.row = dest_tile->tile_center.row;

            //TODO: move distance_left units along the new trajectory -- how best to handle this part? 
            //it's possible that the mob moves very fast, and we cover multiple destinations in one cycle... need to loop?
            migrate_mob();

            //update the tile info
            current_tile = destination_tile;
            destination_tile = dest_tile;
          } else {
            std::cout << "NOTE: mob " << monster_name << " at destination" << std::endl;
            hit_destination = true;
            migrate_mob();
          }
        }
        else
        {
            float dist_mag = speed / target_dist;
            current_position.col += nx_factor * dist_mag;
            current_position.row += ny_factor * dist_mag;
        }
        return std::make_tuple(current_position, hit_destination);
    }

private:

    //move the monster to a new tile, remove from the old tile
    inline void migrate_mob()
    {
      //notify the tiles that the mob is migrating (both the tile it's leaving and the tile it's entering)
      auto mob_wpit = std::find_if(current_tile->resident_mobs.begin(), current_tile->resident_mobs.end(), 
          [this](const std::weak_ptr<Monster> &m)
          {
            if (auto other_mob = m.lock()) {
              auto other_name = other_mob->get_name();
              return other_name == this->get_name();
            }
            return false;
          });
      //move add the mob to the new tile and remove it from the old one
      if(mob_wpit != destination_tile->resident_mobs.end()) {
        if (auto mobp = mob_wpit->lock()) {
          destination_tile->resident_mobs.push_back(mobp); 
        }
        current_tile->resident_mobs.erase(mob_wpit);
      } else {
        std::cout << "ERROR: mob " << monster_name << " doesn't exist in current tile..." << std::endl;
      }
    }

    //the character model ID
    CharacterModels::ModelIDs id;
    std::string monster_name;
    
    //normalized positions wrt the map
    Coordinate<float> current_position;
    Coordinate<float> dest_position;

    //the game time at the point of creation
    uint64_t timestamp;

    //NOTE: we don't 'own' these, the game map owns these, we just have pointers to them
    //(and we know that the game map will outlive any monsters)
    std::list<const MapTile*> path;

    const MapTile* destination_tile;
    const MapTile* current_tile;

    /*
     * TODO: determine which stats are needed for the monster type
     *
     */
    float health;
    float speed;
    //TODO: need some armor type and amount
};


template <typename MonsterT, class ... MonsterArgs>
Monster* make_monster(MonsterArgs ... args)
{
  return new MonsterT (std::forward<MonsterArgs>(args) ...);
}



//TODO: have some more detailed factory scheme for creating the different monsters... need to wait on 
//the creation of these other monster types however


#endif
