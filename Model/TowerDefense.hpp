#ifndef TD_TOWER_DEFENSE_HPP
#define TD_TOWER_DEFENSE_HPP

#include "GameMap.hpp"

#include <memory>
#include <random>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>


//holds the info needed for a tower model
struct TowerModel
{
    TowerModel(std::vector<std::vector<uint32_t>>&& polygon_mesh, std::vector<std::vector<float>>&& polygon_points, const std::string& material_name)
        : polygon_mesh_(polygon_mesh), polygon_points_(polygon_points), tower_material_name_(material_name)
    {}

    const std::vector<std::vector<uint32_t>> polygon_mesh_; 
    const std::vector<std::vector<float>> polygon_points_;
    const std::string tower_material_name_;
};


//this should be the main interface of the TD backend. Pretty uninspired name at the moment...
template <class ViewType>
class TowerDefense
{

public:
    //it's possible that we'll move to normalized coordinates?
    explicit TowerDefense(ViewType* view)
        : td_view(view)
    {
        td_view->draw_maptiles(GameMap::MAP_WIDTH, GameMap::MAP_HEIGHT);
    }

    bool is_obstructed(const int col_coord, const int row_coord) const
    {
        return map.is_obstructed(col_coord, row_coord);
    }

  
    //adds a tower model to the internal list
    bool add_tower(std::vector<std::vector<uint32_t>>&& polygon_mesh, std::vector<std::vector<float>>&& polygon_points, 
                    const std::string& tower_material, const std::string& tower_name)
    {
        auto model_exists = tower_models.find(tower_name); 
        if(model_exists != tower_models.end())
            return false; 
        return tower_models.emplace(tower_name, TowerModel(std::move(polygon_mesh), std::move(polygon_points), tower_material)).second;  
    }

    template <typename OffsetTypes>
    std::tuple<bool, std::string, TowerModel*> make_tower(const float x_coord, const float y_coord, OffsetTypes& world_offsets, OffsetTypes& map_offsets);

private:
    GameMap map;

    //assume we do not own this object
    ViewType* td_view;
    std::map<std::string, TowerModel> tower_models;
};


template <class ViewType>
template <typename OffsetTypes>
std::tuple<bool, std::string, TowerModel*> TowerDefense<ViewType>::make_tower(const float x_coord, const float y_coord, 
                                           OffsetTypes& world_offsets, OffsetTypes& map_offsets)
{
///////////////////////////////////////////////////////////////////////////////////////////////////
    //the tower name and everything would have to be specified by the user when they make it, but until 
    //we implement a GUI we'll have to simulate that part here
    //pick a random tower from the tower model list 
    std::random_device rdev{};
    std::default_random_engine eng{rdev()};
    std::uniform_int_distribution<> dis(0, tower_models.size()-1);
    auto selected_tower = tower_models.begin();
    std::advance(selected_tower, dis(eng));
///////////////////////////////////////////////////////////////////////////////////////////////////

    //using the user-coords, snap the tower model to the appropriate game map tile
    if(map.is_obstructed(x_coord, y_coord))
    {
        std::cout << "Cannot build at [" << x_coord << ", " << y_coord << "] -- is obstructed!" << std::endl; 
        return std::make_tuple(false, "", nullptr);
    }

    //we can assume that the location is valid now
    //TODO: need to have other checks, i.e. if the player has enough $$ to build it
    //TODO: need to generate the tower object
    //TODO: need to update whatever datastructures need the towers, etc.

    //get the center in map coordinates of the selected location
    auto tile_offset = map.get_tile_center(x_coord, y_coord);
    map_offsets[0] = tile_offset.col;
    map_offsets[1] = tile_offset.row;
    
    //set the tile to be obstructed
    map.set_obstructed(x_coord, y_coord, true);
    
    auto tilename = map.get_bounding_tile(x_coord, y_coord);
    const std::string tower_name {"tower_" + selected_tower->first + "__" + std::to_string(tilename.col) + "_" + std::to_string(tilename.row)};

    //get the average for each dimension (to get the center point)
    std::vector<float> dim_avgs (3, 0);
    std::for_each(selected_tower->second.polygon_points_.begin(), selected_tower->second.polygon_points_.end(), [&dim_avgs]
            (const std::vector<float>& pt)  
            {
                dim_avgs[0] += pt[0];           
                dim_avgs[1] += pt[1];           
                dim_avgs[2] += pt[2];       
            });

    //get the average coordinate in WORLD COORDINATES. Also NOTE: Since the fractals are 
    //generated as [row, col, depth], we need to shuffle the indices to [col, row, depth]
    const size_t fractal_ptfactor = selected_tower->second.polygon_points_.size();
    world_offsets[0] = dim_avgs[1] / fractal_ptfactor;
    world_offsets[1] = dim_avgs[0] / fractal_ptfactor;
    world_offsets[2] = dim_avgs[2] / fractal_ptfactor;
    return std::make_tuple(true, tower_name, &selected_tower->second);
}

#endif
