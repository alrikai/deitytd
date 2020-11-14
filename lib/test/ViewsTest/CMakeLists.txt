CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (VIEWS_SRCS frontend_test.cpp)                                                            
ADD_EXECUTABLE(TestView ${VIEWS_SRCS})
TARGET_LINK_LIBRARIES(TestView 
                      TDView Controller TDCommon
                      ${OPENCL_LIBRARIES} ${OpenCV_LIBS} 
                      ${OGREVIEW_LIBS})

#					  ${PCL_COMMON_LIBRARIES}
#					  ${PCL_SURFACE_LIBRARIES}
#					  ${PCL_FEATURES_LIBRARIES}
	
