CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

SET (CNTRL_SRCS controller_test.cpp)                                                            
ADD_EXECUTABLE(TestController ${CNTRL_SRCS})
SET(Controller "/home/alrik/TowerDefense/build/Controller/libController.a")
TARGET_LINK_LIBRARIES(TestController ${Controller} ${OIS_LIBRARIES} ${OGRE_LIBRARIES})

find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if( OPENCL_HAS_CPP_BINDINGS )
    message( "OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS} )
else( OPENCL_HAS_CPP_BINDINGS )
    message( "No OpenCL CPP bindings found" )
endif( OPENCL_HAS_CPP_BINDINGS )

find_package(PCL REQUIRED COMPONENTS common surface visualization features)
include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})

include_directories(
    ${OGRE_INCLUDE_DIRS}
    ${OGRE_SAMPLES_INCLUDEPATH}
    ${OGRE_Overlay_INCLUDE_DIRS}
)

SET (VIEWS_SRCS views_test.cpp)                                                            
ADD_EXECUTABLE(TestView ${VIEWS_SRCS})
SET(TDView "/home/alrik/TowerDefense/build/Views/libTDView.a")
TARGET_LINK_LIBRARIES(TestView 
                      ${TDView} 
                      ${Controller} 
                      ${OPENCL_LIBRARIES} 
					  ${OpenCV_LIBS} 
					  ${PCL_COMMON_LIBRARIES}
					  ${PCL_SURFACE_LIBRARIES}
					  ${PCL_FEATURES_LIBRARIES}
					  ${OIS_LIBRARIES} 
					  ${OGRE_LIBRARIES})
