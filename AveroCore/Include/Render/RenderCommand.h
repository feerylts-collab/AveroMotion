#pragma once

#include "Core/AveroTypes.h"
#include "Render/Texture.h"
#include "Render/RenderTarget.h"
#include "Math/Vector4.h"

#include <vector>
#include <memory>

namespace Avero::Render
{
    using Avero::Core::uint32;

    enum class RenderCommandType : uint32
    {
        Clear = 0,
        BindTexture,
        BindRenderTarget,
        Draw,
        SetBlendState,
        SetViewport
    };

    struct RenderCommand
    {
        RenderCommandType Type = RenderCommandType::Clear;

        Math::Vector4 ClearColor;

        std::shared_ptr<Texture> BoundTexture;
        std::shared_ptr<RenderTarget> BoundRenderTarget;

        uint32 ViewportX = 0;
        uint32 ViewportY = 0;
        uint32 ViewportWidth = 0;
        uint32 ViewportHeight = 0;

        BlendState Blend;

        RenderCommand() = default;

        static RenderCommand CreateClear(const Math::Vector4& color)
        {
            RenderCommand cmd;
            cmd.Type = RenderCommandType::Clear;
            cmd.ClearColor = color;
            return cmd;
        }

        static RenderCommand CreateBindTexture(std::shared_ptr<Texture> texture)
        {
            RenderCommand cmd;
            cmd.Type = RenderCommandType::BindTexture;
            cmd.BoundTexture = texture;
            return cmd;
        }

        static RenderCommand CreateBindRenderTarget(std::shared_ptr<RenderTarget> target)
        {
            RenderCommand cmd;
            cmd.Type = RenderCommandType::BindRenderTarget;
            cmd.BoundRenderTarget = target;
            return cmd;
        }

        static RenderCommand CreateSetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
        {
            RenderCommand cmd;
            cmd.Type = RenderCommandType::SetViewport;
            cmd.ViewportX = x;
            cmd.ViewportY = y;
            cmd.ViewportWidth = width;
            cmd.ViewportHeight = height;
            return cmd;
        }
    };

    class RenderCommandBuffer
    {
    public:
        std::vector<RenderCommand> Commands;

        void Clear()
        {
            Commands.clear();
        }

        void AddCommand(const RenderCommand& cmd)
        {
            Commands.push_back(cmd);
        }

        size_t GetCommandCount() const
        {
            return Commands.size();
        }
    };
}