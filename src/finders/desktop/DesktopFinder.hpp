#pragma once

#include "../IFinder.hpp"

class CDesktopEntry;

class CDesktopFinder : public IFinder {
  public:
    CDesktopFinder();
    virtual ~CDesktopFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query);

  private:
    std::vector<SP<CDesktopEntry>> m_desktopEntryCache;

    void                           cacheEntry(const std::string& path);
};

inline UP<CDesktopFinder> g_desktopFinder;
