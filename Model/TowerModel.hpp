#ifndef TD_TOWER_MODEL_HPP
#define TD_TOWER_MODEL_HPP


#include <vector>
#include <string>


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


#endif
