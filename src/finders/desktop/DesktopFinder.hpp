#pragma once

#include "../IFinder.hpp"

#include <hyprutils/os/FileDescriptor.hpp>

class CDesktopEntry;

class CDesktopFinder : public IFinder {
  public:
    CDesktopFinder();
    virtual ~CDesktopFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query);

    Hyprutils::OS::CFileDescriptor     m_inotifyFd;

    void                               onInotifyEvent();

  private:
    std::vector<SP<CDesktopEntry>> m_desktopEntryCache;
    std::vector<SP<IFinderResult>> m_desktopEntryCacheGeneric;

    std::vector<std::string>       m_desktopEntryPaths;
    std::vector<int>               m_watches;

    void                           cacheEntry(const std::string& path);
    void                           replantWatch();
    void                           recache();
};

inline UP<CDesktopFinder> g_desktopFinder;
