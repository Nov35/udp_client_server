#include "utils.h"

#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <iostream>
namespace utils
{
    std::string CurrentDateString()
    {
        using namespace std::literals;
        using std::chrono::system_clock;

        const std::time_t t_c = system_clock::to_time_t(system_clock::now());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t_c), "%F") << std::flush;

        return ss.str();
    }
}

const size_t max_length = 4096;

#ifdef __linux__

#include <memory>
#include <unistd.h>

namespace utils
{

    std::filesystem::path CurrentExecutableFilePath()
    {
        char path_buffer[max_length];

        const size_t length = readlink("/proc/self/exe", path_buffer, max_length);

        if (length < 1)
            throw std::runtime_error("Can't get path of the executable");

        path_buffer[length] = 0;
        return path_buffer;
    }

}

#elif defined _WIN32

#include <windows.h>

namespace utils
{
    std::filesystem::path CurrentExecutableFilePath()
    {
        char path_buffer[max_length];

        const size_t length = GetModuleFileNameA(nullptr, path_buffer, max_length);

        if (length < 1)
            throw std::runtime_error("Can't get path of the executable");

        path_buffer[length] = 0;
        return path_buffer;
    }

}

#else

static_assert(false, "Unsupported OS");

#endif
