cmake_minimum_required(VERSION 2.8)

include_directories(${OpenCV_INCLUDE_DIRS})
add_subdirectory(FractalFlames)

include_directories(UI)
set(ViewSRCS GameBackground.cpp ViewUtil.cpp UI/GameGUI.cpp)
include_directories(FractalFlames) 
include_directories(
	${OIS_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIRS}
	${OGRE_SAMPLES_INCLUDEPATH}
	${OGRE_Overlay_INCLUDE_DIRS}
	${CEGUI_INCLUDE_DIR})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/scripts")

add_library(TDView SHARED ${ViewSRCS})
target_link_libraries(TDView 
	TowersBackend
	${OpenCV_LIBS}
	${OGREVIEW_LIBS})
