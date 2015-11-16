#ifndef TD_PATHFINDER_HPP
#define TD_PATHFINDER_HPP

#include <list>
#include <array>
#include <limits>
#include <algorithm>

//does a BFS on the map, finds the costs of traversal for any given point to the destination
//Then from this we can find the optimal path for any of the mobs 
template <typename MapT>
class Pathfinder
{
  using map_tile_t = typename MapT::MapTileT;
  using index_coord_t = typename MapT::IndexCoordinate;

  static constexpr int GRID_WIDTH  = MapT::MAP_WIDTH;
  static constexpr int GRID_HEIGHT = MapT::MAP_HEIGHT;

  //arranged as (col, row)
  static const index_coord_t four_neighbor [4];

public:

  Pathfinder()
    : dest (nullptr)
  {}

  //returns true if path exists to the dest, false if no path exists
  template <size_t CONN=4>
  bool operator() (const MapT& gmap, const index_coord_t spawn_tile, const index_coord_t dest_tile)
  {
    reset_state();

    const map_tile_t* spawn = gmap.get_tile(spawn_tile);
    std::queue<const map_tile_t*> tile_frontier;
    tile_frontier.push(spawn);

    visited_tiles [spawn_tile.row * GRID_WIDTH + spawn_tile.col] = true; 
    distance_tiles [spawn_tile.row * GRID_WIDTH + spawn_tile.col] = 0; 

    //TODO: figure out how best to determine if a path exists to the dest -- is it enough to check if the 
    //dest tile's visited_tile flag is set?
    bool is_valid_path = false;
    const int dest_idx = dest_tile.row * GRID_WIDTH + dest_tile.col;

    int visit_count = 0;
    //std::map<map_tile_t, bool> visited_tiles = {{spawn, true}};
    while(!tile_frontier.empty()) {
      auto current_tile = tile_frontier.front();
      tile_frontier.pop();

      const int flat_tile_idx = current_tile->idx_location.row * GRID_WIDTH + current_tile->idx_location.col;
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

        if(neighbor_row >= 0 && neighbor_row < GRID_HEIGHT) {
          if(neighbor_col >= 0 && neighbor_col < GRID_WIDTH) {
            if(!gmap.is_obstructed(neighbor_col, neighbor_row)) {
              auto neighbor_tile = gmap.get_tile(neighbor_col, neighbor_row);
              const int flat_neighbor_idx = neighbor_row * GRID_WIDTH + neighbor_col;
              if(!visited_tiles[flat_neighbor_idx]) {
                tile_frontier.push(neighbor_tile);
                //keep track of the path that's been used to get to the current point
                path_tiles[flat_neighbor_idx] = current_tile;
                //... also keep track of the distance of the tile wrt the spawn point
                distance_tiles[flat_neighbor_idx] = distance_tiles[flat_tile_idx] + 1;

                visited_tiles[flat_neighbor_idx] = true;
                visit_count++;
              }
            }
          }
        }
      }
    }

    if(is_valid_path) {
      dest = gmap.get_tile(dest_tile);
    }
    return is_valid_path;
  }

  //TODO: give some accessor to get the path from any given point to the destination -- might need to re-arrange the
  //path_tiles datastructure to make this a bit easier? -- what should we return it as? a vector, or a list? 

  //makes the path list from the specified source tile to the dest tile
  //NOTE: this function has some (hidden) dependancy on the above function -- that is, if there is NO valid path,
  //then this function will not have anything to return other than the source tile. Not really much to do about this,
  //we rely on the caller to take the appropriate measures if no valid path exists
  std::list<const map_tile_t*> get_path(const map_tile_t* source_tile)
  {
    std::list<const map_tile_t*> path;
    //NOTE: add the current tile so that the mob knows its starting point 
    path.push_front(source_tile);

    auto path_node = path_tiles[source_tile->idx_location.row * GRID_WIDTH + source_tile->idx_location.col];
    while(path_node && path_node != dest) {
      //path.push_back(path_node);
      path.push_front(path_node);
      //get the index of the node
      const int path_nodeidx = path_node->idx_location.row * GRID_WIDTH + path_node->idx_location.col;
      path_node = path_tiles[path_nodeidx];
    }

    return path;
  }


private:  
  inline void reset_state()
  {
    //initialize/reset the pathing data structures
    std::fill(visited_tiles.begin(), visited_tiles.end(), false);
    std::fill(path_tiles.begin(), path_tiles.end(), nullptr);
    std::fill(distance_tiles.begin(), distance_tiles.end(), std::numeric_limits<int>::max());
  }

  std::array<bool, GRID_HEIGHT*GRID_WIDTH> visited_tiles;
  std::array<const map_tile_t*, GRID_HEIGHT*GRID_WIDTH> path_tiles;
  std::array<int, GRID_HEIGHT*GRID_WIDTH> distance_tiles;

  const map_tile_t* dest;
};

template <typename MapT>
const typename Pathfinder<MapT>::index_coord_t Pathfinder<MapT>::four_neighbor [4] = 
    {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

#endif
