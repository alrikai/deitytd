#include <iostream>
#include <memory>
#include <chrono>
#include <string>

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include "Views/OgreDisplay.hpp"
#include "Controller/Controller.hpp"

#include "pcl_mesh.hpp"

/*
 *  The View on its own is pretty dull -- a better test would be for combining the 
 *  Controller and the views
 */

template <typename PixelType>
void add_mesh(OgreDisplay& display)
{
    std::cout << "Generating Point Cloud + Mesh..." << std::endl;
    //how to draw shapes to the display? should these be seperate from the Views, or a method of the Views?
    std::vector<std::vector<uint32_t>> polygon_mesh;
    std::vector<std::vector<float>> polygon_points; 
    std::tie(polygon_mesh, polygon_points) = MeshMaker::make_mesh<PixelType>();

    std::cout << "...Done Generating Point Cloud + Mesh" << std::endl;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
// just some stuff for verifying the point cloud/polygon correctness
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    const size_t poly_size = polygon_mesh.at(0).size();
    bool all_equal = std::all_of(polygon_mesh.begin()+1, polygon_mesh.end(),
                      [poly_size] (std::vector<uint32_t>& poly) {return poly.size()==poly_size;});
    assert(all_equal);

    double max_value = 0;
    std::for_each(polygon_mesh.begin(), polygon_mesh.end(), [&max_value]
            (std::vector<uint32_t>& poly)
            {
                auto poly_val = *std::max_element(poly.begin(), poly.end());
                if(poly_val > max_value)
                    max_value = poly_val;
            });

    std::cout << "max mesh value: " << max_value << std::endl;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    display.draw_tower(polygon_mesh, polygon_points, "", "ViewTest");
}

int main()
{   
    OgreDisplay display;
    Controller controller (display.get_root(), display.get_render_window());
 
    display.register_input_controller(&controller);

    using PixelType = uint8_t;
    add_mesh<PixelType>(display);

    display.start_display();

    return 0;
}


