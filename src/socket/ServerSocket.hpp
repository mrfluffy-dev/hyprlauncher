#pragma once

#include <hyprwire/hyprwire.hpp>

#include "../helpers/Memory.hpp"

class CServerIPCSocket {
  public:
    CServerIPCSocket();
    ~CServerIPCSocket() = default;

  private:
    SP<Hyprwire::IServerSocket> m_socket;

    std::string                 m_socketPath = "";

    friend class CUI;
};

inline UP<CServerIPCSocket> g_serverIPCSocket;
