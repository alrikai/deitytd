#include "ifs.hpp"
#include "ifs_types.hpp"
#include "fractal_flame.hpp"

#include <opencv2/opencv.hpp>

#include <random>
#include <string>
#include <vector>


/*
 * sort of a test driver for the fractal flames, independant of the views. Helps locate if 
 * any errors are in the fractal flame code or in the TD code
 */
int main(int argc, char* argv[])
{
    static constexpr uint8_t num_frames = 30;
    static constexpr int imheight = 1024;
    static constexpr int imwidth = 1024;


    uint8_t num_working_variants = 5;

    using data_t = double;
    using pixel_t = cv::Vec<data_t, 3>;
    
    std::random_device flame_rd;
    std::mt19937 flame_gen;
    std::uniform_int_distribution<uint64_t> flame_dist (0, std::numeric_limits<uint64_t>::max());
    fflame_randutil::fast_rand rand_gen(flame_dist(flame_gen), flame_dist(flame_gen));

    if(argc > 1)
    {
        std::vector<std::string> user_variants(&argv[1], &argv[1]+argc-1);
        std::cout << user_variants.size() << " cmdline args:" << std::endl;
        for (auto s: user_variants)
            std::cout << s << "\t";
        std::cout << std::endl;

        //run with the input arguments
        affine_fcns::variant_list<data_t> variant_maker;

        num_working_variants = user_variants.size();
        std::vector<std::shared_ptr<affine_fcns::variant<data_t>>> working_variants(num_working_variants); 
        std::vector<std::string> selected_variants;
        for (int i = 0; i < num_working_variants; ++i)
        {
            const std::string input_variant = user_variants.at(i);
            //double check that input argument is in the list of variants
            auto variant_it = std::find_if(affine_fcns::variant_list<data_t>::variant_names.begin(), 
                                           affine_fcns::variant_list<data_t>::variant_names.end(),
                                           [input_variant](const std::string& variant_name)
                                           {
                                               return input_variant == variant_name;
                                           });

            //user input variant is correct, use it in the working variants
            if(variant_it != affine_fcns::variant_list<data_t>::variant_names.end())
            {
                working_variants.at(i) = std::shared_ptr<affine_fcns::variant<data_t>>(variant_maker.flame_maker.create_variant(input_variant));
                selected_variants.emplace_back(input_variant);
            }
            else
                std::cout << "input variant " << input_variant << " is not a valid variant" << std::endl;
        }

        auto flamer = std::unique_ptr<affine_fcns::invoker<data_t>> (new affine_fcns::invoker<data_t>(std::move(working_variants)));
        flamer->randomize_parameters(-2, 2);
        cv::Mat_<pixel_t> image;
        auto fflame_histdata = std::unique_ptr<fflame_data<data_t, pixel_t>>(new fflame_data<data_t, pixel_t>());;
        run_fflame<data_t, pixel_t>(flamer.get(), fflame_constants::num_samples, fflame_histdata.get(), rand_gen);

        auto hist_info = std::unique_ptr<std::vector<histogram_info<pixel_t>>>(new std::vector<histogram_info<pixel_t>>(imheight * imwidth));
        fflame_histdata->get_and_reset(*hist_info); 
    
        image = cv::Mat_<pixel_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);
        render_fractal_flame<data_t, pixel_t>(image, std::move(hist_info));        

        std::string outfile_name = "ff_";
        for (auto vstr : selected_variants)
            outfile_name += vstr + "_";
        std::string outname = outfile_name + ".png";
        cv::imwrite(outname, image);

        //experiment with clamping the image to a certain pixel range
        cv::Mat outfile_image;
        //image.convertTo(outfile_image, CV_32FC3);
        image.convertTo(outfile_image, CV_8UC3);
        int non_zero_px = 0;
        for (int i = 0; i < image.rows; ++i)
            for (int j = 0; j < image.cols; ++j)
                for (int k = 0; k < 3; ++k)
                    if(image(i,j)[k] > 1e-3)
                        non_zero_px++;
        std::cout << "Has " << non_zero_px << " # non-zero pixels" << std::endl;
        
        std::string norm_outfile_name = outfile_name + "_norm.png";
        cv::imwrite(norm_outfile_name, outfile_image);

        return 0;
    }

    

    std::uniform_int_distribution<> total_variant_rng (0, affine_fcns::variant_list<data_t>::variant_names.size()-1);

    //for debugging -- keep track of the variant names
    std::vector<std::string> current_variants (num_working_variants);

    affine_fcns::variant_list<data_t> variant_maker;
    std::vector<std::shared_ptr<affine_fcns::variant<data_t>>> working_variants(num_working_variants); 
    for (int i = 0; i < num_working_variants; ++i) 
    {
        auto selected_variant = affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
        working_variants.at(i) = std::shared_ptr<affine_fcns::variant<data_t>>(variant_maker.flame_maker.create_variant(selected_variant));
        current_variants.at(i) = selected_variant;
    } 

    auto flamer = std::unique_ptr<affine_fcns::invoker<data_t>> (new affine_fcns::invoker<data_t>(std::move(working_variants)));
    flamer->randomize_parameters(-2, 2);


    cv::Mat_<pixel_t> image;
    auto fflame_histdata = std::unique_ptr<fflame_data<data_t, pixel_t>>(new fflame_data<data_t, pixel_t>());;
    
    for (int frame_idx = 0; frame_idx < num_frames; ++frame_idx)
    {    
        run_fflame<data_t, pixel_t>(flamer.get(), fflame_constants::num_samples, fflame_histdata.get(), rand_gen);

        auto hist_info = std::unique_ptr<std::vector<histogram_info<pixel_t>>>(new std::vector<histogram_info<pixel_t>>(imheight * imwidth));
        fflame_histdata->get_and_reset(*hist_info); 
    
        image = cv::Mat_<pixel_t>::zeros(fflame_constants::imheight, fflame_constants::imwidth);
        render_fractal_flame<data_t, pixel_t>(image, std::move(hist_info));        

        //4. mutate the variants
        auto selected_variant = affine_fcns::variant_list<data_t>::variant_names[total_variant_rng(flame_gen)];
        //replace a random variant (that's not the linear variant)                                                                                                                                                                     
        int mod_idx = total_variant_rng(flame_gen) % num_working_variants;
        flamer->fcn.at(mod_idx).reset(variant_maker.flame_maker.create_variant(selected_variant));
        flamer->randomize_parameters(-2, 2);


        //print the variants
        std::cout << "Frame " << frame_idx << " variants: " << std::endl << "{" << std::endl;
        for (auto v_str : current_variants)
            std::cout << v_str << "\t" << std::endl;
        std::cout << "}" << std::endl;
        current_variants.at(mod_idx) = selected_variant;

        std::string outfile_name = "ffout_" + std::to_string(frame_idx) + ".png";
        cv::imwrite(outfile_name, image);
    }


}
