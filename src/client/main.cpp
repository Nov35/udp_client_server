#include "client.h"

#include "configuration.h"
#include "utils.h"

#include <asio/io_context.hpp>

#include <iostream>
#include <thread>

int main()
{
    std::filesystem::path configPath;
    config::Client settings;

    try
    {
        using namespace config;

        configPath = utils::CurrentExecutableFilePath().replace_filename("config.json");
        settings = LoadOrCreate<config::Client>(configPath);
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

        auto& address = settings.address_;

        spdlog::info("Initializing application with server address: {}:{}", address.hostname_, address.port_);
        Client cl(io_context, address.hostname_, address.port_, settings.range_constant_);
        cl.start();

        // std::thread worker([&io_context]()
        //                    { io_context.run(); });
        io_context.run();
        // worker.join();
    }
    catch (std::exception &e)
    {
        spdlog::critical("Server was terminated due to exception: {}", e.what());
    }

    return 0;
}
