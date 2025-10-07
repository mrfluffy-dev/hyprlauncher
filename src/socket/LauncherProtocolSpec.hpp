#pragma once

#include <hyprwire/core/types/MessageMagic.hpp>
#include <hyprwire/hyprwire.hpp>

class CHyprlauncherManagerObject : public Hyprwire::IProtocolObjectSpec {
  public:
    CHyprlauncherManagerObject()          = default;
    virtual ~CHyprlauncherManagerObject() = default;

    virtual std::string objectName() {
        return "hyprlanuncher_manager_v1";
    }

    std::vector<Hyprwire::SMethod> m_c2s = {
        Hyprwire::SMethod{
            /* set_state */
            .idx    = 0,
            .params = {Hyprwire::HW_MESSAGE_MAGIC_TYPE_UINT /* 0 - toggle, 1 - open, 2 - close */},
        },
    };

    std::vector<Hyprwire::SMethod>                m_s2c = {};

    virtual const std::vector<Hyprwire::SMethod>& c2s() {
        return m_c2s;
    }
    virtual const std::vector<Hyprwire::SMethod>& s2c() {
        return m_s2c;
    }
};

class CHyprlauncherProtocolSpec : public Hyprwire::IProtocolSpec {
  public:
    CHyprlauncherProtocolSpec()          = default;
    virtual ~CHyprlauncherProtocolSpec() = default;

    virtual std::string specName() {
        return "hyprlauncher_v1";
    }

    virtual uint32_t specVer() {
        return 1;
    }

    virtual std::vector<Hyprutils::Memory::CSharedPointer<Hyprwire::IProtocolObjectSpec>> objects() {
        return {Hyprutils::Memory::makeShared<CHyprlauncherManagerObject>()};
    }
};

inline Hyprutils::Memory::CSharedPointer<CHyprlauncherProtocolSpec> g_hyprlauncherProto = Hyprutils::Memory::makeShared<CHyprlauncherProtocolSpec>();
