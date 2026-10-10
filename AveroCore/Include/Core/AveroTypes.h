#pragma once

#include <cstdint>
#include <cstddef>

namespace Avero::Core
{
    // Фиксированные целочисленные типы
    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;

    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    // Типы с плавающей точкой
    using float32 = float;
    using float64 = double;

    // Размерные типы
    using size_t = std::size_t;
    using ptrdiff_t = std::ptrdiff_t;

    // Константы
    constexpr float32 PI = 3.14159265358979323846f;
    constexpr float32 TWO_PI = 6.28318530717958647692f;
    constexpr float32 HALF_PI = 1.57079632679489661923f;
    constexpr float32 EPSILON = 1e-6f;

    // Утилиты для работы с типами
    template<typename T>
    constexpr T Clamp(T value, T min, T max)
    {
        return value < min ? min : (value > max ? max : value);
    }

    template<typename T>
    constexpr T Min(T a, T b)
    {
        return a < b ? a : b;
    }

    template<typename T>
    constexpr T Max(T a, T b)
    {
        return a > b ? a : b;
    }
}