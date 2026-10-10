#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Layer/LayerTypes.h"

#include <string>
#include <memory>

namespace Avero::Mask
{
    using Avero::Core::uint64;
    using Avero::Core::uint8;
    using Avero::Core::float32;

    enum class MaskMode : uint8
    {
        Add = 0,
        Subtract,
        Intersect,
        Difference
    };

    class Mask
    {
    public:
        uint64 ID = 0;
        std::string Name;
        MaskMode Mode = MaskMode::Add;
        float32 Opacity = 1.0f;
        bool Inverted = false;
        bool Enabled = true;

        Mask() = default;
        virtual ~Mask() = default;
    };
}