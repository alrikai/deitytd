CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(TowerDefense)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11 -Wall")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
if(EXISTS "/usr/local/lib/OGRE/cmake")
  set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
  set(OGRE_SAMPLES_INCLUDEPATH "/usr/local/share/OGRE/samples/Common/include/")
else ()
  message(SEND_ERROR "Failed to find module path.")
endif()

ADD_DEFINITIONS(-DRTSHADER_SYSTEM_BUILD_CORE_SHADERS)
ADD_DEFINITIONS(-DRTSHADER_SYSTEM_BUILD_EXT_SHADERS)

#Ogre
find_package(OGRE REQUIRED)
message(STATUS "Found OGRE Libs: ${OGRE_LIBRARIES}")
FIND_PACKAGE(OIS REQUIRED)

#OpenCL 
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if(OPENCL_HAS_CPP_BINDINGS)
    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
else(OPENCL_HAS_CPP_BINDINGS )
    message("No OpenCL CPP bindings found")
endif(OPENCL_HAS_CPP_BINDINGS)

#OpenCV
FIND_PACKAGE(OpenCV COMPONENTS core highgui imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message("OpenCV include: " ${OpenCV_INCLUDE_DIRS})
message("OpenCV link: " ${OpenCV_LIBS})

#Boost
FIND_PACKAGE(Boost 1.55.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost include: " ${Boost_INCLUDE_DIRS})
message("Boost link: " ${Boost_LIBS})

#set(OGRE_BOOST_COMPONENTS thread date_time)
#find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Controller)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Views)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Model)

ADD_SUBDIRECTORY(Controller)
ADD_SUBDIRECTORY(Model)
ADD_SUBDIRECTORY(Views)

ADD_SUBDIRECTORY(Testing)
