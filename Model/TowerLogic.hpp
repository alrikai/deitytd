#ifndef TD_TOWER_LOGIC_HPP
#define TD_TOWER_LOGIC_HPP

#include "GameMap.hpp"
#include "Towers/Tower.hpp"
#include "Towers/TowerAttack.hpp"
#include "util/Types.hpp"

#include <memory>
#include <thread>
#include <atomic>
#include <list>

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

class TowerLogic
{
public:
    TowerLogic() 
     {
         //anything else to initialize goes here...
     }

//    void register_tower_build_queue(std::shared_ptr<UserTowerEvents::BuildTowerEventQueueType> build_queue);
//    void stop_backend();
//    void start_backend();

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

    //methods called in response to frontend events, dispatched from the gameloop
	bool make_tower(const int tier, const float x_coord, const float y_coord);
	bool modify_tower(essence* modifier, const float x_coord, const float y_coord);
    bool print_tower(const float x_coord, const float y_coord);


    void cycle_update(const double onset_timestamp);

private:

    GameMap map;
    tower_generator tower_gen;
    std::map<std::string, TowerModel> tower_models;

    std::unique_ptr<Tower> t_list [GameMap::MAP_HEIGHT][GameMap::MAP_WIDTH];
    
    //think if there's a better datastructure for these -- we will just be iterating through them,
    //but we will also have to frequently insert and delete elements -- insertion can just be a 
    //block insertion, while deletion will be of random elements. 
    std::list<std::shared_ptr<TowerAttack>> attack_list;

/*    
    void backend_evtloop();
    std::shared_ptr<UserTowerEvents::BuildTowerEventQueueType> tbuild_queue;

    //NOTE: still conflicted on whether to have a backend event loop as well -- might make sense when
    //we get to managing the monsters as well?
    std::unique_ptr<std::thread> backend_thread;
    std::atomic<bool> backend_continue;
*/    
};

#endif

