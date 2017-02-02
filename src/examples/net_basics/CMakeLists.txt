find_package(Boost COMPONENTS system REQUIRED) #TCP iostream
find_package(Threads REQUIRED) #multithreading (at least in Boost)
find_package(Allegro REQUIRED) #OpenGL for osciloscope
find_package(GSL REQUIRED) #Basic math utils

add_executable (redpitaya "redpitaya.cpp")
target_link_libraries(redpitaya ${Boost_LIBRARIES})
target_link_libraries(redpitaya ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(redpitaya ${ALLEGRO_LIBRARIES})
target_link_libraries(redpitaya ${GSL_LIBRARIES})
set_property(TARGET redpitaya PROPERTY CXX_STANDARD 11)