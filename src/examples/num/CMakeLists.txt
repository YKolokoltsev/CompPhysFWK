find_package(GSL REQUIRED) #Basic math utils
find_package(Boost COMPONENTS system filesystem iostreams REQUIRED) #Basic math utils
find_package(GnuplotIOStream REQUIRED)#Kill this

add_executable (ex_num_differentiation differentiation.cpp)
target_include_directories(ex_num_differentiation PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(ex_num_differentiation ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_IOSREAMS_LIBRARY})
target_link_libraries(ex_num_differentiation ${GNUPLOT_IOS_LIBRARIES})
target_link_libraries(ex_num_differentiation ${GSL_LIBRARIES})
set_property(TARGET ex_num_differentiation PROPERTY CXX_STANDARD 11)

add_executable (ex_num_integration integration.cpp)
target_include_directories(ex_num_integration PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(ex_num_integration PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(ex_num_integration ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(ex_num_integration ${GSL_LIBRARIES})
set_property(TARGET ex_num_integration PROPERTY CXX_STANDARD 11)

add_executable (ex_num_homogen homogen.cpp)
target_include_directories(ex_num_homogen PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_include_directories(ex_num_homogen PUBLIC ${GSL_INCLUDE_DIR})
target_link_libraries(ex_num_homogen ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(ex_num_homogen ${GSL_LIBRARIES})
set_property(TARGET ex_num_homogen PROPERTY CXX_STANDARD 11)

add_executable (ex_num_numerov numerov.cpp)
set_property(TARGET ex_num_numerov PROPERTY CXX_STANDARD 11)

add_executable (ex_num_inout inout.cpp)
target_include_directories(ex_num_inout PUBLIC ${GNUPLOT_IOS_INCLUDE_DIR})
target_link_libraries(ex_num_inout ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
set_property(TARGET ex_num_inout PROPERTY CXX_STANDARD 11)