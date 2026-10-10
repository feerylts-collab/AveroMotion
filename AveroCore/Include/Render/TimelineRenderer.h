#pragma once

#include "Core/Time.h"
#include "Media/MediaEngine.h"
#include "Render/RenderTarget.h"
#include "Timeline/Timeline.h"
#include "Render/VideoPreviewRenderer.h"

namespace Avero::Render
{
    class TimelineRenderer
    {
    public:
        bool RenderFrame(
            Media::MediaEngine& media,
            const Timeline::Timeline& timeline,
            Core::Time time,
            RenderTarget& target) const;

    private:
        VideoPreviewRenderer m_videoPreview;
    };
}
