CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(CMAKE_INCLUDE_CURRENT_DIR on)
SET(Controller "/home/alrik/TowerDefense/build/Controller/libController.a")
add_subdirectory(ControllerTest)

find_package(PCL REQUIRED COMPONENTS common surface features)
include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})

SET(TDView "/home/alrik/TowerDefense/build/Views/libTDView.a")
add_subdirectory(ViewsTest)

include_directories(ViewsTest)
add_subdirectory(ModelTest)

