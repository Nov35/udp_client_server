#pragma once

#include <filesystem>
#include <string>

namespace utils {

std::string CurrentTimeString();
std::filesystem::path CurrentExecutableFilePath();

}
