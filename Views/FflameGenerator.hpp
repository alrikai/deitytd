#ifndef TD_FFLAME_GENERATOR_HPP
#define TD_FFLAME_GENERATOR_HPP

#include "FractalFlames/ifs.hpp"
#include "FractalFlames/ifs_types.hpp"
#include "FractalFlames/fractal_flame.hpp"
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

template <typename data_t, typename pixel_t>
class fflame_generator
{
public:
    fflame_generator (const int imheight, const int imwidth, const int num_workers = std::thread::hardware_concurrency())
        : num_workers(num_workers), imheight(imheight), imwidth(imwidth), fflame_histoqueue(100, 30),  
         flame_prebarrier(num_workers), flame_postbarrier(num_workers), fflame_th(nullptr), fflame_histdata(nullptr),
         total_variant_rng (0, affine_fcns::variant_list<data_t>::variant_names.size()-1)
    {
        //NOTE: this has to be shared between all the worker threads
        fflame_histdata = std::unique_ptr<fflame_data<data_t, pixel_t>>(new fflame_data<data_t, pixel_t>());        
        
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
        std::vector<std::shared_ptr<affine_fcns::variant<data_t>>> working_variants(num_working_variants); 
        for (int i = 0; i < num_working_variants; ++i) 
        {
            auto selected_variant = affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
            working_variants.at(i) = std::shared_ptr<affine_fcns::variant<data_t>>(variant_maker.flame_maker.create_variant(selected_variant));
        } 
    
        flamer = std::unique_ptr<affine_fcns::invoker<data_t>> (new affine_fcns::invoker<data_t>(std::move(working_variants)));
        flamer->randomize_parameters(-2, 2);
    }
    
    void start_fflame_generation();
    void generate_fflame(fflame_randutil::fast_rand& rand_gen);
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

    EventQueue<std::vector<histogram_info<pixel_t>>> fflame_histoqueue;
    //for holding the final images. provided by the caller
    EventQueue<uint8_t[]>* fflame_imagequeue; 

    barrier flame_prebarrier;
    barrier flame_postbarrier;

    //the rendering/controller flame thread
    std::unique_ptr<std::thread> fflame_th;
    //the fractal flame generation threads
    std::vector<std::unique_ptr<flame_thread>> fflame_workers;

    //flag for starting/stopping the whole fflame pipeline
    std::atomic<bool> fflame_state;

    //holds the list of the current variation functions to use
    std::unique_ptr<affine_fcns::invoker<data_t>> flamer; 
    affine_fcns::variant_list<data_t> variant_maker;
    std::unique_ptr<fflame_data<data_t, pixel_t>> fflame_histdata;

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
        fflame_workers.emplace_back(new flame_thread(flame_dist(flame_gen), flame_dist(flame_gen)));
        //this step is the one that actually spawns the thread
        bool flame_run;
        std::thread::id tid;
        std::tie(flame_run, tid) = fflame_workers.at(th_idx)->do_flame(&fflame_generator::generate_fflame, this);
        if(!flame_run)
            std::cout << "NOTE: flame task failed" << std::endl;

        if(th_idx == 0)
            worker_overloard_id = tid;
    }
    
    render_fflame();
}

template <typename pixel_t>
histogram_info<pixel_t> print_mat(const cv::Mat_<histogram_info<pixel_t>>& hdata, int row, int col)
{
    return hdata(row, col);
}

//makes the fractal flame histogram using the chaos game. Is invoked by N threads
template <typename data_t, typename pixel_t>
void fflame_generator<data_t, pixel_t>::generate_fflame(fflame_randutil::fast_rand& rand_gen)
{

    while(fflame_state.load())
    {
        //0. check if the generation should pause (i.e. too many frames in queue) --> leave this for after you get it working
        
        //1. start the next flame generation 
        run_fflame<data_t, pixel_t>(flamer.get(), fflame_constants::num_samples/num_workers, fflame_histdata.get(), rand_gen);
        
        //2. wait for all the threads to finish the previous round. have 1 thread execute the following steps:
        std::cout << "TID START " << std::this_thread::get_id() << std::endl;
        flame_prebarrier.wait();
        if(std::this_thread::get_id() == worker_overloard_id)
        {
            std::cout << "TID @ " << std::this_thread::get_id() << " @semaphore section " << std::endl;
            //somewhat unfortunate, but need to have this on the heap to avoid scoping problems
            auto hist_info = std::unique_ptr<std::vector<histogram_info<pixel_t>>>(new std::vector<histogram_info<pixel_t>>(imheight * imwidth));
            fflame_histdata->get_and_reset(*hist_info);

            //3.5 pass the finished histogram to the shared-buffer for rendering
            fflame_histoqueue.push(std::move(hist_info));

            //4. mutate the variants
            auto selected_variant = affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
            //replace a random variant (that's not the linear variant)
            int mod_idx = total_variant_rng(flame_gen) % num_working_variants;
            flamer->fcn.at(mod_idx).reset(variant_maker.flame_maker.create_variant(selected_variant)); 
            flamer->randomize_parameters(-2, 2);

            flame_prebarrier.reset(num_workers);
        }
        flame_postbarrier.wait();
        
        std::cout << "TID END " << std::this_thread::get_id() << std::endl;
        if(std::this_thread::get_id() == worker_overloard_id)
            flame_postbarrier.reset(num_workers);

    }
}


//generates an image based on the fflame histogram
template <typename data_t, typename pixel_t>
void fflame_generator<data_t, pixel_t>::render_fflame()
{
    bool got_histdata = false;
    cv::Mat_<pixel_t> image;

    while(fflame_state.load())
    {
        //1. get the histogram
        auto hist_info = fflame_histoqueue.pop(got_histdata);
        if(got_histdata && hist_info)
        {
            std::cout << "Got histogram to render... -- #elem: " << hist_info->size() << std::endl;
            //2. call the rendering routine, get resultant image
            image = cv::Mat_<pixel_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);
            render_fractal_flame<data_t, pixel_t>(image, std::move(hist_info));

            //I think we need to normalize the pixels for Ogre3d to show them
            cv::Mat outfile_image;
            image.convertTo(outfile_image, CV_8UC3); 

            //get the raw image data from the rendered image
            std::unique_ptr<uint8_t []> im_data = std::unique_ptr<uint8_t[]>(new uint8_t [3 * fflame_constants::imheight * fflame_constants::imwidth]);
            int im_data_idx = 0;
            auto px_it = outfile_image.begin<cv::Vec<uint8_t, 3>>();
            while(px_it != outfile_image.end<cv::Vec<uint8_t, 3>>())
            {
                im_data[im_data_idx++] = (*px_it)[0];
                im_data[im_data_idx++] = (*px_it)[1];
                im_data[im_data_idx++] = (*px_it)[2];
                px_it++;
            }
            fflame_imagequeue->push(std::move(im_data));
            std::cout << "Added rendered image" << std::endl;
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
