#pragma once

#include <hyprwire/hyprwire.hpp>

#include "../helpers/Memory.hpp"

class CClientIPCSocket {
  public:
    CClientIPCSocket();
    ~CClientIPCSocket() = default;

    bool m_connected = false;

    void sendOpen();

  private:
    SP<Hyprwire::IClientSocket> m_socket;

    SP<Hyprwire::IObject>       m_protocolMgrObject;

    std::string                 m_socketPath = "";
};
