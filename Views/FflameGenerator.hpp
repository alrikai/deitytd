/* FflameGenerator.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef TD_FFLAME_GENERATOR_HPP
#define TD_FFLAME_GENERATOR_HPP

#include "FractalFlames/ifs.hpp"
#include "FractalFlames/ifs_types.hpp"
#include "FractalFlames/fractal_flame.hpp"
#include "FractalFlames/render_flame.hpp"
#include "FractalFlames/util/ff_utils.hpp"
#include "util/EventQueue.hpp"

#include <opencv2/opencv.hpp>

#include <thread>
#include <memory>
#include <atomic>
#include <random>
#include <limits>
#include <mutex>
#include <condition_variable>
#include <string>

template <typename pixel_t>
using frame_t = FFlames::flame_frame<pixel_t>;

template <typename data_t, typename pixel_t>
class fflame_generator
{
public:
    fflame_generator (const int imheight, const int imwidth, const int num_workers = std::thread::hardware_concurrency())
        : num_workers(num_workers), imheight(imheight), imwidth(imwidth), fflame_histoqueue(100, 30),  
         flame_prebarrier(num_workers), flame_postbarrier(num_workers), fflame_th(nullptr), fflame_histdata(nullptr),
         total_variant_rng (0, FFlames::affine_fcns::variant_list<data_t>::variant_names.size()-1)
    {
        //NOTE: this has to be shared between all the worker threads
        fflame_histdata = std::make_unique<FFlames::fflame_data<data_t, pixel_t>>();        
        ff_renderer = std::make_unique<FFlames::fflame_renderer<data_t>>(imheight, imwidth);
        
        initialize_variants();
        fflame_state.store(false);
    }

    ~fflame_generator()
    {
        if(fflame_state.load())
            stop_generation();
    }

    void start_generation()
    {
        //if already started, don't try to start again
        if(fflame_state.load())
            return;

        fflame_state.store(true);    
        fflame_th = std::unique_ptr<std::thread> (new std::thread(&fflame_generator::start_fflame_generation, this)); 
    }

    void stop_generation()
    {
        if(fflame_state.load())
        {
            fflame_state.store(false);
            for (int th_idx = 0; th_idx < fflame_workers.size(); ++th_idx)
                fflame_workers.at(th_idx)->finish_flame();
            fflame_th->join();
        }
    }

    void register_framequeue(EventQueue<uint8_t[]>* queue)
    {
        fflame_imagequeue = queue;
    }

private:
    
    void initialize_variants()
    {
        //std::vector<std::shared_ptr<affine_fcns::variant<data_t>>> working_variants(num_working_variants); 
        std::vector<std::string> working_variants(num_working_variants); 
        for (int i = 0; i < num_working_variants; ++i) 
        {
            working_variants.at(i) = FFlames::affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
            //auto selected_variant = affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
            //working_variants.at(i) = std::shared_ptr<affine_fcns::variant<data_t>>(variant_maker.flame_maker.create_product(selected_variant));
        } 
    
        flamer = std::make_unique<FFlames::affine_fcns::invoker<data_t>> (std::move(working_variants));
        flamer->randomize_parameters(-2, 2);
    }
    
    void start_fflame_generation();
    void generate_fflame(FFlames::fflame_util::fast_rand& rand_gen);
    void render_fflame();

    //the number of variants to have active
    static constexpr uint8_t num_working_variants = 5;
    std::thread::id worker_overloard_id;

    //pause generation if above the max, resume if paused and below the min
    static constexpr size_t max_image_thresh = 25;
    static constexpr size_t min_image_thresh = 5;

    //controls the starting and stopping of the worker threads
    std::mutex gen_mtx;
    std::condition_variable gen_cv;

    //number of threads used for the generation (not counting rendering)
    int num_workers;
    int imheight;
    int imwidth;

    //for passing results asynchronously between the generate & render steps
    //EventQueue<cv::Mat_<histogram_info<pixel_t>>> fflame_histoqueue;

    EventQueue<std::vector<FFlames::histogram_info<pixel_t>>> fflame_histoqueue;
    //for holding the final images. provided by the caller
    EventQueue<uint8_t[]>* fflame_imagequeue; 

    FFlames::fflame_util::barrier flame_prebarrier;
    FFlames::fflame_util::barrier flame_postbarrier;

    //the rendering/controller flame thread
    std::unique_ptr<std::thread> fflame_th;
    //the fractal flame generation threads
    std::vector<std::unique_ptr<FFlames::flame_thread>> fflame_workers;

    //flag for starting/stopping the whole fflame pipeline
    std::atomic<bool> fflame_state;

    //holds the list of the current variation functions to use
    std::unique_ptr<FFlames::affine_fcns::invoker<data_t>> flamer; 
    FFlames::affine_fcns::variant_list<data_t> variant_maker;
    std::unique_ptr<FFlames::fflame_data<data_t, pixel_t>> fflame_histdata;
    std::unique_ptr<FFlames::fflame_renderer<data_t>> ff_renderer;

    //the various RNGs needed for the generation
    std::random_device flame_rd;
    std::mt19937 flame_gen;
    //std::uniform_int_distribution<uint8_t> working_variant_rng; 
    std::uniform_int_distribution<> total_variant_rng; 
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
template <typename data_t, typename pixel_t>
void fflame_generator<data_t, pixel_t>:: start_fflame_generation()
{
    fflame_state.store(true);

    std::uniform_int_distribution<uint64_t> flame_dist (0, std::numeric_limits<uint64_t>::max());         

    //spawn the worker threads
    for (int th_idx = 0; th_idx < num_workers; ++th_idx)
    {
        fflame_workers.emplace_back(new FFlames::flame_thread(flame_dist(flame_gen), flame_dist(flame_gen)));
        //this step is the one that actually spawns the thread
        bool flame_run;
        std::thread::id tid;
        std::tie(flame_run, tid) = fflame_workers.at(th_idx)->do_flame(&fflame_generator::generate_fflame, this);

        //this would be sort of a big deal. should probably throw something here
        if(!flame_run)
            std::cout << "NOTE: flame task failed" << std::endl;

        if(th_idx == 0)
            worker_overloard_id = tid;
    }
    
    render_fflame();
}

template <typename pixel_t>
FFlames::histogram_info<pixel_t> print_mat(const cv::Mat_<FFlames::histogram_info<pixel_t>>& hdata, int row, int col)
{
    return hdata(row, col);
}

//makes the fractal flame histogram using the chaos game. Is invoked by N threads
template <typename data_t, typename pixel_t>
void fflame_generator<data_t, pixel_t>::generate_fflame(FFlames::fflame_util::fast_rand& rand_gen)
{

    while(fflame_state.load())
    {
        //0. check if the generation should pause (i.e. too many frames in queue) --> leave this for after you get it working
        
        //1. start the next flame generation 
		FFlames::run_fflame<data_t, pixel_t>(flamer.get(), FFlames::fflame_constants::num_samples/num_workers, fflame_histdata.get(), rand_gen);
        
        //2. wait for all the threads to finish the previous round. have 1 thread execute the following steps:
        flame_prebarrier.wait();
        if(std::this_thread::get_id() == worker_overloard_id)
        {
            //somewhat unfortunate, but need to have this on the heap to avoid scoping problems
            auto hist_info = std::make_unique<std::vector<FFlames::histogram_info<pixel_t>>>(imheight * imwidth);
            fflame_histdata->get_and_reset(*hist_info);

            //3.5 pass the finished histogram to the shared-buffer for rendering
            fflame_histoqueue.push(std::move(hist_info));

            //4. mutate the variants
            auto selected_variant = FFlames::affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
            //replace a random variant (that's not the linear variant)
            int mod_idx = total_variant_rng(flame_gen) % num_working_variants;
            flamer->fcn.at(mod_idx).reset(variant_maker.flame_maker.create_product(selected_variant)); 
            flamer->randomize_parameters(-2, 2);

            flame_prebarrier.reset(num_workers);
        }
        flame_postbarrier.wait();
        
        if(std::this_thread::get_id() == worker_overloard_id)
            flame_postbarrier.reset(num_workers);
    }
}


//generates an image based on the fflame histogram
template <typename data_t, typename pixel_t>
void fflame_generator<data_t, pixel_t>::render_fflame()
{
    bool got_histdata = false;
    frame_t<pixel_t> image;
    int raw_counter = 0; 

    while(fflame_state.load())
    {
        //1. get the histogram
        auto hist_info = fflame_histoqueue.pop(got_histdata);
        if(got_histdata && hist_info)
        {
            //2. call the rendering routine, get resultant image
            image = frame_t<pixel_t>(FFlames::fflame_constants::imheight, FFlames::fflame_constants::imwidth, pixel_t(0,0,0));
            ff_renderer->template render<frame_t, pixel_t>(&image, std::move(hist_info));
            cv::Mat_<pixel_t> cv_wrap(image.rows, image.cols, image.data);
			cv::normalize(cv_wrap, cv_wrap, 0, 255, cv::NORM_MINMAX, -1);

			/*
            //I think we need to normalize the pixels for Ogre3d to show them
            //filter out the flames that are too sparse
            int num_nonzero = 0;
            const double image_threshold = 0.1 * imwidth * imheight;
            const double px_threshold = 1.0;
            for (int r = 0; r < imheight; ++r)
            {
                for (int c = 0; c < imwidth; ++c)
                {


                    //size_t px_sum = cv::sum(outfile_image.at<cv::Vec<uint8_t,3>>(r, c))[0];
                    auto px_sum = cv::sum(outfile_image.at<cv::Vec<uint8_t,3>>(r, c))[0];
                    if(px_sum > px_threshold)
                        num_nonzero++;
                }
            }
            if(num_nonzero < image_threshold)
            {
                std::cout << "NOTE: flame too dark" << std::endl;
                continue;
            }
			*/

            //mostly for debugging -- save the images to disk
            const std::string raw_impath = "TDraw_image_" + std::to_string(raw_counter++) + ".png";
			cv::Mat outfile_image;
            cv_wrap.convertTo(outfile_image, CV_8UC3);
            cv::imwrite(raw_impath, outfile_image);

            //get the raw image data from the rendered image
            std::unique_ptr<uint8_t []> im_data = std::unique_ptr<uint8_t[]>(new uint8_t [3 * FFlames::fflame_constants::imheight * FFlames::fflame_constants::imwidth]);
            int im_data_idx = 0;
            auto px_it = image.data;
			for (int row = 0; row < image.rows; row++) {
				for (int col = 0; col < image.cols; col++) {
					im_data[im_data_idx++] = (*px_it)[0];
					im_data[im_data_idx++] = (*px_it)[1];
					im_data[im_data_idx++] = (*px_it)[2];
					px_it++;
				}
			}
            fflame_imagequeue->push(std::move(im_data));
        }

/*
         //check if the execution should pause
        if(!fflame_state.load())
        {
            //trigger it to start generating frames again
            if(fflame_imagequeue->size() < min_image_thresh)
            {
                fflame_state.store(true);
                gen_cv.notify_all();
            }
        }
        else if(fflame_imagequeue->size() > max_image_thresh)
        {
            fflame_state.store(false);
            gen_cv.notify_all();
        }
*/        
    }
}
    
    


#endif
