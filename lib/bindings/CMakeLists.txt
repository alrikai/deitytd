cmake_minimum_required(VERSION 2.8.12)

add_subdirectory(pybind11)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pyDeityTD")
message("pyDTD output dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set (wrapper_src src/dtdwrap.cpp src/event_wrap.cpp src/tower_modifier_wrap.cpp src/playerstate_wrap.cpp src/gameserver_wrap.cpp)

pybind11_add_module( _pyDTD ${wrapper_src})
target_link_libraries(_pyDTD PUBLIC TowersBackend TDShared TDCommon)
