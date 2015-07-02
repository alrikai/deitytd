#ifndef TD_TOWER_DEFENSE_HPP
#define TD_TOWER_DEFENSE_HPP

#include "GameMap.hpp"
#include "TowerLogic.hpp"
#include "util/TDEventTypes.hpp"

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
template <template <class>  class ViewType, class ModelType = TowerLogic>
class TowerDefense
{
public:
    //it's possible that we'll move to normalized coordinates?
    explicit TowerDefense(ViewType<ModelType>* view)
        : td_view(view)
    {
        td_view->draw_maptiles(ModelType::TLIST_WIDTH, ModelType::TLIST_HEIGHT);
        td_backend = std::unique_ptr<TowerLogic>(new TowerLogic);
        
        timestamp = 0;

        std::string td_rootpath = TDHelpers::get_TD_path(); 
        std::cout << "TD rootpath: " << td_rootpath << std::endl;
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
    //aim for 30Hz 
    static constexpr double TIME_PER_ROUND = 1000.0/30.0;

    void gameloop();
    //break out the gameloop stages
    void gloop_preprocessing();
    void gloop_processing();
    void gloop_postprocessing();

    //the frontend
    std::unique_ptr<ViewType<ModelType>> td_view;
    //the backend
    std::unique_ptr<TowerLogic> td_backend;
    
    std::unique_ptr<std::thread> gameloop_thread;
    std::atomic<bool> continue_gameloop;

    using TowerEventQueueType = typename ViewType<ModelType>::TowerEventQueueType; 
    std::unique_ptr<TowerEventQueueType> td_towerevents;

    uint64_t timestamp;
};


template <template <class>  class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::init_game()
{
    //register the tower-event queues, as triggered by user input
    td_towerevents = std::unique_ptr<TowerEventQueueType> (new TowerEventQueueType());
    td_view->register_tower_eventqueue(td_towerevents.get());

    //hook up the backend and frontends, so we can send events from backend to frontend 
    td_view->register_backend_eventqueue(td_backend->get_frontend_eventqueue());

    //...
}


template <template <class>  class ViewType, class ModelType>
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

        //check if we're too slow, enforce the iteration speed to operate at a fixed timestep
        auto end_iter_time = std::chrono::high_resolution_clock::now();
        double time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_iter_time - start_iter_time).count(); 
        if(time_elapsed > TIME_PER_ROUND)
              std::cout << "Over the per-round target! -- " << time_elapsed << " ms" << std::endl;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(std::llround(std::floor(TIME_PER_ROUND - time_elapsed))));

        //NOTE: we hope to say that each timestamp is equal to 1 iter (in ms). What do we do about the rounds that are over-time however?
        //--> the backend shouldnt care, since the backend just works in terms of timestamp values (and it doesn't care about the time 
        //associated with them). The issue would arise in the frontend, where we would move based on the conversion of the backend timestamp 
        //value to ms. However, if we have all of our backend->frontend events as absolute positions (e.g. attack destination, mob destination,
        //etc). then this shouldn't produce a noticable skew, and it would be self-correcting (i.e. if we send position updates every 150ms)
        timestamp++;
    }
    std::cout << "Exiting GameLoop" << std::endl;

}

template <template <class>  class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_preprocessing()
{

    //handle the dispatching of the user-input tower events
    std::unique_ptr<UserTowerEvents::tower_event<ModelType>> td_evt (nullptr);
    while(!td_towerevents->empty())
    {
        bool got_data = false;
        td_evt = std::move(td_towerevents->pop(got_data));

        std::cout << "Got frontend event, dispatching!" << std::endl;

        //call the event functor -- since it's a pointer, I opted to use a regular 
        //function rather than operator overloading (the syntax looks bad)
        if(got_data && td_evt)
            td_evt->apply(td_backend.get());
    }

    //TODO: also check for -- 
    //1. tower modifications
    //2. tower upgrades
    //3. user-specified tower attack targetting
    //...
}

template <template <class>  class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_processing()
{
    //TODO:  
    //1. apply time-dependant tower modifiers
    //2. trigger tower attacks
    //3. update monster & attack positions
    //4. check for attack & monster collisions, apply attacks
    //...

    td_backend->cycle_update(timestamp);    

    //assume we have some list of generated tower attacks from this cycle @here
    //merge them with the existing list of attacks
    //update all the attack positions
    //update all the monster positions (and all oher moveables)
    //check for collisions

}
    
template <template <class>  class ViewType, class ModelType>
void TowerDefense<ViewType, ModelType>::gloop_postprocessing()
{

}

#endif
