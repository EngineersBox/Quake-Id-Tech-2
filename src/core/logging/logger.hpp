#pragma once

#ifndef QUAKE_LOGGER_HPP
#define QUAKE_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>

namespace Core::Logger {
    static constexpr char DATETIME_LITERAL_FORMAT[] = "%Y-%m-%d_%H-%M-%S";
    static constexpr char LOG_FORMAT[] = "[%Y/%m/%d %H:%M:%S] [%n] [%^%l%$] [thread %t] :: %v";

    static std::string getCurrentTime(const char* date_format) {
        std::time_t rawtime;
        char date_buffer[80];
        std::time(&rawtime);
        struct tm* timeinfo = std::localtime(&rawtime);
        std::strftime(date_buffer, sizeof(date_buffer), date_format, timeinfo);
        return {date_buffer};
    }

    static std::string getLogFileName() {
        return "logs/quake_log_" + getCurrentTime(DATETIME_LITERAL_FORMAT) + ".log";
    }

    static void generalErrorHandler(const std::string& msg) {
        spdlog::error("[EXCEPTION] :: {0}", msg);
    }

    static void init() {
        try {
            std::vector<spdlog::sink_ptr> sinks;

            auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
            stdoutSink->set_level(spdlog::level::info);
            sinks.push_back(stdoutSink);

            auto basicFileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(getLogFileName());
            basicFileSink->set_level(spdlog::level::trace);
            sinks.push_back(basicFileSink);

            std::shared_ptr<spdlog::logger> combinedLogger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
            spdlog::set_default_logger(combinedLogger);
            combinedLogger->set_level(spdlog::level::trace);

            spdlog::set_error_handler(Core::Logger::generalErrorHandler)
        } catch (const spdlog::spdlog_ex& ex) {
            std::cout << "Logger init failed: " << ex.what() << std::endl;
            exit(1);
        }
        spdlog::set_pattern(LOG_FORMAT);
    }

    static void errorCallbackGLFW(int code, char* message) {
        spdlog::error("GLFW Error ({0}): {1}", code, message);
    }
}

#endif //QUAKE_LOGGER_HPP
