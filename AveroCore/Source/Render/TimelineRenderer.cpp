#include "Render/TimelineRenderer.h"

namespace Avero::Render
{
    bool TimelineRenderer::RenderFrame(
        Media::MediaEngine& media,
        const Timeline::Timeline& timeline,
        Core::Time time,
        RenderTarget& target) const
    {
        if (!target.IsValid())
            return false;

        bool rendered = false;

        // Higher track indices are treated as being visually above lower ones.
        // The current CPU preview target is a single RGBA8 surface, so the
        // last successfully rendered clip becomes the visible result.
        for (auto it = timeline.VideoTracks.rbegin();
             it != timeline.VideoTracks.rend(); ++it)
        {
            const auto& track = *it;
            if (!track || !track->Enabled)
                continue;

            const auto clip = track->FindClipAt(time);
            if (!clip)
                continue;

            if (m_videoPreview.RenderClip(media, *clip, time, target))
                rendered = true;
        }

        return rendered;
    }
}
