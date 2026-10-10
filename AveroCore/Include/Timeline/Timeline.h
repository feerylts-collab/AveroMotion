#pragma once

#include "Core/Time.h"
#include "Composition/Composition.h"
#include "Timeline/VideoTrack.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace Avero::Timeline
{
    class Timeline
    {
    public:
        std::shared_ptr<Composition::Composition> ActiveComposition;
        std::vector<std::shared_ptr<VideoTrack>> VideoTracks;
        Core::Time CurrentTime;
        bool IsPlaying = false;
        float PlaybackSpeed = 1.0f;

        void SetComposition(std::shared_ptr<Composition::Composition> comp)
        {
            ActiveComposition = comp;
            CurrentTime = Core::Time::FromSeconds(0.0);
        }

        std::shared_ptr<VideoTrack> AddVideoTrack()
        {
            auto track = std::make_shared<VideoTrack>();
            VideoTracks.push_back(track);
            return track;
        }

        // Allows callers/tests to create a track themselves and add it to the timeline.
        void AddVideoTrack(const std::shared_ptr<VideoTrack>& track)
        {
            if (!track) return;
            const auto it = std::find(VideoTracks.begin(), VideoTracks.end(), track);
            if (it == VideoTracks.end())
                VideoTracks.push_back(track);
        }

        bool RemoveVideoTrack(const std::shared_ptr<VideoTrack>& track)
        {
            const auto it = std::find(VideoTracks.begin(), VideoTracks.end(), track);
            if (it == VideoTracks.end()) return false;
            VideoTracks.erase(it);
            return true;
        }

        void Play() { IsPlaying = true; }
        void Pause() { IsPlaying = false; }

        void Seek(Core::Time time)
        {
            CurrentTime = time;
            if (ActiveComposition) ActiveComposition->Evaluate(CurrentTime);
        }

        void Tick(double deltaTime)
        {
            if (!IsPlaying || !ActiveComposition) return;
            CurrentTime = CurrentTime + Core::Time::FromSeconds(deltaTime * PlaybackSpeed);
            if (CurrentTime >= ActiveComposition->Duration)
            {
                CurrentTime = ActiveComposition->Duration;
                IsPlaying = false;
            }
            ActiveComposition->Evaluate(CurrentTime);
        }
    };
}
