#include "UI.hpp"
#include "ResultButton.hpp"

#include "../query/QueryProcessor.hpp"
#include "../socket/ServerSocket.hpp"
#include "../helpers/Log.hpp"
#include "../config/ConfigManager.hpp"

#include <hyprutils/string/String.hpp>
#include <xkbcommon/xkbcommon-keysyms.h>

using namespace Hyprutils::Math;
using namespace Hyprutils::String;

constexpr const size_t MAX_RESULTS_IN_LAUNCHER = 25;

CUI::CUI() {
    m_backend = Hyprtoolkit::CBackend::create();
    m_window  = Hyprtoolkit::CWindowBuilder::begin()
                   ->appClass("hyprlauncher")
                   ->type(Hyprtoolkit::HT_WINDOW_LAYER)
                   ->preferredSize({400, 260})
                   ->anchor(1 | 2 | 4 | 8)
                   ->exclusiveZone(-1)
                   ->layer(3)
                   ->kbInteractive(2)
                   ->commence();

    m_background = Hyprtoolkit::CRectangleBuilder::begin()
                       ->color([this] { return m_backend->getPalette()->m_colors.background; })
                       ->rounding(10)
                       ->borderColor([this] { return m_backend->getPalette()->m_colors.accent.darken(0.2F); })
                       ->borderThickness(1)
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1, 1}})
                       ->commence();

    m_layout =
        Hyprtoolkit::CColumnLayoutBuilder::begin()->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1, 1}})->gap(4)->commence();
    m_layout->setMargin(4);

    m_inputBox = Hyprtoolkit::CTextboxBuilder::begin()
                     ->placeholder("Search something...")
                     ->onTextEdited([](SP<Hyprtoolkit::CTextboxElement>, const std::string& query) { g_queryProcessor->scheduleQueryUpdate(query); })
                     ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 40.F}})
                     ->commence();

    m_hr = Hyprtoolkit::CRectangleBuilder::begin()
               ->color([this] { return m_backend->getPalette()->m_colors.accent.darken(0.2F); })
               ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {0.8F, 1.F}})
               ->commence();
    m_hr->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_HCENTER);

    m_scrollArea = Hyprtoolkit::CScrollAreaBuilder::begin()
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 1.F}})
                       ->scrollY(true)
                       ->commence();
    m_scrollArea->setGrow(true);

    m_resultsLayout =
        Hyprtoolkit::CColumnLayoutBuilder::begin()->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO, {1, 1}})->gap(2)->commence();

    m_window->m_rootElement->addChild(m_background);

    m_background->addChild(m_layout);

    m_layout->addChild(m_inputBox);
    m_layout->addChild(m_hr);
    m_layout->addChild(m_scrollArea);

    m_scrollArea->addChild(m_resultsLayout);

    m_window->m_events.keyboardKey.listenStatic([this](Hyprtoolkit::Input::SKeyboardKeyEvent e) {
        if (e.xkbKeysym == XKB_KEY_Escape)
            setWindowOpen(false);
        else if (e.xkbKeysym == XKB_KEY_Down) {
            if (m_activeElementId < m_currentResults.size())
                m_activeElementId++;
            updateActive();
        } else if (e.xkbKeysym == XKB_KEY_Up) {
            if (m_activeElementId > 0)
                m_activeElementId--;
            updateActive();
        } else if (e.xkbKeysym == XKB_KEY_Return) {
            m_currentResults.at(m_activeElementId).result->run();
            setWindowOpen(false);
        }
    });
}

CUI::~CUI() = default;

void CUI::run() {
    m_resultButtons.reserve(MAX_RESULTS_IN_LAUNCHER);
    for (size_t i = 0; i < MAX_RESULTS_IN_LAUNCHER; ++i) {
        auto b = m_resultButtons.emplace_back(makeShared<CResultButton>());
        m_resultsLayout->addChild(b->m_background);
    }

    setWindowOpen(true);

    if (g_serverIPCSocket->m_socket) {
        m_backend->addFd(g_serverIPCSocket->m_socket->extractLoopFD(), [] {
            Debug::log(TRACE, "got an ipc event");
            g_serverIPCSocket->m_socket->dispatchEvents(false);
        });
    }

    m_backend->addFd(g_configManager->m_inotifyFd.get(), [] { g_configManager->onInotifyEvent(); });

    m_backend->enterLoop();
}

void CUI::setWindowOpen(bool open) {
    if (open == m_open)
        return;

    m_open = open;

    if (open) {
        // FIXME: why does the LS take like 5s to open if we just do close() open()?!

        m_window = Hyprtoolkit::CWindowBuilder::begin()
                       ->appClass("hyprlauncher")
                       ->type(Hyprtoolkit::HT_WINDOW_LAYER)
                       ->preferredSize({400, 260})
                       ->anchor(1 | 2 | 4 | 8)
                       ->exclusiveZone(-1)
                       ->layer(3)
                       ->kbInteractive(2)
                       ->commence();

        m_window->m_rootElement->addChild(m_background);

        m_window->m_events.keyboardKey.listenStatic([this](Hyprtoolkit::Input::SKeyboardKeyEvent e) {
            if (e.xkbKeysym == XKB_KEY_Escape)
                setWindowOpen(false);
            else if (e.xkbKeysym == XKB_KEY_Down) {
                if (m_activeElementId < m_resultButtons.size())
                    m_activeElementId++;
                updateActive();
            } else if (e.xkbKeysym == XKB_KEY_Up) {
                if (m_activeElementId > 0)
                    m_activeElementId--;
                updateActive();
            } else if (e.xkbKeysym == XKB_KEY_Return) {
                m_currentResults.at(m_activeElementId).result->run();
                setWindowOpen(false);
            }
        });

        m_window->open();

        m_inputBox->rebuild()->defaultText("")->commence();
        m_inputBox->focus();
    } else {
        m_window->close();
        m_window.reset();
    }
}

bool CUI::windowOpen() {
    return m_open;
}

void CUI::updateResults(std::vector<SFinderResult>&& results) {
    m_currentResults = std::move(results);

    m_activeElementId = 0;

    for (size_t i = 0; i < m_resultButtons.size(); ++i) {
        m_resultButtons[i]->setLabel(m_currentResults.size() <= i ? "" : m_currentResults[i].label);
    }

    if (m_resultButtons.size() > 0)
        m_resultButtons[m_activeElementId]->setActive(true);
}

void CUI::updateActive() {
    for (size_t i = 0; i < m_resultButtons.size(); ++i) {
        m_resultButtons[i]->setActive(i == m_activeElementId);
    }
}
