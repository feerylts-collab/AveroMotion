#include "Core/AveroCore.h"
#include "Render/VideoPreviewRenderer.h"
#include "Timeline/VideoClip.h"

#include <memory>

// Optional smoke test for the next editor milestone.
// Call this from AveroHost only after the new files are added to AveroCore.
int RunVideoPreviewSmokeTest()
{
    using namespace Avero;

    Media::MediaEngine media;
    auto asset = media.LoadMedia(1, "test_pattern.avero");
    if (!asset)
        return 1;

    auto target = std::make_shared<Render::RenderTarget>();
    target->Allocate(1280, 720);

    Render::VideoPreviewRenderer preview;
    if (!preview.RenderAssetFrame(
        media,
        asset->ID,
        Core::Time::FromSeconds(0.0),
        *target))
    {
        return 2;
    }

    return 0;
}
