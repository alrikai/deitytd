/* frontend_test.cpp -- part of the DietyTD tests 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#include "Controller/Controller.hpp"
#include "../fractal_util.hpp"

#include "Model/GameMap.hpp"
#include "Model/util/Types.hpp"
#include "Views/ViewEventTypes.hpp"

#include <chrono>
#include <thread>

/*
 * much like the backend test, this one should test the frontend without the
 * real backend or gameloop in place
 * 1. be able to handle tower rendering -- this means creation and deletion
 * 2. be able to handle tower attacks -- this means creation, modification, and deletion
 */

struct TowerAttackStub
{
    TowerAttackStub(const std::string& atk_name, uint64_t time, int row, int col)
        : name(atk_name), timestamp(time), origin_row(row), origin_col(col)
    {}

    std::string name;
    uint64_t timestamp;
    int origin_row;
    int origin_col;
};

struct TDBackendStub
{
    constexpr static int MAP_W = 4;
    constexpr static int MAP_H = 3;

    constexpr static double TILE_W = 1.0/MAP_W;
    constexpr static double TILE_H = 1.0/MAP_H;

    void make_tower(int tier, const float col, const float row)
    {
    
    }

};

template <template <class>  class ViewType, class ModelType = TDBackendStub>
struct GLooper
{
    GLooper(ViewType<ModelType>* view_obj) 
    {
        view = view_obj;

        td_towerevents = std::unique_ptr<TowerEventQueueType>(new TowerEventQueueType());
        view->register_tower_eventqueue(td_towerevents.get());

        td_front_to_backend_events = std::unique_ptr<ViewEvents>(new ViewEvents());
        view->register_backend_eventqueue(td_front_to_backend_events.get());

        gloop_stop.store(false);
        gloop_thread = std::unique_ptr<std::thread>(new std::thread(&GLooper::gloop, this));
    }

    void stop_gloop()
    {
        gloop_stop.store(true);
        gloop_thread->join();
    }

    

