#pragma once

#include "../IFinder.hpp"

class CMathEntry;

class CMathFinder : public IFinder {
  public:
    CMathFinder();
    virtual ~CMathFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query);
};

inline UP<CMathFinder> g_mathFinder;
