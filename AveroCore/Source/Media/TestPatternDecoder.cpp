#include "Media/TestPatternDecoder.h"
#include <algorithm>
#include <cmath>

namespace Avero::Media
{
    bool TestPatternDecoder::CanOpen(const std::string& filePath) const
    {
        return filePath.rfind("test://", 0) == 0;
    }

    MediaOpenResult TestPatternDecoder::Open(const std::string& filePath)
    {
        if (!CanOpen(filePath)) return MediaOpenResult::UnsupportedFormat;
        m_info = {};
        m_info.Type = MediaType::Video;
        m_info.Resolution = Math::Vector2(640.0f, 360.0f);
        m_info.FrameRate = 30.0f;
        m_info.Duration = Core::Time::FromSeconds(10.0);
        m_info.FilePath = filePath;
        m_info.FileName = filePath;
        m_open = true;
        m_currentTime = Core::Time::FromTicks(0);
        return MediaOpenResult::Success;
    }

    void TestPatternDecoder::Close() { m_open = false; }

    bool TestPatternDecoder::Seek(Core::Time time)
    {
        if (!m_open) return false;
        if (time < Core::Time::FromTicks(0)) time = Core::Time::FromTicks(0);
        if (time > m_info.Duration) time = m_info.Duration;
        m_currentTime = time;
        return true;
    }

    bool TestPatternDecoder::ReadVideoFrame(VideoFrame& outFrame)
    {
        return DecodeVideoFrame(m_currentTime, outFrame);
    }

    bool TestPatternDecoder::ReadAudio(AudioSamples& outSamples, Core::Time start, Core::Time end)
    {
        if (!m_open || end <= start) return false;
        outSamples = {};
        outSamples.StartTime = start;
        outSamples.EndTime = end;
        outSamples.SampleRate = 48000;
        outSamples.Channels = 2;
        const double seconds = std::max(0.0, (end - start).ToSeconds());
        const size_t frames = static_cast<size_t>(std::min(seconds * outSamples.SampleRate, 48000.0));
        outSamples.Samples.assign(frames * 2, 0.0f);
        return true;
    }

    bool TestPatternDecoder::DecodeVideoFrame(Core::Time time, VideoFrame& outFrame)
    {
        if (!m_open || time < Core::Time::FromTicks(0) || time > m_info.Duration) return false;
        const uint32_t w = static_cast<uint32_t>(m_info.Resolution.X);
        const uint32_t h = static_cast<uint32_t>(m_info.Resolution.Y);
        outFrame = {};
        outFrame.Time = time;
        outFrame.FrameNumber = static_cast<uint64_t>(std::max<int64_t>(0, time.ToFrames(m_info.FrameRate)));
        outFrame.Width = w;
        outFrame.Height = h;
        outFrame.Format = PixelFormat::RGBA8;
        outFrame.PixelData.resize(static_cast<size_t>(w) * h * 4);
        const uint8_t r = static_cast<uint8_t>(outFrame.FrameNumber % 256);
        for (size_t i = 0; i < outFrame.PixelData.size(); i += 4)
        {
            outFrame.PixelData[i + 0] = r;
            outFrame.PixelData[i + 1] = static_cast<uint8_t>((i / 4) % 256);
            outFrame.PixelData[i + 2] = 96;
            outFrame.PixelData[i + 3] = 255;
        }
        m_currentTime = time;
        return true;
    }
}
