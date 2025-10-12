#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

class CEntryCache {
  public:
    CEntryCache(const std::string& name);
    ~CEntryCache() = default;

    bool     good();
    uint32_t getCachedEntry(const std::string& entry);
    void     incrementCachedEntry(const std::string& entry);

  private:
    void trimCache();
    void save();

    bool m_good = false;

    struct SCacheEntry {
        uint32_t value = 0;
        uint64_t epoch = 0;
    };

    std::unordered_map<std::string, SCacheEntry> m_cache;
    std::string                                  m_cacheFullPath;
};