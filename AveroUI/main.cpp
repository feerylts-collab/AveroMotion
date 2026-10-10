#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

namespace fs = std::filesystem;
static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static IDXGISwapChain* g_swapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static UINT g_pendingWidth = 0, g_pendingHeight = 0;
static bool g_projectOpen = false;
static bool g_showCreateProject = false;
static bool g_projectCreated = false;
static char g_projectName[128] = "My First Project";
static char g_projectPath[MAX_PATH * 2] = "";
static int g_resolution = 0;
static int g_frameRate = 1;
static std::wstring g_currentProjectPath;
static std::string g_statusMessage;
static ImFont* g_font = nullptr;
static ImFont* g_fontLarge = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static std::wstring Utf8ToWide(const char* text) {
    if (!text || !*text) return {};
    int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), count);
    if (!result.empty() && result.back() == L'\0') result.pop_back();
    return result;
}
static std::string WideToUtf8(const std::wstring& text) {
    if (text.empty()) return {};
    int count = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), count, nullptr, nullptr);
    if (!result.empty() && result.back() == '\0') result.pop_back();
    return result;
}
static void BrowseProjectFolder() {
    BROWSEINFOW info{};
    info.lpszTitle = L"Choose a folder for AveroMotion projects";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&info);
    if (item) {
        wchar_t folder[MAX_PATH]{};
        if (SHGetPathFromIDListW(item, folder)) {
            std::string utf8 = WideToUtf8(folder);
            strncpy_s(g_projectPath, sizeof(g_projectPath), utf8.c_str(), _TRUNCATE);
        }
        CoTaskMemFree(item);
    }
}
static bool CreateProject() {
    std::string name(g_projectName);
    name.erase(name.begin(), std::find_if(name.begin(), name.end(), [](unsigned char c) { return !std::isspace(c); }));
    name.erase(std::find_if(name.rbegin(), name.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), name.end());
    if (name.empty()) { g_statusMessage = "Enter a project name."; return false; }
    if (name.find_first_of("<>:\"/\\|?*") != std::string::npos) {
        g_statusMessage = "Project name contains characters Windows does not allow."; return false;
    }
    std::wstring root = Utf8ToWide(g_projectPath);
    if (root.empty()) { g_statusMessage = "Choose a project location."; return false; }
    fs::path projectDir = fs::path(root) / fs::path(Utf8ToWide(name));
    std::error_code ec;
    if (fs::exists(projectDir, ec)) { g_statusMessage = "A folder with this project name already exists."; return false; }
    fs::create_directories(projectDir / L"Media", ec);
    if (ec) { g_statusMessage = "Could not create the project folder. Check the selected location."; return false; }
    fs::create_directories(projectDir / L"Cache", ec);
    std::ofstream projectFile(projectDir / L"project.avero", std::ios::binary);
    if (!projectFile) { g_statusMessage = "Could not write project.avero."; return false; }
    const int widths[] = { 1920, 3840, 1080 };
    const int heights[] = { 1080, 2160, 1920 };
    const int fps[] = { 24, 30, 60, 25, 50 };
    int width = widths[g_resolution], height = heights[g_resolution];
    projectFile << "AVEROMOTION_PROJECT\n"
                << "version=1\n"
                << "name=" << name << "\n"
                << "width=" << width << "\n"
                << "height=" << height << "\n"
                << "fps=" << fps[g_frameRate] << "\n";
    projectFile.close();
    g_currentProjectPath = projectDir.wstring();
    g_projectOpen = true;
    g_projectCreated = true;
    g_showCreateProject = false;
    g_statusMessage.clear();
    return true;
}
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
    sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
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
    CreateRenderTarget(); return g_mainRenderTargetView != nullptr;
}
static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context) { g_context->Release(); g_context = nullptr; }
    if (g_device) { g_device->Release(); g_device = nullptr; }
}
static void ApplyTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 0; s.ChildRounding = 10; s.FrameRounding = 7;
    s.PopupRounding = 8; s.ScrollbarRounding = 8; s.GrabRounding = 6;
    s.WindowPadding = ImVec2(24, 20); s.FramePadding = ImVec2(12, 10);
    s.ItemSpacing = ImVec2(12, 10); s.ItemInnerSpacing = ImVec2(8, 6);
    s.ChildBorderSize = 1; s.FrameBorderSize = 0; s.WindowBorderSize = 0;
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0.047f, 0.055f, 0.075f, 1);
    s.Colors[ImGuiCol_ChildBg] = ImVec4(0.070f, 0.082f, 0.108f, 1);
    s.Colors[ImGuiCol_PopupBg] = ImVec4(0.075f, 0.086f, 0.115f, 1);
    s.Colors[ImGuiCol_Border] = ImVec4(0.15f, 0.17f, 0.22f, 1);
    s.Colors[ImGuiCol_Text] = ImVec4(0.91f, 0.93f, 0.97f, 1);
    s.Colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.53f, 0.62f, 1);
    s.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.12f, 0.16f, 1);
    s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.16f, 0.22f, 1);
    s.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.29f, 0.88f, 1);
    s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.37f, 0.98f, 1);
    s.Colors[ImGuiCol_ButtonActive] = ImVec4(0.29f, 0.24f, 0.75f, 1);
    s.Colors[ImGuiCol_Header] = ImVec4(0.23f, 0.20f, 0.47f, 1);
    s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.29f, 0.25f, 0.59f, 1);
    s.Colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.54f, 1.0f, 1);
    s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.45f, 0.98f, 1);
}
static void CenteredText(const char* text, float width = 0) {
    if (width <= 0) width = ImGui::GetContentRegionAvail().x;
    float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}
