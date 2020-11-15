CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(DTD)

enable_testing()
add_definitions(-DTDPath=${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++17 -Wall -Wextra")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/share/OGRE/cmake/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/OGRE/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
MESSAGE("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

#OpenCL 
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if(OPENCL_HAS_CPP_BINDINGS)
    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
else(OPENCL_HAS_CPP_BINDINGS )
    message("No OpenCL CPP bindings found")
endif(OPENCL_HAS_CPP_BINDINGS)

#OpenCV
FIND_PACKAGE(OpenCV COMPONENTS core highgui imgproc imgcodecs REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message("OpenCV include: " ${OpenCV_INCLUDE_DIRS})
message("OpenCV link: " ${OpenCV_LIBS})

#Boost
FIND_PACKAGE(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost include: " ${Boost_INCLUDE_DIRS})
message("Boost link: " ${Boost_LIBS})

set(YAML-CPP yaml-cpp)
find_package(yaml-cpp REQUIRED)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/core)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/core/Model)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/core/Common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/core/Common/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/core/Common/util)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(bindings)
ADD_SUBDIRECTORY(test)
