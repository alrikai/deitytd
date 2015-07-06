#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include "ModelUtils.hpp"

#include <tuple>
#include <memory>

#include <limits>
#include <algorithm>

//does a BFS on the map, finds the costs of traversal for any given point to the destination
//Then from this we can find the optimal path for any of the mobs 
template <typename MapT>
class Pathfinder
{
  using map_tile_t = typename MapT::MapTileT;
public:
  Pathfinder(const MapT& game_map, const map_tile_t spawn_idx, const map_tile_t dest_idx)
    : gmap(game_map), spawn(spawn_idx), dest(dest_idx)
  {
    //initialize/reset the pathing data structures
    std::fill(visited_tiles.begin(), visited_tiles.end(), false);
    std::fill(path_tiles.begin(), path_tiles.end(), nullptr);
    std::fill(distance_tiles.begin(), distance_tiles.end(), std::numeric_limits<int>::max());

    visited_tiles [spawn.row * GameMap::MAP_WIDTH + spawn.col] = true; 
    distance_tiles [spawn.row * GameMap::MAP_WIDTH + spawn.col] = 0; 
  }

  //returns true if path exists to the dest, false if no path exists
  template <size_t CONN=4>
  bool operator() ()
  {
    std::queue<map_tile_t*> tile_frontier;
    tile_frontier.push_back(spawn);

    //TODO: figure out how best to determine if a path exists to the dest -- is it enough to check if the 
    //dest tile's visited_tile flag is set?
    bool is_valid_path = false;
    const int dest_idx = dest->idx_location.row * GameMap::MAP_WIDTH + dest->idx_location.col;

    //std::map<map_tile_t, bool> visited_tiles = {{spawn, true}};
    while(!tile_frontier.empty()) {
      auto current_tile = tile_frontier.front();
      tile_frontier.pop();

      const int flat_tile_idx = current_tile->row * GameMap::MAP_WIDTH + current_tile->col;
      visited_tiles[flat_tile_idx] = true;

      //check if the current tile is the destination tile
      if(dest_idx == flat_tile_idx) {
        //TODO: do we want to exit the BFS if we are at the dest?
        is_valid_path = true;
      }
      //NOTE: do we want 4-neighbor or 8-neighbor? For now, just do 4 neighbor?
      const int row_idx = current_tile->idx_location.row;
      const int col_idx = current_tile->idx_location.col;

      for (int neighbor_idx = 0; neighbor_idx < CONN; ++neighbor_idx) {
        auto idx_offsets = four_neighbor[neighbor_idx];
        const int neighbor_row = row_idx + idx_offsets.row;
        const int neighbor_col = col_idx + idx_offsets.col;

        if(neighbor_row > 0 && neighbor_row < GameMap::MAP_HEIGHT) {
          if(neighbor_col > 0 && neighbor_col < GameMap::MAP_WIDTH) {
            if(!gmap.is_obstructed(neighbor_col, neighbor_row)) {
              auto neighbor_tile = gmap.get_tile(neighbor_col, neighbor_row);
              if(!visited_tiles[neighbor_tile->row * GameMap::MAP_WIDTH + neighbor_tile->col]) {
                tile_frontier.push_back(neighbor_tile);
                const int flat_neighbor_idx = neighbor_row * GameMap::MAP_WIDTH + neighbor_col;
                //keep track of the path that's been used to get to the current point
                path_tiles[flat_neighbor_idx] = current_tile;
                //... also keep track of the distance of the tile wrt the spawn point
                distance_tiles[flat_neighbor_idx] = distance_tiles[flat_tile_idx] + 1;
              }
            }
          }
        }
      }
    }

    
    return is_valid_path;
  }

  //TODO: give some accessor to get the path from any given point to the destination -- might need to re-arrange the
  //path_tiles datastructure to make this a bit easier? -- what should we return it as? a vector, or a list? 

  //makes the path list from the specified source tile to the dest tile
  std::list<map_tile_t*> get_path(GameMap::IndexCoordinate source_tile)
  {
    std::list<map_tile_t*> path;
    auto source_tile = gmap.get_tile(source_tile.col, source_tile.row);
    path.push_back(source_tile);

    auto path_node = path_tiles[source_tile.row * GameMap::MAP_WIDTH + source_tile.col];
    while(path_node && path_node != dest) {
      path.push_back(path_node);
      //get the index of the node
      const int path_nodeidx = path_node->idx_location.row * GameMap::MAP_WIDTH + path_node->idx_location.col;
      path_node = path_tiles[path_nodeidx];
    }
    return path;
  }


private:  
  const MapT& gmap;
  map_tile_t* spawn;
  map_tile_t* dest;

  std::array<bool, GameMap::MAP_HEIGHT*GameMap::MAP_WIDTH> visited_tiles;
  std::array<map_tile_t*, GameMap::MAP_HEIGHT*GameMap::MAP_WIDTH> path_tiles;
  std::array<int, GameMap::MAP_HEIGHT*GameMap::MAP_WIDTH> distance_tiles;


  //arranged as (col, row)
  static const GameMap::IndexCoordinate four_neighbor [4] = 
    {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

};

/*
 * The monster class -- just a placeholder for now
 */
class Monster
{
public:
    
    Monster(float starting_row, float starting_col)
        : row_pos(starting_row), col_pos(starting_col)
    {
      //placeholder model -- TODO: make some sort of factory arrangement for making the different mobs
      id = CharacterModels::ModelIDs::ogre_S;
    }

    //returned as [row, col]
    std::tuple<float, float> get_position() const
    {
        return std::make_tuple(row_pos, col_pos);
    }

private:
    //normalized positions wrt the map
    float row_pos;
    float col_pos;

		float row_dest;
    float col_dest;

		/*
		 * TODO: determine which stats are needed for the monster type
		 *
		 */
		float health;
		float speed;

    //TODO: need some armor type and amount

		//the character model ID
    CharacterModels::ModelIDs id;
};

template <typename MonsterT, class ... MonsterArgs>
Monster* make_monster(MonsterArgs ... args)
{
  return new MonsterT (std::forward<MonsterArgs>(args) ...);
}

//TODO: have some more detailed factory scheme for creating the different monsters... need to wait on 
//the creation of these other monster types however


#endif
