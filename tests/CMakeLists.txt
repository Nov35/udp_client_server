include(CTest)

file(GLOB storage_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/server/storage/*.h" 
                                        "${PROJECT_SOURCE_DIR}/src/server/storage/*.cpp")

add_library(Storage STATIC ${storage_SRC})

target_link_libraries(Storage PRIVATE
    Common
)

add_executable(UnitTests unit_tests.cpp)
target_compile_options(UnitTests PUBLIC -fconcepts -std=gnu++2a)


target_link_libraries(UnitTests PRIVATE
    gtest_main
    Common
    Storage
)

add_test(
    NAME UnitTests
    COMMAND UnitTests
)

include_directories("${PROJECT_SOURCE_DIR}/src/server/storage/")
