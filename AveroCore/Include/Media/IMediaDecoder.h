#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Media/MediaAsset.h"
#include "Media/MediaInfo.h"
#include "Media/MediaTypes.h"
#include <memory>
#include <string>

namespace Avero::Media
{
    using Avero::Core::uint32;

    class IMediaDecoder
    {
    public:
        virtual ~IMediaDecoder() = default;

        virtual bool CanOpen(const std::string& filePath) const = 0;
        virtual MediaOpenResult Open(const std::string& filePath) = 0;
        virtual void Close() = 0;
        virtual MediaInfo GetMediaInfo() const = 0;

        virtual bool Seek(Core::Time time) = 0;
        virtual bool ReadVideoFrame(VideoFrame& outFrame) = 0;
        virtual bool ReadAudio(AudioSamples& outSamples, Core::Time start, Core::Time end) = 0;
        virtual bool DecodeVideoFrame(Core::Time time, VideoFrame& outFrame) = 0;

        virtual bool GenerateThumbnail(Core::Time time, uint32 width, uint32 height, VideoFrame& outFrame)
        {
            (void)time; (void)width; (void)height; (void)outFrame;
            return false;
        }
    };
}
