cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

set(PROJECT_NAME "UDP_client_server")
add_custom_target(${PROJECT_NAME})

project(UDP_client_server VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

add_dependencies(${PROJECT_NAME} UDP_Server UDP_Client)
add_subdirectory(version_info)
add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
