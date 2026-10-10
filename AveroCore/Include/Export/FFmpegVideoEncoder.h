#pragma once

#include "Core/AveroTypes.h"
#include "Media/MediaAsset.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Avero::Export
{
    enum class VideoEncoderCodec : uint32_t
    {
        H264,
        H265,
        AV1,
        ProRes
    };

    struct VideoEncoderSettings
    {
        VideoEncoderCodec Codec = VideoEncoderCodec::H264;
        uint32_t Width = 1920;
        uint32_t Height = 1080;
        double FrameRate = 30.0;
        uint32_t Bitrate = 12'000'000;
        int Quality = 18;
        bool PreferHardware = true;
    };

    // Streams RGBA frames directly into FFmpeg. Hardware Media Foundation encoders are
    // preferred on Windows; software encoders remain available as a deterministic fallback.
    class FFmpegVideoEncoder final
    {
    public:
        FFmpegVideoEncoder() = default;
        ~FFmpegVideoEncoder();

        bool Start(const std::string& outputPath, const VideoEncoderSettings& settings);
        bool WriteFrame(const Media::VideoFrame& frame);
        bool Finish();
        bool IsRunning() const { return m_running; }

        static bool IsRuntimeAvailable();

    private:
        static std::string Quote(const std::string& value);
        static std::string FindFFmpeg();
        std::string BuildCommand(const std::string& outputPath) const;
        void CloseProcess();

#ifdef _WIN32
        void* m_process = nullptr;
        void* m_stdinWrite = nullptr;
#endif
        VideoEncoderSettings m_settings;
        std::string m_outputPath;
        bool m_running = false;
    };
}
