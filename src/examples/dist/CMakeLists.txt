add_subdirectory(swp_static)

#find_package(GSL REQUIRED) #Basic math utils
find_package(Boost REQUIRED COMPONENTS system iostreams)

add_executable (ex_dist_swp_fifo "swp_fifo.cpp")
target_link_libraries(ex_dist_swp_fifo ${Boost_LIBRARIES})
set_property(TARGET ex_dist_swp_fifo PROPERTY CXX_STANDARD 11)
