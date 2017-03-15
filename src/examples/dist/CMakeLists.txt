
#find_package(GSL REQUIRED) #Basic math utils
find_package(Boost REQUIRED COMPONENTS system iostreams)

add_executable (distsim "distsim.cpp")
target_link_libraries(distsim ${Boost_LIBRARIES})
set_property(TARGET distsim PROPERTY CXX_STANDARD 11)
