#ifndef TD_TOWER_DEFENSE_HPP
#define TD_TOWER_DEFENSE_HPP

#include "GameMap.hpp"
#include "TowerLogic.hpp"
#include "util/Types.hpp"

#include <memory>
#include <random>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>


//this should be the main interface of the TD backend. Pretty uninspired name at the moment...
template <class ViewType, class ModelType = TowerLogic>
class TowerDefense
{
public:
    //it's possible that we'll move to normalized coordinates?
    explicit TowerDefense(ViewType* view)
        : td_view(view)
    {
        td_view->draw_maptiles(GameMap::MAP_WIDTH, GameMap::MAP_HEIGHT);
        td_backend = std::unique_ptr<TowerLogic>(new TowerLogic);
    }

    void init_game();
    void start_game()
    {
        //should we have a seperate thread for the gameloop? and do we need any other initialization beforehand?
        std::cout << "Starting Gameloop" << std::endl;
        
        continue_gameloop.store(true, std::memory_order_seq_cst);
        gameloop_thread = std::unique_ptr<std::thread>(new std::thread(&TowerDefense::gameloop, this));

    }
    void stop_game()
    {
        continue_gameloop.store(false, std::memory_order_seq_cst);
        std::cout << "Stopping Gameloop" << std::endl;
        gameloop_thread->join();
    }

    bool add_tower(std::vector<std::vector<uint32_t>>&& polygon_mesh, std::vector<std::vector<float>>&& polygon_points, 
                    const std::string& tower_material, const std::string& tower_name)
    { return td_backend->add_tower(std::move(polygon_mesh), std::move(polygon_points), tower_material, tower_name); }

private:

    void gameloop();
    //break out the gameloop stages
    void gloop_preprocessing();
    void gloop_processing();
    void gloop_postprocessing();

    //the frontend
    std::unique_ptr<ViewType> td_view;
    //the backend
    std::unique_ptr<TowerLogic> td_backend;
    
    std::unique_ptr<std::thread> gameloop_thread;
    std::atomic<bool> continue_gameloop;

    //frontend events
    using BuildEvtQueueType = UserTowerEvents::EventQueueType<UserTowerEvents::build_tower_event>::QType;
    using EssenceModEvt = UserTowerEvents::modify_tower_event<typename ViewType::ModifierType>;
    using ModEvtQueueType = typename UserTowerEvents::EventQueueType<EssenceModEvt>::QType;
    std::shared_ptr<BuildEvtQueueType> towerbuild_events; 
    std::shared_ptr<ModEvtQueueType> towermod_events; 

};

template <class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::init_game()
{
    //register the requisite event queues
    towerbuild_events = std::make_shared<BuildEvtQueueType> ();
    td_view->register_tower_build_queue(towerbuild_events);

    towermod_events = std::make_shared<ModEvtQueueType> ();
    td_view->register_tower_mod_queue(towermod_events);

    //...
}

template <class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gameloop()
{
    
    //the main gameloop. checks the frontend and backend, mediates communication between the two
    //applies updates, etc
    while(continue_gameloop.load())
    {
        auto start_iter_time = std::chrono::high_resolution_clock::now();

        //cycle through the 3 stages:
        //1. pre-processing stage
        //2. processing stage
        //3. post-processing stage

        gloop_preprocessing();
        gloop_processing();
        gloop_postprocessing();

        //check if we're too slow
        auto end_iter_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> time_duration (end_iter_time - start_iter_time);
        double iter_time = time_duration.count();
        if(iter_time > 33)
            std::cout << "Iteration took: " << iter_time << " ms" << std::endl;
    }
    std::cout << "Exiting GameLoop" << std::endl;

}

template <class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_preprocessing()
{
    //get any tower-build events -- note: if theyre all the same type of tower, we could just make them all in 1 go
    if(!towerbuild_events->empty())
    {
        UserTowerEvents::build_tower_event build_evt;
        while(!towerbuild_events->empty())
        {
            std::cout << "Dispatching Tower Construction @GameLoop" << std::endl;
            towerbuild_events->pop(build_evt);
            td_backend->make_tower(build_evt.tier_, build_evt.col_, build_evt.row_);
        }
    }

    if(!towermod_events->empty())
    {
        EssenceModEvt mod_evt;
        while(!towermod_events->empty())
        {
            std::cout << "Dispatching Tower Modification @GameLoop" << std::endl;
            towermod_events->pop(mod_evt);
            td_backend->modify_tower(mod_evt.modifier_, mod_evt.col_, mod_evt.row_);
        }
    }


    //TODO: also check for -- 
    //1. tower modifications
    //2. tower upgrades
    //3. user-specified tower attack targetting
    //...
}

template <class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_processing()
{
    //TODO:  
    //1. apply time-dependant tower modifiers
    //2. trigger tower attacks
    //3. update monster & attack positions
    //4. check for attack & monster collisions, apply attacks
    //...



}
    
template <class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_postprocessing()
{

}

#endif
