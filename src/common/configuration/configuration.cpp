#include "configuration.h"

#include "utils.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace config
{

    static void WriteToFile(nlohmann::json &config,
                            const std::filesystem::path &filePath)
    {
        std::ofstream configFile(filePath);

        if (configFile)
        {
            configFile << config.dump(1);
            configFile.close();
        }
        else
            throw std::runtime_error("Cannot create the config file");
    }

    void Server::Load(nlohmann::json &config)
    {
        port_ = config.at("port");
        logger_.Load(config.at("logger"));
    }

    void Server::SaveOrCreate(const std::filesystem::path &filePath)
    {
        nlohmann::json config;
        config["port"] = port_;
        logger_.Save(config["logger"]);

        WriteToFile(config, filePath);
    }

    void Client::Load(nlohmann::json &config)
    {
        range_constant_ = config.at("range constant");
        address_.Load(config.at("endpoint"));
        logger_.Load(config.at("logger"));
    }

    void Client::SaveOrCreate(const std::filesystem::path &filePath)
    {
        nlohmann::json config;
        config["range constant"] = range_constant_;
        address_.Save(config["endpoint"]);
        logger_.Save(config["logger"]);

        WriteToFile(config, filePath);
    }

    void Address::Load(const nlohmann::json &element)
    {
        port_ = element.at("port");
        hostname_ = element.at("hostname");
    }

    void Address::Save(nlohmann::json &element)
    {
        element["port"] = port_;
        element["hostname"] = hostname_;
    }

    void Logger::Load(const nlohmann::json &element)
    {
        output_flags_ = 0;

        if (element.at("output").at("console"))
            output_flags_ |= Console;

        if (element.at("output").at("file"))
            output_flags_ |= File;

        logging_level_ = spdlog::level::from_str(element.at("logging level"));
        flush_level_ = spdlog::level::from_str(element.at("flush level"));
    }

    void Logger::Save(nlohmann::json &element)
    {
        element["output"] =
            {
                {"console", static_cast<bool>(output_flags_ & Console)},
                {"file", static_cast<bool>(output_flags_ & File)}};

        element["logging level"] = spdlog::level::to_string_view(logging_level_).data();
        element["flush level"] = spdlog::level::to_string_view(flush_level_).data();
    }

    void InitializeLogger(const Logger &settings)
    {
        try
        {
            std::vector<spdlog::sink_ptr> sinkList;

            if (settings.output_flags_ & Logger::Console)
            {
                auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                consoleSink->set_level(settings.logging_level_);
                consoleSink->set_pattern("%^[%Y-%m-%d %a %T] [%l]\t[%v]%$");
                sinkList.push_back(consoleSink);
            }

            if (settings.output_flags_ & Logger::File)
            {
                auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    fmt::format("{}{}.log", "logs/", utils::CurrentTimeString()),
                    std::numeric_limits<size_t>::max(), 10, true);

                fileSink->set_level(settings.logging_level_);
                fileSink->set_pattern("[%Y-%m-%d %a %T] [%l]\t[%v]");
                sinkList.push_back(fileSink);
            }

            auto logger = std::make_shared<spdlog::logger>("Logger", sinkList.begin(), sinkList.end());
            logger->set_level(settings.logging_level_);
            spdlog::set_default_logger(logger);
            spdlog::flush_on(settings.flush_level_);
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            spdlog::error("Log initialization failed: {}", std::string(ex.what()));
        }
    }
}
