#pragma once

#include "Core/AveroTypes.h"
#include <chrono>
#include <string>
#include <unordered_map>

namespace Avero::Platform
{
    using Avero::Core::float32;
    using Avero::Core::uint64;

    class PerformanceProfiler
    {
    public:
        PerformanceProfiler() = default;

        void BeginFrame()
        {
            m_frameStart = std::chrono::high_resolution_clock::now();
        }

        float32 EndFrame()
        {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float32> elapsed = end - m_frameStart;
            m_lastFrameTime = elapsed.count();
            m_frameCount++;
            return m_lastFrameTime;
        }

        float32 GetLastFrameTime() const { return m_lastFrameTime; }
        float32 GetFPS() const { return m_lastFrameTime > 0.0f ? 1.0f / m_lastFrameTime : 0.0f; }
        uint64 GetTotalFrames() const { return m_frameCount; }

        void StartTimer(const std::string& name)
        {
            m_timers[name] = std::chrono::high_resolution_clock::now();
        }

        float32 EndTimer(const std::string& name)
        {
            auto it = m_timers.find(name);
            if (it == m_timers.end()) return 0.0f;

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float32> elapsed = end - it->second;
            m_timers.erase(it);
            return elapsed.count();
        }

    private:
        std::chrono::high_resolution_clock::time_point m_frameStart;
        float32 m_lastFrameTime = 0.0f;
        uint64 m_frameCount = 0;
        std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_timers;
    };
}