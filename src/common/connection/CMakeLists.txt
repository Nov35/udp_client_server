file(GLOB connection_SRC CONFIGURE_DEPENDS "*.h" "*.cpp")

find_package(Threads REQUIRED)

add_library(Connection STATIC ${connection_SRC})

target_link_libraries(Connection PUBLIC Threads::Threads ${CMAKE_DL_LIBS})

target_include_directories(Connection PUBLIC ${PROJECT_SOURCE_DIR}/external/asio/asio/include)
target_include_directories(Connection PUBLIC "./")
