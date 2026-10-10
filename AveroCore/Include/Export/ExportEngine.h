#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Composition/Composition.h"
#include "Render/RenderEngine.h"

#include <string>
#include <functional>

namespace Avero::Export
{
    using Avero::Core::float32;
    using Avero::Core::uint32;
    using Avero::Core::int64;

    enum class ExportFormat : uint32
    {
        MP4_H264 = 0,
        MOV_ProRes,
        PNG_Sequence,
        JPEG_Sequence
    };

    struct ExportSettings
    {
        ExportFormat Format = ExportFormat::MP4_H264;
        uint32 Width = 1920;
        uint32 Height = 1080;
        float32 FrameRate = 30.0f;
        uint32 Bitrate = 10000000;
        bool UseGPU = true;
    };

    class ExportEngine
    {
    public:
        ExportEngine() = default;
        ~ExportEngine() = default;

        using FrameCallback = std::function<void(uint32 frameIndex, Core::Time time)>;

        bool StartExport(std::shared_ptr<Composition::Composition> comp, const ExportSettings& settings, const std::string& outputPath)
        {
            if (!comp) return false;

            m_composition = comp;
            m_outputPath = outputPath;
            m_settings = settings;
            m_totalFrames = static_cast<uint32>(comp->Duration.ToSeconds() * settings.FrameRate);
            m_currentFrame = 0;

            return true;
        }

        void ProcessNextFrame()
        {
            if (m_currentFrame >= m_totalFrames) return;

            Core::Time currentTime = Core::Time::FromFrames(
                static_cast<int64>(m_currentFrame),
                static_cast<double>(m_settings.FrameRate)
            );

            if (m_composition)
            {
                m_composition->Evaluate(currentTime);
            }

            if (m_onFrameProcessed)
            {
                m_onFrameProcessed(m_currentFrame, currentTime);
            }

            m_currentFrame++;
        }

        bool IsExportFinished() const
        {
            return m_currentFrame >= m_totalFrames;
        }

        float32 GetProgress() const
        {
            if (m_totalFrames == 0) return 0.0f;
            return static_cast<float32>(m_currentFrame) / static_cast<float32>(m_totalFrames);
        }

        void SetComposition(std::shared_ptr<Composition::Composition> comp) { m_composition = comp; }
        void SetFrameCallback(FrameCallback callback) { m_onFrameProcessed = callback; }

    private:
        std::shared_ptr<Composition::Composition> m_composition;
        ExportSettings m_settings;
        std::string m_outputPath;

        uint32 m_totalFrames = 0;
        uint32 m_currentFrame = 0;

        FrameCallback m_onFrameProcessed;
    };
}