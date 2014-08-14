#include "Controller/Controller.hpp"
#include "../fractal_util.hpp"

#include "Model/GameMap.hpp"
#include "Views/ViewEventTypes.hpp"


/*
 * much like the backend test, this one should test the frontend without the
 * real backend or gameloop in place
 */

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
                    
                    view->add_render_event(std::move(t_evt));
                }
            }

            
            //assume we have everything to be rendered at this point -- need to come up with a baseclass
            //for renderable things perhaps?
            //
            //TODO: populate list of things for the frontend to render

            std::list<std::shared_ptr<TowerModel>> render_targets;
            for (int i = 0; i < TDBackendStub::MAP_H; ++i)
                for (int j = 0; j < TDBackendStub::MAP_W; ++j)
                    if(t_list[i][j])
                        render_targets.push_back(t_list[i][j]);
            
            
            //trigger the rendering
        
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
