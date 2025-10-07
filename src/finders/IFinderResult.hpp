#pragma once

#include "FinderTypes.hpp"

class IFinderResult {
  public:
    virtual ~IFinderResult() = default;

    virtual eFinderTypes type() = 0;
    virtual void         run()  = 0;

  protected:
    IFinderResult() = default;
};