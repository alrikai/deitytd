#ifndef TD_GAME_MAP_HPP__
#define TD_GAME_MAP_HPP__

#include <array>

#include "MapTile.hpp"



/*
 *  This is enforcing that the map has 600 x 800 tiles. The #pixels per tile
 *  is configurable at the point of construction
 */
class GameMap
{
public:

    GameMap(const int mapsize_pxheight, const int mapsize_pxwidth) 
        : map_pxheight(mapsize_pxheight), map_pxwidth(mapsize_pxwidth), 
          tile_pxheight(mapsize_pxheight / MAP_HEIGHT), tile_pxwidth(mapsize_pxwidth / MAP_WIDTH)
    {
        setmap_dims();
    }


    //return the index of the tile containing the point (note: the point is in terms of pixels)
    Coordinate get_bounding_tile(Coordinate location)
    {
        //check boundary
        if(location.row > map_pxheight || location.col > map_pxwidth)
            return Coordinate(-1, -1);

        int tile_row = map_pxheight / location.row;
        int tile_col = map_pxwidth / location.col;
        return Coordinate (tile_row, tile_col);
    }

    //would likely have other helper functions -- 

private:        

    void setmap_dims()
    {
        for (int tile_row = 0; tile_row < MAP_HEIGHT; ++tile_row)
        {
            for (int tile_col = 0; tile_col < MAP_WIDTH; ++tile_col)
            {
                map[tile_row][tile_col].loc.row = tile_row;
                map[tile_row][tile_col].loc.col = tile_col;
                map[tile_row][tile_col].width = tile_pxwidth;
                map[tile_row][tile_col].height = tile_pxheight;
            }
        }
    }

    //number of tiles to have in the map
    const static int MAP_HEIGHT = 600;
    const static int MAP_WIDTH = 800;
    using MapElements = std::array<std::array<MapTile, MAP_HEIGHT>, MAP_WIDTH>;


    const int map_pxheight;
    const int map_pxwidth;
    const int tile_pxheight;
    const int tile_pxwidth;

    MapElements map;
};

#endif
