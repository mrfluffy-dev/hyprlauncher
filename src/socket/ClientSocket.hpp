#pragma once

#include <hyprwire/hyprwire.hpp>
#include <hyprlauncher_core-client.hpp>

#include "../helpers/Memory.hpp"

class CClientIPCSocket {
  public:
    CClientIPCSocket();
    ~CClientIPCSocket() = default;

    bool m_connected = false;

    void sendOpen();

  private:
    SP<Hyprwire::IClientSocket>         m_socket;

    SP<CCHyprlauncherCoreManagerObject> m_manager;

    std::string                         m_socketPath = "";
};
