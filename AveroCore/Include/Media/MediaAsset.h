#pragma once

#include "Core/AveroTypes.h"
#include "Media/MediaInfo.h"
#include "Core/Time.h"
#include <string>
#include <memory>
#include <vector>

namespace Avero::Media
{
    using Avero::Core::uint64;
    using Avero::Core::uint32;
    using Avero::Core::int32;

    struct VideoFrame
    {
        uint64 FrameNumber = 0;
        Core::Time Time;
        std::vector<uint8_t> PixelData;
        uint32 Width = 0;
        uint32 Height = 0;
        PixelFormat Format = PixelFormat::RGBA8;

        bool IsValid() const
        {
            return Width > 0 && Height > 0 && !PixelData.empty();
        }
    };

    struct AudioSamples
    {
        Core::Time StartTime;
        Core::Time EndTime;
        std::vector<float> Samples;
        int32 SampleRate = 0;
        int32 Channels = 0;

        bool IsValid() const
        {
            return SampleRate > 0 && Channels > 0 && !Samples.empty();
        }
    };

    class MediaAsset
    {
    public:
        uint64 ID = 0;
        MediaInfo Info;
        bool IsLoaded = false;

        MediaAsset() = default;
        virtual ~MediaAsset() = default;
    };
}
