#include <iostream>
#include <memory>
#include <chrono>
#include <string>

#include <fstream>

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include "TowerModel.hpp"
#include "pcl_mesh.hpp"
#include "Views/OgreDisplay.hpp"


namespace views_utils
{

/*
 * helper for either loading or generating a fractal model
 */
template <typename PixelType>
void add_mesh(std::vector<std::vector<uint32_t>>& polygon_mesh, std::vector<std::vector<float>>& polygon_points, std::string mesh_filename = "")
{
    std::cout << "Generating Point Cloud + Mesh..." << std::endl;
    bool loaded_meshfile = false;
    loaded_meshfile = TowerModelUtil::load_mesh(mesh_filename, polygon_mesh, polygon_points);
    //the fallback measure...
    if(!loaded_meshfile)
        std::tie(polygon_mesh, polygon_points) = MeshMaker::make_mesh<PixelType>();

    //have an optional comparison mode for the loaded mesh files
    bool compare_ref = false;
    if(compare_ref)
    {
        std::vector<std::vector<uint32_t>> polygon_mesh_ref;
        std::vector<std::vector<float>> polygon_points_ref;
        std::tie(polygon_mesh_ref, polygon_points_ref) = MeshMaker::make_mesh<PixelType>();
  
        //compare the two -- should be equal
        assert(polygon_mesh_ref.size() == polygon_mesh.size());
        for (int i = 0; i < polygon_mesh.size(); ++i)
            for (int j = 0; j < polygon_mesh.at(i).size(); ++j)
                if(std::abs(polygon_mesh_ref[i][j] - polygon_mesh[i][j]) > 1)
                    std::cout << "ERR@ [" << i << ", " << j << "]: " << polygon_mesh_ref[i][j] << " vs " << polygon_mesh[i][j] << std::endl;

        assert(polygon_points_ref.size() == polygon_points.size()); 
        for (int i = 0; i < polygon_points.size(); ++i)
            for (int j = 0; j < polygon_points.at(i).size(); ++j)
                if(std::abs(polygon_points_ref[i][j] - polygon_points[i][j]) > 1)
                    std::cout << "ERR@ [" << i << ", " << j << "]: " << polygon_points_ref[i][j] << " vs " << polygon_points[i][j] << std::endl;
    }

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
}

}
