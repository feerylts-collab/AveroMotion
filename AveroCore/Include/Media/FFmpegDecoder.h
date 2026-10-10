#pragma once

#include "Media/IMediaDecoder.h"
#include <memory>
#include <string>

namespace Avero::Media
{
    // Production media bridge. It uses an installed FFmpeg runtime (ffmpeg.exe/ffprobe.exe)
    // so AveroCore stays independent of FFmpeg's ABI and can ship different FFmpeg builds.
    class FFmpegDecoder final : public IMediaDecoder
    {
    public:
        FFmpegDecoder() = default;
        ~FFmpegDecoder() override;

        static bool IsRuntimeAvailable();
        static bool IsSupportedPath(const std::string& filePath);

        bool CanOpen(const std::string& filePath) const override;
        MediaOpenResult Open(const std::string& filePath) override;
        void Close() override;
        MediaInfo GetMediaInfo() const override { return m_info; }

        bool Seek(Core::Time time) override;
        bool ReadVideoFrame(VideoFrame& outFrame) override;
        bool ReadAudio(AudioSamples& outSamples, Core::Time start, Core::Time end) override;
        bool DecodeVideoFrame(Core::Time time, VideoFrame& outFrame) override;

    private:
        bool Probe();
        bool RunFFmpegFrame(double seconds, VideoFrame& outFrame);
        static std::string Quote(const std::string& value);
        static std::string FindTool(const char* name);
        static std::string RunCapture(const std::string& command, int& exitCode);
        static VideoCodec ParseVideoCodec(const std::string& value);
        static AudioCodec ParseAudioCodec(const std::string& value);
        static double ParseRate(const std::string& value);

        std::string m_filePath;
        std::string m_ffmpeg;
        std::string m_ffprobe;
        MediaInfo m_info;
        Core::Time m_position;
        bool m_open = false;
    };

    std::unique_ptr<IMediaDecoder> CreateFFmpegDecoder();
}
