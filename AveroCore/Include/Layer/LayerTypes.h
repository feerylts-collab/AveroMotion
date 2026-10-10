#pragma once

#include "Core/AveroTypes.h"

namespace Avero::Layer
{
    using Avero::Core::uint8;

    enum class LayerType : uint8
    {
        Null = 0,
        Video,
        Image,
        Text,
        Shape,
        Audio,
        Adjustment,
        Camera,
        Light
    };

    enum class BlendMode : uint8
    {
        Normal = 0,
        Multiply,
        Screen,
        Overlay,
        Darken,
        Lighten,
        ColorDodge,
        ColorBurn,
        HardLight,
        SoftLight,
        Difference,
        Exclusion
    };
}