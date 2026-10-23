#pragma once
#include "ui/Menus.hpp"
#include "audio/SineProvider.hpp"
#include "audio/MusicProvider.hpp"

class MenuSpeaker : public Menus::Menu
{
public:
    MenuSpeaker() = default;
    MenuSpeaker(Menu* parent);
    virtual ~MenuSpeaker() = default;

protected:
    virtual bool onBack() override;
    virtual bool onSelect() override;
    virtual bool onNext() override;
    virtual bool onPrev() override;

    virtual void onShow() override;
    virtual void onHide() override;
    virtual void onRender() override;
    virtual void onUpdate() override;

private:
    float frequency = 440.0f; // A4 note
    float volume = 0.1f; // 10% volume
    bool enabled = false;
    bool sound = false;

    int optIndex = 0; // 0: frequency, 1: volume, 2: enabled, 3: sound
    bool editing = false;

    void applySoundSettings();

    MusicProvider* musicProvider = nullptr;
    SineProvider sineProvider;
    bool isProviderAdded = false;
};