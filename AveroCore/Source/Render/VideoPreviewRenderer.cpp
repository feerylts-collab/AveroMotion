#include "Render/VideoPreviewRenderer.h"

#include "Media/MediaEngine.h"
#include "Media/MediaAsset.h"
#include "Render/RenderTarget.h"
#include "Render/Texture.h"

#include <algorithm>
#include <cstring>

namespace Avero::Render
{
    bool VideoPreviewRenderer::RenderAssetFrame(
        Media::MediaEngine& mediaEngine,
        Avero::Core::uint64 assetID,
        Avero::Core::Time time,
        RenderTarget& target) const
    {
        auto asset = mediaEngine.GetAsset(assetID);

        if (!asset)
            return false;

        if (!target.ColorAttachment)
            return false;

        Media::VideoFrame frame;

        if (!mediaEngine.DecodeVideoFrame(assetID, time, frame))
            return false;

        if (frame.Width == 0 || frame.Height == 0)
            return false;

        if (frame.PixelData.empty())
            return false;

        return CopyFrameFit(frame, target);
    }

    bool VideoPreviewRenderer::RenderClip(
        Media::MediaEngine& mediaEngine,
        const Timeline::VideoClip& clip,
        Core::Time timelineTime,
        RenderTarget& target) const
    {
        if (!clip.Enabled || !clip.Asset || !clip.IsActive(timelineTime))
        {
            Clear(target);
            return false;
        }
        return RenderAssetFrame(mediaEngine, clip.Asset->ID,
            clip.TimelineToSource(timelineTime), target);
    }

    void VideoPreviewRenderer::Clear(RenderTarget& target)
    {
        if (!target.ColorAttachment) return;
        std::fill(target.ColorAttachment->PixelData.begin(),
                  target.ColorAttachment->PixelData.end(), static_cast<uint8_t>(0));
    }

    bool VideoPreviewRenderer::CopyFrameFit(const Media::VideoFrame& frame, RenderTarget& target)
    {
        if (!frame.IsValid() || !target.ColorAttachment) return false;
        const auto tw = static_cast<uint32_t>(target.Size.X);
        const auto th = static_cast<uint32_t>(target.Size.Y);
        if (!tw || !th) return false;
        const uint64_t srcBytes = static_cast<uint64_t>(frame.Width) * frame.Height * 4;
        if (srcBytes > frame.PixelData.size()) return false;
        target.ColorAttachment->Allocate(tw, th, TextureFormat::RGBA8);
        auto& dst = target.ColorAttachment->PixelData;
        const uint64_t dstBytes = static_cast<uint64_t>(tw) * th * 4;
        if (dstBytes > dst.size()) return false;
        std::fill(dst.begin(), dst.end(), static_cast<uint8_t>(0));
        const double scale = std::min(static_cast<double>(tw) / frame.Width,
                                      static_cast<double>(th) / frame.Height);
        const uint32_t rw = std::max<uint32_t>(1, static_cast<uint32_t>(frame.Width * scale));
        const uint32_t rh = std::max<uint32_t>(1, static_cast<uint32_t>(frame.Height * scale));
        const uint32_t ox = (tw - rw) / 2, oy = (th - rh) / 2;
        for (uint32_t y = 0; y < rh; ++y) {
            const uint32_t sy = std::min<uint32_t>(frame.Height - 1,
                static_cast<uint32_t>((static_cast<uint64_t>(y) * frame.Height) / rh));
            for (uint32_t x = 0; x < rw; ++x) {
                const uint32_t sx = std::min<uint32_t>(frame.Width - 1,
                    static_cast<uint32_t>((static_cast<uint64_t>(x) * frame.Width) / rw));
                const size_t si = (static_cast<size_t>(sy) * frame.Width + sx) * 4;
                const size_t di = (static_cast<size_t>(oy + y) * tw + (ox + x)) * 4;
                std::memcpy(dst.data() + di, frame.PixelData.data() + si, 4);
            }
        }
        return true;
    }

}