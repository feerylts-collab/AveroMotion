#include "Core/AveroLog.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace Avero::Core
{
    LogLevel Logger::s_MinLevel = LogLevel::Trace;
    bool Logger::s_Initialized = false;

    void Logger::Initialize()
    {
        if (s_Initialized) return;

        s_Initialized = true;
        s_MinLevel = LogLevel::Trace;

        std::cout << "[Logger] Initialized\n";
    }

    void Logger::Shutdown()
    {
        if (!s_Initialized) return;

        std::cout << "[Logger] Shutdown\n";
        s_Initialized = false;
    }

    void Logger::SetMinLevel(LogLevel level)
    {
        s_MinLevel = level;
    }

    void Logger::Trace(std::string_view message)
    {
        Log(LogLevel::Trace, message);
    }

    void Logger::Debug(std::string_view message)
    {
        Log(LogLevel::Debug, message);
    }

    void Logger::Info(std::string_view message)
    {
        Log(LogLevel::Info, message);
    }

    void Logger::Warning(std::string_view message)
    {
        Log(LogLevel::Warning, message);
    }

    void Logger::Error(std::string_view message)
    {
        Log(LogLevel::Error, message);
    }

    void Logger::Fatal(std::string_view message)
    {
        Log(LogLevel::Fatal, message);
    }

    void Logger::Log(LogLevel level, std::string_view message)
    {
        if (!s_Initialized) return;
        if (level < s_MinLevel) return;

        // Получаем текущее время
        auto now = std::chrono::system_clock::now();        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        // Определяем префикс и цвет (для консоли)
        const char* levelStr = "";
        const char* colorCode = "";

        switch (level)
        {
        case LogLevel::Trace:
            levelStr = "TRACE";
            colorCode = "\033[90m"; // Серый
            break;
        case LogLevel::Debug:
            levelStr = "DEBUG";
            colorCode = "\033[36m"; // Циан
            break;
        case LogLevel::Info:
            levelStr = "INFO";
            colorCode = "\033[32m"; // Зеленый
            break;
        case LogLevel::Warning:
            levelStr = "WARN";
            colorCode = "\033[33m"; // Желтый
            break;
        case LogLevel::Error:
            levelStr = "ERROR";
            colorCode = "\033[31m"; // Красный
            break;
        case LogLevel::Fatal:
            levelStr = "FATAL";
            colorCode = "\033[35m"; // Пурпурный
            break;
        }

        // Выводим в консоль
        std::cout << colorCode
            << "[" << levelStr << "] "
            << message
            << "\033[0m\n"; // Сброс цвета
    }
}