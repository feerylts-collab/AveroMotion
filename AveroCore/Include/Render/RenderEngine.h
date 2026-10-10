#pragma once

#include "Core/AveroTypes.h"
#include "Render/RenderBackend.h"
#include "Render/RenderCommand.h"
#include "Render/CacheSystem.h"

#include <memory>

namespace Avero::Render
{
    class RenderEngine
    {
    public:
        RenderEngine() = default;
        ~RenderEngine() = default;

        bool Initialize(std::shared_ptr<IRenderBackend> backend)
        {
            if (!backend) return false;

            m_backend = backend;

            if (!m_backend->Initialize())
            {
                m_backend.reset();
                return false;
            }

            m_cacheSystem = std::make_unique<CacheSystem>();
            m_commandBuffer = std::make_unique<RenderCommandBuffer>();

            return true;
        }

        void Shutdown()
        {
            if (m_backend)
            {
                m_backend->Shutdown();
                m_backend.reset();
            }

            m_cacheSystem.reset();
            m_commandBuffer.reset();
        }

        void BeginFrame()
        {
            if (!m_backend || !m_commandBuffer) return;

            m_backend->BeginFrame();
            m_commandBuffer->Clear();
        }

        void EndFrame()
        {
            if (!m_backend || !m_commandBuffer) return;

            m_backend->ExecuteCommands(*m_commandBuffer);
            m_backend->EndFrame();
        }

        void AddCommand(const RenderCommand& cmd)
        {
            if (m_commandBuffer)
            {
                m_commandBuffer->AddCommand(cmd);
            }
        }

        CacheSystem* GetCacheSystem()
        {
            return m_cacheSystem.get();
        }

        IRenderBackend* GetBackend()
        {
            return m_backend.get();
        }

    private:
        std::shared_ptr<IRenderBackend> m_backend;
        std::unique_ptr<CacheSystem> m_cacheSystem;
        std::unique_ptr<RenderCommandBuffer> m_commandBuffer;
    };
}