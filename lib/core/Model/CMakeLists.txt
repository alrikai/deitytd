CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(make_include_current_dir on)
#add_subdirectory(util)
#include_directories(util)

include_directories(Towers)
add_subdirectory(Towers)

set (LogicSrc TowerLogic.cpp AttackLogic.cpp)
add_library(TowersBackend STATIC ${LogicSrc})
target_link_libraries(TowersBackend TDTowers TDUtils TDShared TDTowerCombiner ${YAML-CPP})
