#ifndef TD_GAME_MAP_HPP
#define TD_GAME_MAP_HPP

#include "MapTile.hpp"
#include "Monster.hpp"

#include <array>
#include <cmath>


/*
 * will have 2 seperate space metrics; space in pixels, and space in tiles. 
 * when taking cooridnates from the Views, they'll be normalized coordinate, specifically
 * real #'s between [0 ~ 1]. The Game Map will then map these normalized coordinates to
 * the correct tiles
 */
class GameMap
{
public:
    using IndexCoordinate = Coordinate<int>;
    using PointCoordinate = Coordinate<float>;

    //towers can cover a range of tiles (e.g. a 2x2 block)
    using TowerCoordinate = Coordinate<std::tuple<int, int>>;

    //number of tiles to have in the map -- 3:4 ratio of height:width
    static constexpr int MAP_HEIGHT = 128;    //15;
    static constexpr int MAP_WIDTH = 128;     //20;

    //number of map tiles each tower occupies
    static constexpr int TowerTileHeight = 8*2;
    static constexpr int TowerTileWidth = 8*2;

    static constexpr double NormFactorHeight = 1.0/MAP_HEIGHT;
    static constexpr double NormFactorWidth = 1.0/MAP_WIDTH;
    //indexed as [row][column]
    using MapTileT = MapTile;
    using MapElements = std::array<std::array<MapTile, MAP_WIDTH>, MAP_HEIGHT>;
    

    GameMap() 
        : tile_height(NormFactorHeight), tile_width(NormFactorWidth)
    {
        setmap_dims();
    }

    const MapTile* get_tile(const int t_col, const int t_row) const
    {
        return &map[t_row][t_col];
    }

		const MapTile* get_tile(const IndexCoordinate& coord) const
    {
        return &map[coord.row][coord.col];
    }



    MapTile* get_tile(const int t_col, const int t_row) 
    {
        return &map[t_row][t_col];
    }

		MapTile* get_tile(const IndexCoordinate& coord) 
    {
        return &map[coord.row][coord.col];
    }

    //assume these are normalized coordinates 
    IndexCoordinate get_bounding_tile(const float col_location, const float row_location) const 
    {
        //check boundary
        if(row_location > 1.0f || col_location > 1.0f || row_location < 0.0f || col_location < 0.0f)
            return IndexCoordinate(-1, -1);

        //we want to get incides row e [0, MAP_HEIGHT), col e [0, MAP_WIDTH) from the normalized input row | col vals
        const int tile_row = std::floor(row_location/tile_height);
        const int tile_col = std::floor(col_location/tile_width);
        return IndexCoordinate (tile_col, tile_row);
    }

    //assume these are normalized coordinates 
    IndexCoordinate get_bounding_tile(const Coordinate<float>& location) const 
    {
        //check boundary
        if(location.row > 1.0f || location.col > 1.0f || location.row < 0.0f || location.col < 0.0f)
            return IndexCoordinate(-1, -1);

        //we want to get incides row e [0, MAP_HEIGHT), col e [0, MAP_WIDTH) from the normalized input row | col vals
        const int tile_row = std::floor(location.row/tile_height);
        const int tile_col = std::floor(location.col/tile_width);
        return IndexCoordinate (tile_col, tile_row);
    }

    TowerCoordinate get_tower_block(const float col_location, const float row_location) const
    {
        //check boundary
        if(row_location > 1.0f || col_location > 1.0f || row_location < 0.0f || col_location < 0.0f)
            return TowerCoordinate(std::make_tuple(-1, -1), std::make_tuple(-1, -1));

        const int tower_row = GameMap::TowerTileHeight * (std::lround(std::floor(row_location/tile_height)) / GameMap::TowerTileHeight);
        const int tower_col = GameMap::TowerTileWidth * (std::lround(std::floor(col_location/tile_width)) / GameMap::TowerTileWidth);
       
        //NOTE: returned range is [inclusive, exclusive)
        return TowerCoordinate(std::make_tuple(tower_col, tower_col + GameMap::TowerTileWidth), 
                               std::make_tuple(tower_row, tower_row + GameMap::TowerTileHeight));
    }

    bool is_obstructed(const int col_location, const int row_location) const 
    {
      return map[row_location][col_location].occupied;
    }

