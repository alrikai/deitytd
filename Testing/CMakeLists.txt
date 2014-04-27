CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (CNTRL_SRCS controller_test.cpp)                                                            
ADD_EXECUTABLE(TestController ${CNTRL_SRCS})
SET(Controller "/home/alrik/TowerDefense/build/Controller/libController.a")
TARGET_LINK_LIBRARIES(TestController ${Controller} ${OIS_LIBRARIES} ${OGRE_LIBRARIES})


SET (VIEWS_SRCS views_test.cpp)                                                            
ADD_EXECUTABLE(TestView ${VIEWS_SRCS})
SET(TDView "/home/alrik/TowerDefense/build/Views/libTDView.a")
TARGET_LINK_LIBRARIES(TestView ${TDView} ${Controller} ${OpenCV_LIBS} ${OIS_LIBRARIES} ${OGRE_LIBRARIES})
