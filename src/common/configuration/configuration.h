#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

namespace config
{

    struct Address
    {
        std::string hostname_ = "127.0.0.1";
        uint16_t port_ = 5555;

        void Load(const nlohmann::json &element);
        void Save(nlohmann::json &element);
    };

    struct Logger
    {
        enum LoggerOutputFlag
        {
            Console = 0b01,
            File = 0b10
        };

        int output_flags_ = Console | File;
        spdlog::level::level_enum logging_level_ = spdlog::level::info;
        spdlog::level::level_enum flush_level_ = spdlog::level::trace;

        void Load(const nlohmann::json &element);
        void Save(nlohmann::json &element);
    };

    struct Server
    {
        uint16_t port_ = 5555;
        Logger logger_;
        void Load(nlohmann::json &config);
        void SaveOrCreate(const std::filesystem::path &filePath);
    };

    struct Client
    {
        double range_constant_ = 100.0;
        Address address_;
        Logger logger_;

        void Load(nlohmann::json &config);
        void SaveOrCreate(const std::filesystem::path &filePath);
    };

    template<class Component>
    void Load(const std::filesystem::path &filePath, Component& settings)
    {
        std::ifstream configFile(filePath);

        if (configFile)
        {
            nlohmann::json config;
            configFile >> config;
            settings.Load(config);
        }
        else
            throw std::runtime_error("Cannot open the config file");
    }

    template<class Component>
    Component LoadOrCreate(const std::filesystem::path &filePath)
    {
        Component settings;

        if (std::filesystem::exists(filePath))
            Load(filePath, settings);
        else
            settings.SaveOrCreate(filePath);

        return settings;
    }

    void InitializeLogger(const Logger &settings);

}