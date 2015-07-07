#ifndef TD_MAP_TILE_HPP
#define TD_MAP_TILE_HPP

#include "util/Types.hpp"

#include <tuple>
#include <memory>
#include <list>

class Monster;

struct MapTile
{
    MapTile()
        : idx_location(), tile_coord(), width(0), height(0), 
          tile_center(), occupied(false)
    {}

    //store the tile ROI via upper left coordinate and width, height
    Coordinate<int> idx_location;
    Coordinate<double> tile_coord;
    double width;
    double height;

    Coordinate<double> tile_center;

    std::list<std::shared_ptr<Monster>> resident_mobs;
    bool occupied;
};

#endif
