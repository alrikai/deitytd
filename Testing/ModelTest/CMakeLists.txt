CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

#SET (Model_SRCS model_views_test.cpp)                                                            
#ADD_EXECUTABLE(TestModel ${Model_SRCS})
#TARGET_LINK_LIBRARIES(TestModel 
#                      ${TDView} 
#                      ${Controller}
#					  ${TowersBackend}
#                      ${OPENCL_LIBRARIES} 
#					  ${OpenCV_LIBS} 
#					  ${PCL_COMMON_LIBRARIES}
#					  ${PCL_SURFACE_LIBRARIES}
#					  ${PCL_FEATURES_LIBRARIES}
#					  ${OIS_LIBRARIES} 
#					  ${OGRE_LIBRARIES})

SET (ModelBackend_SRCS backend_test.cpp)                                                            
ADD_EXECUTABLE(TestModelBackend ${ModelBackend_SRCS})

#NOTE: we need to link to the Models/Runes library. Will need some refactoring 
MESSAGE(STATUS "View Lib: ${TDView}")
MESSAGE(STATUS "Tower Lib: ${TDTowers}")
set(TowersBackend "/home/alrik/TowerDefense/build/Model/libTowersBackend.so")
set(TDTowers_LIB "/home/alrik/TowerDefense/build/Model/Towers/libTDTowers.so")	

TARGET_LINK_LIBRARIES(TestModelBackend 
    ${TowersBackend}
	${TDTowers_LIB}
    ${OPENCL_LIBRARIES} 
    ${OpenCV_LIBS} 
    ${OGRE_LIBRARIES}
    ${PCL_COMMON_LIBRARIES}
    ${PCL_SURFACE_LIBRARIES}
    ${PCL_FEATURES_LIBRARIES})			  
