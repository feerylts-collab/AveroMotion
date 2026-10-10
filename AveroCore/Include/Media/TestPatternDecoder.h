#pragma once

#include "Media/IMediaDecoder.h"

namespace Avero::Media
{
    // Deterministic decoder used by Core tests and host smoke tests.
    // It does not read real media files; production builds should register FFmpeg or another backend.
    class TestPatternDecoder final : public IMediaDecoder
    {
    public:
        bool CanOpen(const std::string& filePath) const override;
        MediaOpenResult Open(const std::string& filePath) override;
        void Close() override;
        MediaInfo GetMediaInfo() const override { return m_info; }
        bool Seek(Core::Time time) override;
        bool ReadVideoFrame(VideoFrame& outFrame) override;
        bool ReadAudio(AudioSamples& outSamples, Core::Time start, Core::Time end) override;
        bool DecodeVideoFrame(Core::Time time, VideoFrame& outFrame) override;

    private:
        MediaInfo m_info;
        Core::Time m_currentTime;
        bool m_open = false;
    };
}
