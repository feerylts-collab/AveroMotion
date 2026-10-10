#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector4.h"

namespace Avero::Render
{
    using Avero::Core::uint32;
    using Avero::Core::float32;

    enum class TextureFormat : uint32
    {
        RGBA8 = 0,
        RGB8,
        RG8,
        R8,
        RGBA16F,
        RGBA32F,
        Depth24Stencil8,
        Depth32F
    };

    enum class TextureFilter : uint32
    {
        Nearest = 0,
        Linear,
        Anisotropic
    };

    enum class TextureWrap : uint32
    {
        Repeat = 0,
        Clamp,
        Mirror
    };

    enum class BlendFactor : uint32
    {
        Zero = 0,
        One,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class BlendOperation : uint32
    {
        Add = 0,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    struct BlendState
    {
        bool Enabled = true;
        BlendFactor SrcFactor = BlendFactor::SrcAlpha;
        BlendFactor DstFactor = BlendFactor::OneMinusSrcAlpha;
        BlendOperation Operation = BlendOperation::Add;
    };
}