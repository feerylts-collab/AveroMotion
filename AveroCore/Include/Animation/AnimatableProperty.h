#pragma once

#include "Animation/AnimationTrack.h"
#include <memory>

namespace Avero::Animation
{
    template<typename T>
    class AnimatableProperty
    {
    public:
        AnimatableProperty() = default;

        explicit AnimatableProperty(const T& defaultValue)
            : m_defaultValue(defaultValue)
        {
        }

        // Copy constructor for deep copying tracks
        AnimatableProperty(const AnimatableProperty& other)
            : m_defaultValue(other.m_defaultValue)
        {
            if (other.m_track)
            {
                m_track = std::make_unique<AnimationTrack<T>>(*other.m_track);
            }
        }

        AnimatableProperty& operator=(const AnimatableProperty& other)
        {
            if (this != &other)
            {
                m_defaultValue = other.m_defaultValue;
                if (other.m_track)
                {
                    m_track = std::make_unique<AnimationTrack<T>>(*other.m_track);
                }
                else
                {
                    m_track.reset();
                }
            }
            return *this;
        }

        void SetDefaultValue(const T& value) { m_defaultValue = value; }
        const T& GetDefaultValue() const { return m_defaultValue; }

        AnimationTrack<T>& GetTrack()
        {
            if (!m_track)
            {
                m_track = std::make_unique<AnimationTrack<T>>();
            }
            return *m_track;
        }

        bool HasTrack() const
        {
            return m_track != nullptr && !m_track->Keyframes.empty();
        }

        T Evaluate(Core::Time time) const
        {
            if (HasTrack())
            {
                return m_track->Evaluate(time);
            }
            return m_defaultValue;
        }

    private:
        T m_defaultValue{};
        std::unique_ptr<AnimationTrack<T>> m_track;
    };
}