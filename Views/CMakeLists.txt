cmake_minimum_required(VERSION 2.8)

#set(CMAKE_DEBUG_POSTFIX "_d")
#set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")
 
find_package(OIS REQUIRED)
if(NOT OIS_FOUND)
	message(SEND_ERROR "Failed to find OIS.")
endif()

SET(SRCS OgreDisplay.cpp GameBackground.cpp)

include_directories( ${OIS_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIRS}
	${OGRE_SAMPLES_INCLUDEPATH}
	${OGRE_Overlay_INCLUDE_DIRS}
)
 
add_library(TDView ${SRCS})
#set_target_properties(TDView PROPERTIES DEBUG_POSTFIX _d)
target_link_libraries(TDView 
	${OpenCV_LIBS}
	${OGRE_LIBRARIES} 
	${OIS_LIBRARIES} 
	${OGRE_Overlay_LIBRARIES})
 
#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin)
#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/media)
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/dist/bin)
