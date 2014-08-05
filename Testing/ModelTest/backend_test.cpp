#include "Model/TowerDefense.hpp"
#include "util/Types.hpp"
#include "../fractal_util.hpp"

#include <boost/algorithm/string.hpp>

#include <memory>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>


/*
 *  The idea here is to test the Model/backend without the frontend
 *  involved. We will generate events akin to how the frontend will,
 *  then see how the backend does with them. We want to test:
 *
 *  1. Tower creation
 */

//mock-up of the frontend
struct FrontStub
{
    using ModifierType = essence;
    using EssenceModEvt = UserTowerEvents::modify_tower_event<ModifierType>;
    using ModEvtQueueType = typename UserTowerEvents::EventQueueType<EssenceModEvt>::QType;      

    void draw_maptiles(const int width, const int height) {}
    void register_tower_build_queue(std::shared_ptr<UserTowerEvents::EventQueueType<UserTowerEvents::build_tower_event>::QType> build_queue)
    {
        tbuild_queue = build_queue; 
    }

    void register_tower_mod_queue(std::shared_ptr<ModEvtQueueType> modify_queue)
    {
        tmod_queue = modify_queue; 
    }

    std::shared_ptr<UserTowerEvents::EventQueueType<UserTowerEvents::build_tower_event>::QType> tbuild_queue;
    std::shared_ptr<ModEvtQueueType> tmod_queue;

};

template <typename TDType>
void add_testtower(TDType& td)
{
    using PixelType = uint8_t;
    std::string mesh_filename {"/home/alrik/TowerDefense/build/meshfractal3d.vtk"};

    std::vector<std::vector<uint32_t>> polygon_mesh;
    std::vector<std::vector<float>> polygon_points;
    views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

    const std::string tower_material {"Examples/Chrome"};
    const std::string tower_name {"ViewTest"};
    td.add_tower(std::move(polygon_mesh), std::move(polygon_points), tower_material, tower_name);
}

int main()
{ 
    using TDType = TowerDefense<FrontStub>;
    FrontStub* view = new FrontStub();
    TDType td(view);
    //add a tower (assume this to be the fundamental tower)
    td.init_game();
    add_testtower(td);
   
    //spawns its own thread
    td.start_game();
    std::random_device rd;
    std::mt19937 gen(rd());

    //now, mock-up some user inputs -- first, tower creation

    std::string user_input {""};
    bool continue_running = true;
    while(continue_running)
    {
    auto start_wave_time = std::chrono::high_resolution_clock::now();

        std::getline(std::cin, user_input);
        if(!user_input.empty())
        {
            std::vector<std::string> input_tokens;
            boost::split(input_tokens, user_input, boost::is_any_of(" "));

            int token_cnt = 0;
            if(input_tokens.at(token_cnt) == "q")
            {
                std::cout << "quitting input loop... " << std::endl;
                continue_running = false;
                break;
            }
            else if(input_tokens.at(token_cnt) == "bt")
            {
                token_cnt += 1;
                assert(input_tokens.size() > 3);
                int tier = std::stoi(input_tokens.at(token_cnt++));
                float t_row = std::stof(input_tokens.at(token_cnt++));
                float t_col = std::stof(input_tokens.at(token_cnt++));

                UserTowerEvents::build_tower_event t_evt(tier, t_row, t_col);
                view->tbuild_queue->push(t_evt);
                std::cout << "Building a Tower..." << std::endl;
            }
            else if(input_tokens.at(token_cnt) == "mt")
            {
                token_cnt += 1;
                assert(input_tokens.size() > 3);

                std::string essence_name = input_tokens.at(token_cnt++);
 
                //based on the index, make the appropriate essence type
                FrontStub::ModifierType* e_type = nullptr;
                if(essence_name == "aph")
                    e_type = new aphrodite();
                else if(essence_name == "apo")
                    e_type = new apollo();                
                else if(essence_name == "are")
                    e_type = new ares();
                else if(essence_name == "art")
                    e_type = new artemis();
                else if(essence_name == "ath")
                    e_type = new athena();
                else if(essence_name == "dem")
                    e_type = new demeter();
                else if(essence_name == "dio")
                    e_type = new dionysus();
                else if(essence_name == "had")
                    e_type = new hades();
                else if(essence_name == "hep")
                    e_type = new hephaestus();
                else if(essence_name == "her")
                    e_type = new hera();
                else if(essence_name == "herm")
                    e_type = new hermes();
                else if(essence_name == "hes")
                    e_type = new hestia();
                else if(essence_name == "pos")
                    e_type = new poseidon();
                else if(essence_name == "zeu")
                    e_type = new zeus();
                else
                {
                    std::cout << "NOTE: invalid essence specified -- valid names are the following: " << std::endl <<
                        "aph, apo, are, art, ath, dem, dio, had, hep, her, herm, hes, pos, zeu" << std::endl;
                    continue;
                }

                float t_row = std::stof(input_tokens.at(token_cnt++));
                float t_col = std::stof(input_tokens.at(token_cnt++));

                std::cout << "Modifying a Tower..." << std::endl;
                UserTowerEvents::modify_tower_event<FrontStub::ModifierType> t_evt(e_type, t_row, t_col);
                view->tmod_queue->push(t_evt);
            }
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
    auto end_wave_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_duration (end_wave_time - start_wave_time);
    }

    td.stop_game();

}
