find_package(Boost COMPONENTS system REQUIRED) #TCP iostream
find_package(Threads REQUIRED) #multithreading (at least in Boost)
find_package(Allegro REQUIRED) #OpenGL for osciloscope
find_package(GSL REQUIRED) #Basic math utils

add_executable (redpitaya_scpi "redpitaya_scpi.cpp")
target_link_libraries(redpitaya_scpi ${Boost_LIBRARIES})
target_link_libraries(redpitaya_scpi ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(redpitaya_scpi ${ALLEGRO_LIBRARIES})
set_property(TARGET redpitaya_scpi PROPERTY CXX_STANDARD 11)

add_executable (redpitaya_2 "redpitaya_2.cpp")
target_link_libraries(redpitaya_2 ${Boost_LIBRARIES})
target_link_libraries(redpitaya_2 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(redpitaya_2 ${ALLEGRO_LIBRARIES})
target_link_libraries(redpitaya_2 ${GSL_LIBRARIES})
set_property(TARGET redpitaya_2 PROPERTY CXX_STANDARD 11)