#include "Controller/Controller.hpp"
#include "../fractal_util.hpp"

#include "Model/GameMap.hpp"
#include "Views/ViewEventTypes.hpp"


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
        std::string mesh_filename {"/home/alrik/TowerDefense/build/meshfractal3d.vtk"};

        uint64_t round_idx = 0;
        std::list<std::shared_ptr<TowerAttackStub>> current_attacks;

        while(!gloop_stop.load())
        {
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
                    const std::string tower_id = "Tower_" + std::to_string(tile_row * TDBackendStub::MAP_W + tile_col);

                    //load a fractal mesh
                    std::vector<std::vector<uint32_t>> polygon_mesh;
                    std::vector<std::vector<float>> polygon_points;
                    views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);
                    std::string t_material {"BaseWhiteNoLighting"};

                    t_list[tile_row][tile_col] = std::make_shared<TowerModel>(tower_id, std::move(polygon_mesh), std::move(polygon_points), t_material);
           
                    std::cout << "Tower @ [" << tile_row << ", " << tile_col << "]" << std::endl;

                    const float col_map_offset = TDBackendStub::TILE_W * (tile_col + 0.5);
                    const float row_map_offset = TDBackendStub::TILE_H * (tile_row + 0.5);
                    std::vector<float> map_offset {col_map_offset, row_map_offset, 0.0f};
                    std::unique_ptr<RenderEvents::create_tower> t_evt = std::unique_ptr<RenderEvents::create_tower>
                        (new RenderEvents::create_tower(t_list[tile_row][tile_col], tower_id, std::move(map_offset)));
                    
                    view->add_render_event(std::move(t_evt));
                }
            }

            //pretend we have an (actual) tower, with an attack speed. spawn attacks based on that speed
            bool spawn_attack = ((round_idx % 10000000) == 0);
          
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

                            std::string tower_atkid = t_list[row][col]->tower_id_;
                            const std::string attack_name = tower_atkid + "_attack_" + std::to_string(round_idx);

                            //std::vector<float> map_offset {static_cast<float>(col), static_cast<float>(row), 0.0f};
                            std::unique_ptr<RenderEvents::create_attack> t_evt = std::unique_ptr<RenderEvents::create_attack>
                                (new RenderEvents::create_attack(attack_name, tower_atkid, std::move(target)));
                            view->add_atk_event(std::move(t_evt));   

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
                //TODO: read the remove_attack events and remove the corresponding attack object
                //TODO: calculate next attack position and spawn an attack_movement event for the frontend
                bool do_movement = ((round_idx % 5000000) == 0);
                if(do_movement)
                {
                    std::cout << "Spawning Movement Event -- " << current_attacks.size() << " # attacks" << std::endl;
                    //TODO: figure out how to actually do this. Need a simple way to go from frontend global coords <--> backend coords
                    const std::vector<float> movement {10.0f, 10.0f, 10.0f};
                    std::unique_ptr<RenderEvents::move_attack> t_evt = std::unique_ptr<RenderEvents::move_attack>
                        (new RenderEvents::move_attack((*attack_it)->name, movement));
                    view->add_atkmove_event(std::move(t_evt));   
                }

                
                bool remove_atk = false;
                //NOTE: the remove_atk should trigger when the attack either collides with a mob, or goes off screen. 
                //      we will simulate that with timestamp measurements. Note that we would also have to notify the
                //      frontend to stop them from being rendered... 
                auto t_delta = static_cast<double>(round_idx - (*attack_it)->timestamp);
                if(t_delta/5000000 > 7)
                    remove_atk = true;
                attack_it = ((remove_atk) ? current_attacks.erase(attack_it) : std::next(attack_it, 1));
            }
       
            round_idx++;
        }

    }

    using TowerEventQueueType = typename ViewType<ModelType>::TowerEventQueueType; 
    std::unique_ptr<TowerEventQueueType> td_towerevents;
    
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
