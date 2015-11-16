CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(make_include_current_dir on)
include_directories(include)

set (CommonSrc src/ModelUtils.cpp)
add_library(TDCommon SHARED ${CommonSrc})
