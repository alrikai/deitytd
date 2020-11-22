CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#include_directories("/home/alrik/Projects/towerdefense/Model/util")

set (TowerComboSrc ModifierConfigParser.cpp ModifierParser.cpp TowerCombiner.cpp)
add_library(TDTowerCombiner STATIC ${TowerComboSrc})

SET(TDCOMBOSRCS main.cpp)
ADD_EXECUTABLE(TowerCombinationTest ${TDCOMBOSRCS})
target_link_libraries(TowerCombinationTest TDTowerCombiner TDUtils TDShared ${YAML-CPP})
