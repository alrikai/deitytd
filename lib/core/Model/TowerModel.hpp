/* TowerModel.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_TOWER_MODEL_HPP
#define TD_TOWER_MODEL_HPP

#include <string>
#include <vector>

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

// holds the info needed for a tower model
struct TowerModel {
  TowerModel(std::vector<std::vector<uint32_t>> &&polygon_mesh,
             std::vector<std::vector<float>> &&polygon_points,
             const std::string &material_name)
      : polygon_mesh_(polygon_mesh), polygon_points_(polygon_points),
        tower_material_name_(material_name) {}

  std::vector<std::vector<uint32_t>> polygon_mesh_;
  std::vector<std::vector<float>> polygon_points_;
  std::string tower_material_name_;
};

namespace TowerModelUtil {
// something quick and dirty to load VTK mesh files (shouldnt this already exist
// somewhere?)
template <typename MeshType = std::vector<std::vector<uint32_t>>,
          typename PointType = std::vector<std::vector<float>>>
bool load_mesh(const std::string &mesh_filename, MeshType &polygon_mesh,
               PointType &polygon_points) {
  std::ifstream mesh_in;
  mesh_in.open(mesh_filename, std::ifstream::in);
  if (!mesh_in.is_open()) {
    std::cout << "couldn't open file " << mesh_filename << std::endl;
    return false;
  }

  // 5 lines of header info -- skip first 4
  for (int i = 0; i < 4; ++i)
    mesh_in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  std::string meta_info;
  int num_points;
  mesh_in >> meta_info >> num_points >> meta_info;

  polygon_points.resize(num_points);
  float x_pt, y_pt, z_pt;
  for (int pt_idx = 0; pt_idx < num_points; ++pt_idx) {
    if (!mesh_in.good())
      return false;

    std::vector<float> poly_pts(3);
    mesh_in >> x_pt >> y_pt >> z_pt;
    poly_pts[0] = x_pt;
    poly_pts[1] = y_pt;
    poly_pts[2] = z_pt;
    polygon_points[pt_idx] = std::move(poly_pts);
  }

  // not sure what these ones are for, so skip them and the surrounding 3 lines
  // (2 above and 1 below)...
  for (int pt_idx = 0; pt_idx < num_points + 3; ++pt_idx) {
    if (!mesh_in.good())
      return false;
    mesh_in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  // we know that we're using triangles, so there are 4 points (#vertices,
  // vtx#1, vtx#2, vtx#3)
  int total_num_elem;
  uint32_t num_vals, x_idx, y_idx, z_idx;
  mesh_in >> meta_info >> num_points >> total_num_elem;
  assert(total_num_elem / num_points == 4);
  polygon_mesh.resize(num_points);
  for (int vertex_idx = 0; vertex_idx < num_points; ++vertex_idx) {
    if (!mesh_in.good())
      return false;
    // discard the 1st number
    mesh_in >> num_vals >> x_idx >> y_idx >> z_idx;
    if (num_vals != 3)
      std::cout << "houston, we have a breakpoint (hopefully) @" << vertex_idx
                << std::endl;

    std::vector<uint32_t> vtx_pts(3);
    vtx_pts[0] = x_idx;
    vtx_pts[1] = y_idx;
    vtx_pts[2] = z_idx;
    polygon_mesh[vertex_idx] = std::move(vtx_pts);
  }
  return true;
}

} // namespace TowerModelUtil

#endif
