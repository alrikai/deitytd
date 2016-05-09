/* TowerLogic.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef TD_TOWER_LOGIC_HPP
#define TD_TOWER_LOGIC_HPP

#include "GameMap.hpp"
#include "Monster.hpp"
#include "Towers/Tower.hpp"
#include "Towers/TowerAttack.hpp"
#include "TowerModel.hpp"
#include "util/Types.hpp"
#include "util/TDEventTypes.hpp"
#include "shared/common_information.hpp"
#include "shared/Player.hpp"
#include "Views/ViewEventTypes.hpp"
#include "Pathfinder.hpp"

#include <memory>
#include <thread>
#include <atomic>
#include <list>

struct mobwave_info
{
    CharacterModels::ModelIDs mob_model_id;
    std::string mob_id;
    int num_mobs;

    //TODO: anything else?
};



class TowerLogic
{
    public:
        static constexpr int MAP_NUMTILES_HEIGHT = GameMap::MAP_HEIGHT / GameMap::TowerTileHeight;
        static constexpr int MAP_NUMTILES_WIDTH = GameMap::MAP_WIDTH / GameMap::TowerTileHeight;

        //TODO: need to move the player initial state setting to elsewhere
        TowerLogic(TDPlayerInformation default_pstate) 
            : player_state(default_pstate)
        {
            //anything else to initialize goes here...
            td_frontend_events = std::unique_ptr<ViewEvents>(new ViewEvents());    

            /*
            //NOTE: THE FOLLOWING IS FOR TESTING
            auto mtile = map.get_tile(GameMap::MAP_WIDTH - 1, GameMap::MAP_HEIGHT - 1);
            std::list<std::shared_ptr<Monster>> tile_contents {std::make_shared<Monster>(mtile->tile_center.row, mtile->tile_center.col)};
            mtile->resident_mobs = tile_contents;
            std::cout << "Added Mob @ [" << GameMap::MAP_HEIGHT - 1 << ", " << GameMap::MAP_WIDTH - 1 << "]" << std::endl;
            */
        }

        bool is_obstructed(const int col_coord, const int row_coord) const
        {
            return map.is_obstructed(col_coord, row_coord);
        }

        void register_shared_info(std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_info)
        {
            shared_tower_info = shared_info;
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

        //adds a tower model to the internal list
        void make_mob(const CharacterModels::ModelIDs mob_id, const std::string& mob_name, const GameMap::IndexCoordinate& map_tile)
        {
            auto mtile = map.get_tile(map_tile);
            auto mobtile_center = mtile->tile_center;
            const float mob_row = mobtile_center.row;
            const float mob_col = mobtile_center.col;

            //should be {x, y, z} --> hence at creation it's {col, row, z}
            std::vector<float> map_offsets {mob_col, mob_row, 0};

            //TODO: use the mob_id to dispatch the appropriate monster creation (will need some factory for this)
            auto mob_ = std::shared_ptr<Monster>(make_monster<Monster>(mob_id, mob_name, mob_col, mob_row));
            //TODO: anything else we need to do here?
            //
            mtile->resident_mobs.push_back(mob_);
            live_mobs.push_back(mob_);

            //NOTE: should we have the mobs duplicated like this? --> probably not, makes no sense to maintain 2 lists of 
            //mobs for TowerLogic and the tiles. If anything, we should have 1 list, and have the other reference said list

            //notify the frontend that a mob has been made
            std::unique_ptr<RenderEvents::create_mob> m_evt = std::unique_ptr<RenderEvents::create_mob>
                (new RenderEvents::create_mob(mob_id, mob_name, std::move(map_offsets)));
            td_frontend_events->add_makemob_event(std::move(m_evt));
        }

        //methods called in response to frontend events, dispatched from the gameloop
        bool make_tower(const uint32_t ID, const int tier, const float x_coord, const float y_coord);
        bool modify_tower(tower_property_modifier* modifier, const float x_coord, const float y_coord);
        bool print_tower(const float x_coord, const float y_coord);
        bool tower_taget(const float tower_xcoord, const float tower_ycoord, const float target_xcoord, const float target_ycoord);
        //is run at the start of the round (i.e. in the transition from IDLE --> INROUND), assuming the obstructions
        //don't change during the round
        bool find_paths(const GameMap::IndexCoordinate spawn_idx, const GameMap::IndexCoordinate dest_idx);

        void cycle_update(const uint64_t onset_timestamp);

        inline int get_num_live_mobs() const
        {
            return live_mobs.size();
        }

        //for the end of the round -- clean all the state (i.e. live mobs, status effects, map tile mobs, etc)
        void reset_state()
        {      
            if(live_mobs.size() > 0) {
                std::cout << "mobs still around?" << std::endl;
            }
            live_mobs.clear();
            active_attacks.clear();
        }

        //NOTE: this is called when moving from active to idle state
        void enter_idle_state()
        {
            reset_state();

            std::unique_ptr<RenderEvents::state_transition> s_evt = 
                std::unique_ptr<RenderEvents::state_transition> (new RenderEvents::state_transition());
            s_evt->old_state = GAME_STATE::ACTIVE;
            s_evt->new_state = GAME_STATE::IDLE;
            td_frontend_events->add_statetransition_event(std::move(s_evt));
        }

        //NOTE: this is called when moving from idle to active state
        bool enter_active_state(std::vector<mobwave_info>&& mob_info, GameMap::IndexCoordinate spawn_point, GameMap::IndexCoordinate dest_point)
        {
            for (auto mob_metadata : mob_info) {
                make_mob(mob_metadata.mob_model_id, mob_metadata.mob_id, spawn_point);
            }

            //checks if there's a valid maze from the spawn to the destination point -- if not, then we need to do... something...
            const bool has_valid_path = find_paths(spawn_point, dest_point);

            std::unique_ptr<RenderEvents::state_transition> s_evt = 
                std::unique_ptr<RenderEvents::state_transition> (new RenderEvents::state_transition());
            s_evt->old_state = GAME_STATE::IDLE;
            s_evt->new_state = GAME_STATE::ACTIVE;
            td_frontend_events->add_statetransition_event(std::move(s_evt));

            return has_valid_path;
        }

        inline ViewEvents* get_frontend_eventqueue() const 
        {
            return td_frontend_events.get();
        }

        inline TDPlayerInformation get_player_state() const 
        {
            return player_state;
        }

        //again, we assume the map dimensions and tile dimensions to be even multiples
        static constexpr int TLIST_HEIGHT = GameMap::MAP_HEIGHT / GameMap::TowerTileHeight;
        static constexpr int TLIST_WIDTH = GameMap::MAP_WIDTH / GameMap::TowerTileWidth;

    private:

        void cycle_update_attacks(const uint64_t onset_timestamp);
        void cycle_update_towers(const uint64_t onset_timestamp);
        void cycle_update_mobs(const uint64_t onset_timestamp);

        //handles tower auto-targeting: attacks closest (L2 distance) mob
        bool get_targets(Tower* tower, const int t_col, const int t_row);

        GameMap map;
        //tower_generator tower_gen;
        //TowerCombiner tower_gen;
        std::map<std::string, TowerModel> tower_models;

        Pathfinder<GameMap> path_finder;

        std::unique_ptr<Tower> t_list [TLIST_HEIGHT][TLIST_WIDTH];
        std::unique_ptr<ViewEvents> td_frontend_events;
        std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_tower_info;
        TDPlayerInformation player_state;

        //the set of monsters still among the living
        std::list<std::shared_ptr<Monster>> live_mobs;
        std::list<std::unique_ptr<TowerAttackBase>> active_attacks; 
};

#endif

