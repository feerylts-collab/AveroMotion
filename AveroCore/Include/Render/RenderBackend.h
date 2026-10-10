#pragma once

#include "Core/AveroTypes.h"
#include "Render/Texture.h"
#include "Render/RenderTarget.h"
#include "Render/RenderCommand.h"

#include <string>

namespace Avero::Render
{
    using Avero::Core::uint32;

    // Абстрактный интерфейс GPU бэкенда
    class IRenderBackend
    {
    public:
        virtual ~IRenderBackend() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;

        virtual std::string GetBackendName() const = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void ExecuteCommands(const RenderCommandBuffer& commands) = 0;

        virtual std::shared_ptr<Texture> CreateTexture() = 0;
        virtual std::shared_ptr<RenderTarget> CreateRenderTarget() = 0;
    };
}