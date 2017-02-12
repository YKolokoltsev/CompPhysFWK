find_package(Allegro REQUIRED) #OpenGL
find_package(Threads REQUIRED) #Multithreading

add_executable (ex_fdtd_minimal "minimal.cpp")
include_directories(ex_fdtd_minimal ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_minimal ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_minimal ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_minimal PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_lense "lense.cpp")
include_directories(ex_fdtd_lense ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_lense ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_lense ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_lense PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_mur_2nd "mur_2nd.cpp")
include_directories(ex_fdtd_mur_2nd ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_mur_2nd ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_mur_2nd ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_mur_2nd PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_2dite "2dite.cpp")
include_directories(ex_fdtd_2dite ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_2dite ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_2dite ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_2dite PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_laser "laser.cpp")
include_directories(ex_fdtd_laser ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_laser ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_laser ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_laser PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_diafragm "diafragm.cpp")
include_directories(ex_fdtd_diafragm ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_diafragm ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_diafragm ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_fdtd_diafragm PROPERTY CXX_STANDARD 11)

add_executable (ex_fdtd_foton "foton.cpp")
include_directories(ex_fdtd_foton ${ALLEGRO_INCLUDE_DIR})
target_link_libraries(ex_fdtd_foton ${ALLEGRO_LIBRARIES})
target_link_libraries(ex_fdtd_foton ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(ex_fdtd_foton utils)
set_property(TARGET ex_fdtd_foton PROPERTY CXX_STANDARD 11)