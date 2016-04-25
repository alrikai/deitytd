/* TowerDefense.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef TD_TOWER_DEFENSE_HPP
#define TD_TOWER_DEFENSE_HPP

#include "GameMap.hpp"
#include "TowerLogic.hpp"
#include "util/TDEventTypes.hpp"
#include "shared/common_information.hpp"
#include "shared/Player.hpp"

#include <memory>
#include <random>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

//NOTE: Ogre by convention has all its coordinates specified as (col, row). Thus, we'll also adopt
//that convention


//this should be the main interface of the TD backend. Pretty uninspired name at the moment...
template <template <class>  class ViewType, class ModelType = TowerLogic>
class TowerDefense
{
  struct TDState;
public:
//the game states -- (at the moment), it's just in-round (mobs running, towers attacking, etc) and
//idle (between rounds, so towers building, upgrading, etc).
enum class GAME_STATE { ACTIVE, IDLE, PAUSED };


    //it's possible that we'll move to normalized coordinates?
    explicit TowerDefense(ViewType<ModelType>* view)
        : td_view(view)
    {
        //choose the word dictionary and default modifier stats 
        const std::string config_file {"resources/default_attribute_values.yaml"};
        const std::string dict_file {"resources/word_list.txt"};

		TDPlayerInformation defaultplayer_state(20, 0, 20);

        //td_view->draw_maptiles(ModelType::TLIST_WIDTH, ModelType::TLIST_HEIGHT);
        td_backend = std::unique_ptr<ModelType>(new ModelType(dict_file, config_file, defaultplayer_state));

        spawn_point = GameMap::IndexCoordinate (GameMap::MAP_WIDTH-1, GameMap::MAP_HEIGHT-1);  
        dest_point = GameMap::IndexCoordinate (0, 0); 
        timestamp = 0;

        //-----------------------------------------------------------------
        //register the shared info to be used by the front and backends
        shared_game_info = std::make_shared<game_info_t>(defaultplayer_state);
        td_backend->register_shared_info(shared_game_info);
        td_view->register_shared_info(shared_game_info);
        //-----------------------------------------------------------------

        //make the game state (default to paused, since the game logic shouldn't be started yet)
        game_state = std::unique_ptr<TDState> (new PausedState(this)); 
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
    //15 seconds to build 
    static constexpr double TIME_BETWEEN_ROUND = 1000.0*15.0;


    void gameloop();
    //break out the gameloop stages
    void gloop_preprocessing();
    void gloop_processing();
    void gloop_postprocessing();

    std::unique_ptr<TDState> game_state;

    //the frontend
    std::unique_ptr<ViewType<ModelType>> td_view;
    //the backend
    std::unique_ptr<ModelType> td_backend;

    //threadsafe and shared between the frontend and backend for faster information providing
    using game_info_t = GameInformation<CommonTowerInformation, TDPlayerInformation>;
    std::shared_ptr<game_info_t> shared_game_info;
    
    std::unique_ptr<std::thread> gameloop_thread;
    std::atomic<bool> continue_gameloop;

    using TowerEventQueueType = typename ViewType<ModelType>::TowerEventQueueType; 
    std::unique_ptr<TowerEventQueueType> td_towerevents;

    //NOTE: this is where the monsters should spawn at -- this likely(?) won't ever change during the course of the game
    //these are normalized coordinates
    GameMap::IndexCoordinate spawn_point;
    GameMap::IndexCoordinate dest_point;
    uint64_t timestamp;


//-----------------------------------------------------------------------------------------------
//Nested state machine types

struct TDState
{
  TDState(TowerDefense* towerd)
    : td(towerd)
  {}

  virtual ~TDState() 
  {}
   
  using time_pt = std::chrono::time_point<std::chrono::high_resolution_clock>;
  virtual void enter_state(GAME_STATE previous_state) = 0;
  virtual GAME_STATE cycle_update(time_pt current_timestamp) = 0;


protected:
  TowerDefense* td;

};



struct IdleState : TDState
{
  static constexpr GAME_STATE state = GAME_STATE::IDLE;
  IdleState(TowerDefense* towerd)
    : TDState(towerd)
  {}

  void enter_state(GAME_STATE previous_state) override
  {
    if(previous_state == state) {
      return;
    }
 
    initial_timestamp = std::chrono::high_resolution_clock::now();
  }

  GAME_STATE cycle_update(typename TDState::time_pt current_timestamp) override
  {
    //see how long we have been in IDLE -- if it's > the between round times, transition to ACTIVE
    double idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_timestamp - initial_timestamp).count(); 
    
    TDState::td->gloop_preprocessing();

    if (idle_time > TIME_BETWEEN_ROUND) {
      //TODO: ... do whatever other things needed before transitioning states
      std::cout << idle_time/1000 << " #sec elapsed -- transitioning to ACTIVE" << std::endl;
      return GAME_STATE::ACTIVE;
    }

    return state;
  }

private:
  typename TDState::time_pt initial_timestamp;
};

struct ActiveState : TDState
{
  static constexpr GAME_STATE state = GAME_STATE::ACTIVE;
  ActiveState(TowerDefense* towerd)
    : TDState(towerd)
  {}

  void enter_state(GAME_STATE previous_state) override
  {
    if(previous_state == state) {
      return;
    }

    //this would be the start of the (new) round. Right now (for testing) we put in a mob -- eventually we'll need to make this 
    //more extensive (and should refactor most of the logic out to somewhere else)
    if(previous_state == GAME_STATE::IDLE) {

      //spawn a monster -- since we have no game mechanics in place, it's effectivly immortal
      const auto mob_model_id = CharacterModels::ModelIDs::ogre_S;
      //nomenclature (at the moment) -- model_id + wave id
      const std::string mob_id = CharacterModels::id_names[static_cast<int>(mob_model_id)] + "_w" + std::to_string(0);
      TDState::td->td_backend->make_mob(mob_model_id, mob_id, TDState::td->spawn_point);
 
      const bool has_valid_path = TDState::td->td_backend->find_paths(TDState::td->spawn_point, TDState::td->dest_point);
      //TODO: need to notify the user that their maze is more like a wall
      if(!has_valid_path) {
        std::cout << "ERROR -- No valid path from [ " << TDState::td->spawn_point.col << ", " << TDState::td->spawn_point.row 
                  << "] to [" << TDState::td->dest_point.col << ", " << TDState::td->dest_point.row << std::endl;
      }
      
      std::cout << "Entering IDLE state" << std::endl;     
    }
  }

  GAME_STATE cycle_update(typename TDState::time_pt current_timestamp) override
  {
    //in-round, cycle through the 3 stages:
    //1. pre-processing stage
    //2. processing stage
    //3. post-processing stage
    TDState::td->gloop_preprocessing();
    TDState::td->gloop_processing();
    TDState::td->gloop_postprocessing();

    //check if we're too slow, enforce the iteration speed to operate at a fixed timestep
    auto end_iter_time = std::chrono::high_resolution_clock::now();
    double time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_iter_time - current_timestamp).count(); 
    if(time_elapsed > TIME_PER_ROUND) {
          std::cout << "Over the per-round target! -- " << time_elapsed << " ms" << std::endl;
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::llround(std::floor(TIME_PER_ROUND - time_elapsed))));
    }

    //NOTE: we hope to say that each timestamp is equal to 1 iter (in ms). What do we do about the rounds that are over-time however?
    //--> the backend shouldnt care, since the backend just works in terms of timestamp values (and it doesn't care about the time 
    //associated with them). The issue would arise in the frontend, where we would move based on the conversion of the backend timestamp 
    //value to ms. However, if we have all of our backend->frontend events as absolute positions (e.g. attack destination, mob destination,
    //etc). then this shouldn't produce a noticable skew, and it would be self-correcting (i.e. if we send position updates every 150ms)
    TDState::td->timestamp++;

    //if no more mobs alive, transition states
    if(TDState::td->td_backend->get_num_live_mobs() == 0) {
        std::cout << "No more mobs, transitioning to IDLE" << std::endl;
        TDState::td->td_backend->reset_state();
        return GAME_STATE::IDLE;
    } else {
        return state;
    }
  }
};

struct PausedState : TDState
{
  static constexpr GAME_STATE state = GAME_STATE::PAUSED;
  PausedState(TowerDefense* towerd)
    : TDState(towerd)
  {}

  void enter_state(GAME_STATE previous_state) override
  {
    if(previous_state == state) {
      return;
    }
  }

  GAME_STATE cycle_update(typename TDState::time_pt current_timestamp) override
  {
    return state;
  }
};



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
    //start the loop off in idle
    game_state = std::unique_ptr<TDState> (new IdleState(this)); 
    game_state->enter_state(GAME_STATE::PAUSED);
    GAME_STATE current_state = GAME_STATE::IDLE;

    //TODO: have a timer to keep track of how long it has been in the current state, and transition accordingly
    //at the game state transitions, we need to do certain things (i.e. at the transition from IDLE --> INROUND, 
    //we need to spawn the mobs, calculate the pathfinding, etc)
  
    //the main gameloop. checks the frontend and backend, mediates communication between the two
    //applies updates, etc
    while(continue_gameloop.load())
    {
        auto start_iter_time = std::chrono::high_resolution_clock::now();
        auto next_state = game_state->cycle_update(start_iter_time);
        
        //TODO: need to have a better state transitioning system
        if(next_state != current_state) {
          if(next_state == GAME_STATE::IDLE) {
            game_state = std::unique_ptr<TDState> (new IdleState(this)); 
          }

          if(next_state == GAME_STATE::ACTIVE) {
            game_state = std::unique_ptr<TDState> (new ActiveState(this)); 
          }

          game_state->enter_state(current_state);
          current_state = next_state; 
        }

        //std::cout << "Game state: " << current_state << std::endl;
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
        if(got_data && td_evt) {
            td_evt->apply(td_backend.get());
        }
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
	//write a copy of the player's current state to the shared state
	auto player_state_snapshot = td_backend->get_player_state();
	shared_game_info->set_player_state_snapshot(std::move(player_state_snapshot));
}

#endif
