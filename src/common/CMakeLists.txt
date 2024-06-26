find_package(Threads REQUIRED)

file(GLOB connection_SRC CONFIGURE_DEPENDS "network/*.h" "network/*.cpp")
file(GLOB config_SRC CONFIGURE_DEPENDS "configuration/*.h" "configuration/*.cpp")
file(GLOB utils_SRC CONFIGURE_DEPENDS "utils/*.h" "utils/*.cpp")

add_library(Common STATIC ${connection_SRC} ${utils_SRC} ${config_SRC})

target_link_libraries(Common PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Bitsery::bitsery
    Threads::Threads 
    ${CMAKE_DL_LIBS})

target_include_directories(Common PUBLIC "${PROJECT_SOURCE_DIR}/external/asio/asio/include"
"./network"
"./utils"
"./configuration"
"${CMAKE_BINARY_DIR}/generated_files/include")
target_include_directories(Common PUBLIC  )
