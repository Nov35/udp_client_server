project(UDP_Client VERSION 1.0.0 LANGUAGES C CXX)

file(GLOB client_SRC CONFIGURE_DEPENDS "*.h" "*.cpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/client)

add_executable(UDP_Client ${client_SRC})

target_link_libraries(UDP_Client PRIVATE Common)
