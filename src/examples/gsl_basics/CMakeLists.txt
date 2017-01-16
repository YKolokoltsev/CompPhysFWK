find_package(GSL REQUIRED) #Basic math utils
find_package(Boost COMPONENTS system filesystem REQUIRED) #Basic math utils
find_package(GnuplotIOStream REQUIRED) #Gnuplot output

add_executable (differentiation differentiation.cpp)
target_include_directories(differentiation PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(differentiation PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(differentiation ${Boost_LIBRARIES}
        ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(differentiation ${GSL_LIBRARIES})
target_link_libraries(differentiation ${GNUPLOT_IOS_GSL_LIBRARIES})
set_property(TARGET differentiation PROPERTY CXX_STANDARD 11)

add_executable (integration integration.cpp)
target_include_directories(integration PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(integration PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(integration ${Boost_LIBRARIES}
        ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(integration ${GSL_LIBRARIES})
target_link_libraries(integration ${GNUPLOT_IOS_GSL_LIBRARIES})
set_property(TARGET integration PROPERTY CXX_STANDARD 11)