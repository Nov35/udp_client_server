project(UDP_Client VERSION 1.0.0 LANGUAGES C CXX)

add_executable(UDP_Client main.cpp)

target_link_libraries(UDP_Client PUBLIC Connection)
