#include "ui/menus/Microphone.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

MenuMicrophone::MenuMicrophone(Menu* parent)
    : Menu("Microphone", parent, Icons::MicrophoneMenu)
{
}

bool MenuMicrophone::onBack()
{
    return false;
}

bool MenuMicrophone::onSelect()
{
    return false;
}

bool MenuMicrophone::onNext()
{
    return false;
}

bool MenuMicrophone::onPrev()
{
    return false;
}


void MenuMicrophone::onShow()
{
}

void MenuMicrophone::onHide()
{ 
}

void MenuMicrophone::onRender()
{
    renderHeader();

    Microphone* mic = Robot::GetInstance().getAudioManager().getMicrophone();
    if (mic == nullptr)
    {
        Draw::Text(0, HEADER_HEIGHT + HEADER_PADDING, "Microphone is disabled.");
        return;
    }

    uint16_t screen_width = Screen::GetInstance()->getInfo().width;
    uint16_t screen_height = Screen::GetInstance()->getInfo().height;
    uint16_t preview_width = 80;
    uint16_t preview_height = 40;
    uint16_t preview_x = screen_width / 2 - preview_width / 2;
    uint16_t preview_y = screen_height / 2 - preview_height / 2;

    Microphone::Sample buffer[80];
    size_t nb_read = mic->readSamples(buffer, sizeof(buffer) / sizeof(buffer[0]));
    if (nb_read == 0)
    {
        Draw::Text(0, HEADER_HEIGHT + HEADER_PADDING, "No samples read.");
        return;
    }

    // Draw preview box
    Draw::Rect(preview_x - 1, preview_y - 1, preview_width + 2, preview_height + 2);

    // Draw samples
    for (size_t i = 0; i < nb_read - 1; i++)
    {
        int16_t sample1 = buffer[i];
        int16_t sample2 = buffer[i + 1];

        int16_t y1 = preview_y + (preview_height / 2) - (sample1 * (preview_height / 2) / std::numeric_limits<Microphone::Sample>::max());
        int16_t y2 = preview_y + (preview_height / 2) - (sample2 * (preview_height / 2) / std::numeric_limits<Microphone::Sample>::max());

        Draw::Line(preview_x + i, y1, preview_x + i + 1, y2);
    }
}

void MenuMicrophone::onUpdate()
{
    triggerRender();
}
