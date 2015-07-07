#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include "MapTile.hpp"
#include "ModelUtils.hpp"
#include "util/Types.hpp"

#include <tuple>
#include <memory>
#include <list>

/*
 * The monster class -- just a placeholder for now
 */
class Monster
{
public:
    
    Monster(const CharacterModels::ModelIDs mob_id, const std::string& mob_name, float starting_row, float starting_col)
        : id(mob_id), monster_name(mob_name), current_position(starting_col, starting_row)
    {
      //placeholder model -- TODO: make some sort of factory arrangement for making the different mobs
      id = CharacterModels::ModelIDs::ogre_S;
      speed = 0.05f;
    }

    //returned as [row, col]
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

    void set_path(std::list<const MapTile*> mob_path)
    {
      path = std::move(mob_path);
      //get the 1st destination on the mob path 
      auto dest_tile = path.front();
      path.pop_front();
      dest_position = Coordinate<float>(dest_tile->tile_center.col, dest_tile->tile_center.row);
    }

    Coordinate<float> move_update(const uint64_t time)
    {
        timestamp = time;        

        float nx_factor = (dest_position.col - current_position.col);
        float ny_factor = (dest_position.row - current_position.row);
        float target_dist = std::sqrt(nx_factor*nx_factor + ny_factor*ny_factor);

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

            //TODO: move distance_left units along the new trajectory
            //TODO: how best to handle this part? it's possible that the mob moves very fast, and we cover multiple destinations in one cycle... need to loop?
            //
          }
          else {
            //this means we hit the final destination. Need the game logic to remove the mob, reduce player health etc.
            //TODO: implement whatever state changes are needed...
          }
        }
        else
        {
            float dist_mag = speed / target_dist;
            current_position.col += nx_factor * dist_mag;
            current_position.row += ny_factor * dist_mag;
        }
        return current_position;
    }

private:
    //normalized positions wrt the map
    Coordinate<float> current_position;
    Coordinate<float> dest_position;

    //the game time at the point of creation
    uint64_t timestamp;

    //NOTE: we don't 'own' these, the game map owns these, we just have pointers to them
    //(and we know that the game map will outlive any monsters)
    std::list<const MapTile*> path;

		/*
		 * TODO: determine which stats are needed for the monster type
		 *
		 */
		float health;
		float speed;

    //TODO: need some armor type and amount

		//the character model ID
    CharacterModels::ModelIDs id;
    std::string monster_name;
};

template <typename MonsterT, class ... MonsterArgs>
Monster* make_monster(MonsterArgs ... args)
{
  return new MonsterT (std::forward<MonsterArgs>(args) ...);
}

//TODO: have some more detailed factory scheme for creating the different monsters... need to wait on 
//the creation of these other monster types however


#endif