static bool PrimaryButton(const char* label, ImVec2 size) {
    return ImGui::Button(label, size);
}
static void DrawHome() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::SetCursorPos(ImVec2(48, 34));
    ImGui::TextColored(ImVec4(0.65f, 0.60f, 1.0f, 1), "A V E R O");
    ImGui::SameLine(0, 12); ImGui::TextDisabled("M O T I O N");
    ImGui::SameLine(); ImGui::SetCursorPosX(io.DisplaySize.x - 160);
    ImGui::TextDisabled("MOTION STUDIO  /  01");
    float contentWidth = (std::min)(920.0f, io.DisplaySize.x - 96.0f);
    ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - contentWidth) * 0.5f, 130));
    ImGui::BeginChild("Welcome", ImVec2(contentWidth, 180), false);
    if (g_fontLarge) ImGui::PushFont(g_fontLarge);
    ImGui::TextUnformatted("Make something");
    ImGui::SameLine(0, 8); ImGui::TextColored(ImVec4(0.62f, 0.56f, 1.0f, 1), "move.");
    if (g_fontLarge) ImGui::PopFont();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.58f, 0.62f, 0.71f, 1), "A focused workspace for motion design, compositing and video.");
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - contentWidth) * 0.5f, 330));
    ImGui::BeginChild("Actions", ImVec2(contentWidth, 220), true);
    ImGui::TextDisabled("GET STARTED"); ImGui::Spacing();
    float cardW = (contentWidth - 64.0f) * 0.5f;
    ImGui::BeginChild("NewProjectCard", ImVec2(cardW, 142), true);
    ImGui::TextColored(ImVec4(0.66f, 0.61f, 1.0f, 1), "+  NEW PROJECT");
    ImGui::Spacing(); ImGui::TextWrapped("Start with a clean canvas and choose your format.");
    ImGui::SetCursorPosY(85);
    if (PrimaryButton("Create a project  ->", ImVec2(-1, 38))) {
        g_statusMessage.clear(); g_showCreateProject = true; ImGui::OpenPopup("Create project");
    }
    ImGui::EndChild(); ImGui::SameLine();
    ImGui::BeginChild("OpenProjectCard", ImVec2(0, 142), true);
    ImGui::TextColored(ImVec4(0.55f, 0.77f, 0.95f, 1), "↗  OPEN PROJECT");
    ImGui::Spacing(); ImGui::TextWrapped("Continue working on an existing .avero project.");
    ImGui::SetCursorPosY(85);
    if (ImGui::Button("Browse project folder  ->", ImVec2(-1, 38))) BrowseProjectFolder();
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - contentWidth) * 0.5f, 585));
    ImGui::BeginChild("RecentProjects", ImVec2(contentWidth, 115), false);
    ImGui::TextDisabled("RECENT PROJECTS");
    ImGui::Spacing();
    ImGui::TextDisabled("Your recent projects will appear here as you create and open them.");
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2(48, io.DisplaySize.y - 36));
    ImGui::TextDisabled("AVEROMOTION  •  EARLY ACCESS");
    ImGui::SameLine(); ImGui::SetCursorPosX(io.DisplaySize.x - 190); ImGui::TextDisabled("DESIGNED TO MOVE.");
    if (g_showCreateProject) {
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Create project", &g_showCreateProject, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Create a new project");
            ImGui::TextDisabled("Set up your canvas. You can change things later.");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::Text("PROJECT NAME");
            ImGui::SetNextItemWidth(-1); ImGui::InputText("##projectname", g_projectName, sizeof(g_projectName));
            ImGui::Spacing(); ImGui::Text("LOCATION");
            ImGui::SetNextItemWidth(-90); ImGui::InputText("##projectpath", g_projectPath, sizeof(g_projectPath));
            ImGui::SameLine(); if (ImGui::Button("Browse...", ImVec2(82, 0))) BrowseProjectFolder();
            ImGui::Spacing(); ImGui::Text("CANVAS PRESET");
            const char* presets[] = { "1920 x 1080  /  Landscape", "3840 x 2160  /  4K", "1080 x 1920  /  Portrait" };
            ImGui::SetNextItemWidth(-1); ImGui::Combo("##resolution", &g_resolution, presets, IM_ARRAYSIZE(presets));
            ImGui::Spacing(); ImGui::Text("FRAME RATE");
            const char* rates[] = { "24 FPS", "30 FPS", "60 FPS", "25 FPS", "50 FPS" };
            ImGui::SetNextItemWidth(-1); ImGui::Combo("##fps", &g_frameRate, rates, IM_ARRAYSIZE(rates));
            if (!g_statusMessage.empty()) {
                ImGui::Spacing(); ImGui::TextColored(ImVec4(1.0f, 0.40f, 0.40f, 1), "%s", g_statusMessage.c_str());
            }
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            if (ImGui::Button("Create project", ImVec2(180, 40))) CreateProject();
            ImGui::SameLine(); if (ImGui::Button("Cancel", ImVec2(110, 40))) { g_showCreateProject = false; ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}
