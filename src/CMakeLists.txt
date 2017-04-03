add_subdirectory(examples)

find_package(GSL REQUIRED) #Basic math utils
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads REQUIRED) #Multithreading

add_executable (test_concept "test_concept.cpp" lib_dsp/uncasual.hpp lib_dsp/routines.hpp lib_dsp/sources.hpp lib_dsp/filters.hpp lib_dsp/signal.hpp lib_dist/swp_fifo.hpp)
target_link_libraries(test_concept ${GSL_LIBRARIES})
target_link_libraries(test_concept ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(test_concept ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET test_concept PROPERTY CXX_STANDARD 11)

