find_package(Threads REQUIRED)

file(GLOB connection_SRC CONFIGURE_DEPENDS "network/*.h" "network/*.cpp")
file(GLOB utils_SRC CONFIGURE_DEPENDS "utils/*.h" "utils/*.cpp")

add_library(Common STATIC ${connection_SRC} ${utils_SRC})

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
target_compile_options(Common PUBLIC -fconcepts)
# endif()

target_link_libraries(Common PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    ser20::ser20
    Threads::Threads 
    ${CMAKE_DL_LIBS})

target_include_directories(Common PUBLIC "${PROJECT_SOURCE_DIR}/external/asio/asio/include"
"./network"
"./utils"
"${CMAKE_BINARY_DIR}/generated_files/include")
target_include_directories(Common PUBLIC  )
