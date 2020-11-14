/* backend_test.cpp -- part of the DietyTD tests
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "../fractal_util.hpp"
#include "Model/TowerDefense.hpp"
#include "Model/util/Types.hpp"
#include "Events/ViewEventTypes.hpp"

#include <boost/algorithm/string.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

/*
 *  The idea here is to test the Model/backend without the frontend
 *  involved. We will generate events akin to how the frontend will,
 *  then see how the backend does with them. We want to test:
 */

// mock-up of the frontend
template <typename BackendType> struct FrontStub {
  using ModifierType = essence;
  using TowerEventQueueType = typename UserTowerEvents::EventQueueType<
      UserTowerEvents::tower_event<BackendType>>::QType;

  void draw_maptiles(const int width, const int height) {}

  void register_backend_eventqueue(ViewEvents *events) {}

  void register_tower_eventqueue(TowerEventQueueType *tevt_queue) {
    td_event_queue = tevt_queue;
  }
  TowerEventQueueType *td_event_queue;
};

template <typename TDType> void add_testtower(TDType &td) {
  using PixelType = uint8_t;
  std::string mesh_filename{TDHelpers::get_basepath() +
                            "/data/meshfractal3d.vtk"};

  std::vector<std::vector<uint32_t>> polygon_mesh;
  std::vector<std::vector<float>> polygon_points;
  views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

  const std::string tower_material{"Examples/Chrome"};
  const std::string tower_name{"ViewTest"};
  td.add_tower(std::move(polygon_mesh), std::move(polygon_points),
               tower_material, tower_name);
}

int main() {
  using TDBackendType = TowerLogic;
  using TDType = TowerDefense<FrontStub, TDBackendType>;
  FrontStub<TDBackendType> *view = new FrontStub<TDBackendType>();
  TDType td(view);

  // add a tower (assume this to be the fundamental tower)
  td.init_game();
  add_testtower(td);

  // spawns its own thread
  td.start_game();
  std::random_device rd;
  std::mt19937 gen(rd());

  // now, mock-up some user inputs -- first, tower creation

  using base_evt_t = UserTowerEvents::tower_event<TDBackendType>;
  std::string user_input{""};
  bool continue_running = true;
  while (continue_running) {
    auto start_wave_time = std::chrono::high_resolution_clock::now();

    std::getline(std::cin, user_input);
    if (!user_input.empty()) {
      std::vector<std::string> input_tokens;
      boost::split(input_tokens, user_input, boost::is_any_of(" "));

      int token_cnt = 0;
      if (input_tokens.at(token_cnt) == "q") {
        std::cout << "quitting input loop... " << std::endl;
        continue_running = false;
        break;
      } else if (input_tokens.at(token_cnt) == "p") {
        token_cnt += 1;
        assert(input_tokens.size() > 2);
        float t_row = std::stof(input_tokens.at(token_cnt++));
        float t_col = std::stof(input_tokens.at(token_cnt++));

        using print_evt_t = UserTowerEvents::print_tower_event<TDBackendType>;
        std::unique_ptr<base_evt_t> td_evt =
            std::unique_ptr<print_evt_t>(new print_evt_t(t_row, t_col));
        view->td_event_queue->push(std::move(td_evt));
        std::cout << "Building a Tower..." << std::endl;
      } else if (input_tokens.at(token_cnt) == "bt") {
        token_cnt += 1;
        assert(input_tokens.size() > 3);
        int tier = std::stoi(input_tokens.at(token_cnt++));
        float t_row = std::stof(input_tokens.at(token_cnt++));
        float t_col = std::stof(input_tokens.at(token_cnt++));

        using build_evt_t = UserTowerEvents::build_tower_event<TDBackendType>;
        std::unique_ptr<base_evt_t> td_evt =
            std::unique_ptr<build_evt_t>(new build_evt_t(tier, t_row, t_col));
        view->td_event_queue->push(std::move(td_evt));
        std::cout << "Building a Tower..." << std::endl;
      } else if (input_tokens.at(token_cnt) == "mt") {
        token_cnt += 1;
        assert(input_tokens.size() > 3);

        std::string essence_name = input_tokens.at(token_cnt++);

        // based on the index, make the appropriate essence type
        FrontStub<TDBackendType>::ModifierType *e_type = nullptr;
        if (essence_name == "aph")
          e_type = new aphrodite();
        else if (essence_name == "apo")
          e_type = new apollo();
        else if (essence_name == "are")
          e_type = new ares();
        else if (essence_name == "art")
          e_type = new artemis();
        else if (essence_name == "ath")
          e_type = new athena();
        else if (essence_name == "dem")
          e_type = new demeter();
        else if (essence_name == "dio")
          e_type = new dionysus();
        else if (essence_name == "had")
          e_type = new hades();
        else if (essence_name == "hep")
          e_type = new hephaestus();
        else if (essence_name == "her")
          e_type = new hera();
        else if (essence_name == "herm")
          e_type = new hermes();
        else if (essence_name == "hes")
          e_type = new hestia();
        else if (essence_name == "pos")
          e_type = new poseidon();
        else if (essence_name == "zeu")
          e_type = new zeus();
        else {
          std::cout << "NOTE: invalid essence specified -- valid names are the "
                       "following: "
                    << std::endl
                    << "aph, apo, are, art, ath, dem, dio, had, hep, her, "
                       "herm, hes, pos, zeu"
                    << std::endl;
          continue;
        }

        float t_row = std::stof(input_tokens.at(token_cnt++));
        float t_col = std::stof(input_tokens.at(token_cnt++));

        std::cout << "Modifying a Tower..." << std::endl;

        using evt_t = typename UserTowerEvents::modify_tower_event<
            FrontStub<TDBackendType>::ModifierType, TDBackendType>;
        std::unique_ptr<base_evt_t> td_evt =
            std::unique_ptr<evt_t>(new evt_t(e_type, t_row, t_col));
        view->td_event_queue->push(std::move(td_evt));
      }
    }

    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto end_wave_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_duration(end_wave_time -
                                                            start_wave_time);
  }

  td.stop_game();
}
