#include "ResultButton.hpp"
#include "../finders/IFinder.hpp"

#include "UI.hpp"

CResultButton::CResultButton() {
    m_background = Hyprtoolkit::CRectangleBuilder::begin()
                       ->color([]() {
                           auto c = g_ui->m_backend->getPalette()->m_colors.accent.darken(0.3F);
                           c.a    = 0.F;
                           return c;
                       })
                       ->rounding(4)
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1, 30}})
                       ->commence();

    m_container = Hyprtoolkit::CNullBuilder::begin()->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1, 1}})->commence();
    m_container->setMargin(4);

    m_label = Hyprtoolkit::CTextBuilder::begin()
                  ->text(std::string{m_lastLabel})
                  ->align(Hyprtoolkit::HT_FONT_ALIGN_LEFT)
                  ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1, 1}})
                  ->commence();

    m_label->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_LEFT);

    m_background->addChild(m_container);
    m_container->addChild(m_label);
}

void CResultButton::setActive(bool active) {
    if (active == m_active)
        return;

    m_active = active;

    m_background->rebuild()
        ->color([this]() {
            auto c = g_ui->m_backend->getPalette()->m_colors.accent.darken(0.3F);
            c.a    = m_active ? 0.4F : 0.F;
            return c;
        })
        ->commence();
}

void CResultButton::setLabel(const std::string& x) {
    if (x == m_lastLabel)
        return;

    m_lastLabel = x;

    m_label->rebuild()->text(std::string{x})->commence();
}
