CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11 -Wall")

set (TowerSrc Essences.cpp EssenceSynthesis.cpp Tower.cpp)
add_library(TDTowers SHARED ${TowerSrc})

SET(SRCS main.cpp)
ADD_EXECUTABLE(EssenceTest ${SRCS})
target_link_libraries(EssenceTest TDTowers)
