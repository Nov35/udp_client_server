project(UDP_Server VERSION 1.0.0 LANGUAGES C CXX)

file(GLOB server_SRC CONFIGURE_DEPENDS "*.h" "*.cpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/server)

add_executable(UDP_Server ${server_SRC})

target_link_libraries(UDP_Server PUBLIC 
    Common)
