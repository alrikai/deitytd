#include <iostream>
#include <memory>
#include <chrono>
#include <string>

#include <fstream>

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include "pcl_mesh.hpp"
#include "Views/OgreDisplay.hpp"

namespace views_utils
{

//something quick and dirty to load VTK mesh files (shouldnt this already exist somewhere?)
template <typename MeshType = std::vector<std::vector<uint32_t>>, typename PointType = std::vector<std::vector<float>>>
bool load_mesh(const std::string& mesh_filename, MeshType& polygon_mesh, PointType& polygon_points)
{
    std::ifstream mesh_in;
    mesh_in.open(mesh_filename, std::ifstream::in);
    if(!mesh_in.is_open())
    {
        std::cout << "couldn't open file " << mesh_filename << std::endl;
        return false;
    }

    //5 lines of header info -- skip first 4
    for (int i = 0; i < 4; ++i)
        mesh_in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string meta_info;
    int num_points;
    mesh_in >> meta_info >> num_points >> meta_info;

    polygon_points.resize(num_points);
    float x_pt, y_pt, z_pt;
    std::cout << "#points:  " << num_points << std::endl;
    for (int pt_idx = 0; pt_idx < num_points; ++pt_idx)
    {
        if(!mesh_in.good())
            return false;
    
        std::vector<float> poly_pts (3);
        mesh_in >> x_pt >> y_pt >> z_pt;
        poly_pts[0] = x_pt;
        poly_pts[1] = y_pt;
        poly_pts[2] = z_pt;   
        polygon_points[pt_idx] = std::move(poly_pts);
    }

    //not sure what these ones are for, so skip them and the surrounding 3 lines (2 above and 1 below)...
    for (int pt_idx = 0; pt_idx < num_points + 3; ++pt_idx)
    {
        if(!mesh_in.good())
            return false;
        mesh_in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }


    //we know that we're using triangles, so there are 4 points (#vertices, vtx#1, vtx#2, vtx#3)
    int total_num_elem;
    uint32_t num_vals, x_idx, y_idx, z_idx;
    mesh_in >> meta_info >> num_points >> total_num_elem;
    assert(total_num_elem/num_points == 4);
    polygon_mesh.resize(num_points);
    for (int vertex_idx = 0; vertex_idx < num_points; ++vertex_idx)
    {
        if(!mesh_in.good())
            return false;
        //discard the 1st number
        mesh_in >> num_vals >> x_idx >> y_idx >> z_idx;
        if(num_vals != 3)
            std::cout << "houston, we have a breakpoint (hopefully) @" << vertex_idx << std::endl;

        std::vector<uint32_t> vtx_pts (3);
        vtx_pts[0] = x_idx;
        vtx_pts[1] = y_idx;
        vtx_pts[2] = z_idx;
        polygon_mesh[vertex_idx] = std::move(vtx_pts);
    }
    return true;
}

/*
 *  The View on its own is pretty dull -- a better test would be for combining the 
 *  Controller and the views
 */

template <typename PixelType>
void add_mesh(std::vector<std::vector<uint32_t>>& polygon_mesh, std::vector<std::vector<float>>& polygon_points, std::string mesh_filename = "")
{
    std::cout << "Generating Point Cloud + Mesh..." << std::endl;
    bool loaded_meshfile = false;
    loaded_meshfile = load_mesh(mesh_filename, polygon_mesh, polygon_points);
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
