add_subdirectory(utils)
add_subdirectory(examples)

find_package(GSL REQUIRED) #Basic math utils
find_package(Boost REQUIRED COMPONENTS system iostreams)
find_package(Threads REQUIRED) #Multithreading

add_executable (test_gnuplot "test_gnuplot.cpp")
target_link_libraries(test_gnuplot ${Boost_LIBRARIES})
set_property(TARGET test_gnuplot PROPERTY CXX_STANDARD 11)

add_executable (test_concept "test_concept.cpp" lib_dsp/uncasual.hpp lib_dsp/routines.hpp lib_dsp/sources.hpp lib_dsp/filters.hpp lib_dsp/signal.hpp)
target_link_libraries(test_concept ${GSL_LIBRARIES})
target_link_libraries(test_concept utils)
target_link_libraries(test_concept ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET test_concept PROPERTY CXX_STANDARD 11)

