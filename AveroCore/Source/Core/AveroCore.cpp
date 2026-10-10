#include "Core/AveroCore.h"
#include <iostream>

namespace Avero::Core
{
    void InitializeEngine()
    {
        // Инициализируем логгер
        Logger::Initialize();

        AVERO_LOG_INFO("=== AveroMotion Engine Initializing ===");

        auto info = GetEngineInfo();
        AVERO_LOG_INFO("Engine: " + std::string(info.EngineName));
        AVERO_LOG_INFO("Version: " + std::string(info.Version));

        AVERO_LOG_INFO("=== Engine Initialized Successfully ===");
    }

    void ShutdownEngine()
    {
        AVERO_LOG_INFO("=== Engine Shutting Down ===");

        // Завершаем работу логгера
        Logger::Shutdown();
    }

    EngineInfo GetEngineInfo()
    {
        return EngineInfo{};
    }
}