#include "Export/FFmpegVideoEncoder.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace Avero::Export
{
    std::string FFmpegVideoEncoder::FindFFmpeg()
    {
        #ifdef _WIN32
        char* buffer = nullptr;
        size_t required = 0;
        if (_dupenv_s(&buffer, &required, "AVERO_FFMPEG") == 0 && buffer)
        {
            std::string result(buffer);
            std::free(buffer);
            return result;
        }
#else
        const char* env = std::getenv("AVERO_FFMPEG");
        if (env && *env) return env;
#endif
        return "ffmpeg";
    }

    std::string FFmpegVideoEncoder::Quote(const std::string& value)
    {
        std::string out = "\"";
        for (char c : value) out += (c == '"' ? "\\\"" : std::string(1, c));
        out += "\"";
        return out;
    }

    bool FFmpegVideoEncoder::IsRuntimeAvailable()
    {
#ifdef _WIN32
        const std::string command = FindFFmpeg() + " -version >nul 2>nul";
        return std::system(command.c_str()) == 0;
#else
        return false;
#endif
    }

    std::string FFmpegVideoEncoder::BuildCommand(const std::string& outputPath) const
    {
        std::string codec;
        std::string pixfmt = "yuv420p";
        switch (m_settings.Codec)
        {
        case VideoEncoderCodec::H264:
            codec = m_settings.PreferHardware ? "h264_mf" : "libx264";
            break;
        case VideoEncoderCodec::H265:
            codec = m_settings.PreferHardware ? "hevc_mf" : "libx265";
            break;
        case VideoEncoderCodec::AV1:
            codec = m_settings.PreferHardware ? "av1_mf" : "libaom-av1";
            break;
        case VideoEncoderCodec::ProRes:
            codec = "prores_ks";
            pixfmt = "yuv422p10le";
            break;
        }

        std::string cmd = FindFFmpeg() + " -y -v error -f rawvideo -pix_fmt rgba -s " +
            std::to_string(m_settings.Width) + "x" + std::to_string(m_settings.Height) +
            " -r " + std::to_string(m_settings.FrameRate) + " -i pipe:0 ";

        if (codec == "h264_mf" || codec == "hevc_mf" || codec == "av1_mf")
            cmd += "-c:v " + codec + " -hw_encoding 1 -b:v " + std::to_string(m_settings.Bitrate) + " -pix_fmt " + pixfmt + " ";
        else if (m_settings.Codec == VideoEncoderCodec::ProRes)
            cmd += "-c:v prores_ks -profile:v 3 -pix_fmt " + pixfmt + " ";
        else
            cmd += "-c:v " + codec + " -crf " + std::to_string(m_settings.Quality) + " -preset medium -pix_fmt " + pixfmt + " ";

        return cmd + Quote(outputPath);
    }

    bool FFmpegVideoEncoder::Start(const std::string& outputPath, const VideoEncoderSettings& settings)
    {
        Finish();
        if (outputPath.empty() || settings.Width == 0 || settings.Height == 0 || settings.FrameRate <= 0.0) return false;
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        HANDLE readPipe = nullptr;
        HANDLE writePipe = nullptr;
        if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) return false;
        SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = readPipe;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION pi{};

        std::string cmd = BuildCommand(outputPath);
        std::vector<char> mutableCmd(cmd.begin(), cmd.end());
        mutableCmd.push_back('\0');
        if (!CreateProcessA(nullptr, mutableCmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
            nullptr, nullptr, &si, &pi))
        {
            CloseHandle(readPipe);
            CloseHandle(writePipe);
            return false;
        }
        CloseHandle(pi.hThread);
        CloseHandle(readPipe);
        m_process = pi.hProcess;
        m_stdinWrite = writePipe;
        m_settings = settings;
        m_outputPath = outputPath;
        m_running = true;
        return true;
#else
        (void)outputPath; (void)settings;
        return false;
#endif
    }

    bool FFmpegVideoEncoder::WriteFrame(const Media::VideoFrame& frame)
    {
        if (!m_running || frame.Width != m_settings.Width || frame.Height != m_settings.Height || frame.PixelData.size() < static_cast<size_t>(frame.Width) * frame.Height * 4u) return false;
#ifdef _WIN32
        DWORD written = 0;
        const DWORD bytes = static_cast<DWORD>(m_settings.Width * m_settings.Height * 4ull);
        return WriteFile(static_cast<HANDLE>(m_stdinWrite), frame.PixelData.data(), bytes, &written, nullptr) && written == bytes;
#else
        return false;
#endif
    }

    void FFmpegVideoEncoder::CloseProcess()
    {
#ifdef _WIN32
        if (m_stdinWrite) { CloseHandle(static_cast<HANDLE>(m_stdinWrite)); m_stdinWrite = nullptr; }
        if (m_process)
        {
            WaitForSingleObject(static_cast<HANDLE>(m_process), INFINITE);
            CloseHandle(static_cast<HANDLE>(m_process));
            m_process = nullptr;
        }
#endif
        m_running = false;
    }

    bool FFmpegVideoEncoder::Finish()
    {
        if (!m_running) return true;
        CloseProcess();
        return true;
    }

    FFmpegVideoEncoder::~FFmpegVideoEncoder() { Finish(); }
}
