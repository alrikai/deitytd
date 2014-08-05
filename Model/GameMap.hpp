#ifndef TD_GAME_MAP_HPP
#define TD_GAME_MAP_HPP

#include <array>
#include <cmath>

#include "MapTile.hpp"

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
    //number of tiles to have in the map -- 3:4 ratio of height:width
    const static int MAP_HEIGHT = 15;
    const static int MAP_WIDTH = 20;

    //indexed as [row][column]
    using MapElements = std::array<std::array<MapTile, MAP_WIDTH>, MAP_HEIGHT>;

    GameMap() 
        : tile_height(1.0/MAP_HEIGHT), tile_width(1.0/MAP_WIDTH)
    {
        setmap_dims();
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

    //return the index of the tile containing the point (note: the point is in terms of pixels)
    IndexCoordinate get_bounding_tile(PointCoordinate location) const 
    {
        return get_bounding_tile(location.col, location.row);
    }

    bool is_obstructed(const float col_location, const float row_location) const 
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        return map[map_coord.row][map_coord.col].occupied;
    }

    void set_obstructed(const float col_location, const float row_location, bool obstruct_flag)
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        map[map_coord.row][map_coord.col].occupied = obstruct_flag;
    }

    Coordinate<double> get_tile_center(const float col_location, const float row_location) const
    {
        auto map_coord = get_bounding_tile(col_location, row_location);
        return map[map_coord.row][map_coord.col].tile_center;
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
