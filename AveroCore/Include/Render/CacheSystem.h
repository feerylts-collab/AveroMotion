#pragma once

#include "Core/AveroTypes.h"
#include "Render/Texture.h"

#include <unordered_map>
#include <list>
#include <memory>

namespace Avero::Render
{
    using Avero::Core::uint64;

    class CacheSystem
    {
    public:
        CacheSystem() = default;
        ~CacheSystem() = default;

        void SetMemoryLimit(uint64 limitBytes)
        {
            m_memoryLimit = limitBytes;
            EnforceLimit();
        }

        void CacheTexture(uint64 key, std::shared_ptr<Texture> texture)
        {
            if (!texture) return;

            uint64 newTextureSize = static_cast<uint64>(texture->PixelData.size());

            auto it = m_cacheMap.find(key);
            if (it != m_cacheMap.end())
            {
                uint64 oldSize = static_cast<uint64>(it->second.first->PixelData.size());
                m_currentMemory -= oldSize;
                m_lruList.erase(it->second.second);
                m_cacheMap.erase(it);

                if (newTextureSize > m_memoryLimit)
                    return;
            }
            else if (newTextureSize > m_memoryLimit)
            {
                return;
            }

            while (m_currentMemory + newTextureSize > m_memoryLimit && !m_lruList.empty())
            {
                EvictOldest();
            }

            m_lruList.push_front(key);
            m_cacheMap[key] = { texture, m_lruList.begin() };
            m_currentMemory += newTextureSize;
        }

        std::shared_ptr<Texture> GetCachedTexture(uint64 key)
        {
            auto it = m_cacheMap.find(key);
            if (it != m_cacheMap.end())
            {
                m_lruList.erase(it->second.second);
                m_lruList.push_front(key);
                it->second.second = m_lruList.begin();

                return it->second.first;
            }
            return nullptr;
        }

        void ClearCache()
        {
            m_cacheMap.clear();
            m_lruList.clear();
            m_currentMemory = 0;
        }

        uint64 GetCurrentMemoryUsage() const
        {
            return m_currentMemory;
        }

    private:
        void EvictOldest()
        {
            if (m_lruList.empty()) return;

            uint64 oldestKey = m_lruList.back();
            m_lruList.pop_back();

            auto it = m_cacheMap.find(oldestKey);
            if (it != m_cacheMap.end())
            {
                m_currentMemory -= static_cast<uint64>(it->second.first->PixelData.size());
                m_cacheMap.erase(it);
            }
        }

        void EnforceLimit()
        {
            while (m_currentMemory > m_memoryLimit && !m_lruList.empty())
            {
                EvictOldest();
            }
        }

        std::unordered_map<uint64, std::pair<std::shared_ptr<Texture>, std::list<uint64>::iterator>> m_cacheMap;
        std::list<uint64> m_lruList;

        uint64 m_memoryLimit = 512 * 1024 * 1024;
        uint64 m_currentMemory = 0;
    };
}