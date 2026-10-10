#include "Media/FFmpegDecoder.h"
#include "Core/AveroLog.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace Avero::Media
{
    namespace
    {
        std::string Trim(std::string s)
        {
            while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
            size_t p = 0;
            while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\r' || s[p] == '\n')) ++p;
            return s.substr(p);
        }

        std::vector<std::string> SplitCSV(const std::string& s)
        {
            std::vector<std::string> result;
            std::string current;
            bool quoted = false;
            for (char c : s)
            {
                if (c == '"') quoted = !quoted;
                else if (c == ',' && !quoted) { result.push_back(Trim(current)); current.clear(); }
                else current.push_back(c);
            }
            result.push_back(Trim(current));
            return result;
        }
    }

    FFmpegDecoder::~FFmpegDecoder() { Close(); }

    std::string FFmpegDecoder::Quote(const std::string& value)
    {
#ifdef _WIN32
        std::string out = "\"";
        for (char c : value)
        {
            if (c == '"') out += "\\\"";
            else out += c;
        }
        out += "\"";
        return out;
#else
        std::string out = "'";
        for (char c : value) out += (c == '\'' ? "'\\''" : std::string(1, c));
        out += "'";
        return out;
#endif
    }

    std::string FFmpegDecoder::FindTool(const char* name)
    {
        std::string envValue;
#ifdef _WIN32
        const char* variable = name == std::string("ffmpeg") ? "AVERO_FFMPEG" : "AVERO_FFPROBE";
        char* buffer = nullptr;
        size_t required = 0;
        if (_dupenv_s(&buffer, &required, variable) == 0 && buffer)
        {
            envValue.assign(buffer);
            std::free(buffer);
        }
#else
        const char* env = std::getenv(name == std::string("ffmpeg") ? "AVERO_FFMPEG" : "AVERO_FFPROBE");
        if (env) envValue = env;
#endif
        if (!envValue.empty()) return envValue;
        return name;
    }

    bool FFmpegDecoder::IsRuntimeAvailable()
    {
        const std::string ffmpeg = FindTool("ffmpeg");
        const std::string ffprobe = FindTool("ffprobe");
        int ec1 = -1, ec2 = -1;
        RunCapture(ffmpeg + " -version", ec1);
        RunCapture(ffprobe + " -version", ec2);
        return ec1 == 0 && ec2 == 0;
    }

    bool FFmpegDecoder::IsSupportedPath(const std::string& filePath)
    {
        const auto ext = std::filesystem::path(filePath).extension().string();
        std::string e = ext;
        std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        static constexpr std::array<const char*, 20> supported = {
            ".mp4", ".mov", ".m4v", ".mkv", ".avi", ".webm", ".wmv", ".mts", ".m2ts", ".ts",
            ".mxf", ".prores", ".3gp", ".flv", ".ogv", ".wav", ".mp3", ".aac", ".flac", ".jpg"
        };
        return std::find_if(supported.begin(), supported.end(), [&](const char* x){ return e == x; }) != supported.end();
    }

    bool FFmpegDecoder::CanOpen(const std::string& filePath) const
    {
        return !filePath.empty() && std::filesystem::exists(filePath) && IsSupportedPath(filePath) && IsRuntimeAvailable();
    }

    std::string FFmpegDecoder::RunCapture(const std::string& command, int& exitCode)
    {
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "rb");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (!pipe) { exitCode = -1; return {}; }
        std::string output;
        std::array<char, 4096> buffer{};
        while (true)
        {
            const size_t n = std::fread(buffer.data(), 1, buffer.size(), pipe);
            if (n) output.append(buffer.data(), n);
            if (n < buffer.size()) break;
        }
#ifdef _WIN32
        exitCode = _pclose(pipe);
#else
        exitCode = pclose(pipe);
#endif
        return output;
    }

    double FFmpegDecoder::ParseRate(const std::string& value)
    {
        const auto slash = value.find('/');
        try
        {
            if (slash != std::string::npos)
            {
                const double a = std::stod(value.substr(0, slash));
                const double b = std::stod(value.substr(slash + 1));
                return b != 0.0 ? a / b : 0.0;
            }
            return std::stod(value);
        }
        catch (...) { return 0.0; }
    }

    VideoCodec FFmpegDecoder::ParseVideoCodec(const std::string& value)
    {
        if (value == "h264") return VideoCodec::H264;
        if (value == "hevc" || value == "h265") return VideoCodec::H265;
        if (value == "vp9") return VideoCodec::VP9;
        if (value == "av1") return VideoCodec::AV1;
        if (value.find("prores") != std::string::npos) return VideoCodec::ProRes;
        return VideoCodec::Unknown;
    }

    AudioCodec FFmpegDecoder::ParseAudioCodec(const std::string& value)
    {
        if (value == "aac") return AudioCodec::AAC;
        if (value == "mp3") return AudioCodec::MP3;
        if (value == "flac") return AudioCodec::FLAC;
        if (value == "pcm_s16le" || value == "pcm_s24le" || value == "pcm_s32le" || value == "pcm_f32le") return AudioCodec::WAV;
        return AudioCodec::Unknown;
    }

    bool FFmpegDecoder::Probe()
    {
        int ec = -1;
        const std::string command = m_ffprobe + " -v error -show_entries stream=index,codec_type,codec_name,width,height,r_frame_rate,duration,sample_rate,channels -show_entries format=duration -of csv=p=0 " + Quote(m_filePath);
        const std::string text = RunCapture(command, ec);
        if (ec != 0 || text.empty()) return false;

        m_info = MediaInfo{};
        m_info.FilePath = m_filePath;
        m_info.FileName = std::filesystem::path(m_filePath).filename().string();
        std::error_code fsEc;
        m_info.FileSize = static_cast<uint64>(std::filesystem::file_size(m_filePath, fsEc));

        double duration = 0.0;

        std::istringstream lines(text);
        std::string line;
        while (std::getline(lines, line))
        {
            const auto fields = SplitCSV(line);
            if (fields.size() < 3) continue;
            // ffprobe emits stream entries in its canonical field order: index, codec_name, codec_type, ...
            if (fields[2] == "video")
            {
                m_info.Type = MediaType::Video;
                if (fields.size() > 4) { try { m_info.Resolution.X = std::stof(fields[3]); m_info.Resolution.Y = std::stof(fields[4]); } catch (...) {} }
                if (fields.size() > 5) m_info.FrameRate = static_cast<float32>(ParseRate(fields[5]));
                if (fields.size() > 6) { try { duration = std::max(duration, std::stod(fields[6])); } catch (...) {} }
                m_info.VideoCodecType = ParseVideoCodec(fields[1]);
            }
            else if (fields[2] == "audio")
            {
                if (m_info.Type == MediaType::Unknown) m_info.Type = MediaType::Audio;
                if (fields.size() > 7) { try { m_info.SampleRate = std::stoi(fields[7]); } catch (...) {} }
                if (fields.size() > 8) { try { m_info.Channels = std::stoi(fields[8]); } catch (...) {} }
                m_info.AudioCodecType = ParseAudioCodec(fields[1]);
                if (fields.size() > 6) { try { duration = std::max(duration, std::stod(fields[6])); } catch (...) {} }
            }
        }
        m_info.Duration = Core::Time::FromSeconds(duration);
        return m_info.Type != MediaType::Unknown;
    }

    MediaOpenResult FFmpegDecoder::Open(const std::string& filePath)
    {
        Close();
        if (!std::filesystem::exists(filePath)) return MediaOpenResult::InvalidPath;
        if (!IsSupportedPath(filePath)) return MediaOpenResult::UnsupportedFormat;
        if (!IsRuntimeAvailable()) return MediaOpenResult::DecoderUnavailable;
        m_ffmpeg = FindTool("ffmpeg");
        m_ffprobe = FindTool("ffprobe");
        m_filePath = filePath;
        if (!Probe()) { Close(); return MediaOpenResult::OpenFailed; }
        m_open = true;
        return MediaOpenResult::Success;
    }

    void FFmpegDecoder::Close()
    {
        m_open = false;
        m_filePath.clear();
        m_info = MediaInfo{};
        m_position = Core::Time{};
    }

    bool FFmpegDecoder::Seek(Core::Time time)
    {
        if (!m_open) return false;
        m_position = time;
        return true;
    }

    bool FFmpegDecoder::RunFFmpegFrame(double seconds, VideoFrame& outFrame)
    {
        if (!m_open || m_info.Resolution.X <= 0 || m_info.Resolution.Y <= 0) return false;
        const uint32 width = static_cast<uint32>(m_info.Resolution.X);
        const uint32 height = static_cast<uint32>(m_info.Resolution.Y);
        const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
        const std::string command = m_ffmpeg + " -v error -i " + Quote(m_filePath) + " -ss " + std::to_string(std::max(0.0, seconds)) + " -frames:v 1 -f rawvideo -pix_fmt rgba pipe:1";
        int ec = -1;
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "rb");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (!pipe) return false;
        outFrame.PixelData.resize(expected);
        size_t read = 0;
        while (read < expected)
        {
            const size_t n = std::fread(outFrame.PixelData.data() + read, 1, expected - read, pipe);
            if (!n) break;
            read += n;
        }
