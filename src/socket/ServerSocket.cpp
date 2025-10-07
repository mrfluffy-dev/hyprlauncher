#include "ServerSocket.hpp"
#include "LauncherProtocolSpec.hpp"
#include "../ui/UI.hpp"

#include <cstdlib>

constexpr const char* SOCKET_NAME = ".hyprlauncher.sock";

//
static void onSetState(Hyprwire::IObject* o, uint32_t mode) {
    if (mode == 0)
        g_ui->setWindowOpen(!g_ui->windowOpen());
    else if (mode == 1)
        g_ui->setWindowOpen(true);
    else
        g_ui->setWindowOpen(false);
}

static void onBind(SP<Hyprwire::IObject> obj) {
    obj->listen(0, rc<void*>(::onSetState));
}

class CHyprlauncherServerProtocolImpl : public Hyprwire::IProtocolServerImplementation {
  public:
    virtual ~CHyprlauncherServerProtocolImpl() = default;

    virtual SP<Hyprwire::IProtocolSpec> protocol() {
        return g_hyprlauncherProto;
    }

    virtual std::vector<SP<Hyprwire::SServerObjectImplementation>> implementation() {
        return {
            makeShared<Hyprwire::SServerObjectImplementation>(Hyprwire::SServerObjectImplementation{
                .objectName = "hyprlanuncher_manager_v1",
                .version    = 1,
                .onBind     = ::onBind,
            }),
        };
    }
};

CServerIPCSocket::CServerIPCSocket() {
    const auto RTDIR = getenv("XDG_RUNTIME_DIR");

    if (!RTDIR)
        return;

    m_socketPath = RTDIR + std::string{"/"} + SOCKET_NAME;

    m_socket = Hyprwire::IServerSocket::open(m_socketPath);

    if (!m_socket)
        return;

    auto x = makeShared<CHyprlauncherServerProtocolImpl>();

    m_socket->addImplementation(x);
}
