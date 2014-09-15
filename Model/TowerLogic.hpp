#ifndef TD_TOWER_LOGIC_HPP
#define TD_TOWER_LOGIC_HPP

#include "GameMap.hpp"
#include "Towers/Tower.hpp"
#include "Towers/TowerAttack.hpp"
#include "util/Types.hpp"
#include "TowerModel.hpp"
#include "Views/ViewEventTypes.hpp"


#include <memory>
#include <thread>
#include <atomic>
#include <list>

class TowerLogic
{
public:
    TowerLogic() 
     {
         //anything else to initialize goes here...
         td_frontend_events = std::unique_ptr<ViewEvents>(new ViewEvents());    
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
        return tower_models.emplace(tower_name, TowerModel(tower_name, std::move(polygon_mesh), std::move(polygon_points), tower_material)).second;  
    }

    //methods called in response to frontend events, dispatched from the gameloop
	bool make_tower(const int tier, const float x_coord, const float y_coord);
	bool modify_tower(essence* modifier, const float x_coord, const float y_coord);
    bool print_tower(const float x_coord, const float y_coord);
    bool tower_taget(const float tower_xcoord, const float tower_ycoord, const float target_xcoord, const float target_ycoord);

    void cycle_update(const uint64_t onset_timestamp, std::list<std::shared_ptr<TowerAttack>>& new_attacks);

    ViewEvents* get_frontend_eventqueue() const 
    {
        return td_frontend_events.get();
    }

private:

    GameMap map;
    tower_generator tower_gen;
    std::map<std::string, TowerModel> tower_models;

    std::unique_ptr<Tower> t_list [GameMap::MAP_HEIGHT][GameMap::MAP_WIDTH];
    std::unique_ptr<ViewEvents> td_frontend_events;
    
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

