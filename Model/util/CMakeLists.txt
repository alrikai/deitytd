CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set (UtilSrc TowerProperties.cpp TowerModifiers.cpp) 
add_library(TDUtils SHARED ${UtilSrc})
