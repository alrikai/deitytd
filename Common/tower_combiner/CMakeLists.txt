CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#include_directories("/home/alrik/Projects/towerdefense/Model/util")

set (TowerComboSrc ModifierParser.cpp  TowerCombiner.cpp)
add_library(TDTowerCombiner SHARED ${TowerComboSrc})

SET(TDCOMBOSRCS main.cpp)
ADD_EXECUTABLE(TowerCombinationTest ${TDCOMBOSRCS})
target_link_libraries(TowerCombinationTest TDTowerCombiner TDUtils ${YAML-CPP})
