CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(TowerDefense)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
if(EXISTS "/usr/local/lib/OGRE/cmake")
  set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
  set(OGRE_SAMPLES_INCLUDEPATH "/usr/local/share/OGRE/samples/Common/include/")
else ()
  message(SEND_ERROR "Failed to find module path.")
endif()

find_package(OGRE REQUIRED)
if(NOT OGRE_FOUND)
	message(SEND_ERROR "Failed to find OGRE")
else()
    message(STATUS "Found OGRE Libs: ${OGRE_LIBRARIES}")
endif()

FIND_PACKAGE(OpenCV COMPONENTS core highgui imgproc REQUIRED)

#set(OGRE_BOOST_COMPONENTS thread date_time)
#find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11 -Wall")


#ADD_SUBDIRECTORY(Controller)
ADD_SUBDIRECTORY(Views)
ADD_SUBDIRECTORY(Model)


