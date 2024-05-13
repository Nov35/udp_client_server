include(CTest)

add_executable(UnitTests connection_tests.cpp)

target_link_libraries(UnitTests PUBLIC
    gtest_main
    Common
)

add_test(
    NAME UnitTests
    COMMAND UnitTests
)

include_directories("${PROJECT_SOURCE_DIR}/src/server"
"${PROJECT_SOURCE_DIR}/src/client")
