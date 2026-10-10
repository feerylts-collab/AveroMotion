#pragma once

#include "Core/AveroTypes.h"

namespace Avero::Media
{
    using Avero::Core::uint8;

    enum class MediaType : uint8 { Unknown = 0, Video, Image, Audio };
    enum class VideoCodec : uint8 { Unknown = 0, H264, H265, VP9, AV1, ProRes };
    enum class AudioCodec : uint8 { Unknown = 0, AAC, MP3, WAV, FLAC };
    enum class ImageFormat : uint8 { Unknown = 0, PNG, JPEG, BMP, TIFF, WebP };
    enum class PixelFormat : uint8 { Unknown = 0, RGBA8, BGRA8, RGB8, Gray8 };

    enum class MediaOpenResult : uint8
    {
        Success = 0,
        InvalidPath,
        UnsupportedFormat,
        DecoderUnavailable,
        OpenFailed,
        InvalidAsset
    };
}
