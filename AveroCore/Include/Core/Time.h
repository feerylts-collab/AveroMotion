#pragma once

#include "Core/AveroTypes.h"
#include <limits>
#include <cmath>

namespace Avero::Core
{
    using Avero::Core::int64;
    using Avero::Core::float64;

    struct Time
    {
        static constexpr int64 TicksPerSecond = 10'000'000;
        static constexpr int64 TicksPerMillisecond = 10'000;
        static constexpr int64 TicksPerMicrosecond = 10;

        static constexpr int64 MinTicks = std::numeric_limits<int64>::min() / 2;
        static constexpr int64 MaxTicks = std::numeric_limits<int64>::max() / 2;

        int64 Ticks = 0;

        constexpr Time() = default;
        explicit constexpr Time(int64 ticks) : Ticks(ClampTicks(ticks)) {}

        static constexpr Time FromTicks(int64 ticks) { return Time(ticks); }

        static Time FromSeconds(float64 seconds)
        {
            return FromDoubleTicks(seconds, static_cast<float64>(TicksPerSecond));
        }

        static Time FromMilliseconds(float64 ms)
        {
            return FromDoubleTicks(ms, static_cast<float64>(TicksPerMillisecond));
        }
static constexpr Time FromMicroseconds(int64 us)
        {
            return Time(SafeMultiply(us, TicksPerMicrosecond));
        }

        static Time FromFrames(int64 frame, float64 fps)
        {
            if (fps <= 0.0 || std::isnan(fps) || std::isinf(fps)) return Time(0);
            float64 seconds = static_cast<float64>(frame) / fps;
            return FromDoubleTicks(seconds, static_cast<float64>(TicksPerSecond));
}

        constexpr int64 ToTicks() const { return Ticks; }

        constexpr float64 ToSeconds() const
        {
            return static_cast<float64>(Ticks) / TicksPerSecond;
        }

        constexpr float64 ToMilliseconds() const
        {
            return static_cast<float64>(Ticks) / TicksPerMillisecond;
        }

        constexpr int64 ToMicroseconds() const
        {
            return Ticks / TicksPerMicrosecond;
        }

        constexpr int64 ToFrames(float64 fps) const
        {
            if (fps <= 0.0 || std::isnan(fps) || std::isinf(fps)) return 0;
            float64 seconds = ToSeconds();
            if (std::isnan(seconds) || std::isinf(seconds)) return 0;

            float64 frames = seconds * fps;
            if (!std::isfinite(frames))
                return 0;

            frames += (frames >= 0.0) ? 0.5 : -0.5;

            if (frames >= static_cast<float64>(std::numeric_limits<int64>::max()))
                return std::numeric_limits<int64>::max();
            if (frames <= static_cast<float64>(std::numeric_limits<int64>::min()))
                return std::numeric_limits<int64>::min();

            return static_cast<int64>(frames);
}

        constexpr Time operator+(const Time& other) const
        {
            return Time(SafeAdd(Ticks, other.Ticks));
        }

        constexpr Time operator-(const Time& other) const
        {
            return Time(SafeSub(Ticks, other.Ticks));
        }

        Time operator*(float64 scalar) const
        {
            return FromDoubleTicks(static_cast<float64>(Ticks), scalar);
        }

        Time operator/(float64 scalar) const
        {
            if (!std::isfinite(scalar) || scalar == 0.0) return Time(0);
            return FromDoubleTicks(static_cast<float64>(Ticks), 1.0 / scalar);
        }
constexpr bool operator==(const Time& other) const { return Ticks == other.Ticks; }
        constexpr bool operator!=(const Time& other) const { return Ticks != other.Ticks; }
        constexpr bool operator<(const Time& other) const { return Ticks < other.Ticks; }
        constexpr bool operator>(const Time& other) const { return Ticks > other.Ticks; }
        constexpr bool operator<=(const Time& other) const { return Ticks <= other.Ticks; }
        constexpr bool operator>=(const Time& other) const { return Ticks >= other.Ticks; }

    private:
        static Time FromDoubleTicks(float64 value, float64 multiplier)
        {
            if (std::isnan(value) || std::isnan(multiplier))
                return Time(0);

            const float64 ticks = value * multiplier;
            if (std::isinf(ticks))
                return Time(ticks > 0.0 ? MaxTicks : MinTicks);
            if (std::isnan(ticks))
                return Time(0);

            if (ticks >= static_cast<float64>(MaxTicks))
                return Time(MaxTicks);
            if (ticks <= static_cast<float64>(MinTicks))
                return Time(MinTicks);

            return Time(static_cast<int64>(ticks));
        }

        static constexpr int64 ClampTicks(int64 ticks)
        {
            if (ticks < MinTicks) return MinTicks;
            if (ticks > MaxTicks) return MaxTicks;
            return ticks;
        }

        static constexpr int64 SafeAdd(int64 a, int64 b)
        {
            if (b > 0 && a > MaxTicks - b) return MaxTicks;
            if (b < 0 && a < MinTicks - b) return MinTicks;
            return a + b;
        }

        static constexpr int64 SafeSub(int64 a, int64 b)
        {
            if (b < 0 && a > MaxTicks + b) return MaxTicks;
            if (b > 0 && a < MinTicks + b) return MinTicks;
            return a - b;
        }

        static constexpr int64 SafeMultiply(int64 a, int64 b)
        {
            if (a == 0 || b == 0) return 0;
            if (a > 0 && b > 0 && a > MaxTicks / b) return MaxTicks;
            if (a > 0 && b < 0 && b < MinTicks / a) return MinTicks;
            if (a < 0 && b > 0 && a < MinTicks / b) return MinTicks;
            if (a < 0 && b < 0 && a < MaxTicks / b) return MaxTicks;
            return a * b;
        }
    };

    struct Framerate
    {
        float64 Value = 30.0;

        constexpr Framerate() = default;
        explicit constexpr Framerate(float64 fps) : Value(fps) {}

        // Убрали constexpr, так как вызывают не-constexpr Time::FromFrames
        Time FrameDuration() const
        {
            return Time::FromFrames(1, Value);
        }

        Time FramesToTime(int64 frames) const
        {
            return Time::FromFrames(frames, Value);
        }

        constexpr int64 TimeToFrames(Time time) const
        {
            return time.ToFrames(Value);
        }
    };
}