static void DrawWorkspace() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::TextColored(ImVec4(0.65f, 0.60f, 1.0f, 1), "AVEROMOTION");
    ImGui::SameLine(); ImGui::TextDisabled("/ %s", Utf8ToWide(g_projectName).empty() ? "Project" : g_projectName);
    ImGui::SameLine(); ImGui::SetCursorPosX(io.DisplaySize.x - 150);
    if (ImGui::SmallButton("< Home")) { g_projectOpen = false; }
    ImGui::Separator();
    ImGui::TextDisabled("PROJECT"); ImGui::SameLine(); ImGui::Text("%s", g_projectName);
    ImGui::SameLine(); ImGui::TextDisabled("  •  %s", WideToUtf8(g_currentProjectPath).c_str());
    ImGui::Spacing();
    float h = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("EditorTop", ImVec2(0, h * 0.68f), false);
    ImGui::BeginChild("EditorLayers", ImVec2(220, 0), true);
    ImGui::TextUnformatted("PROJECT"); ImGui::Separator();
    ImGui::TextDisabled("MEDIA"); ImGui::TextDisabled("  No imported media");
    ImGui::Spacing(); ImGui::TextDisabled("LAYERS");
    ImGui::TextDisabled("  No layers yet");
    ImGui::EndChild(); ImGui::SameLine();
    ImGui::BeginChild("EditorPreview", ImVec2(0, 0), true);
    ImGui::TextUnformatted("COMPOSITION"); ImGui::SameLine(); ImGui::TextDisabled("  Preview");
    ImGui::Separator();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cw = (std::min)(avail.x * 0.82f, avail.y * 16.0f / 9.0f);
    float ch = cw * 9.0f / 16.0f;
    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - cw) * 0.5f, (ImGui::GetWindowHeight() - ch) * 0.5f));
    ImGui::BeginChild("Canvas", ImVec2(cw, ch), true, ImGuiWindowFlags_NoScrollbar);
    ImVec2 p = ImGui::GetCursorScreenPos(); ImVec2 a = ImGui::GetContentRegionAvail();
    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + a.x, p.y + a.y), IM_COL32(15, 18, 27, 255), 6);
    ImGui::SetCursorPosY(a.y * 0.42f);
    CenteredText("Your canvas is ready");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    CenteredText("Import media or add a layer to begin");
    ImGui::EndChild(); ImGui::EndChild(); ImGui::EndChild();
    ImGui::BeginChild("Timeline", ImVec2(0, 0), true);
    ImGui::TextUnformatted("TIMELINE"); ImGui::SameLine(); ImGui::TextDisabled("  00:00:00:00");
    ImGui::Separator();
    ImVec2 origin = ImGui::GetCursorScreenPos(); ImVec2 region = ImGui::GetContentRegionAvail();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (int i = 0; i <= 10; ++i) {
        float x = origin.x + region.x * i / 10.0f;
        dl->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + region.y), IM_COL32(42, 48, 63, 255));
        char label[16]; sprintf_s(label, "%02d s", i); dl->AddText(ImVec2(x + 4, origin.y + 4), IM_COL32(130, 140, 160, 255), label);
    }
    ImGui::EndChild(); ImGui::End();
}
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return TRUE;
    switch (msg) {
    case WM_SIZE: if (g_device && wParam != SIZE_MINIMIZED) { g_pendingWidth = LOWORD(lParam); g_pendingHeight = HIWORD(lParam); } return 0;
    case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return 0; break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCmd) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSEXW wc{ sizeof(wc), CS_CLASSDC, WndProc, 0, 0, instance, nullptr, nullptr, nullptr, nullptr, L"AveroMotionWindow", nullptr };
    if (!RegisterClassExW(&wc)) return 1;
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"AveroMotion — Motion Studio", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1440, 920, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) { UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); DestroyWindow(hwnd); UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ApplyTheme();
    g_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 17.0f);
    g_fontLarge = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 32.0f);
    if (!g_font) g_font = io.Fonts->AddFontDefault();
    if (!g_fontLarge) g_fontLarge = g_font;
    ImGui_ImplWin32_Init(hwnd); ImGui_ImplDX11_Init(g_device, g_context);
    PWSTR documents = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &documents))) {
        std::wstring defaultPath = std::wstring(documents) + L"\\AveroMotion Projects";
        std::string defaultPathUtf8 = WideToUtf8(defaultPath);
        strncpy_s(g_projectPath, sizeof(g_projectPath), defaultPathUtf8.c_str(), _TRUNCATE);
        CoTaskMemFree(documents);
    } else {
        strncpy_s(g_projectPath, sizeof(g_projectPath), "C:\\AveroMotion Projects", _TRUNCATE);
    }
    ShowWindow(hwnd, showCmd); UpdateWindow(hwnd);
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;
        if (g_pendingWidth && g_pendingHeight) {
            CleanupRenderTarget(); g_swapChain->ResizeBuffers(0, g_pendingWidth, g_pendingHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_pendingWidth = g_pendingHeight = 0; CreateRenderTarget();
        }
        ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
        if (g_font) ImGui::PushFont(g_font);
        if (g_projectOpen) DrawWorkspace(); else DrawHome();
        if (g_font) ImGui::PopFont();
        ImGui::Render();
        const float clearColor[] = { 0.035f, 0.041f, 0.058f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_context->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); g_swapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    CleanupDeviceD3D(); DestroyWindow(hwnd); UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
