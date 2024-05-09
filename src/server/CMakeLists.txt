project(UDP_Server VERSION 1.0.0 LANGUAGES C CXX)

add_executable(UDP_Server main.cpp)

target_link_libraries(UDP_Server PUBLIC Connection)
