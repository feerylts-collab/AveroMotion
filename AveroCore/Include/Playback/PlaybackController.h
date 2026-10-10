#pragma once

#include "Core/Time.h"
#include "Timeline/Timeline.h"
#include <memory>

namespace Avero::Playback
{
    enum class PlaybackState : unsigned char { Stopped, Playing, Paused, Seeking };

    class PlaybackController
    {
    public:
        void SetTimeline(std::shared_ptr<Timeline::Timeline> timeline);
        std::shared_ptr<Timeline::Timeline> GetTimeline() const { return m_timeline; }

        void Play();
        void Pause();
        void Stop();
        void Seek(Core::Time time);
        void Tick(double deltaSeconds);

        PlaybackState GetState() const { return m_state; }
        Core::Time GetTime() const;
        void SetLooping(bool enabled) { m_looping = enabled; }
        bool IsLooping() const { return m_looping; }
        void SetSpeed(double speed);
        double GetSpeed() const { return m_speed; }

    private:
        std::shared_ptr<Timeline::Timeline> m_timeline;
        PlaybackState m_state = PlaybackState::Stopped;
        bool m_looping = true;
        double m_speed = 1.0;
    };
}
