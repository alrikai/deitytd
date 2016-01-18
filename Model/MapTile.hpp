/* MapTile.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



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

    mutable std::list<std::weak_ptr<Monster>> resident_mobs;
    bool occupied;
};

#endif
