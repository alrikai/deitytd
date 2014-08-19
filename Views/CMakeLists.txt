cmake_minimum_required(VERSION 2.8)

SET(ViewSRCS GameBackground.cpp ViewUtil.cpp)   #OgreDisplay.cpp 

include_directories( ${OIS_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIRS}
	${OGRE_SAMPLES_INCLUDEPATH}
	${OGRE_Overlay_INCLUDE_DIRS}
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/scripts")

add_library(TDView ${ViewSRCS})
#set_target_properties(TDView PROPERTIES DEBUG_POSTFIX _d)
set(TowersBackend "/home/alrik/TowerDefense/build/Model/libTowersBackend.so")
target_link_libraries(TDView 
	${TowersBackend}
	${OpenCV_LIBS}
	${OGRE_LIBRARIES} 
	${OIS_LIBRARIES} 
	${OGRE_Overlay_LIBRARIES})
