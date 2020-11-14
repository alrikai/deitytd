#ifndef DTD_TESTING_MODELTEST_UTILS_FRONTEND_HPP
#define DTD_TESTING_MODELTEST_UTILS_FRONTEND_HPP

#include "Testing/fractal_util.hpp"
#include "Common/shared/common_information.hpp"
#include "Common/shared/Player.hpp"
#include "Common/shared/Player.hpp"

#include <memory>
#include <vector>
#include <string>

namespace TestStubs {
// mock-up of the frontend
template <typename BackendType> struct FrontStub {
  using ModifierType = tower_properties;
  using TowerEventQueueType = typename UserTowerEvents::EventQueueType<
      UserTowerEvents::tower_event<BackendType>>::QType;
  using GameInformationType =
      GameInformation<CommonTowerInformation, TDPlayerInformation>;

  void draw_maptiles(const int width, const int height) {}

  void register_backend_eventqueue(ViewEvents *events) {}

  void register_tower_eventqueue(TowerEventQueueType *tevt_queue) {
    td_event_queue = tevt_queue;
  }

  void register_shared_info(std::shared_ptr<GameInformationType> s_info) {
    shared_gamestate_info = s_info;
  }

  std::shared_ptr<GameInformationType> shared_gamestate_info;
  TowerEventQueueType *td_event_queue;
};

template <typename TDType> void add_tower_displayinfo(TDType &td) {
  using PixelType = uint8_t;
  std::string mesh_filename{TDHelpers::get_basepath() +
                            "/data/meshfractal3d.vtk"};

  std::vector<std::vector<uint32_t>> polygon_mesh;
  std::vector<std::vector<float>> polygon_points;
  views_utils::add_mesh<PixelType>(polygon_mesh, polygon_points, mesh_filename);

  const std::string tower_material{"Examples/Chrome"};
  const std::string tower_name{"ViewTest"};
  td->add_tower(std::move(polygon_mesh), std::move(polygon_points),
                tower_material, tower_name);
}
} // namespace TestStubs

#endif
