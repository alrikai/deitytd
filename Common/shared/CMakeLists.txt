CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set (SharedSrc PlayerInventory.cpp) 
add_library(TDShared SHARED ${SharedSrc})
