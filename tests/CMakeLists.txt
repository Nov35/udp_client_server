include(CTest)

add_executable(UnitTests connection_tests.cpp)

target_link_libraries(UnitTests PUBLIC
    gtest_main
    Connection
)

add_test(
    NAME UnitTests
    COMMAND UnitTests
)