#include "Controller/Controller.hpp"
#include "../fractal_util.hpp"
#include "Model/TowerDefense.hpp"
#include "Model/util/Types.hpp"
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

/*
    OgreDisplay<TDBackendStub> display;
    Controller controller (display.get_root(), display.get_render_window());
    display.register_input_controller(&controller);
    GLooper<OgreDisplay, TDBackendStub> gloop (&display);

    display.draw_maptiles(TDBackendStub::MAP_W, TDBackendStub::MAP_H);
    display.start_display();

    gloop.stop_gloop();
*/
template <typename TDType>
void add_testtower(TDType* td)
{
    using PixelType = uint8_t;
    std::string mesh_filename { TDHelpers::get_basepath() + "/data/meshfractal3d.vtk"};

    std::vector<std::vector<uint32_t>> polygon_mesh;
    std::vector<std::vector<float>> polygon_points;
    views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

    const std::string tower_material {"Examples/Chrome"};
    const std::string tower_name {"ViewTest"};
    td->add_tower(std::move(polygon_mesh), std::move(polygon_points), tower_material, tower_name);
}

int main()
{
  	std::cout << "Path is: " << TDHelpers::get_basepath() << std::endl;
    using TDBackendType = TowerLogic;
    using TDType = TowerDefense<OgreDisplay, TDBackendType>;

    auto display = new OgreDisplay<TDBackendType>();
    Controller controller (display->get_root(), display->get_render_window());

    std::unique_ptr<TDType> td = std::unique_ptr<TDType>(new TDType(display));
    display->register_input_controller(&controller);  

    td->init_game();
    td->start_game();
    add_testtower(td.get());

    display->start_display();
    std::cout << "All Done" << std::endl;

    //kill the backend too
    td->stop_game();
}
