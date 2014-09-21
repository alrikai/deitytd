#ifndef TD_MAP_TILE_HPP
#define TD_MAP_TILE_HPP

#include "Monster.hpp"

#include <tuple>
#include <memory>
#include <list>

template <typename T>
struct Coordinate
{
    Coordinate()
        : col(0), row(0)
    {}

    Coordinate (const T col_, const T row_)
        : col(col_), row(row_)    
    {}

    T col;
    T row;
};

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
