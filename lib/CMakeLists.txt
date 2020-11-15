CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(DTD)

enable_testing()
add_definitions(-DTDPath=${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -ggdb -std=c++17 -Wall -Wextra")

#OpenCL 
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if(OPENCL_HAS_CPP_BINDINGS)
    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
else(OPENCL_HAS_CPP_BINDINGS )
    message("No OpenCL CPP bindings found")
endif(OPENCL_HAS_CPP_BINDINGS)

#OpenCV
FIND_PACKAGE(OpenCV COMPONENTS core imgproc imgcodecs REQUIRED)
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

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
add_definitions(-DRANDOM_SEED_TESTING=42)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(bindings)
ADD_SUBDIRECTORY(test)
