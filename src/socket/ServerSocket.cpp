#include "ServerSocket.hpp"
#include "../ui/UI.hpp"

#include <cstdlib>

constexpr const char*            SOCKET_NAME = ".hyprlauncher.sock";

static SP<CHyprlauncherCoreImpl> g_coreImpl;

CServerIPCSocket::CServerIPCSocket() {
    const auto RTDIR = getenv("XDG_RUNTIME_DIR");

    if (!RTDIR)
        return;

    m_socketPath = RTDIR + std::string{"/"} + SOCKET_NAME;

    m_socket = Hyprwire::IServerSocket::open(m_socketPath);

    if (!m_socket)
        return;

    g_coreImpl = makeShared<CHyprlauncherCoreImpl>(1, [this](SP<Hyprwire::IObject> obj) {
        auto manager = m_managers.emplace_back(makeShared<CHyprlauncherCoreManagerObject>(std::move(obj)));

        manager->setSetOpenState([this](uint32_t state) { setOpenState(state); });
        manager->setOnDestroy([this, weak = WP<CHyprlauncherCoreManagerObject>{manager}] { std::erase(m_managers, weak); });
    });

    m_socket->addImplementation(g_coreImpl);
}

void CServerIPCSocket::setOpenState(uint32_t state) {
    switch (state) {
        case 0: g_ui->setWindowOpen(!g_ui->windowOpen()); break;
        case 1: g_ui->setWindowOpen(true); break;
        case 2: g_ui->setWindowOpen(false); break;
        default: break;
    }
}
