CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(CMAKE_INCLUDE_CURRENT_DIR on)
add_subdirectory(ControllerTest)

#find_package(PCL REQUIRED COMPONENTS common surface features)
#include_directories(${PCL_INCLUDE_DIRS})
#add_definitions(${PCL_DEFINITIONS})
#link_directories(${PCL_LIBRARY_DIRS})

include_directories(ViewsTest)

add_subdirectory(ModelTest)

