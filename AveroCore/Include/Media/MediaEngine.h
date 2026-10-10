#pragma once

#include "Core/AveroTypes.h"
#include "Media/MediaAsset.h"
#include "Media/IMediaDecoder.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Avero::Media
{
    using Avero::Core::uint64;

    class MediaEngine
    {
    public:
        using DecoderFactory = std::function<std::unique_ptr<IMediaDecoder>()>;

        MediaEngine() = default;
        ~MediaEngine() { Shutdown(); }

        void Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        void RegisterDecoder(DecoderFactory factory);
        bool HasDecoderFor(const std::string& filePath) const;

        std::shared_ptr<MediaAsset> LoadMedia(uint64 id, const std::string& filePath);
        std::shared_ptr<MediaAsset> GetAsset(uint64 id) const;
        IMediaDecoder* GetDecoder(uint64 id) const;

        bool DecodeVideoFrame(uint64 id, Core::Time time, VideoFrame& outFrame);
        bool ReadAudio(uint64 id, Core::Time start, Core::Time end, AudioSamples& outSamples);

        void UnloadAsset(uint64 id);
        void ClearAll();
        size_t GetAssetCount() const;

    private:
        struct AssetState
        {
            std::shared_ptr<MediaAsset> Asset;
            std::unique_ptr<IMediaDecoder> Decoder;
        };

        mutable std::mutex m_mutex;
        bool m_initialized = false;
        std::vector<DecoderFactory> m_decoderFactories;
        std::unordered_map<uint64, AssetState> m_assets;
    };
}
