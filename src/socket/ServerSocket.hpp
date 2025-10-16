#pragma once

#include <hyprwire/hyprwire.hpp>
#include <hyprlauncher_core-server.hpp>

#include "../helpers/Memory.hpp"

class CServerIPCSocket {
  public:
    CServerIPCSocket();
    ~CServerIPCSocket() = default;

  private:
    void                                            setOpenState(uint32_t state);

    SP<Hyprwire::IServerSocket>                     m_socket;

    std::string                                     m_socketPath = "";

    std::vector<SP<CHyprlauncherCoreManagerObject>> m_managers;

    friend class CUI;
};

inline UP<CServerIPCSocket> g_serverIPCSocket;
