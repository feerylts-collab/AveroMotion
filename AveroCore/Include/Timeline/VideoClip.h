#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Media/MediaAsset.h"
#include <memory>
#include <string>

namespace Avero::Timeline
{
    class VideoClip
    {
    public:
        Core::uint64 ID = 0;
        std::string Name;
        std::shared_ptr<Media::MediaAsset> Asset;

        Core::Time StartTime;
        Core::Time Duration;
        Core::Time SourceIn;
        Core::Time SourceOut;
        double PlaybackRate = 1.0;
        bool Enabled = true;
        bool LoopMedia = false;

        bool IsActive(Core::Time timelineTime) const;
        Core::Time TimelineToSource(Core::Time timelineTime) const;
        Core::Time SourceDuration() const;
        bool IsValid() const;
    };
}