    bool is_obstructed(const float col_location, const float row_location) const 
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        return map[map_coord.row][map_coord.col].occupied;
    }

    bool is_obstructed(const TowerCoordinate& tcoord) const
    {
        for (int map_r = std::get<0>(tcoord.row); map_r < std::get<1>(tcoord.row); ++map_r)
            for (int map_c = std::get<0>(tcoord.col); map_c < std::get<1>(tcoord.col); ++map_c)
                if(map[map_r][map_c].occupied)
                    return true;
        return false;
    }

    void set_obstructed(const float col_location, const float row_location, bool obstruct_flag)
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        map[map_coord.row][map_coord.col].occupied = obstruct_flag;
    }

    void set_obstructed(const TowerCoordinate& tcoord, bool obstruct_flag)
    {
        for (int map_r = std::get<0>(tcoord.row); map_r < std::get<1>(tcoord.row); ++map_r)
            for (int map_c = std::get<0>(tcoord.col); map_c < std::get<1>(tcoord.col); ++map_c)
                map[map_r][map_c].occupied = obstruct_flag;
    }

    Coordinate<double> get_tile_center(const float col_location, const float row_location) const
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        return map[map_coord.row][map_coord.col].tile_center;
    }

    Coordinate<double> get_block_center(const TowerCoordinate& tcoord) const
    {
        double block_center_row = std::get<0>(tcoord.row) + (std::get<1>(tcoord.row) - std::get<0>(tcoord.row))/2.0;
        double block_center_col = (std::get<0>(tcoord.col) + (std::get<1>(tcoord.col) - std::get<0>(tcoord.col))/2.0);
        return Coordinate<double>(tile_width * block_center_col, tile_height * block_center_row);
    }

    template <typename MobID>
    void remove_mob(Coordinate<float> tile_coord, const MobID& mob_id)
    {
      auto tile_idx = get_bounding_tile(tile_coord);
      auto target_tile = get_tile(tile_idx);

      auto mob_it = std::find_if(target_tile->resident_mobs.begin(), target_tile->resident_mobs.end(), 
        [mob_id](const std::weak_ptr<Monster> &m)
        {
          //TODO: we might want to use different comparison schemes for identifying a mob (comparing strings is slower than say, ints)
          //will want to move this kind of thing into the Monster class itself
          if (auto target_mob = m.lock()) {
            auto target_name = target_mob->get_name();
            return target_name == mob_id;
          }
          return false;
        });

      if(mob_it != target_tile->resident_mobs.end()) {
        if (auto target_mob = mob_it->lock()) {
        std::cout << "removing mob " << target_mob->get_name() << " from tile (" << tile_idx.col << ", " << tile_idx.row << ")" << std::endl;
        }
        target_tile->resident_mobs.erase(mob_it);

        std::cout << "Target tile @(" << tile_idx.col << ", " << tile_idx.row << ")" << " has " << target_tile->resident_mobs.size() << " #mobs left" << std::endl;
      } else {
        std::cout << "Mob isn't in tile" << std::endl;
      }

      /*
      std::remove_if(target_tile->resident_mobs.begin(), target_tile->resident_mobs.end(), 
        [mob_id](const std::weak_ptr<Monster> &m)
        {
          //TODO: we might want to use different comparison schemes for identifying a mob (comparing strings is slower than say, ints)
          //will want to move this kind of thing into the Monster class itself
          if (auto target_mob = m.lock()) {
            auto target_name = target_mob->get_name();
            return target_name == mob_id;
          }
          return false;
        });
      */
    }

    //would likely have other helper functions -- 
    //TODO: think of some other helper functions 
    // ...

private:        

    void setmap_dims()
    {
        for (int tile_row = 0; tile_row < MAP_HEIGHT; ++tile_row)
        {
            for (int tile_col = 0; tile_col < MAP_WIDTH; ++tile_col)
            {
                map[tile_row][tile_col].idx_location.row = tile_row;
                map[tile_row][tile_col].idx_location.col = tile_col;

                //set the tile ROI
                map[tile_row][tile_col].width = tile_width;
                map[tile_row][tile_col].height = tile_height;
                map[tile_row][tile_col].tile_coord.row = tile_height * tile_row;
                map[tile_row][tile_col].tile_coord.col = tile_width * tile_col;
            
                //set the tile center
                map[tile_row][tile_col].tile_center.col = tile_width * (tile_col + 0.5); 
                map[tile_row][tile_col].tile_center.row = tile_height * (tile_row + 0.5); 
            }
        }
    }

    const double tile_height;
    const double tile_width;

    MapElements map;
};

#endif
