find_package(Boost REQUIRED COMPONENTS system filesystem)

file(GLOB SWP_FIFO_GRAPH_SRC *.cpp)

add_executable (ex_dist_swp_fifo_graph ${SWP_FIFO_GRAPH_SRC})
target_link_libraries(ex_dist_swp_fifo_graph ${Boost_LIBRARIES})
target_link_libraries(ex_dist_swp_fifo_graph ${Boost_SYSTEM_LIBRARY})
target_link_libraries(ex_dist_swp_fifo_graph ${Boost_FILESYSTEM_LIBRARY})
set_property(TARGET ex_dist_swp_fifo_graph PROPERTY CXX_STANDARD 11)