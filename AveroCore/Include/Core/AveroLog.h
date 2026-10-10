#pragma once

#include "Core/AveroTypes.h"
#include <string>
#include <string_view>

namespace Avero::Core
{
    // Уровни логгирования
    enum class LogLevel : uint8
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5
    };

    // Класс логгера
    class Logger
    {
    public:
        // Инициализация логгера
        static void Initialize();

        // Завершение работы логгера
        static void Shutdown();

        // Установка минимального уровня логгирования
        static void SetMinLevel(LogLevel level);

        // Функции логгирования
        static void Trace(std::string_view message);
        static void Debug(std::string_view message);
        static void Info(std::string_view message);
        static void Warning(std::string_view message);
        static void Error(std::string_view message);
        static void Fatal(std::string_view message);

        // Универсальная функция логгирования
        static void Log(LogLevel level, std::string_view message);

    private:
        static LogLevel s_MinLevel;
        static bool s_Initialized;
    };

    // Макросы для удобного логгирования
#define AVERO_LOG_TRACE(msg) ::Avero::Core::Logger::Trace(msg)
#define AVERO_LOG_DEBUG(msg) ::Avero::Core::Logger::Debug(msg)
#define AVERO_LOG_INFO(msg) ::Avero::Core::Logger::Info(msg)
#define AVERO_LOG_WARNING(msg) ::Avero::Core::Logger::Warning(msg)
#define AVERO_LOG_ERROR(msg) ::Avero::Core::Logger::Error(msg)
#define AVERO_LOG_FATAL(msg) ::Avero::Core::Logger::Fatal(msg)
}