cmake_minimum_required(VERSION 2.8.12)
#project(pyDTD)

#find_package(OpenCV REQUIRED HINTS /usr/local/opt/opencv3 /usr/local/opt/opencv@3)

#set (CMAKE_CXX_FLAGS "-Wall -std=c++14 -fPIC -O0 -g -ggdb")
add_subdirectory(pybind11)

set (wrapper_src dtdwrap.cpp)
pybind11_add_module(pyDTD ${wrapper_src})
target_link_libraries(pyDTD PUBLIC TowersBackend)
