#include "Playback/PlaybackController.h"
#include <algorithm>
#include <cmath>

namespace Avero::Playback
{
    void PlaybackController::SetTimeline(std::shared_ptr<Timeline::Timeline> timeline)
    {
        m_timeline = std::move(timeline);
        m_state = PlaybackState::Stopped;
        if (m_timeline) m_timeline->Pause();
    }

    void PlaybackController::Play()
    {
        if (!m_timeline) return;
        m_timeline->PlaybackSpeed = static_cast<float>(m_speed);
        m_timeline->Play();
        m_state = PlaybackState::Playing;
    }

    void PlaybackController::Pause()
    {
        if (!m_timeline) return;
        m_timeline->Pause();
        m_state = PlaybackState::Paused;
    }

    void PlaybackController::Stop()
    {
        if (!m_timeline) return;
        m_timeline->Pause();
        m_timeline->Seek(Core::Time::FromTicks(0));
        m_state = PlaybackState::Stopped;
    }

    void PlaybackController::Seek(Core::Time time)
    {
        if (!m_timeline) return;
        m_state = PlaybackState::Seeking;
        if (auto comp = m_timeline->ActiveComposition)
        {
            if (time < Core::Time::FromTicks(0)) time = Core::Time::FromTicks(0);
            if (time > comp->Duration) time = comp->Duration;
        }
        m_timeline->Seek(time);
        m_state = PlaybackState::Paused;
    }

    void PlaybackController::Tick(double deltaSeconds)
    {
        if (!m_timeline || m_state != PlaybackState::Playing || !std::isfinite(deltaSeconds) || deltaSeconds <= 0.0)
            return;

        m_timeline->PlaybackSpeed = static_cast<float>(m_speed);
        m_timeline->Tick(deltaSeconds);

        if (m_timeline->ActiveComposition && m_timeline->CurrentTime >= m_timeline->ActiveComposition->Duration)
        {
            if (m_looping)
            {
                m_timeline->Seek(Core::Time::FromTicks(0));
                m_timeline->Play();
                m_state = PlaybackState::Playing;
            }
            else
            {
                m_state = PlaybackState::Paused;
            }
        }
    }

    Core::Time PlaybackController::GetTime() const
    {
        return m_timeline ? m_timeline->CurrentTime : Core::Time::FromTicks(0);
    }

    void PlaybackController::SetSpeed(double speed)
    {
        if (!std::isfinite(speed)) return;
        m_speed = std::clamp(speed, 0.01, 16.0);
        if (m_timeline) m_timeline->PlaybackSpeed = static_cast<float>(m_speed);
    }
}
