#pragma once

#include "Timeline/VideoClip.h"
#include <memory>
#include <vector>

namespace Avero::Timeline
{
    class VideoTrack
    {
    public:
        Core::uint64 ID = 0;
        std::string Name;
        bool Enabled = true;
        std::vector<std::shared_ptr<VideoClip>> Clips;

        void AddClip(std::shared_ptr<VideoClip> clip);
        void RemoveClip(std::shared_ptr<VideoClip> clip);
        std::shared_ptr<VideoClip> FindClipAt(Core::Time time) const;
        void SortClips();
    };
}
