#include "ui/menus/Sound.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

MenuSound::MenuSound(Menu* parent)
    : Menu("Sound", parent, Icons::SoundMenu)
{
}

bool MenuSound::onBack()
{
    if (editing)
    {
        editing = false;
        triggerRender();
        return true; // handled
    }
    triggerRender();
    return false; // not handled, trigger default behavior
}

bool MenuSound::onSelect()
{
    editing = true;
    triggerRender();
    return true; // handled
}

bool MenuSound::onNext()
{
    if (editing)
    {
        switch (optIndex)
        {
        case 0:
            frequency += 10.0f;
            if (frequency > 2000.0f)
                frequency = 2000.0f;
            break;
        case 1:
            volume += 0.1f;
            if (volume > 1.0f)
                volume = 1.0f;
            break;
        case 2:
            enabled = !enabled;
            break;
        case 3:
            sound = !sound;
            break;
        default:
            break;
        }
        applySoundSettings();
    } else optIndex = (optIndex + 1) % 4;
    triggerRender();
    return true; // handled
}

bool MenuSound::onPrev()
{
    if (editing)
    {
        switch (optIndex)
        {
        case 0:
            frequency -= 10.0f;
            if (frequency < 20.0f)
                frequency = 20.0f;
            break;
        case 1:
            volume -= 0.1f;
            if (volume < 0.0f)
                volume = 0.0f;
            break;
        case 2:
            enabled = !enabled;
            break;
        case 3:
            sound = !sound;
        default:
            break;
        }
        applySoundSettings();
    } else optIndex = (optIndex - 1 + 4) % 4;
    triggerRender();
    return true; // handled
}


void MenuSound::onShow()
{
    if (!isProviderAdded)
    {
        Robot::GetInstance().getAudioManager().getMixer().addSoundProvider(&sineProvider);
        sineProvider.setVolume(0.0f);
        isProviderAdded = true;
    }
}

void MenuSound::onHide()
{ 
}

void MenuSound::onRender()
{
    renderHeader();

    {
        char str[128/8+1];
        sprintf(str, "Freq: %d Hz", (int)frequency);
        Draw::Text(4, HEADER_HEIGHT + 4, str);
    }
    {
        char str[128/8+1];
        sprintf(str, "Volume: %d %%", (int)(volume * 100.0f));
        Draw::Text(4, HEADER_HEIGHT + 16, str);
    }
    {
        char str[128/8+1];
        sprintf(str, "Enabled: %s", enabled ? "Yes" : "No");
        Draw::Text(4, HEADER_HEIGHT + 28, str);
    }
    {
        char str[128/8+1];
        sprintf(str, "Sound: %s", sound ? "Yes" : "No");
        Draw::Text(4, HEADER_HEIGHT + 40, str);
    }

    if (editing)
        Draw::RectFilled(0, HEADER_HEIGHT + 4 + optIndex * 12, 2, 8, Screen::COLOR_WHITE);
    else Draw::RectFilled(0, HEADER_HEIGHT + 4 + optIndex * 12 + 2, 2, 4, Screen::COLOR_WHITE);
}

void MenuSound::onUpdate()
{
}

void MenuSound::applySoundSettings()
{
    AudioManager& man = Robot::GetInstance().getAudioManager();
    man.getMixer().setVolume((enabled || sound) ? volume : 0.0f);

    if (sound)
    {
        if (musicProvider == nullptr)
        {
            musicProvider = new MusicProvider();
            musicProvider->loadFromFile("/userdata/song.wav");
            man.getMixer().addSoundProvider(musicProvider);
        }
    }
    else
    {
        if (musicProvider != nullptr)
        {
            musicProvider->stop();
            musicProvider = nullptr; // will be deleted by mixer
        }
    }

    if (enabled)
    {
        sineProvider.setFrequency(frequency);
        sineProvider.setVolume(1.0f);
    } else sineProvider.setVolume(0.0f);
}