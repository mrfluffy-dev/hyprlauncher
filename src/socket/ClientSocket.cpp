#include "ClientSocket.hpp"

#include <cstdlib>

constexpr const char*             SOCKET_NAME = ".hyprlauncher.sock";
static SP<CCHyprlauncherCoreImpl> g_coreImpl;

CClientIPCSocket::CClientIPCSocket() {
    const auto RTDIR = getenv("XDG_RUNTIME_DIR");

    if (!RTDIR)
        return;

    m_socketPath = RTDIR + std::string{"/"} + SOCKET_NAME;

    m_socket = Hyprwire::IClientSocket::open(m_socketPath);

    if (!m_socket)
        return;

    g_coreImpl = makeShared<CCHyprlauncherCoreImpl>(1);

    m_socket->addImplementation(g_coreImpl);

    if (!m_socket->waitForHandshake())
        return;

    auto spec = m_socket->getSpec(g_coreImpl->protocol()->specName());

    if (!spec) {
        m_socket.reset();
        return;
    }

    m_manager = makeShared<CCHyprlauncherCoreManagerObject>(m_socket->bindProtocol(g_coreImpl->protocol(), 1));

    if (!m_manager) {
        m_socket.reset();
        return;
    }

    m_connected = true;
}

void CClientIPCSocket::sendOpen() {
    m_manager->sendSetOpenState(1 /* open */);
}
