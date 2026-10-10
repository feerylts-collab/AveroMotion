#include "Media/MediaEngine.h"
#include <algorithm>
#include <fstream>

namespace Avero::Media
{
    void MediaEngine::Initialize()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_initialized = true;
    }

    void MediaEngine::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, state] : m_assets)
            if (state.Decoder) state.Decoder->Close();
        m_assets.clear();
        m_decoderFactories.clear();
        m_initialized = false;
    }

    void MediaEngine::RegisterDecoder(DecoderFactory factory)
    {
        if (!factory) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_decoderFactories.push_back(std::move(factory));
    }

    bool MediaEngine::HasDecoderFor(const std::string& filePath) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& factory : m_decoderFactories)
        {
            auto decoder = factory ? factory() : nullptr;
            if (decoder && decoder->CanOpen(filePath)) return true;
        }
        return false;
    }

    std::shared_ptr<MediaAsset> MediaEngine::LoadMedia(uint64 id, const std::string& filePath)
    {
        if (filePath.empty()) return nullptr;

        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& factory : m_decoderFactories)
        {
            auto decoder = factory ? factory() : nullptr;
            if (!decoder || !decoder->CanOpen(filePath)) continue;
            if (decoder->Open(filePath) != MediaOpenResult::Success) continue;

            auto asset = std::make_shared<MediaAsset>();
            asset->ID = id;
            asset->Info = decoder->GetMediaInfo();
            asset->Info.FilePath = filePath;
            const size_t pos = filePath.find_last_of("/\\");
            asset->Info.FileName = pos == std::string::npos ? filePath : filePath.substr(pos + 1);
            asset->IsLoaded = true;

            AssetState state;
            state.Asset = asset;
            state.Decoder = std::move(decoder);
            m_assets[id] = std::move(state);
            return asset;
        }
        return nullptr;
    }

    std::shared_ptr<MediaAsset> MediaEngine::GetAsset(uint64 id) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_assets.find(id);
        return it == m_assets.end() ? nullptr : it->second.Asset;
    }

    IMediaDecoder* MediaEngine::GetDecoder(uint64 id) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_assets.find(id);
        return it == m_assets.end() ? nullptr : it->second.Decoder.get();
    }

    bool MediaEngine::DecodeVideoFrame(uint64 id, Core::Time time, VideoFrame& outFrame)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_assets.find(id);
        return it != m_assets.end() && it->second.Decoder && it->second.Decoder->DecodeVideoFrame(time, outFrame);
    }

    bool MediaEngine::ReadAudio(uint64 id, Core::Time start, Core::Time end, AudioSamples& outSamples)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_assets.find(id);
        return it != m_assets.end() && it->second.Decoder && it->second.Decoder->ReadAudio(outSamples, start, end);
    }

    void MediaEngine::UnloadAsset(uint64 id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_assets.find(id);
        if (it == m_assets.end()) return;
        if (it->second.Decoder) it->second.Decoder->Close();
        m_assets.erase(it);
    }

    void MediaEngine::ClearAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, state] : m_assets)
            if (state.Decoder) state.Decoder->Close();
        m_assets.clear();
    }

    size_t MediaEngine::GetAssetCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_assets.size();
    }
}
