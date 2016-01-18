/* fractal_flame.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef FF_FRACTAL_FLAME_HPP
#define FF_FRACTAL_FLAME_HPP

#include "ifs.hpp"
#include "ifs_types.hpp"

#include <opencv2/opencv.hpp>

#include <random>
#include <iostream>
#include <utility>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <algorithm>

template <typename data_t, typename pixel_t>
class fflame_data
{
public:    
    //using hist_t = std::array<std::array<histogram_info<pixel_t>, fflame_constants::imwidth>, fflame_constants::imheight>;
    using hist_t = std::vector<histogram_info<pixel_t>>;

    fflame_data(int h_height = fflame_constants::imheight, int h_width = fflame_constants::imwidth)
        : hist_height(h_height), hist_width(h_width), fflame_hist(hist_t(hist_height * hist_width, histogram_info<pixel_t>())) 
    {} 

    //NOTE: no copy-constructor, since std::mutex is move-only
    fflame_data(const fflame_data<data_t, pixel_t>&) = delete;
    fflame_data<data_t, pixel_t>& operator= (const fflame_data<data_t, pixel_t>&) = delete;

    void apply_fflame_run(std::map<std::pair<size_t, size_t>, histogram_info<pixel_t>>&& hist_data)
    {
        std::lock_guard<std::mutex> lk (histdata_mtx);
        auto hist_it = hist_data.begin();
        while(hist_it != hist_data.end())
        {
            int idx = hist_it->first.first * hist_width + hist_it->first.second;
            fflame_hist[idx].update(hist_it->second);
            hist_it++;
        }
    }

    void get_and_reset(hist_t& hist_data)
    {
        std::lock_guard<std::mutex> lk (histdata_mtx);

        std::copy(fflame_hist.begin(), fflame_hist.end(), hist_data.begin());
        std::for_each(fflame_hist.begin(), fflame_hist.end(), 
                [](histogram_info<pixel_t>& hdata)
                {
                    hdata.reset();
                });
    }

private:
    int hist_height;
    int hist_width;

    std::mutex histdata_mtx;
    hist_t fflame_hist;

    //cv::Mat_<histogram_inf>>pixel_t>> fflame_hist;
};

/////////////////////////////////////////////////////////////////////////////

template <typename data_t, typename pixel_t>
void run_fflame(const affine_fcns::invoker<data_t>* const flamer, const int num_points, fflame_data<data_t, pixel_t>* fdata, fflame_randutil::fast_rand& f_rand)
{
    std::map<std::pair<size_t, size_t>, histogram_info<pixel_t>> hist_data;


    //for generating the random point to use
    std::uniform_real_distribution<> dis(fflame_constants::min_pt, fflame_constants::max_pt);

    for (int sample = 0; sample < num_points; ++sample)
    {
        flame_point<data_t> flame_pt (dis(fflame_randutil::get_engine()), dis(fflame_randutil::get_engine()));
        for (int i = 0; i < fflame_constants::max_iter; ++i)
        {
            //NOTE: if we have per-function probability weights, would have to use them here to select the function
            int fcn_idx = f_rand.xorshift128plus(flamer->fcn.size());
            flamer->invoke(fcn_idx, flame_pt);
         
            //don't store the first 20 iterations, as they'll apparently be too far from the solution
            if(i > 20)
            {
                //store the intermediate result to the histogram datastructures
                data_t rotated_x = flame_pt.x * std::cos(2 * fflame_constants::PI) - flame_pt.y * std::sin(2 * fflame_constants::PI);
                data_t rotated_y = flame_pt.x * std::sin(2 * fflame_constants::PI) + flame_pt.y * std::cos(2 * fflame_constants::PI);
                long col_idx = std::lround(fflame_constants::imwidth - ((fflame_constants::max_pt - rotated_x) / (fflame_constants::max_pt - fflame_constants::min_pt)) * fflame_constants::imwidth);
                long row_idx = std::lround(fflame_constants::imheight - ((fflame_constants::max_pt - rotated_y) / (fflame_constants::max_pt - fflame_constants::min_pt)) * fflame_constants::imheight);

                //if the point (at image dimensions) is within bounds, add 
                if((col_idx >= 0 && col_idx < fflame_constants::imwidth) && (row_idx >= 0 && row_idx < fflame_constants::imheight))
                {
                    auto hist_idx = std::make_pair(row_idx, col_idx);
                    pixel_t color (flame_pt.color[0], flame_pt.color[1], flame_pt.color[2]);
                    
                    //update the map -- add to an existing entry or generate a new one
                    auto bin_entry = hist_data.find(hist_idx); 
                    if(bin_entry != hist_data.end())
                        bin_entry->second.update(color, 1);
                    else
                        hist_data.insert(std::make_pair(hist_idx,histogram_info<pixel_t>(color, 1)));
                }
            }
        }
    }

    //apply the data gathered from this run to the overall histograms
    fdata->apply_fflame_run(std::move(hist_data));
}

template <typename data_t, typename pixel_t = cv::Vec<data_t, 3>>
void render_fractal_flame(cv::Mat_<pixel_t>& image, std::unique_ptr<std::vector<histogram_info<pixel_t>>> hist_data)
{

    cv::Mat_<pixel_t> raw_image = cv::Mat_<data_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);
    const int max_freq = (*std::max_element(hist_data->begin(), hist_data->end(), 
                [](histogram_info<pixel_t> lhs, histogram_info<pixel_t> rhs)
                { return lhs.frequency_count < rhs.frequency_count; })).frequency_count;
    //get the log of the maximum frequency count
    const data_t freq_max_log = std::log10(static_cast<data_t>(max_freq));
    
    //density estimation parameters
    const double min_est = 0.0;
    const double est_radius = 9.0;
    const double est_curve = 0.4;

    std::map<size_t, cv::Mat_<data_t>> density_est_kernels;
    cv::Mat_<uint64_t> image_density = cv::Mat_<uint64_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);

    //NOTE: we assume the histogram dimensions are multiples of the image dimensions
    const int rowpx_factor = fflame_constants::hist_height/fflame_constants::imheight;
    const int colpx_factor = fflame_constants::hist_width/fflame_constants::imwidth;

    int hist_data_idx = 0;
    //make the final output image -- NOTE: this is currently broken for any supersampling. 
    //we need to take both dimensions into account
    for (int im_row = 0; im_row < fflame_constants::imheight; ++im_row)
    {
        int hist_row = im_row*rowpx_factor;     
        for (int im_col = 0; im_col < fflame_constants::imwidth; ++im_col)
        {
            data_t freq_avg = 0;
            pixel_t color_avg = 0;
            int hist_col = im_col*colpx_factor;    
            for (int supersample_row = 0; supersample_row < rowpx_factor; ++supersample_row)
            {
                for (int supersample_col = 0; supersample_col < colpx_factor; ++supersample_col)
                {
                    auto h_data = hist_data->at(hist_data_idx++);
                    freq_avg += h_data.frequency_count;
                    color_avg += h_data.color;
                }
            }

            const auto freq_count = freq_avg;
            freq_avg /= rowpx_factor*colpx_factor;
            color_avg /= rowpx_factor*colpx_factor;

            data_t alpha = std::log10(freq_avg)/freq_max_log;
            raw_image(im_row, im_col) = 255 * color_avg * std::pow(alpha, fflame_constants::gamma_factor); 

            //next, we apply the density estimation filtering
            int kernel_width = std::lround(std::max(min_est, (est_radius / (std::pow(static_cast<double>(freq_count), est_curve)))));

            //dont do anything if we have a kernel size not-larger than 1 element
            if(kernel_width > 1)
            {
                //make sure we have an odd kernel size
                if(kernel_width % 2 == 0)
                    kernel_width++;

                //add the kernel matrix if it doesnt already exist
                auto kernel_it = density_est_kernels.find(kernel_width);
                if(kernel_it == density_est_kernels.end())
                {
                    //opencv default sigma: 0.3*((ksize-1)*0.5 - 1) + 0.8
                    cv::Mat_<data_t> kernel = cv::getGaussianKernel(kernel_width, -1);

                    cv::Mat_<data_t> kernel_2d = kernel*kernel.t();
                    //store a square 2D gaussian kernel
                    density_est_kernels.insert(std::make_pair(kernel_width, kernel_2d));
                }

                //associate the kernel to the current pixel
                image_density(im_row, im_col) = kernel_width;
            }
        }
    }

    //get rid of any NaNs
    cv::Mat_<cv::Vec<uint8_t, 3>> mask = cv::Mat(raw_image != raw_image);
    for (int r = 0; r < fflame_constants::imheight; ++r)
        for (int c = 0; c < fflame_constants::imwidth; ++c)
            for (int ch = 0; ch < 3; ++ch)
                if(mask(r,c)(ch))
                    raw_image(r,c)(ch) = 0;

    //NOTE: have to apply the density estimation afterwards, since it requires pixels following the anchor pixel
    for (int im_row = 0; im_row < fflame_constants::imheight; ++im_row)
    {
        for (int im_col = 0; im_col < fflame_constants::imwidth; ++im_col)
        {
            const int kernel_width = image_density(im_row, im_col);
            const int kernel_hwidth = std::floor(kernel_width/2);
            auto kernel_it = density_est_kernels.find(kernel_width);
            if(kernel_it != density_est_kernels.end())
            {
                auto kernel = kernel_it->second;
                int kernel_ridx = 0;
                int kernel_cidx = 0;
                //clamps out of range indices at the borders
                for (int k_row = std::max(0, im_row-kernel_hwidth); k_row < std::min(fflame_constants::imheight-1, im_row+kernel_hwidth); ++k_row, ++kernel_ridx)
                    for (int k_col = std::max(0, im_col-kernel_hwidth); k_col < std::min(fflame_constants::imwidth-1, im_col+kernel_hwidth); ++k_col, ++kernel_cidx)
                        if (kernel_ridx < kernel.rows || kernel_cidx < kernel.cols)
                            image(im_row, im_col) += raw_image(k_row, k_col) * kernel(kernel_ridx, kernel_cidx);
                        else
                            std::cout << "NOTE: out of bounds on the kernel" << std::endl;
            }
            else
                image(im_row, im_col) = raw_image(im_row, im_col);
        }
    }
}


/*
 
template <typename data_t, typename pixel_t = cv::Vec<data_t, 3>>
void generate_fractal_flame(affine_fcns::invoker<data_t>& flamer, cv::Mat_<pixel_t>& image, const int num_workers = std::thread::hardware_concurrency())
{ 
    //start timing (just for informative purposes)
    auto start_time = std::chrono::high_resolution_clock::now();
 
    //for seeding the flame thread rng 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> fast_dis(0, std::numeric_limits<uint64_t>::max());

    std::vector<std::unique_ptr<flame_thread>> fflame_tasks;
    //spawn the threads...
    for (int th_count = 0; th_count < num_workers; ++th_count)
    { 
        fflame_tasks.emplace_back(new flame_thread(fast_dis(gen), fast_dis(gen)));
        bool flame_run = fflame_tasks.at(th_count)->do_flame(&run_fflame<data_t, pixel_t>, flamer, fflame_constants::num_samples/num_workers);
        if(!flame_run)
            std::cout << "NOTE: flame task failed" << std::endl;
    }
        
    //...join the threads
    for (int th_count = 0; th_count < num_workers; ++th_count)
    {
        fflame_tasks.at(th_count)->finish_flame();
    }

    std::cout << "Done with generation, moving to image generation..." << std::endl;
    image = cv::Mat_<pixel_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);
    render_fractal_flame<data_t, pixel_t>(image);

    auto current_time = std::chrono::high_resolution_clock::now();
    double time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    std::cout << "Took " << time_elapsed << " ms" << std::endl;
}
*/

#endif


