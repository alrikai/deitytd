#include "Model/util/Types.hpp"

#include <CL/cl.hpp>
#include <opencv2/opencv.hpp>

//#include <pcl/common/common.h>
//#include <pcl/point_types.h>

#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <tuple>
#include <complex>
#include <memory>
#include <array>
#include <chrono>

namespace fractal_meshmaker
{
namespace cpu_fractals
{

template <typename T>
struct PixelPoint
{
    PixelPoint(T y, T x, T z)
        : row(y), col(x), depth(z)
    {}

    inline T get_magnitude() { return std::sqrt(row*row + col*col + depth*depth); }
    inline void add_point(const PixelPoint& other)
    {
        row += other.row;
        col += other.col;
        depth += other.depth;
    }

    T row;
    T col;
    T depth;
};

template <typename T>
struct FractalLimits
{
    explicit FractalLimits(const PixelPoint<size_t> dims, const T MIN = -1.2, const T MAX = 1.2)
        : DIMENSIONS(dims), MIN_LIMIT(MIN), MAX_LIMIT(MAX), LIMIT_DIFF(MAX-MIN)
    {}

    inline T offset_X(const T x_idx) { return MIN_LIMIT + x_idx * (LIMIT_DIFF / DIMENSIONS.col);   }
    inline T offset_Y(const T y_idx) { return MIN_LIMIT + y_idx * (LIMIT_DIFF / DIMENSIONS.row);   }
    inline T offset_Z(const T z_idx) { return MIN_LIMIT + z_idx * (LIMIT_DIFF / DIMENSIONS.depth); }

