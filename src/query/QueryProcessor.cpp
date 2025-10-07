#include "QueryProcessor.hpp"
#include "../ui/UI.hpp"

#include "../finders/desktop/DesktopFinder.hpp"

CQueryProcessor::CQueryProcessor() {
    m_queryThread = std::thread([this] {
        while (!m_quit) {
            std::unique_lock lk(m_threadMutex);
            m_threadCV.wait(lk, [this] { return m_event; });
            m_event = false;

            if (m_quit)
                break;

            m_queryStrMutex.lock();

            std::string query = m_pendingQuery;
            m_pendingQuery    = "";

            m_queryStrMutex.unlock();

            if (query.empty())
                continue;

            auto RESULTS = g_desktopFinder->getResultsForQuery(query);

            if (g_ui->m_backend)
                g_ui->m_backend->addIdle([r = std::move(RESULTS)] mutable { g_ui->updateResults(std::move(r)); });
        }
    });
}

CQueryProcessor::~CQueryProcessor() {
    m_quit         = true;
    m_pendingQuery = "exit";
    m_event        = true;
    m_threadCV.notify_all();
    m_queryThread.join();
}

void CQueryProcessor::scheduleQueryUpdate(const std::string& str) {
    m_queryStrMutex.lock();
    m_pendingQuery = str;
    m_event        = true;
    m_queryStrMutex.unlock();
    m_threadCV.notify_all();
}