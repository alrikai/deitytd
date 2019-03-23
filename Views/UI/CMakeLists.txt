cmake_minimum_required(VERSION 2.8)

set(ViewUISRCS GameGUI.cpp TowerUI.cpp)
include_directories(${CEGUI_INCLUDE_DIR})
add_library(TDViewUI SHARED ${ViewUISRCS})
target_link_libraries(TDViewUI TDTowerCombiner ${OGREVIEW_LIBS} ${YAML-CPP})
