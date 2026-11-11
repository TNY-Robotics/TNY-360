#include "ui/menus/Speaker.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

MenuSpeaker::MenuSpeaker(Menu* parent)
    : Menu("Speaker", parent, Icons::SpeakerMenu)
{
}

bool MenuSpeaker::onBack()
{
    return false; // not handled, trigger default behavior
}

bool MenuSpeaker::onSelect()
{
    return true; // handled
}

bool MenuSpeaker::onNext()
{
    return true; // handled
}

bool MenuSpeaker::onPrev()
{
    return true; // handled
}


void MenuSpeaker::onShow()
{
    
}

void MenuSpeaker::onHide()
{ 
}

void MenuSpeaker::onRender()
{
    renderHeader();
}

void MenuSpeaker::onUpdate()
{
}
