find_package(GSL REQUIRED) #Basic math utils
find_package(Boost COMPONENTS system filesystem iostreams REQUIRED) #Basic math utils
find_package(GnuplotIOStream REQUIRED)#Kill this

add_executable (differentiation differentiation.cpp)
target_include_directories(differentiation PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(differentiation ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_IOSREAMS_LIBRARY})
target_link_libraries(differentiation ${GNUPLOT_IOS_LIBRARIES})
target_link_libraries(differentiation ${GSL_LIBRARIES})
set_property(TARGET differentiation PROPERTY CXX_STANDARD 11)

add_executable (integration integration.cpp)
target_include_directories(integration PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(integration PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(integration ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(integration ${GSL_LIBRARIES})
set_property(TARGET integration PROPERTY CXX_STANDARD 11)

add_executable (homogen homogen.cpp)
target_include_directories(homogen PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(homogen PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(homogen ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(homogen ${GSL_LIBRARIES})
set_property(TARGET homogen PROPERTY CXX_STANDARD 11)