#pragma once

#include "Core/Time.h"
#include "Media/MediaAsset.h"
#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Avero::Media
{
    class FrameCache
    {
    public:
        explicit FrameCache(size_t maxFrames = 120) : m_maxFrames(maxFrames) {}

        void SetMaxFrames(size_t maxFrames);
        size_t GetMaxFrames() const;
        size_t Size() const;

        std::shared_ptr<VideoFrame> Find(uint64_t assetId, Core::Time time);
        void Put(uint64_t assetId, Core::Time time, std::shared_ptr<VideoFrame> frame);
        void Clear();

    private:
        struct Key
        {
            uint64_t AssetId = 0;
            int64_t TimeTicks = 0;
            bool operator==(const Key& other) const { return AssetId == other.AssetId && TimeTicks == other.TimeTicks; }
        };
        struct KeyHash
        {
            size_t operator()(const Key& key) const;
        };
        struct Entry
        {
            Key KeyValue;
            std::shared_ptr<VideoFrame> Frame;
        };

        mutable std::mutex m_mutex;
        size_t m_maxFrames;
        std::list<Entry> m_lru;
        std::unordered_map<Key, std::list<Entry>::iterator, KeyHash> m_lookup;
    };
}
