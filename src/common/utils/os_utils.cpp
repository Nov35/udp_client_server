#include "os_utils.h"

#include <stdexcept>

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

} // namespace utils

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

} // namespace utils

#else

static_assert(false, "Unsupported OS");

#endif
