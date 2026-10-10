#include "Render/D3D11RenderBackend.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <algorithm>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#endif

namespace Avero::Render
{
#ifdef _WIN32
    using Microsoft::WRL::ComPtr;

    class D3D11Texture final : public Texture
    {
    public:
        ComPtr<ID3D11Texture2D> Resource;
        ComPtr<ID3D11ShaderResourceView> SRV;
    };

    class D3D11RenderTarget final : public RenderTarget
    {
    public:
        ComPtr<ID3D11Texture2D> Resource;
        ComPtr<ID3D11RenderTargetView> RTV;

        bool CreateNative(ID3D11Device* device, uint32_t width, uint32_t height)
        {
            Allocate(width, height);
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(device->CreateTexture2D(&desc, nullptr, &Resource))) return false;
            return SUCCEEDED(device->CreateRenderTargetView(Resource.Get(), nullptr, &RTV));
        }
    };
#endif

    struct D3D11RenderBackend::Impl
    {
#ifdef _WIN32
        ComPtr<ID3D11Device> Device;
        ComPtr<ID3D11DeviceContext> Context;
        ComPtr<IDXGIFactory1> Factory;
#endif
        bool Initialized = false;
        Math::Vector4 CurrentClear{0.0f, 0.0f, 0.0f, 1.0f};
        std::shared_ptr<RenderTarget> CurrentTarget;
    };

    D3D11RenderBackend::~D3D11RenderBackend() { Shutdown(); }

    bool D3D11RenderBackend::Initialize()
    {
        Shutdown();
        m_impl = std::make_unique<Impl>();
#ifdef _WIN32
        UINT flags = 0;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        D3D_FEATURE_LEVEL created{};
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            levels,
            ARRAYSIZE(levels),
            D3D11_SDK_VERSION,
            &m_impl->Device,
            &created,
            &m_impl->Context);
        if (FAILED(hr))
        {
            hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                &m_impl->Device, &created, &m_impl->Context);
        }
        if (FAILED(hr)) { m_impl.reset(); return false; }
        m_impl->Initialized = true;
        return true;
#else
        return false;
#endif
    }

    void D3D11RenderBackend::Shutdown()
    {
        if (!m_impl) return;
#ifdef _WIN32
        if (m_impl->Context) m_impl->Context->ClearState();
#endif
        m_impl.reset();
    }

    std::string D3D11RenderBackend::GetBackendName() const { return "Direct3D 11"; }

    void D3D11RenderBackend::BeginFrame()
    {
        if (!m_impl || !m_impl->Initialized) return;
        m_impl->CurrentTarget.reset();
    }

    void D3D11RenderBackend::EndFrame() {}

    void D3D11RenderBackend::ExecuteCommands(const RenderCommandBuffer& commands)
    {
        if (!m_impl || !m_impl->Initialized) return;
#ifdef _WIN32
        for (const RenderCommand& command : commands.Commands)
        {
            switch (command.Type)
            {
            case RenderCommandType::BindRenderTarget:
                m_impl->CurrentTarget = command.BoundRenderTarget;
                if (auto target = std::dynamic_pointer_cast<D3D11RenderTarget>(m_impl->CurrentTarget))
                {
                    ID3D11RenderTargetView* rtv = target->RTV.Get();
                    m_impl->Context->OMSetRenderTargets(1, &rtv, nullptr);
                }
                break;
            case RenderCommandType::Clear:
                m_impl->CurrentClear = command.ClearColor;
                if (auto target = std::dynamic_pointer_cast<D3D11RenderTarget>(m_impl->CurrentTarget))
                {
                    const float color[4] = {
                        std::clamp(command.ClearColor.X, 0.0f, 1.0f),
                        std::clamp(command.ClearColor.Y, 0.0f, 1.0f),
                        std::clamp(command.ClearColor.Z, 0.0f, 1.0f),
                        std::clamp(command.ClearColor.W, 0.0f, 1.0f)
                    };
                    m_impl->Context->ClearRenderTargetView(target->RTV.Get(), color);
                }
                break;
            case RenderCommandType::SetViewport:
            {
                D3D11_VIEWPORT vp{};
                vp.TopLeftX = static_cast<float>(command.ViewportX);
                vp.TopLeftY = static_cast<float>(command.ViewportY);
                vp.Width = static_cast<float>(command.ViewportWidth);
                vp.Height = static_cast<float>(command.ViewportHeight);
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                m_impl->Context->RSSetViewports(1, &vp);
                break;
            }
            default:
                // BindTexture/Draw are deliberately kept behind the command abstraction;
                // the next compositor stage can add shaders/geometry without changing the backend API.
                break;
            }
        }
#endif
    }

    std::shared_ptr<Texture> D3D11RenderBackend::CreateTexture()
    {
#ifdef _WIN32
        return std::make_shared<D3D11Texture>();
#else
        return std::make_shared<Texture>();
#endif
    }

    std::shared_ptr<RenderTarget> D3D11RenderBackend::CreateRenderTarget()
    {
#ifdef _WIN32
        return std::make_shared<D3D11RenderTarget>();
#else
        return std::make_shared<RenderTarget>();
#endif
    }

    std::shared_ptr<RenderTarget> D3D11RenderBackend::CreateRenderTarget(uint32_t width, uint32_t height)
    {
#ifdef _WIN32
        if (!IsInitialized() || width == 0 || height == 0) return nullptr;
        auto target = std::make_shared<D3D11RenderTarget>();
        if (!target->CreateNative(m_impl->Device.Get(), width, height)) return nullptr;
        return target;
#else
        (void)width; (void)height;
        return nullptr;
#endif
    }

    std::shared_ptr<Texture> D3D11RenderBackend::CreateTexture(uint32_t width, uint32_t height, TextureFormat format)
    {
#ifdef _WIN32
        if (!IsInitialized() || width == 0 || height == 0 || format != TextureFormat::RGBA8) return nullptr;
        auto texture = std::make_shared<D3D11Texture>();
        texture->Allocate(width, height, format);
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width; desc.Height = height; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(m_impl->Device->CreateTexture2D(&desc, nullptr, &texture->Resource))) return nullptr;
        if (FAILED(m_impl->Device->CreateShaderResourceView(texture->Resource.Get(), nullptr, &texture->SRV))) return nullptr;
        return texture;
#else
        (void)width; (void)height; (void)format;
        return nullptr;
#endif
    }

    bool D3D11RenderBackend::IsInitialized() const { return m_impl && m_impl->Initialized; }
    void* D3D11RenderBackend::GetNativeDevice() const
    {
#ifdef _WIN32
        return m_impl ? m_impl->Device.Get() : nullptr;
#else
        return nullptr;
#endif
    }
    void* D3D11RenderBackend::GetNativeContext() const
    {
#ifdef _WIN32
        return m_impl ? m_impl->Context.Get() : nullptr;
#else
        return nullptr;
#endif
    }
}
