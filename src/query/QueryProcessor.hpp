#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>

#include "../helpers/Memory.hpp"

class CQueryProcessor {
  public:
    CQueryProcessor();
    ~CQueryProcessor();

    void scheduleQueryUpdate(const std::string& str);

  private:
    std::condition_variable m_threadCV;
    std::mutex              m_threadMutex, m_queryStrMutex;
    std::thread             m_queryThread;
    std::string             m_pendingQuery;
    bool                    m_quit = false, m_event = false;
};

inline UP<CQueryProcessor> g_queryProcessor = makeUnique<CQueryProcessor>();