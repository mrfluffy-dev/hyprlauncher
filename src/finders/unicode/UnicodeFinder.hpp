#pragma once

#include "../IFinder.hpp"

class CUnicodeEntry;
class CEntryCache;

class CUnicodeFinder : public IFinder {
  public:
    CUnicodeFinder();
    virtual ~CUnicodeFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query);
    virtual void                       init();

  private:
    std::vector<SP<CUnicodeEntry>> m_unicodeEntryCache;
    std::vector<SP<IFinderResult>> m_unicodeEntryCacheGeneric;

    UP<CEntryCache>                m_entryFrequencyCache;

    void                           cacheEntry(const std::string& path);

    friend class CUnicodeEntry;
};

inline UP<CUnicodeFinder> g_unicodeFinder;
