/* pcl_mesh.hpp -- part of the DietyTD tests 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef PCL_MESH_HPP
#define PCL_MESH_HPP

#include "fractal3d.hpp"

#include "opencv2/opencv.hpp"

#include <pcl/common/common.h>
#include <pcl/point_types.h>

#include <pcl/features/normal_3d.h>
//#include <pcl/surface/mls.h>
#include <pcl/surface/poisson.h>
#include <pcl/conversions.h>

#include <iostream>
#include <string>
#include <vector>

namespace MeshMaker
{

union ToFloat 
{
    float fval;
    uint8_t cloudval [sizeof(float)];
};

template <typename PixelType>
std::tuple<std::vector<std::vector<uint32_t>>, std::vector<std::vector<float>>> make_mesh()
{
	const int imdepth = 64;
	const int imheight = 64;
	const int imwidth = 64;

    pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud (new pcl::PointCloud<pcl::PointXYZ>()); 
    fractal_meshmaker::generate_mesh(pt_cloud, imheight, imwidth, imdepth);

    //in theory, we have a (rectangular) point cloud at this point 
    //from here, we want to:
    //1. smooth the point cloud 
    //2. estimate the normals
    //3. perform reconstruction

    /*
    using SmootherType = pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointXYZ>;
    SmootherType mls;
    mls.setInputCloud(pt_cloud);
    mls.setSearchRadius(0.01);
    mls.setPolynomialFit(true);
    mls.setPolynomialOrder(2);
    mls.setUpsamplingMethod(SmootherType::SAMPLE_LOCAL_PLANE);
    mls.setUpsamplingRadius(0.005);
    mls.setUpsamplingStepSize(0.003);
    pcl::PointCloud<pcl::PointXYZ>::Ptr smooth_pt_cloud (new pcl::PointCloud<pcl::PointXYZ>()); 
    mls.process(*smooth_pt_cloud);
    */

    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
    ne.setInputCloud(pt_cloud);

    //pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ> ());
    //ne.setSearchMethod (tree);

    ne.setRadiusSearch (2);

    Eigen::Vector4f centroid;
    pcl::compute3DCentroid (*pt_cloud, centroid);
    ne.setViewPoint(centroid[0], centroid[1], centroid[2]);

    pcl::PointCloud<pcl::Normal>::Ptr cloud_normals (new pcl::PointCloud<pcl::Normal>);
    ne.compute (*cloud_normals);

    std::for_each(cloud_normals->begin(), cloud_normals->end(), []
            (pcl::Normal& pt)
            {
              pt.normal_x *= -1;
              pt.normal_y *= -1;
              pt.normal_z *= -1;
            });

    std::cout << "#cloud normals: " << cloud_normals->size() << std::endl;
    
    pcl::PointCloud<pcl::PointNormal>::Ptr pt_cloud_with_normals (new pcl::PointCloud<pcl::PointNormal>()); 
    pcl::concatenateFields (*pt_cloud, *cloud_normals, *pt_cloud_with_normals);

    pcl::Poisson<pcl::PointNormal> poisson;
    poisson.setDepth(12);
    poisson.setInputCloud(pt_cloud_with_normals);
    pcl::PolygonMesh mesh;
    poisson.reconstruct(mesh);

//    pcl::io::saveVTKFile ("mesh.vtk", mesh);

    /*
    pcl::visualization::PCLVisualizer viewer("Mesh View"); 
    viewer.addPolygonMesh(mesh);
    while(!viewer.wasStopped()) {
       viewer.spinOnce(); 
    }
    */


    //convert to a format easier to work with in Ogre. Need to figure out a bett way to go about this
    auto mesh_polygons = mesh.polygons;
    std::vector<std::vector<uint32_t>> polymesh;
    std::vector<std::vector<float>> pointcloud;

    auto fields = mesh.cloud.fields;
    std::cout << fields.size() << " #vertices per PointField -- " << std::endl;
    for (int i = 0; i < fields.size(); ++i)
    {
        std::cout << "@ " << i << " -- name: " << fields.at(i).name 
                  << " -- offset: " << fields.at(i).offset 
                  << " -- datatype: " << (unsigned int) fields.at(i).datatype 
                  << " -- count: " << fields.at(i).count << std::endl;
    }

    pcl::PointCloud<pcl::PointXYZ>::Ptr outpt_cloud (new pcl::PointCloud<pcl::PointXYZ>()); 
    pcl::fromPCLPointCloud2 (mesh.cloud, *outpt_cloud);

    for (auto vertex : mesh_polygons)
    {
        polymesh.push_back(vertex.vertices);
    }

    std::cout << "final ptcloud -- " << outpt_cloud->size() << " #pts " << std::endl;
    for (auto vertex_pt : *outpt_cloud)
    {
        std::vector<float> vertex {vertex_pt.x, vertex_pt.y, vertex_pt.z};
        pointcloud.push_back(vertex);
    }

    return std::make_tuple(polymesh, pointcloud);
}

} //namespace MeshMaker
#endif
