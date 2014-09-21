#ifndef TD_TOWER_LOGIC_HPP
#define TD_TOWER_LOGIC_HPP

#include "GameMap.hpp"
#include "Monster.hpp"
#include "Towers/Tower.hpp"
#include "Towers/TowerAttack.hpp"
#include "TowerModel.hpp"
#include "util/Types.hpp"
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

         //NOTE: THE FOLLOWING IS FOR TESTING
         auto mtile = map.get_tile(GameMap::MAP_WIDTH - 1, GameMap::MAP_HEIGHT - 1);
         std::list<std::shared_ptr<Monster>> tile_contents {std::make_shared<Monster>(mtile->tile_center.row, mtile->tile_center.col)};
         mtile->resident_mobs = tile_contents;
         std::cout << "Added Mob @ [" << GameMap::MAP_HEIGHT - 1 << ", " << GameMap::MAP_WIDTH - 1 << "]" << std::endl;
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

    //methods called in response to frontend events, dispatched from the gameloop
	bool make_tower(const int tier, const float x_coord, const float y_coord);
	bool modify_tower(essence* modifier, const float x_coord, const float y_coord);
    bool print_tower(const float x_coord, const float y_coord);
    bool tower_taget(const float tower_xcoord, const float tower_ycoord, const float target_xcoord, const float target_ycoord);

    void cycle_update(const uint64_t onset_timestamp);

    ViewEvents* get_frontend_eventqueue() const 
    {
        return td_frontend_events.get();
    }

    //again, we assume the map dimensions and tile dimensions to be even multiples
    static constexpr int TLIST_HEIGHT = GameMap::MAP_HEIGHT / GameMap::TowerTileHeight;
    static constexpr int TLIST_WIDTH = GameMap::MAP_WIDTH / GameMap::TowerTileWidth;

private:

    //handles tower auto-targeting: attacks closest (L2 distance) mob
    bool get_targets(Tower* tower, const int t_row, const int t_col);

    GameMap map;
    tower_generator tower_gen;
    std::map<std::string, TowerModel> tower_models;

    std::unique_ptr<Tower> t_list [TLIST_HEIGHT][TLIST_WIDTH];
    std::unique_ptr<ViewEvents> td_frontend_events;
    
    //the set of monsters still among the living
    std::list<std::shared_ptr<Monster>> live_mobs;
    std::list<std::unique_ptr<TowerAttack>> active_attacks; 

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

