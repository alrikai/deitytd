#include "Controller/Controller.hpp"
#include "views_util.hpp"
#include "Model/TowerDefense.hpp"

#include <memory>

/*
 *  How to test the Model?
 *  We want to have all 3, the Controller, Views and Model working together.
 *
 *  Hence we need to have the Model communicating with the Views in some manner -- 
 *  how best to do this? We need to have a very loose coupling between all the
 *  components. Hence, how should we do the 2-way communication between the 
 *  Views and the Model?
 *  Maybe just have them templated on eachother and have an implicit interface?
 *  Or should I go a step further and have an adapter class inbetween?
 *
 *  This will mostly be the ViewsTest, but with the backend game logic in the loop
 */

int main()
{
    OgreDisplay display;
    Controller controller (display.get_root(), display.get_render_window());

    using TDType = TowerDefense<OgreDisplay>;
    std::unique_ptr<TDType> td = std::unique_ptr<TDType>(new TDType(&display));

    display.register_input_controller(&controller);   
    display.register_model(td.get());

    using PixelType = uint8_t;
    std::string mesh_filename {"/home/alrik/TowerDefense/build/meshfractal3d.vtk"};

    std::vector<std::vector<uint32_t>> polygon_mesh;
    std::vector<std::vector<float>> polygon_points;
    views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

    const std::string tower_material {"Examples/Chrome"};
    const std::string tower_name {"ViewTest"};

    td->add_tower(std::move(polygon_mesh), std::move(polygon_points), tower_material, tower_name);
    display.start_display();
}
