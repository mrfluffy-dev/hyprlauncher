#pragma once

#include "../IFinder.hpp"

class CUnicodeEntry;

class CUnicodeFinder : public IFinder {
  public:
    CUnicodeFinder();
    virtual ~CUnicodeFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query);

  private:
    std::vector<SP<CUnicodeEntry>> m_unicodeEntryCache;
    std::vector<SP<IFinderResult>> m_unicodeEntryCacheGeneric;

    void                           cacheEntry(const std::string& path);
};

inline UP<CUnicodeFinder> g_unicodeFinder;
