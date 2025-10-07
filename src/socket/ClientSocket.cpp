#include "ClientSocket.hpp"
#include "LauncherProtocolSpec.hpp"

#include <cstdlib>

constexpr const char* SOCKET_NAME = ".hyprlauncher.sock";

class CHyprlauncherProtocolImpl : public Hyprwire::IProtocolClientImplementation {
  public:
    virtual ~CHyprlauncherProtocolImpl() = default;

    virtual SP<Hyprwire::IProtocolSpec> protocol() {
        return g_hyprlauncherProto;
    }

    virtual std::vector<SP<Hyprwire::SClientObjectImplementation>> implementation() {
        return {
            makeShared<Hyprwire::SClientObjectImplementation>(Hyprwire::SClientObjectImplementation{
                .objectName = "hyprlanuncher_manager_v1",
                .version    = 1,
            }),
        };
    }
};

CClientIPCSocket::CClientIPCSocket() {
    const auto RTDIR = getenv("XDG_RUNTIME_DIR");

    if (!RTDIR)
        return;

    m_socketPath = RTDIR + std::string{"/"} + SOCKET_NAME;

    m_socket = Hyprwire::IClientSocket::open(m_socketPath);

    if (!m_socket)
        return;

    m_socket->addImplementation(makeShared<CHyprlauncherProtocolImpl>());

    if (!m_socket->waitForHandshake())
        return;

    auto spec = m_socket->getSpec(g_hyprlauncherProto->specName());

    if (!spec) {
        m_socket.reset();
        return;
    }

    m_protocolMgrObject = m_socket->bindProtocol(g_hyprlauncherProto, 1);

    if (!m_protocolMgrObject) {
        m_socket.reset();
        return;
    }

    m_connected = true;
}

void CClientIPCSocket::sendOpen() {
    m_protocolMgrObject->call(0 /* set_state */, 1);
}
