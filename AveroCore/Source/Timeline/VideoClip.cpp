#include "Timeline/VideoClip.h"
#include <cmath>

namespace Avero::Timeline
{
    bool VideoClip::IsActive(Core::Time timelineTime) const
    {
        return Enabled && IsValid() && timelineTime >= StartTime && timelineTime < StartTime + Duration;
    }

    Core::Time VideoClip::SourceDuration() const
    {
        return SourceOut > SourceIn ? SourceOut - SourceIn : Core::Time::FromTicks(0);
    }

    Core::Time VideoClip::TimelineToSource(Core::Time timelineTime) const
    {
        if (!IsActive(timelineTime) || !std::isfinite(PlaybackRate) || PlaybackRate <= 0.0)
            return SourceIn;

        Core::Time local = timelineTime - StartTime;
        Core::Time source = SourceIn + local * PlaybackRate;
        const Core::Time length = SourceDuration();

        if (LoopMedia && length > Core::Time::FromTicks(0))
        {
            const auto ticks = source.Ticks - SourceIn.Ticks;
            const auto len = length.Ticks;
            const auto wrapped = ((ticks % len) + len) % len;
            return SourceIn + Core::Time::FromTicks(wrapped);
        }

        if (source < SourceIn) return SourceIn;
        if (source > SourceOut) return SourceOut;
        return source;
    }

    bool VideoClip::IsValid() const
    {
        return Asset && Duration > Core::Time::FromTicks(0) &&
               SourceOut > SourceIn && PlaybackRate > 0.0 && std::isfinite(PlaybackRate);
    }
}
