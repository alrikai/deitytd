CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

#SET (ModelBackend_SRCS backend_test.cpp)                                                            
#ADD_EXECUTABLE(TestModelBackend ${ModelBackend_SRCS})

#TARGET_LINK_LIBRARIES(TestModelBackend 
#    TowersBackend TDTowers TDCommon TDUtils 
#    ${OPENCL_LIBRARIES} 
#    ${OpenCV_LIBS} 
#    ${OGRE_LIBRARIES}
#	${CEGUI_LIBRARIES})
##    ${PCL_COMMON_LIBRARIES}
##    ${PCL_SURFACE_LIBRARIES}
##    ${PCL_FEATURES_LIBRARIES})			 

#SET (Model_SRCS model_views_test.cpp)                                                            
#ADD_EXECUTABLE(TestModel ${Model_SRCS})
#TARGET_LINK_LIBRARIES(TestModel 
#	TDView TDTowers TDUtils Controller TowersBackend TDCommon
#    ${OPENCL_LIBRARIES} 
#	${OpenCV_LIBS} 
#    ${OGREVIEW_LIBS})

#${PCL_COMMON_LIBRARIES}
#${PCL_SURFACE_LIBRARIES}
#${PCL_FEATURES_LIBRARIES}
					  
add_executable(GameMechanicsTest TestGameMechanics.cpp)
target_link_libraries(GameMechanicsTest gtest_main ${YAML-CPP} TDTowers TDUtils TDShared TowersBackend TDTowerCombiner TDCommon ${Boost_SYSTEM_LIBRARY} ${OpenCV_LIBS})
add_test(NAME GameMechanics_test COMMAND GameMechanicsTest)
