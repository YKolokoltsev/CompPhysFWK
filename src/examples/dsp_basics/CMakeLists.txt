find_package(GSL REQUIRED) #Basic math utils
find_package(Boost REQUIRED COMPONENTS system iostreams)
find_package(Threads REQUIRED) #Multithreading

add_executable (inverse_rc_filter "inverse_rc_filter.cpp")
target_link_libraries(inverse_rc_filter ${GSL_LIBRARIES})
target_link_libraries(inverse_rc_filter utils)
set_property(TARGET inverse_rc_filter PROPERTY CXX_STANDARD 11)

add_executable (dummy_pipeline "dummy_pipeline.cpp")
target_link_libraries(dummy_pipeline ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET dummy_pipeline PROPERTY CXX_STANDARD 11)

