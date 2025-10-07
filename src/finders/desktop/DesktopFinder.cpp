#include "DesktopFinder.hpp"
#include "../../helpers/Log.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include <hyprutils/string/String.hpp>
#include <hyprutils/os/Process.hpp>

using namespace Hyprutils::String;
using namespace Hyprutils::OS;

static std::optional<std::string> readFileAsString(const std::string& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec)
        return std::nullopt;

    std::ifstream file(path);
    if (!file.good())
        return std::nullopt;

    return trim(std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())));
}

class CDesktopEntry : public IFinderResult {
  public:
    CDesktopEntry()          = default;
    virtual ~CDesktopEntry() = default;

    virtual eFinderTypes type() {
        return FINDER_DESKTOP;
    }

    virtual void run() {
        Debug::log(TRACE, "Running {}", m_exec);

        // replace all funky codes with nothing
        auto toExec = m_exec;
        replaceInString(toExec, "%U", "");
        replaceInString(toExec, "%f", "");
        replaceInString(toExec, "%F", "");
        replaceInString(toExec, "%u", "");
        replaceInString(toExec, "%i", "");
        replaceInString(toExec, "%c", "");
        replaceInString(toExec, "%k", "");
        replaceInString(toExec, "%d", "");
        replaceInString(toExec, "%D", "");
        replaceInString(toExec, "%N", "");
        replaceInString(toExec, "%n", "");

        CProcess proc("/bin/sh", {"-c", toExec});
        proc.runAsync();
    }

    std::string m_name, m_exec, m_icon;
};

static constexpr std::array<const char*, 2> DESKTOP_ENTRY_PATHS = {"/usr/local/share/applications", "/usr/share/applications"};

CDesktopFinder::CDesktopFinder() {
    for (const auto& PATH : DESKTOP_ENTRY_PATHS) {
        std::error_code ec;
        auto            it = std::filesystem::directory_iterator(PATH, ec);
        if (ec)
            continue;
        for (const auto& e : it) {
            if (!e.is_regular_file(ec) || ec)
                continue;

            cacheEntry(e.path().string());
        }
    }
}

void CDesktopFinder::cacheEntry(const std::string& path) {
    Debug::log(TRACE, "desktop: caching entry {}", path);

    const auto READ_RESULT = readFileAsString(path);

    if (!READ_RESULT)
        return;

    const auto& DATA = *READ_RESULT;

    auto        extract = [&DATA](const std::string_view what) -> std::string_view {
        size_t begins = DATA.find("\n" + std::string{what});

        if (begins == std::string::npos)
            return "";

        begins = DATA.find('=', begins);

        if (begins == std::string::npos)
            return "";

        begins += 1; // eat the equals
        while (begins < DATA.size() && std::isspace(DATA[begins])) {
            ++begins;
        }

        size_t ends = DATA.find("\n", begins + 1);

        if (!ends)
            return std::string_view{DATA}.substr(begins);

        return std::string_view{DATA}.substr(begins, ends - begins);
    };

    const auto NAME = extract("Name");
    const auto ICON = extract("Icon");
    const auto EXEC = extract("Exec");

    if (EXEC.empty() || NAME.empty()) {
        Debug::log(TRACE, "Skipping entry, empty name / exec");
        return;
    }

    auto& e   = m_desktopEntryCache.emplace_back(makeShared<CDesktopEntry>());
    e->m_exec = EXEC;
    e->m_icon = ICON;
    e->m_name = NAME;

    Debug::log(TRACE, "Cached: {} with icon {} and exec line of \"{}\"", NAME, ICON, EXEC);
}

std::vector<SFinderResult> CDesktopFinder::getResultsForQuery(const std::string& query) {
    std::vector<SFinderResult> results;
    for (const auto& c : m_desktopEntryCache) {
        if (results.size() >= MAX_RESULTS_PER_FINDER)
            break;

        auto lower = c->m_name;
        std::ranges::transform(lower, lower.begin(), ::tolower);

        // FIXME: this has to be fuzzy.
        if (lower.contains(query))
            results.emplace_back(SFinderResult{.label = c->m_name, .icon = c->m_icon, .result = c});
    }

    return results;
}
