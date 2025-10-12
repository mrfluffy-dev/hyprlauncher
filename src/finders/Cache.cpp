#include "Cache.hpp"
#include "../config/ConfigManager.hpp"

#include <filesystem>
#include <fstream>
#include <chrono>

#include <hyprutils/string/String.hpp>
#include <hyprutils/string/ConstVarList.hpp>
#include <hyprutils/memory/Casts.hpp>

using namespace Hyprutils::String;
using namespace Hyprutils::Memory;

constexpr const size_t CACHE_MAX_SIZE  = 100;
constexpr const size_t CACHE_MAX_VALUE = 10;

CEntryCache::CEntryCache(const std::string& name) {
    const auto HOME = getenv("HOME");

    if (!HOME)
        return;

    std::error_code ec;
    if (!std::filesystem::exists(HOME + std::string{"/.local/share/hyprlauncher"}, ec) || ec)
        std::filesystem::create_directory(HOME + std::string{"/.local/share/hyprlauncher"});
    m_cacheFullPath = HOME + std::string{"/.local/share/hyprlauncher/" + name + ".cache"};

    // load cache
    std::ifstream ifs(m_cacheFullPath);

    if (!ifs.good())
        return;

    std::string line;
    while (std::getline(ifs, line)) {
        CConstVarList data(line, 0, ',', true);
        try {
            SCacheEntry e;
            e.epoch = std::stoull(std::string{data[2]});
            e.value = std::stoul(std::string{data[1]});
            m_cache.emplace(data[0], e);
        } catch (...) { ; }
    }

    m_good = true;
}

bool CEntryCache::good() {
    return m_good;
}

uint32_t CEntryCache::getCachedEntry(const std::string& entry) {
    try {
        return m_cache.at(entry).value;
    } catch (...) { ; }

    return 0;
}

void CEntryCache::incrementCachedEntry(const std::string& entry) {
    auto& e = m_cache[entry];
    e.value = std::clamp(e.value + 1, sc<uint32_t>(0), sc<uint32_t>(CACHE_MAX_VALUE));
    e.epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    save();
}

void CEntryCache::trimCache() {
    if (m_cache.size() <= CACHE_MAX_SIZE)
        return;

    auto removeMin = [this]() {
        size_t      minVal = 31284734285435453;
        std::string minKey = "";

        for (const auto& [k, v] : m_cache) {
            if (v.epoch < minVal) {
                minKey = k;
                minVal = v.epoch;
            }
        }

        if (!minKey.empty())
            m_cache.erase(minKey);
    };

    const auto TO_REMOVE = m_cache.size() - CACHE_MAX_SIZE;
    for (size_t i = 0; i < TO_REMOVE; ++i) {
        removeMin();
    }
}

void CEntryCache::save() {
    static auto PENABLECACHE = Hyprlang::CSimpleConfigValue<Hyprlang::INT>(g_configManager->m_config.get(), "cache:enabled");

    if (!*PENABLECACHE)
        return;

    trimCache();

    std::ofstream ofs(m_cacheFullPath);

    for (const auto& [k, v] : m_cache) {
        ofs << std::format("{},{},{}\n", k, v.value, v.epoch);
    }
}