CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (CNTRL_SRCS controller_test.cpp)                                                            
ADD_EXECUTABLE(TestController ${CNTRL_SRCS})
TARGET_LINK_LIBRARIES(TestController ${Controller} ${OIS_LIBRARIES} ${OGRE_LIBRARIES})
