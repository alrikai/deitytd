CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

add_subdirectory(tower_combiner)

set (TowerSrc Tower.cpp)
add_library(TDTowers SHARED ${TowerSrc})
target_link_libraries(TDTowers TDTowerCombiner)

#SET(SRCS main.cpp)
#ADD_EXECUTABLE(EssenceTest ${SRCS})
#target_link_libraries(EssenceTest TDTowers TDUtils)
