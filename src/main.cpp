#include "ui/UI.hpp"
#include "helpers/Log.hpp"
#include "finders/desktop/DesktopFinder.hpp"
#include "finders/unicode/UnicodeFinder.hpp"
#include "finders/math/MathFinder.hpp"
#include "socket/ClientSocket.hpp"
#include "socket/ServerSocket.hpp"
#include "config/ConfigManager.hpp"

int main(int argc, char** argv, char** envp) {

    auto socket = makeShared<CClientIPCSocket>();

    if (socket->m_connected) {
        Debug::log(LOG, "Active instance already, opening launcher.");
        socket->sendOpen();
        return 0;
    }

    g_desktopFinder = makeUnique<CDesktopFinder>();
    g_unicodeFinder = makeUnique<CUnicodeFinder>();
    g_mathFinder    = makeUnique<CMathFinder>();

    for (int i = 1; i < argc; ++i) {
        std::string_view sv{argv[i]};

        if (sv == "--verbose") {
            Debug::verbose = true;
            continue;
        } else if (sv == "--quiet") {
            Debug::quiet = true;
            continue;
        }
    }

    socket.reset();

    g_serverIPCSocket = makeUnique<CServerIPCSocket>();
    g_configManager   = makeUnique<CConfigManager>();
    g_configManager->parse();

    g_ui = makeUnique<CUI>();
    g_ui->run();
    return 0;
}