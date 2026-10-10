#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Media/MediaEngine.h"
#include "Media/MediaAsset.h"
#include "Render/RenderTarget.h"
#include "Timeline/VideoClip.h"

#include <memory>

namespace Avero::Render
{
    using Avero::Core::uint64;

    // CPU-side preview renderer.
    // Converts a decoded RGBA8 video frame into a RenderTarget while preserving aspect ratio.
    // This is intentionally backend-independent so the editor can have a working preview
    // before a DirectX/OpenGL/Vulkan backend is introduced.
    class VideoPreviewRenderer
    {
    public:
        VideoPreviewRenderer() = default;

        bool RenderAssetFrame(
            Media::MediaEngine& media,
            uint64 assetId,
            Core::Time sourceTime,
            RenderTarget& target) const;

        bool RenderClip(
            Media::MediaEngine& media,
            const Timeline::VideoClip& clip,
            Core::Time timelineTime,
            RenderTarget& target) const;

    private:
        static void Clear(RenderTarget& target);
        static bool CopyFrameFit(const Media::VideoFrame& frame, RenderTarget& target);
    };
}
