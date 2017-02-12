find_package(GSL REQUIRED) #Basic math utils
find_package(Boost REQUIRED COMPONENTS system iostreams)
find_package(Threads REQUIRED) #Multithreading

add_executable (ex_dsp_invert_rc "invert_rc.cpp")
target_link_libraries(ex_dsp_invert_rc ${GSL_LIBRARIES})
set_property(TARGET ex_dsp_invert_rc PROPERTY CXX_STANDARD 11)

add_executable (ex_dsp_pipeline "pipeline.cpp")
target_link_libraries(ex_dsp_pipeline ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET ex_dsp_pipeline PROPERTY CXX_STANDARD 11)

