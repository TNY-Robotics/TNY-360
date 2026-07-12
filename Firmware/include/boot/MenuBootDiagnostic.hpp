#pragma once
#include "ui/Menus.hpp"
#include "ui/widgets/animation.hpp"
#include "ui/widgets/qrcode.hpp"
#include "network/NetworkManager.hpp"

class MenuBootDiagnostic : public Menus::Menu
{
public:
    constexpr static const char* TAG = "MenuBootDiag";

    MenuBootDiagnostic() = default;
    MenuBootDiagnostic(NetworkManager* networkManager);
    virtual ~MenuBootDiagnostic() = default;

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
    UIWidgets::Animation phone_spawn;
    UIWidgets::Animation phone_scan;
    UIWidgets::Animation phone_connected;
    UIWidgets::QRCode qr_code;
    bool spawning = true;
    bool connected = false;
    int scan_delay = 0;
    NetworkManager* networkManager = nullptr;
};