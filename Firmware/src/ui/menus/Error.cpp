#include "ui/menus/Error.hpp"
#include "ui/Draw.hpp"
#include "common/LED.hpp"
#include "ui/Icons.hpp"

MenuError::MenuError()
    : Menu("Error", nullptr, Icons::ErrorMenu)
{
}

bool MenuError::onBack()
{
    LED::clearErrorCode();
    Menus::SetCurrentMenu(Menus::GetMenuFace());
    return true;
}

bool MenuError::onSelect()
{
    return false;
}

bool MenuError::onNext()
{
    return false;
}

bool MenuError::onPrev()
{
    return false;
}

void MenuError::onShow()
{
}

void MenuError::onHide()
{
}

void MenuError::onRender()
{
    renderHeader();

    // constexpr uint8_t NB_LINE_CHARS = 120/8;

    // char errCodeText[NB_LINE_CHARS];
    // sprintf(errCodeText, "Code: 0x%02X", error.code);
    // Draw::Text(2, HEADER_HEIGHT + 4, errCodeText);

    // Draw::Text<true>(2, HEADER_HEIGHT + 4 + 12, "Message:");
    // Draw::Text<true>(2, HEADER_HEIGHT + 4 + 12 + 12, error.msg);
}

void MenuError::onUpdate()
{
    
}

// void MenuError::setError(ErrorStruct::ErrorStruct err)
// {
//     error = err;
// }