    const PixelPoint<size_t> DIMENSIONS;
    const T MIN_LIMIT;
    const T MAX_LIMIT;
    const T LIMIT_DIFF;
};


template <typename PixelType, size_t NUM_ITER>
std::tuple<bool, size_t> mandel_point(const PixelPoint<PixelType> px_idx, const int order) 
{
    PixelPoint<PixelType> coords (0, 0, 0);

    size_t iter_num = 0;
    bool is_valid = true;
    for (; iter_num < NUM_ITER; ++iter_num)
    {
        //get polar coordinates
        PixelType r = coords.get_magnitude();
        PixelType theta = order * std::atan2(std::sqrt(coords.row*coords.row + coords.col*coords.col), coords.depth);
        PixelType phi = order * std::atan2(coords.row, coords.col);

        assert(!std::isnan(theta));
        assert(!std::isnan(phi));

        PixelType r_factor = std::pow(r, order);
        coords.col = r_factor * std::sin(theta) * std::cos(phi);
        coords.row = r_factor * std::sin(theta) * std::sin(phi);
        coords.depth = r_factor * std::cos(theta);

        coords.add_point(px_idx);

        if(coords.get_magnitude() > 2)
        {
            is_valid = false;
            break;
        }
    }
    return std::make_tuple(is_valid, iter_num);
}

}


//note: some of these tricks require c++11 support
std::tuple<cl_uint, cl_platform_id, bool> get_platform_id(const std::string& target_platform)
{
    //get the number of available platforms
    cl_uint num_platforms;
    clGetPlatformIDs (0, nullptr, &num_platforms);
    
    //get the platform IDs
    std::vector<cl_platform_id> platform_IDs (num_platforms);
    clGetPlatformIDs(num_platforms, &platform_IDs[0], nullptr);

    //look for the target platform 
    cl_uint target_platform_ID;
    bool found_target = false;
    for(cl_uint i = 0; i < num_platforms; ++i)
    {
        size_t platform_size;
        clGetPlatformInfo(platform_IDs[i], CL_PLATFORM_PROFILE, 0, nullptr, &platform_size);
        
        std::vector<char> curr_platform(platform_size);       
        clGetPlatformInfo (platform_IDs[i], CL_PLATFORM_NAME, platform_size, &curr_platform[0], nullptr);
        
        auto platform_profile = std::string(curr_platform.begin(), curr_platform.end());
        if (platform_profile.find(target_platform) != std::string::npos) 
        {
            target_platform_ID = i;
            found_target = true;
            break;
        }
    }
    return std::make_tuple(target_platform_ID, platform_IDs[target_platform_ID], found_target);
}

bool load_kernel_file(const std::string& file_name, std::string& kernel_source)
{
    std::ifstream kernel_source_file(file_name);
    std::string str; 

    int i = 0;
    while (std::getline(kernel_source_file, str))
    {
        std::cout << "@ " << i++ << "  " << str << std::endl;
        kernel_source += str; 
    }

    return true;
}


//void generate_mesh(pcl::PointCloud<pcl::PointXYZ>::Ptr& pt_cloud, const int imheight, const int imwidth, const int imdepth, bool debug_mode = false)
void generate_mesh(std::vector<std::tuple<float, float, float>>& pt_cloud, const int imheight, const int imwidth, const int imdepth, bool debug_mode = false)
{
    std::string target_platform_id {"NVIDIA"};
    //get ONE GPU device on the target platform 
    const int num_gpu = 1;
 
    cl_uint ocl_platform;
    bool platform_present;
    cl_platform_id ocl_platform_id;

    std::cout << "Finding platform " << target_platform_id << std::endl;
    std::tie(ocl_platform, ocl_platform_id, platform_present) = get_platform_id(target_platform_id);

    if(platform_present)
        std::cout << target_platform_id << " platform at index " << ocl_platform << std::endl;

    //5 things for every opencl host-side program:
    //1. cl_device_id
    //2. cl_command_queue
    //3. cl_context
    //4. cl_program
    //5. cl_kernel
    cl_device_id device_id;
    clGetDeviceIDs(ocl_platform_id, CL_DEVICE_TYPE_GPU, num_gpu, &device_id, nullptr);
    std::cout << "GPU device id: " << device_id << std::endl; 
    
    cl_int ocl_error_num;
    //create an opencl context
    cl_context ocl_context = clCreateContext(nullptr, num_gpu, &device_id, nullptr, nullptr, nullptr); 
    // Create a command queue for the device in the context
    cl_command_queue ocl_command_queue = clCreateCommandQueue(ocl_context, device_id, 0, nullptr);

    const std::string file_name {  TDHelpers::get_basepath() + "/Testing/ocl_fractal.cl" };
    std::string program_source;
    load_kernel_file(file_name, program_source);

    //create + compile the opencl program
    auto kernel_source_code = program_source.c_str();
    cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, (const char**) &kernel_source_code, 0, &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM CREATION -- " << ocl_error_num << std::endl;

    ocl_error_num = clBuildProgram(ocl_program, 0, 0, 0, 0, 0);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM BUILD -- " << ocl_error_num << std::endl;

    if (ocl_error_num == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(ocl_program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        // Allocate memory for the log
        char *log = (char *) malloc(log_size);
        // Get the log
        clGetProgramBuildInfo(ocl_program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        // Print the log
        printf("%s\n", log);
    }

    // Create kernel instance
    cl_kernel ocl_kernel = clCreateKernel(ocl_program, "fractal3d", &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ KERNEL CREATION -- " << ocl_error_num << std::endl;

    const cl_float MIN_LIMIT = -1.2f;
    const cl_float MAX_LIMIT =  1.2f;

    const cl_float BOUNDARY_VAL = 2.0f;
    const cl_int MAX_ITER = 80;
    const cl_int ORDER = 8;

    using DataType = int;
    //NOTE: need to dynamically allocate, as the memory requirements become prohibitive very fast (e.g. 512 x 512 x 512 of ints --> 4*2^27 bytes)
    std::vector<DataType> h_image_stack (imheight * imwidth * imdepth);
    std::fill(h_image_stack.begin(), h_image_stack.end(), 0);
    //std::array<std::array<std::array<DataType, imdepth>, imwidth>, imheight> h_image_stack {};
   
    cl_mem dev_image;
    dev_image = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY,
            imheight * imwidth * sizeof(cl_int), nullptr, 0);
   
    const cl_int3 dimensions = {{ imheight, imwidth, imdepth }};
    const cl_int2 constants = {{ MAX_ITER, ORDER }};
    const cl_float3 flt_constants = {{MIN_LIMIT, MAX_LIMIT, BOUNDARY_VAL}};
    const cl_uint work_dims = 2;
    const size_t global_kernel_dims [work_dims] = {imheight, imwidth};  
 
    auto start = std::chrono::high_resolution_clock::now();

    clSetKernelArg(ocl_kernel, 2, sizeof(cl_int3),  (void *)&dimensions);
    clSetKernelArg(ocl_kernel, 3, sizeof(cl_int2),  (void *)&constants);
    clSetKernelArg(ocl_kernel, 4, sizeof(cl_float3), (void *)&flt_constants);
    
    for (cl_int depth_idx = 0; depth_idx < imdepth; ++depth_idx)
    {
        clSetKernelArg(ocl_kernel, 0, sizeof(cl_mem),    (void *)&dev_image);
        clSetKernelArg(ocl_kernel, 1, sizeof(cl_int),    (void *)&depth_idx);

        auto ocl_error_num = clEnqueueNDRangeKernel(ocl_command_queue, ocl_kernel, work_dims, nullptr, global_kernel_dims, nullptr, 0, nullptr, nullptr);
        if(ocl_error_num != CL_SUCCESS)
            std::cout << "ERROR @ KERNEL LAUNCH -- " << ocl_error_num << " @depth " << depth_idx << std::endl;

        int h_image_stack_offset = imheight * imwidth * depth_idx;
        ocl_error_num = clEnqueueReadBuffer(ocl_command_queue, dev_image, CL_TRUE, 0, imheight*imwidth*sizeof(cl_int), &h_image_stack[h_image_stack_offset], 0, 0, 0);
        if(ocl_error_num != CL_SUCCESS)
            std::cout << "ERROR @ DATA RETRIEVE -- " << ocl_error_num << std::endl;
    }

    clReleaseKernel(ocl_kernel);
    clReleaseProgram(ocl_program);
    clReleaseMemObject(dev_image);
    clReleaseCommandQueue(ocl_command_queue);
    clReleaseContext(ocl_context);

    std::cout << "Making Point Cloud... " << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Fractal Generation Time: " << duration.count() << " ms" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    //for the cpu version
    cpu_fractals::FractalLimits<DataType> limits(cpu_fractals::PixelPoint<size_t>(imheight, imwidth, imdepth));

    for (int k = 0; k < imdepth; ++k)
    {
        auto z_point = limits.offset_Z(k);

        cv::Mat_<int> slice_diff = cv::Mat_<int>::zeros(imheight, imwidth);
        cv::Mat_<int> ocl_slice = cv::Mat_<int>::zeros(imheight, imwidth);
        cv::Mat_<int> cpu_slice = cv::Mat_<int>::zeros(imheight, imwidth);
        
        int h_image_stack_offset = imheight * imwidth * k;
        DataType* h_image_slice = &h_image_stack[h_image_stack_offset];
        for (int i = 0; i < imheight; ++i)
        {
            auto y_point = limits.offset_Y(i);
            for (int j = 0; j < imwidth; ++j)
            {
                auto fractal_itval = h_image_slice[i*imwidth+j];
                if(fractal_itval == MAX_ITER-1)
                    pt_cloud->push_back(std::make_tuple(j,i,k));

                if(debug_mode)
                {
                auto x_point = limits.offset_X(j);
                bool is_valid;
                size_t iter_num;
                std::tie(is_valid, iter_num) = cpu_fractals::mandel_point<DataType,MAX_ITER>
                    (cpu_fractals::PixelPoint<DataType>(y_point,x_point,z_point), ORDER);  
                 
                slice_diff(i,j) = iter_num - fractal_itval;  
                cpu_slice(i,j) = iter_num;
                }
                ocl_slice(i,j) = fractal_itval;
            }
        }

        if(debug_mode)
        {
        auto diff_extrema = std::minmax_element(slice_diff.begin(), slice_diff.end());
        auto min_diff = *diff_extrema.first;
        auto max_diff = *diff_extrema.second;
        if(max_diff > 1 || min_diff < -1)
            std::cout << "Slice " << k << " differed" << std::endl;

        std::string slice_diff_name = "slice_diff_" + std::to_string(k);
        cv::imwrite((slice_diff_name + ".png"), cv::abs(slice_diff));
        cv::FileStorage slice_storage((slice_diff_name + ".yml"), cv::FileStorage::WRITE);
        slice_storage << slice_diff_name << slice_diff;
        slice_storage << "ocl slice" << ocl_slice;
        slice_storage << "cpu slice" << cpu_slice;
        slice_storage.release();  
        std::string cpu_slice_name = "cpu_slice_" + std::to_string(k) + ".png";
        cv::imwrite(cpu_slice_name, cpu_slice);
        }        

        std::string ocl_slice_name = "ocl_slice_" + std::to_string(k) + ".png";
        cv::imwrite(ocl_slice_name, ocl_slice);
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Fractal Comparison Time: " << duration.count() << " ms" << std::endl;
}

}  //namespace fractal_meshmaker
