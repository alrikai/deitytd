CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (ModelBackend_SRCS backend_test.cpp)                                                            
ADD_EXECUTABLE(TestModelBackend ${ModelBackend_SRCS})

TARGET_LINK_LIBRARIES(TestModelBackend 
    TowersBackend
    TDTowers
		TDCommon
    ${OPENCL_LIBRARIES} 
    ${OpenCV_LIBS} 
    ${OGRE_LIBRARIES})
#    ${PCL_COMMON_LIBRARIES}
#    ${PCL_SURFACE_LIBRARIES}
#    ${PCL_FEATURES_LIBRARIES})			 

SET (Model_SRCS model_views_test.cpp)                                                            
ADD_EXECUTABLE(TestModel ${Model_SRCS})
TARGET_LINK_LIBRARIES(TestModel 
    TDView 
	  TDTowers
    Controller
		TowersBackend
		TDCommon
    ${OPENCL_LIBRARIES} 
		${OpenCV_LIBS} 
  	${OIS_LIBRARIES} 
		${OGRE_LIBRARIES})
 
#${PCL_COMMON_LIBRARIES}
#${PCL_SURFACE_LIBRARIES}
#${PCL_FEATURES_LIBRARIES}
					  

