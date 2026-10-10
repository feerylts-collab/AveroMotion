#pragma once

#include "Core/AveroTypes.h"
#include "Render/RenderTypes.h"
#include "Math/Vector2.h"

#include <string>
#include <vector>
#include <limits>

namespace Avero::Render
{
    using Avero::Core::uint64;
    using Avero::Core::uint32;
    using Avero::Core::uint8;
    using Avero::Core::uint64;

    class Texture
    {
    public:
        uint64 ID = 0;
        std::string Name;
        Math::Vector2 Size;
        TextureFormat Format = TextureFormat::RGBA8;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap Wrap = TextureWrap::Clamp;

        // CPU-side pixel data (для загрузки на GPU)
        std::vector<uint8> PixelData;

        Texture() = default;
        virtual ~Texture() = default;

        void Allocate(uint32 width, uint32 height, TextureFormat format)
        {
            Size = Math::Vector2(static_cast<float32>(width), static_cast<float32>(height));
            Format = format;

            uint32 bytesPerPixel = GetBytesPerPixel(format);
            const uint64 pixelCount = static_cast<uint64>(width) * static_cast<uint64>(height);
            const uint64 byteCount = pixelCount * static_cast<uint64>(bytesPerPixel);
            if (byteCount > static_cast<uint64>(std::numeric_limits<size_t>::max()))
            {
                PixelData.clear();
                return;
            }
            PixelData.resize(static_cast<size_t>(byteCount), 0);
        }

        uint32 GetBytesPerPixel(TextureFormat format) const
        {
            switch (format)
            {
            case TextureFormat::R8: return 1;
            case TextureFormat::RG8: return 2;
            case TextureFormat::RGB8: return 3;
            case TextureFormat::RGBA8: return 4;
            case TextureFormat::RGBA16F: return 8;
            case TextureFormat::RGBA32F: return 16;
            default: return 4;
            }
        }
    };
}