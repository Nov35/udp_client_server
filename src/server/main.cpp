#include "server.h"
#include "configuration.h"
#include "utils.h"

#include <asio/io_context.hpp>

#include <iostream>

int main()
{
    std::filesystem::path configPath;
    config::Server settings;

    try
    {
        using namespace config;

        configPath = utils::CurrentExecutableFilePath().replace_filename("config.json");
        settings = LoadOrCreate<config::Server>(configPath);
    }
    catch (const std::exception &error)
    {
        std::cerr << "Failed to load settings file at: " << configPath << '\n'
                  << "Error: " << error.what() << '\n';

        return -1;
    }

    InitializeLogger(settings.logger_);

    try
    {
        asio::io_context io_context;

        spdlog::info("Initializing server on port: {}", settings.port_);
        Server server(io_context, settings.port_);
        std::vector<std::thread> thread_pool;
        for (size_t i = 0; i < std::thread::hardware_concurrency() / 2; ++i)
        {
            thread_pool.push_back(std::thread([&io_context](){io_context.run();}));
        }
        io_context.run();
        for(auto& thread : thread_pool)
            thread.join();
    }
    catch (std::exception &e)
    {
        spdlog::critical("Server was terminated due to exception: {}", e.what());
    }

    return 0;
}
