#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Math/Vector2.h"
#include "Media/MediaTypes.h"

#include <string>

namespace Avero::Media
{
    using Avero::Core::uint64;
    using Avero::Core::float32;
    using Avero::Core::int32;

    struct MediaInfo
    {
        MediaType Type = MediaType::Unknown;
        std::string FilePath;
        std::string FileName;

        Core::Time Duration;
        uint64 FileSize = 0;

        Math::Vector2 Resolution;
        float32 FrameRate = 0.0f;
        VideoCodec VideoCodecType = VideoCodec::Unknown;

        int32 SampleRate = 0;
        int32 Channels = 0;
        AudioCodec AudioCodecType = AudioCodec::Unknown;

        ImageFormat ImageFormatType = ImageFormat::Unknown;

        MediaInfo() = default;
    };
}