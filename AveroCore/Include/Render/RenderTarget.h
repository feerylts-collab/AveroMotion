#pragma once

#include "Core/AveroTypes.h"
#include "Render/Texture.h"
#include "Math/Vector2.h"
#include <memory>
#include <string>

namespace Avero::Render
{
    using Avero::Core::uint64;
    using Avero::Core::uint32;

    class RenderTarget
    {
    public:
        uint64 ID = 0;
        std::string Name;
        Math::Vector2 Size;
        std::shared_ptr<Texture> ColorAttachment;
        std::shared_ptr<Texture> DepthAttachment;

        RenderTarget() = default;
        virtual ~RenderTarget() = default;

        bool IsValid() const
        {
            return ColorAttachment != nullptr && Size.X > 0.0f && Size.Y > 0.0f;
        }

        void Allocate(uint32 width, uint32 height)
        {
            Size = Math::Vector2(static_cast<float>(width), static_cast<float>(height));
            ColorAttachment = std::make_shared<Texture>();
            ColorAttachment->Allocate(width, height, TextureFormat::RGBA8);
            DepthAttachment = std::make_shared<Texture>();
            DepthAttachment->Allocate(width, height, TextureFormat::Depth24Stencil8);
        }
    };
}
