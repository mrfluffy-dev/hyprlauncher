#pragma once

#include "../helpers/Memory.hpp"
#include "IFinderResult.hpp"

#include <string>
#include <vector>

struct SFinderResult {
    std::string       label;
    std::string       icon;
    SP<IFinderResult> result;
};

constexpr const size_t MAX_RESULTS_PER_FINDER = 15;

class IFinder {
  public:
    virtual ~IFinder() = default;

    virtual std::vector<SFinderResult> getResultsForQuery(const std::string& query) = 0;

    virtual void                       init();

  protected:
    IFinder() = default;
};