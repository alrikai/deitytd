#include "Controller/Controller.hpp"
#include "fractal_util.hpp"

/*
 * how do we test the view without the model?
 */
int main()
{   
    OgreDisplay display;
    Controller controller (display.get_root(), display.get_render_window());
 
    display.register_input_controller(&controller);
/*
    using PixelType = uint8_t;
    std::string mesh_filename {"/home/alrik/TowerDefense/build/meshfractal3d.vtk"};
	views_utils::add_mesh<PixelType>(display, mesh_filename);
*/
    display.start_display();

    return 0;
}
