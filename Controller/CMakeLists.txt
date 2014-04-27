CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (SRCS Controller.cpp InputListener.cpp)
INCLUDE_DIRECTORIES(${OIS_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIRS})

ADD_LIBRARY(Controller ${SRCS})
TARGET_LINK_LIBRARIES(Controller ${OIS_LIBRARIES} ${OGRE_LIBRARIES})

#will want to have some central output location for the libraries?
