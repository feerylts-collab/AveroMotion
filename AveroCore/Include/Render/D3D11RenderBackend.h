#pragma once

#include "Render/RenderBackend.h"
#include <memory>

namespace Avero::Render
{
    class D3D11RenderBackend final : public IRenderBackend
    {
    public:
        D3D11RenderBackend() = default;
        ~D3D11RenderBackend() override;

        bool Initialize() override;
        void Shutdown() override;
        std::string GetBackendName() const override;
        void BeginFrame() override;
        void EndFrame() override;
        void ExecuteCommands(const RenderCommandBuffer& commands) override;
        std::shared_ptr<Texture> CreateTexture() override;
        std::shared_ptr<RenderTarget> CreateRenderTarget() override;
        std::shared_ptr<RenderTarget> CreateRenderTarget(uint32_t width, uint32_t height);
        std::shared_ptr<Texture> CreateTexture(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);

        bool IsInitialized() const;
        void* GetNativeDevice() const;
        void* GetNativeContext() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
