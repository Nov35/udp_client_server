#pragma once

#include <filesystem>
#include <string>

namespace utils {

std::string CurrentDateString();
std::filesystem::path CurrentExecutableFilePath();

}
