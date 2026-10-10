#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <cstdint>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static IDXGISwapChain* g_swapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static UINT g_pendingWidth = 0;
static UINT g_pendingHeight = 0;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static void CreateRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    if (g_swapChain && SUCCEEDED(g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
        g_device->CreateRenderTargetView(backBuffer, nullptr, &g_mainRenderTargetView);
        backBuffer->Release();
    }
}
static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
static bool CreateDeviceD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL level{};
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, &level, &g_context);
#if defined(_DEBUG)
    if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, &level, &g_context);
    }
#endif
    if (FAILED(hr)) return false;
    CreateRenderTarget();
    return g_mainRenderTargetView != nullptr;
}
static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context) { g_context->Release(); g_context = nullptr; }
    if (g_device) { g_device->Release(); g_device = nullptr; }
}
static void DrawEditor() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    constexpr ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("AveroMotion Workspace", nullptr, rootFlags);
    ImGui::TextUnformatted("AVEROMOTION");
    ImGui::SameLine();
    ImGui::TextDisabled("  Motion Studio  /  Untitled Project");
    ImGui::Separator();

    const float available = ImGui::GetContentRegionAvail().y;
    const float timelineHeight = available * 0.31f;
    const float propertiesWidth = 260.0f;
    const float layersWidth = 220.0f;

    ImGui::BeginChild("MainWorkspace", ImVec2(0, available - timelineHeight), false);
    ImGui::BeginChild("LayersPanel", ImVec2(layersWidth, 0), true);
    ImGui::TextUnformatted("LAYERS");
    ImGui::Separator();
    static int selectedLayer = 0;
    const char* layerNames[] = { "Text • Title", "Shape • Accent", "Footage • Main" };
    for (int i = 0; i < 3; ++i) if (ImGui::Selectable(layerNames[i], selectedLayer == i)) selectedLayer = i;
    ImGui::Spacing();
    ImGui::TextDisabled("Add layer");
    ImGui::SameLine();
    if (ImGui::SmallButton("+ Text")) selectedLayer = 0;
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("PreviewPanel", ImVec2(0, 0), true);
    ImGui::TextUnformatted("PREVIEW");
    ImGui::SameLine();
    ImGui::TextDisabled("  00:00:00:00");
    ImGui::Separator();
    ImGui::BeginChild("PreviewCanvas", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float w = (avail.x < avail.y * 16.0f / 9.0f) ? avail.x : avail.y * 16.0f / 9.0f;
    float h = w * 9.0f / 16.0f;
    ImVec2 min(p.x + (avail.x - w) * 0.5f, p.y + (avail.y - h) * 0.5f);
    ImVec2 max(min.x + w, min.y + h);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(min, max, IM_COL32(18, 21, 30, 255), 8.0f);
    dl->AddRect(min, max, IM_COL32(69, 78, 101, 255), 1.0f, 0, 1.0f);
    const char* previewTitle = "Your next frame starts here.";
    ImVec2 ts = ImGui::CalcTextSize(previewTitle);
    dl->AddText(ImVec2(min.x + (w - ts.x) * 0.5f, min.y + h * 0.43f), IM_COL32(238, 241, 250, 255), previewTitle);
    const char* previewSub = "Import media or add a layer to begin";
    ImVec2 ss = ImGui::CalcTextSize(previewSub);
    dl->AddText(ImVec2(min.x + (w - ss.x) * 0.5f, min.y + h * 0.43f + 28.0f), IM_COL32(142, 153, 177, 255), previewSub);
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("PropertiesPanel", ImVec2(propertiesWidth, 0), true);
    ImGui::TextUnformatted("PROPERTIES");
    ImGui::Separator();
    ImGui::Text("%s", layerNames[selectedLayer]);
    static float position[2] = { 0.0f, 0.0f };
    static float scale[2] = { 100.0f, 100.0f };
    static float rotation = 0.0f;
    static float opacity = 100.0f;
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Position", position, 1.0f);
        ImGui::DragFloat2("Scale (%)", scale, 0.5f, 0.0f, 1000.0f);
        ImGui::DragFloat("Rotation", &rotation, 0.5f, -360.0f, 360.0f, "%.1f°");
        ImGui::SliderFloat("Opacity", &opacity, 0.0f, 100.0f, "%.0f%%");
    }
    if (ImGui::CollapsingHeader("Compositing")) ImGui::TextDisabled("Blend mode: Normal");
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::BeginChild("TimelinePanel", ImVec2(0, 0), true);
    ImGui::TextUnformatted("TIMELINE");
    ImGui::SameLine();
    ImGui::TextDisabled("  30 FPS");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 170.0f);
    ImGui::TextDisabled("00:00:10:00");
    ImGui::Separator();
    const float labelWidth = 180.0f;
    ImGui::BeginChild("TrackLabels", ImVec2(labelWidth, 0), true);
    ImGui::TextUnformatted("Track / Layer");
    ImGui::Separator();
    ImGui::TextUnformatted("T   Text • Title");
    ImGui::TextUnformatted("◇   Shape • Accent");
    ImGui::TextUnformatted("▧   Footage • Main");
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("TrackArea", ImVec2(0, 0), true);
    ImDrawList* tdl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 region = ImGui::GetContentRegionAvail();
    for (int i = 0; i <= 10; ++i) {
        float x = origin.x + region.x * i / 10.0f;
        tdl->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + region.y), IM_COL32(55, 62, 79, 255));
        char label[16]; wsprintfA(label, "%02d s", i);
        tdl->AddText(ImVec2(x + 4, origin.y + 2), IM_COL32(135, 146, 168, 255), label);
    }
    const ImU32 trackColors[] = { IM_COL32(105, 86, 210, 255), IM_COL32(56, 151, 166, 255), IM_COL32(73, 103, 160, 255) };
    for (int i = 0; i < 3; ++i) {
        float y = origin.y + 30.0f + i * 28.0f;
        tdl->AddRectFilled(ImVec2(origin.x + region.x * 0.08f, y), ImVec2(origin.x + region.x * (i == 2 ? 0.82f : 0.68f), y + 20.0f), trackColors[i], 4.0f);
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::End();
}
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return TRUE;
    switch (msg) {
    case WM_SIZE:
        if (g_device && wParam != SIZE_MINIMIZED) { g_pendingWidth = LOWORD(lParam); g_pendingHeight = HIWORD(lParam); }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    default: break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCmd) {
    WNDCLASSEXW wc{ sizeof(wc), CS_CLASSDC, WndProc, 0, 0, instance, nullptr, nullptr, nullptr, nullptr, L"AveroMotionWindow", nullptr };
    if (!RegisterClassExW(&wc)) return 1;
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"AveroMotion — Motion Studio", WS_OVERLAPPEDWINDOW,
        100, 100, 1500, 950, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) { UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); DestroyWindow(hwnd); UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    ShowWindow(hwnd, showCmd); UpdateWindow(hwnd);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 7.0f; style.ChildRounding = 6.0f; style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f; style.PopupRounding = 5.0f; style.ScrollbarRounding = 6.0f;
    style.WindowPadding = ImVec2(12, 10); style.FramePadding = ImVec2(8, 5); style.ItemSpacing = ImVec2(8, 7);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.063f, 0.086f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.075f, 0.086f, 0.114f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.17f, 0.19f, 0.24f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.22f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.27f, 0.23f, 0.49f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.36f, 0.30f, 0.63f, 1.0f);
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;
        if (g_pendingWidth && g_pendingHeight) {
            CleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, g_pendingWidth, g_pendingHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_pendingWidth = g_pendingHeight = 0;
            CreateRenderTarget();
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        DrawEditor();
        ImGui::Render();
        const float clearColor[4] = { 0.035f, 0.041f, 0.058f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_context->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    CleanupDeviceD3D(); DestroyWindow(hwnd); UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}