#include "Fuzzy.hpp"
#include <algorithm>
#include <thread>

#include <unistd.h>

static float jaroWinkler(const std::string& a, const std::string& b) {
    const auto LENGTH_A = a.size();
    const auto LENGTH_B = b.size();
    if (LENGTH_A == 0 && LENGTH_B == 0)
        return 1.F;
    if (LENGTH_A == 0 || LENGTH_B == 0)
        return 0.F;

    const auto        MATCH_DIST = (std::max(LENGTH_A, LENGTH_B) / 2) - 1;

    std::vector<bool> matchesA(LENGTH_A, false);
    std::vector<bool> matchesB(LENGTH_B, false);

    size_t            matches = 0;
    for (size_t i = 0; i < LENGTH_A; ++i) {
        const size_t start = (i >= MATCH_DIST) ? i - MATCH_DIST : 0;
        const size_t end   = std::min(i + MATCH_DIST + 1, LENGTH_B);
        for (size_t j = start; j < end; ++j) {
            if (matchesB[j])
                continue;
            if (a[i] == b[j]) {
                matchesA[i] = true;
                matchesB[j] = true;
                ++matches;
                break;
            }
        }
    }
    if (!matches)
        return 0.F;

    size_t transps = 0;
    for (size_t i = 0, j = 0; i < LENGTH_A; ++i) {
        if (!matchesA[i])
            continue;

        while (j < LENGTH_B && !matchesB[j]) {
            ++j;
        }

        if (j < LENGTH_B && a[i] != b[j])
            ++transps;

        ++j;
    }
    const float m   = static_cast<float>(matches);
    const float tps = static_cast<float>(transps) / 2.0f;

    return (m / LENGTH_A + m / LENGTH_B + (m - tps) / m) / 3.0f;
}

static float jaroWinklerFull(const std::string& a, const std::string& b, float prefixScale = 0.1F, float substrScale = 0.4F) {
    const float  JARO_RESULT = jaroWinkler(a, b);
    size_t       prefixLen   = 0;
    const size_t MAXPREFIX   = 4;

    while (prefixLen < std::min({a.size(), b.size(), MAXPREFIX}) && a[prefixLen] == b[prefixLen]) {
        ++prefixLen;
    }

    return JARO_RESULT + (sc<float>(prefixLen) * prefixScale * (1.F - JARO_RESULT)) + (b.contains(a) ? substrScale : 0.F);
}

struct SScoreData {
    float             score = 0.F;
    SP<IFinderResult> result;
};

static void workerFn(std::vector<SScoreData>& scores, const std::vector<SP<IFinderResult>>& in, const std::string& query, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        auto& ref  = scores[i];
        ref.score  = jaroWinklerFull(query, in[i]->fuzzable()) + in[i]->frequency() * 0.05F;
        ref.result = in[i];
    }
}

std::vector<SP<IFinderResult>> Fuzzy::getNResults(const std::vector<SP<IFinderResult>>& in, const std::string& query, size_t results) {
    std::vector<SScoreData> scores;
    scores.resize(in.size());

    // to analyze scores, run this op in parallel
    auto THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    if (THREADS < 1)
        THREADS = 8;

    std::vector<std::thread> workerThreads;
    workerThreads.resize(THREADS);
    size_t workElDone = 0, workElPerThread = in.size() / THREADS;
    for (long i = 0; i < THREADS; ++i) {
        if (i == THREADS - 1) {
            workerThreads[i] = std::thread([&] { workerFn(scores, in, query, workElDone, in.size()); });
            break;
        } else
            workerThreads[i] = std::thread([&] { workerFn(scores, in, query, workElDone, workElDone + workElPerThread); });

        workElDone += workElPerThread;
    }

    for (auto& t : workerThreads) {
        if (t.joinable())
            t.join();
    }

    workerThreads.clear();

    std::partial_sort(scores.begin(), scores.begin() + std::min(results, in.size()), scores.end(), [](const auto& a, const auto& b) { return a.score > b.score; });

    std::vector<SP<IFinderResult>> resVec;
    resVec.resize(std::min(in.size(), results));

    for (size_t i = 0; i < resVec.size(); ++i) {
        resVec[i] = scores[i].result;
    }

    return resVec;
}