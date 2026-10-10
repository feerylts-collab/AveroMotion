#include <cmath>
#include <iostream>
#include <memory>

#include "Core/AveroCore.h"
#include "Media/TestPatternDecoder.h"

using namespace Avero;

namespace
{
    bool Check(bool condition, const char* name)
    {
        if (!condition) std::cerr << "[FAIL] " << name << '\n';
        else std::cout << "[PASS] " << name << '\n';
        return condition;
    }
}

int main()
{
    Core::InitializeEngine();
    const auto info = Core::GetEngineInfo();
    std::cout << "[INFO] " << info.EngineName << " " << info.Version << '\n';

    bool ok = true;

    auto media = std::make_shared<Media::MediaEngine>();
    media->Initialize();
    media->RegisterDecoder([] { return std::make_unique<Media::TestPatternDecoder>(); });

    ok &= Check(media->HasDecoderFor("test://sample"), "Media decoder registration");
    auto asset = media->LoadMedia(1, "test://sample");
    ok &= Check(asset && asset->IsLoaded, "Media asset load");

    Media::FrameCache cache(4);
    Media::VideoFrame frame;
    ok &= Check(media->DecodeVideoFrame(1, Core::Time::FromSeconds(1.0), frame) && frame.IsValid(), "Video frame decode");
    cache.Put(1, frame.Time, std::make_shared<Media::VideoFrame>(frame));
    ok &= Check(cache.Find(1, frame.Time) != nullptr, "Frame cache hit");

    auto clip = std::make_shared<Timeline::VideoClip>();
    clip->ID = 10;
    clip->Asset = asset;
    clip->StartTime = Core::Time::FromSeconds(2.0);
    clip->Duration = Core::Time::FromSeconds(4.0);
    clip->SourceIn = Core::Time::FromSeconds(1.0);
    clip->SourceOut = Core::Time::FromSeconds(5.0);
    clip->PlaybackRate = 1.0;

    auto track = std::make_shared<Timeline::VideoTrack>();
    track->AddClip(clip);
    ok &= Check(track->FindClipAt(Core::Time::FromSeconds(3.0)) == clip, "Timeline clip lookup");
    ok &= Check(clip->TimelineToSource(Core::Time::FromSeconds(3.0)) == Core::Time::FromSeconds(2.0), "Timeline source mapping");

    auto timeline = std::make_shared<Timeline::Timeline>();
    timeline->SetComposition(std::make_shared<Composition::Composition>());
    timeline->ActiveComposition->Duration = Core::Time::FromSeconds(5.0);
    Playback::PlaybackController playback;
    playback.SetTimeline(timeline);
    playback.SetSpeed(1.0);
    playback.Play();
    playback.Tick(0.5);
    ok &= Check(playback.GetState() == Playback::PlaybackState::Playing && playback.GetTime() > Core::Time::FromTicks(0), "Playback tick");
    playback.Stop();
    ok &= Check(playback.GetTime() == Core::Time::FromTicks(0), "Playback stop");

    media->Shutdown();
    Core::ShutdownEngine();
    std::cout << (ok ? "[INFO] Media foundation smoke test passed\n" : "[ERROR] Media foundation smoke test failed\n");
    return ok ? 0 : 1;
}