#ifdef _WIN32
        ec = _pclose(pipe);
#else
        ec = pclose(pipe);
#endif
        if (ec != 0 || read != expected) { outFrame.PixelData.clear(); return false; }
        outFrame.Width = width;
        outFrame.Height = height;
        outFrame.Format = PixelFormat::RGBA8;
        outFrame.Time = Core::Time::FromSeconds(seconds);
        outFrame.FrameNumber = static_cast<uint64>(outFrame.Time.ToFrames(m_info.FrameRate > 0 ? m_info.FrameRate : 30.0));
        return true;
    }

    bool FFmpegDecoder::DecodeVideoFrame(Core::Time time, VideoFrame& outFrame)
    {
        if (!m_open || m_info.Type != MediaType::Video) return false;
        const double seconds = std::clamp(time.ToSeconds(), 0.0, std::max(0.0, m_info.Duration.ToSeconds()));
        const bool ok = RunFFmpegFrame(seconds, outFrame);
        if (ok) m_position = time;
        return ok;
    }

    bool FFmpegDecoder::ReadVideoFrame(VideoFrame& outFrame)
    {
        const double step = m_info.FrameRate > 0.0f ? 1.0 / m_info.FrameRate : 1.0 / 30.0;
        const bool ok = RunFFmpegFrame(m_position.ToSeconds(), outFrame);
        if (ok) m_position = m_position + Core::Time::FromSeconds(step);
        return ok;
    }

    bool FFmpegDecoder::ReadAudio(AudioSamples& outSamples, Core::Time start, Core::Time end)
    {
        if (!m_open || m_info.SampleRate <= 0 || m_info.Channels <= 0 || end <= start) return false;
        const double seconds = std::max(0.0, (end - start).ToSeconds());
        const std::string command = m_ffmpeg + " -v error -ss " + std::to_string(std::max(0.0, start.ToSeconds())) + " -i " + Quote(m_filePath) + " -t " + std::to_string(seconds) + " -f f32le -acodec pcm_f32le -ar " + std::to_string(m_info.SampleRate) + " -ac " + std::to_string(m_info.Channels) + " pipe:1";
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "rb");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (!pipe) return false;
        std::vector<float> samples;
        std::vector<float> buffer(4096);
        while (true)
        {
            const size_t n = std::fread(buffer.data(), sizeof(float), buffer.size(), pipe);
            if (n) samples.insert(samples.end(), buffer.data(), buffer.data() + n);
            if (n < buffer.size()) break;
        }
#ifdef _WIN32
        const int ec = _pclose(pipe);
#else
        const int ec = pclose(pipe);
#endif
        if (ec != 0 || samples.empty()) return false;
        outSamples.StartTime = start;
        outSamples.EndTime = end;
        outSamples.SampleRate = m_info.SampleRate;
        outSamples.Channels = m_info.Channels;
        outSamples.Samples = std::move(samples);
        return true;
    }

    std::unique_ptr<IMediaDecoder> CreateFFmpegDecoder()
    {
        return std::make_unique<FFmpegDecoder>();
    }
}