    void gloop()
    {
        using PixelType = uint8_t;
        std::string mesh_filename {TDHelpers::get_basepath() + "/data/meshfractal3d.vtk"};

        uint64_t round_idx = 0;
        std::list<std::shared_ptr<TowerAttackStub>> current_attacks;

        //aim for 60Hz
        const double TIME_PER_ROUND = 1000.0/30.0;
        double iter_lag = 0;

        while(!gloop_stop.load())
        {
            auto start_time = std::chrono::high_resolution_clock::now();

            //have to handle certain events in here? 
            while(!td_towerevents->empty())
            {
                bool got_evt = false;
                auto td_evt = td_towerevents->pop(got_evt);    
                if(got_evt && td_evt)
                {
                    //generate a tower model and give back to the frontend
                    //...
                    //this will eventually call the fractal generation code. For now, we'll use a saved version
                
                    //NOTE: we dont want the actual event, just the row and column
                    const int tile_row = std::floor(td_evt->row_/(1.0/TDBackendStub::MAP_H));
                    const int tile_col = std::floor(td_evt->col_/(1.0/TDBackendStub::MAP_W));
                    if(t_list[tile_row][tile_col])
                    {
                        std::cout << "Already Occupied" << std::endl;
                        continue;
                    }

                    //load a fractal mesh
                    std::vector<std::vector<uint32_t>> polygon_mesh;
                    std::vector<std::vector<float>> polygon_points;
                    views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);
                    std::string t_material {"BaseWhiteNoLighting"};

                    t_list[tile_row][tile_col] = std::make_shared<TowerModel>(std::move(polygon_mesh), std::move(polygon_points), t_material);
           
                    std::cout << "Tower @ [" << tile_row << ", " << tile_col << "]" << std::endl;

                    const float col_map_offset = TDBackendStub::TILE_W * (tile_col + 0.5);
                    const float row_map_offset = TDBackendStub::TILE_H * (tile_row + 0.5);
                    std::vector<float> map_offset {col_map_offset, row_map_offset, 0.0f};
                    const std::string tower_id = "Tower_" + std::to_string(tile_row * TDBackendStub::MAP_W + tile_col);
                    std::unique_ptr<RenderEvents::create_tower> t_evt = std::unique_ptr<RenderEvents::create_tower>
                        (new RenderEvents::create_tower(t_list[tile_row][tile_col], tower_id, std::move(map_offset)));
                    td_front_to_backend_events->add_maketower_event(std::move(t_evt));
                }
            }

            
            //pretend we have an (actual) tower, with an attack speed. spawn attacks based on that speed.
            //Here we simulate 1 attack per second. Also, we may have to optimize the attack generation -- 
            //instead, collect all of the attacks together, then spawn 1 event with all the attacks resident.
            //Or, just leave the attack movement up to the frontend
            bool spawn_attack = ((round_idx % 60) == 0);
          
            if(spawn_attack)
            {
                //what coordinate system should this be in?
                std::vector<float> target {static_cast<float>(TDBackendStub::MAP_W), static_cast<float>(TDBackendStub::MAP_H), 0.0f};
                //spawn an attack at the first tower location encountered. always have it aim towards the bottom right corner
                for (int row = 0; row < TDBackendStub::MAP_H; ++row)
                {
                    for (int col = 0; col < TDBackendStub::MAP_W; ++col)
                    {
                        if(t_list[row][col])
                        {
                            //have the location either be:
                            //A. a tower object (i.e. the ID to find it with in the frontend scene)
                            //B. a normalized map location (i.e. between [0.0, 1.0] for the map)
                            //option A is preferrable

                            std::string tower_atkid = "tower_" + std::to_string(row) + "_" + std::to_string(col);
                            const std::string attack_name = tower_atkid + "_attack_" + std::to_string(round_idx);

                            //std::vector<float> map_offset {static_cast<float>(col), static_cast<float>(row), 0.0f};
                            std::unique_ptr<RenderEvents::create_attack> t_evt = std::unique_ptr<RenderEvents::create_attack>
                                (new RenderEvents::create_attack(attack_name, tower_atkid, std::move(target)));
                            td_front_to_backend_events->add_makeatk_event(std::move(t_evt));

                            current_attacks.emplace_back(new TowerAttackStub(attack_name, round_idx, row, col));
                        } 
                    }
                }
            }

            //assume we have everything to be rendered at this point -- need to come up with a baseclass
            //for renderable things perhaps?
            //
            //TODO: populate list of things for the frontend to render

            //TODO: spawn the movement events for the current attacks and remove the completed ones
            auto attack_it = current_attacks.begin();
            while(attack_it != current_attacks.end()) 
            {
                //move the attack every 500ms
                bool do_movement = ((round_idx % 30) == 0);
                if(do_movement)
                {
                    std::cout << "Spawning Movement Event -- " << current_attacks.size() << " # attacks" << std::endl;
                    //TODO: figure out how to actually do this. Need a simple way to go from frontend global coords <--> backend coords
                    const std::vector<float> movement {10.0f, 10.0f, 10.0f};
                    std::unique_ptr<RenderEvents::move_attack> t_evt = std::unique_ptr<RenderEvents::move_attack>
                        (new RenderEvents::move_attack((*attack_it)->name,  (*attack_it)->origin_id, movement));
                    td_front_to_backend_events->add_moveatk_event(std::move(t_evt));
                }

                
                bool remove_atk = false;
                //NOTE: the remove_atk should trigger when the attack either collides with a mob, or goes off screen. 
                //      we will simulate that with timestamp measurements. Note that we would also have to notify the
                //      frontend to stop them from being rendered... 
                auto t_delta = static_cast<double>(round_idx - (*attack_it)->timestamp);
                //have the attacks last for 5 rounds (5 seconds
                if(t_delta/60 > 5)
                    remove_atk = true;

                if(remove_atk)
                {
                    std::unique_ptr<RenderEvents::remove_attack> t_evt = std::unique_ptr<RenderEvents::remove_attack>
                        (new RenderEvents::remove_attack((*attack_it)->name));
                    td_front_to_backend_events->add_removeatk_event(std::move(t_evt));
                }

                attack_it = ((remove_atk) ? current_attacks.erase(attack_it) : std::next(attack_it, 1));
            }
            
       
            //still not sure how to do the time steps -- want to have a fixed timestep, but how to do it here?
            auto current_time = std::chrono::high_resolution_clock::now();
            double time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
            
            if(time_elapsed > TIME_PER_ROUND)
                std::cout << "Over the per-round target!" << std::endl;
            else
            {
                //std::cout << "Sleeping for " << (TIME_PER_ROUND - time_elapsed) << " ms" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(std::llround(std::floor(TIME_PER_ROUND - time_elapsed))));
            }
            
            start_time = current_time;
            round_idx++;
        }

    }

    using TowerEventQueueType = typename ViewType<ModelType>::TowerEventQueueType; 
    std::unique_ptr<TowerEventQueueType> td_towerevents;
    std::unique_ptr<ViewEvents> td_front_to_backend_events;

    ViewType<ModelType>* view;
    std::unique_ptr<std::thread> gloop_thread;
    std::atomic<bool> gloop_stop;

    //holds the tower models. The actual TowerDefense class will have a 2D array of Tower
    //objects, which in turn will hold the TowerModel objects
    std::shared_ptr<TowerModel> t_list [TDBackendStub::MAP_H][TDBackendStub::MAP_W];

};

int main()
{   
    OgreDisplay<TDBackendStub> display;
    Controller controller (display.get_root(), display.get_render_window());
    display.register_input_controller(&controller);
    GLooper<OgreDisplay, TDBackendStub> gloop (&display);

    display.draw_maptiles(TDBackendStub::MAP_W, TDBackendStub::MAP_H);
    display.start_display();

    gloop.stop_gloop();
    return 0;
}
