#include "Fuzzy.hpp"
#include <algorithm>

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

static float jaroWinklerFull(const std::string& a, const std::string& b, float prefixScale = 0.1F) {
    const float  JARO_RESULT = jaroWinkler(a, b);
    size_t       prefixLen   = 0;
    const size_t MAXPREFIX   = 4;

    while (prefixLen < std::min({a.size(), b.size(), MAXPREFIX}) && a[prefixLen] == b[prefixLen]) {
        ++prefixLen;
    }

    return JARO_RESULT + (sc<float>(prefixLen) * prefixScale * (1.F - JARO_RESULT));
}

struct SScoreData {
    float             score = 0.F;
    SP<IFinderResult> result;
};

std::vector<SP<IFinderResult>> Fuzzy::getNResults(const std::vector<SP<IFinderResult>>& in, const std::string& query, size_t results) {
    std::vector<SScoreData> scores;
    scores.resize(in.size());

    for (size_t i = 0; i < in.size(); ++i) {
        scores[i] = {.score = jaroWinklerFull(query, in[i]->fuzzable()), .result = in[i]};
    }

    std::partial_sort(scores.begin(), scores.begin() + std::min(results, in.size()), scores.end(), [](const auto& a, const auto& b) { return a.score > b.score; });

    std::vector<SP<IFinderResult>> resVec;
    resVec.resize(std::min(in.size(), results));

    for (size_t i = 0; i < resVec.size(); ++i) {
        resVec[i] = scores[i].result;
    }

    return resVec;
}