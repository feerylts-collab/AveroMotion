#include "Media/FrameCache.h"
#include <functional>

namespace Avero::Media
{
    size_t FrameCache::KeyHash::operator()(const Key& key) const
    {
        const size_t a = std::hash<uint64_t>{}(key.AssetId);
        const size_t b = std::hash<int64_t>{}(key.TimeTicks);
        return a ^ (b + static_cast<size_t>(0x9e3779b9) + (a << 6) + (a >> 2));
    }

    void FrameCache::SetMaxFrames(size_t maxFrames)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxFrames = maxFrames;
        while (m_lru.size() > m_maxFrames)
        {
            m_lookup.erase(m_lru.back().KeyValue);
            m_lru.pop_back();
        }
    }

    size_t FrameCache::GetMaxFrames() const { std::lock_guard<std::mutex> lock(m_mutex); return m_maxFrames; }
    size_t FrameCache::Size() const { std::lock_guard<std::mutex> lock(m_mutex); return m_lru.size(); }

    std::shared_ptr<VideoFrame> FrameCache::Find(uint64_t assetId, Core::Time time)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const Key key{assetId, time.Ticks};
        auto it = m_lookup.find(key);
        if (it == m_lookup.end()) return nullptr;
        m_lru.splice(m_lru.begin(), m_lru, it->second);
        return it->second->Frame;
    }

    void FrameCache::Put(uint64_t assetId, Core::Time time, std::shared_ptr<VideoFrame> frame)
    {
        if (!frame || m_maxFrames == 0) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        const Key key{assetId, time.Ticks};
        auto existing = m_lookup.find(key);
        if (existing != m_lookup.end())
        {
            existing->second->Frame = std::move(frame);
            m_lru.splice(m_lru.begin(), m_lru, existing->second);
            return;
        }
        m_lru.push_front(Entry{key, std::move(frame)});
        m_lookup[key] = m_lru.begin();
        while (m_lru.size() > m_maxFrames)
        {
            m_lookup.erase(m_lru.back().KeyValue);
            m_lru.pop_back();
        }
    }

    void FrameCache::Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lookup.clear();
        m_lru.clear();
    }
}
