#include "Media/MediaEngine.h"
#include "Media/TestPatternDecoder.h"
#include "Render/TimelineRenderer.h"
#include "Render/RenderTarget.h"
#include "Timeline/Timeline.h"
#include "Timeline/VideoTrack.h"
#include "Timeline/VideoClip.h"
#include "Composition/Composition.h"

#include <iostream>
#include <memory>

namespace
{
    bool Check(bool condition, const char* name)
    {
        if (!condition)
            std::cerr << "[FAIL] " << name << '\n';
        else
            std::cout << "[PASS] " << name << '\n';
        return condition;
    }
}

int RunTimelinePreviewSmokeTest()
{
    using namespace Avero;

    Media::MediaEngine media;
    media.Initialize();
    media.RegisterDecoder([] { return std::make_unique<Media::TestPatternDecoder>(); });

    auto asset = media.LoadMedia(100, "test://timeline-preview");
    if (!Check(asset && asset->IsLoaded, "Preview asset load"))
        return 1;

    auto composition = std::make_shared<Composition::Composition>();
    composition->Duration = Core::Time::FromSeconds(10.0);

    Timeline::Timeline timeline;
    timeline.SetComposition(composition);

    auto track = std::make_shared<Timeline::VideoTrack>();
    track->ID = 1;

    auto clip = std::make_shared<Timeline::VideoClip>();
    clip->ID = 1;
    clip->Asset = asset;
    clip->StartTime = Core::Time::FromSeconds(2.0);
    clip->Duration = Core::Time::FromSeconds(4.0);
    clip->SourceIn = Core::Time::FromSeconds(1.0);
    clip->SourceOut = Core::Time::FromSeconds(5.0);
    clip->PlaybackRate = 1.0;

    track->AddClip(clip);
    timeline.AddVideoTrack(track);

    Render::RenderTarget target;
    target.Allocate(320, 180);

    Render::TimelineRenderer renderer;

    bool ok = true;
    ok &= Check(
        renderer.RenderFrame(
            media,
            timeline,
            Core::Time::FromSeconds(3.0),
            target),
        "Timeline preview render");

    ok &= Check(
        !target.ColorAttachment->PixelData.empty(),
        "Preview target contains pixels");

    ok &= Check(
        !renderer.RenderFrame(
            media,
            timeline,
            Core::Time::FromSeconds(8.0),
            target),
        "Timeline preview empty frame");

    media.Shutdown();
    return ok ? 0 : 1;
}
