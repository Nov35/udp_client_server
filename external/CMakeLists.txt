include(AddGitSubmodule)

add_git_submodule(external/asio)

include(FetchContent)

FetchContent_Declare(
    googletest-distribution
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(googletest-distribution)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.14.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    bitsery
    GIT_REPOSITORY https://github.com/fraillt/bitsery
    GIT_TAG v5.2.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(bitsery)
