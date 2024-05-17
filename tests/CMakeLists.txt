include(CTest)


add_executable(UnitTests unit_tests.cpp)
target_compile_options(UnitTests PUBLIC -fconcepts -std=gnu++2a)

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
