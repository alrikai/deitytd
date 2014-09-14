cmake_minimum_required(VERSION 2.8)

SET(FFSRCS flame_gen.cpp)

add_executable(fflame_gen ${FFSRCS})
target_link_libraries(fflame_gen ${OpenCV_LIBS})
