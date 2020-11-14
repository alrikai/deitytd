#ifndef TD_STUB_FRONTEND_HPP
#define TD_STUB_FRONTEND_HPP

#include "shared/common_information.hpp"
#include "shared/Player.hpp"
#include "shared/Player.hpp"
#include "util/TowerProperties.hpp"
#include "util/TDEventTypes.hpp"

#include <thread>
#include <type_traits>

template <typename BackendType> struct FrontStub {
  using ModifierType = tower_properties;
  using TowerEventQueueType = typename UserTowerEvents::EventQueueType<
      UserTowerEvents::tower_event<BackendType>>::QType;
  using GameInformationType =
      GameInformation<CommonTowerInformation, TDPlayerInformation>;

  FrontStub() 
    : shared_gamestate_info(nullptr), td_event_queue(nullptr) {
      std::thread fake_frontend (&FrontStub::consume_events, this);
	  fake_frontend.detach();
  }

  void draw_maptiles(const int width, const int height) {}

  void register_tower_eventqueue(TowerEventQueueType *tevt_queue) {
    td_event_queue = tevt_queue;
  }

  void register_shared_info(std::shared_ptr<GameInformationType> s_info) {
    shared_gamestate_info = s_info;
  }

  void register_backend_eventqueue(ViewEvents *events) { game_events = events; }


  void spawn_build_tower_event(UserTowerEvents::build_tower_event<BackendType> evt) {spawn_event(evt);}

  template <typename ModifyT>
  void spawn_modify_tower_event(UserTowerEvents::modify_tower_event<ModifyT, BackendType> evt) {spawn_event(evt);}
  
  void spawn_print_tower_event(UserTowerEvents::print_tower_event<BackendType> evt) {spawn_event(evt);}
  
  void spawn_tower_target_event(UserTowerEvents::tower_target_event<BackendType> evt) {spawn_event(evt);}

private:

  template <typename T>
  void spawn_event(T&& event) {
	  auto fg_to_bg_event = std::make_unique<typename std::remove_reference<T>::type> (event);
	  td_event_queue->push(std::move(fg_to_bg_event));
  }

  void consume_events() {
	  std::cout << "starting stub frontend" << std::endl;
	  //just run forever and grab whatever events are spawned
	  while(true) {
		  if (shared_gamestate_info) {
			//shared_gamestate_info->
		  }

		  if (game_events) {
			  /*
			  while(not td_event_queue->empty()) {
				  bool got_data = false;
				  auto dtd_evt = td_event_queue->pop(got_data);
				  if (got_data) {
					  std::cout << "backend --> frontend event: " << *(dtd_evt.get()) << std::endl;
				  }
			  }
			  */
			  //just grab and discard any events from the backend --> frontend
			  auto noop_fn = [](auto val){(void)val;};
			  game_events->apply_towerbuild_events(noop_fn);
			  game_events->apply_attackbuild_events(noop_fn);
			  game_events->apply_attackmove_events(noop_fn);
			  game_events->apply_attackremove_events(noop_fn);
  			  game_events->apply_mobbuild_events(noop_fn);
			  game_events->apply_mobmove_events(noop_fn);
			  game_events->apply_mobremove_events(noop_fn);
			  game_events->apply_unitinfo_events(noop_fn);
			  game_events->apply_statetransition_events(noop_fn);
		  }

		  std::this_thread::sleep_for(std::chrono::milliseconds(16));
	  }
  }

  std::shared_ptr<GameInformationType> shared_gamestate_info;
  TowerEventQueueType *td_event_queue;
  ViewEvents *game_events;
};

#endif
