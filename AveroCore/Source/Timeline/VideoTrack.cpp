#include "Timeline/VideoTrack.h"
#include <algorithm>

namespace Avero::Timeline
{
    void VideoTrack::AddClip(std::shared_ptr<VideoClip> clip)
    {
        if (!clip || !clip->IsValid()) return;
        Clips.push_back(std::move(clip));
        SortClips();
    }

    void VideoTrack::RemoveClip(std::shared_ptr<VideoClip> clip)
    {
        Clips.erase(std::remove(Clips.begin(), Clips.end(), clip), Clips.end());
    }

    std::shared_ptr<VideoClip> VideoTrack::FindClipAt(Core::Time time) const
    {
        if (!Enabled) return nullptr;
        // Later clips win, matching the normal top-most timeline convention.
        for (auto it = Clips.rbegin(); it != Clips.rend(); ++it)
            if (*it && (*it)->IsActive(time)) return *it;
        return nullptr;
    }

    void VideoTrack::SortClips()
    {
        std::stable_sort(Clips.begin(), Clips.end(), [](const auto& a, const auto& b) {
            return a && b ? a->StartTime < b->StartTime : static_cast<bool>(b);
        });
    }
}
