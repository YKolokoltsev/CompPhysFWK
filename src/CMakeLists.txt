add_subdirectory(utils)
find_package(Allegro REQUIRED) #OpenGL
find_package(Threads REQUIRED) #Multithreading
find_package(GSL REQUIRED) #Basic math utils

add_executable (example_fdtd_minimal "example_fdtd_minimal.cpp")
include_directories(example_fdtd_minimal ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_minimal ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_minimal ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_minimal PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_lense "example_fdtd_lense.cpp")
include_directories(example_fdtd_lense ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_lense ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_lense ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_lense PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_2nd_mur "example_fdtd_2nd_mur.cpp")
include_directories(example_fdtd_2nd_mur ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_2nd_mur ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_2nd_mur ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_2nd_mur PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_2dite "example_fdtd_2dite.cpp")
include_directories(example_fdtd_2dite ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_2dite ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_2dite ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_2dite PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_resonator "example_fdtd_resonator.cpp")
include_directories(example_fdtd_resonator ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_resonator ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_resonator ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_resonator PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_difint "example_fdtd_difint.cpp")
include_directories(example_fdtd_difint ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_difint ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_difint ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_fdtd_difint PROPERTY CXX_STANDARD 11)

add_executable (example_fdtd_foton "example_fdtd_foton.cpp")
include_directories(example_fdtd_foton ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(example_fdtd_foton ${ALLEGRO_LIBRARIES})
target_link_libraries(example_fdtd_foton ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(example_fdtd_foton utils)
set_property(TARGET example_fdtd_foton PROPERTY CXX_STANDARD 11)

add_executable (example_visual_gnuplot "example_visual_gnuplot.cpp")
target_link_libraries(example_visual_gnuplot ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET example_visual_gnuplot PROPERTY CXX_STANDARD 11)



find_package(Boost REQUIRED COMPONENTS system iostreams)
add_executable (test_gnuplot "test_gnuplot.cpp")
target_link_libraries(test_gnuplot ${Boost_LIBRARIES})
set_property(TARGET test_gnuplot PROPERTY CXX_STANDARD 11)



add_executable (test_concept "test_concept.cpp")
include_directories(test_concept ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(test_concept ${ALLEGRO_LIBRARIES})
target_link_libraries(test_concept ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_concept ${GSL_LIBRARIES})
target_link_libraries(test_concept utils)
set_property(TARGET test_concept PROPERTY CXX_STANDARD 11)

