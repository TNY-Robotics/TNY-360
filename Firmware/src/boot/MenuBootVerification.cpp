#include "boot/MenuBootVerification.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "common/config.hpp"
#include "common/NVS.hpp"
#include "common/Log.hpp"
#include "drivers/MotorDriver.hpp"
#include "ui/Animations.hpp"

MenuBootVerification::MenuBootVerification(NetworkManager* networkManager)
    : Menu("Verification"),
    phone_spawn((uint8_t*)Animation::PhoneQRSpawn, 7, 0, 0, 64, 64, false, false),
    phone_scan((uint8_t*)Animation::PhoneQRScan, 15, 0, 0, 64, 64, false, false),
    phone_connected((uint8_t*)Animation::PhoneQRConnected, 6, 0, 0, 64, 64, false, false),
    qr_code(nullptr, 64, 0, 64, 64),
    networkManager(networkManager)
    {
        char wifi_str[128];
        if (strlen(WIFI_AP_PASSWORD) > 0)
        {
            snprintf(wifi_str, sizeof(wifi_str), "WIFI:S:%s;T:WPA;P:%s;;", WIFI_AP_SSID, WIFI_AP_PASSWORD);
        }
        else
        {
            snprintf(wifi_str, sizeof(wifi_str), "WIFI:S:%s;T:nopass;;", WIFI_AP_SSID);
        }
        qr_code.setData(wifi_str);
    }

bool MenuBootVerification::onBack()
{
    return false;
}

bool MenuBootVerification::onSelect()
{
    return false;
}

bool MenuBootVerification::onNext()
{
    return false;
}

bool MenuBootVerification::onPrev()
{
    return false;
}


void MenuBootVerification::onShow()
{
    triggerRender();
}

void MenuBootVerification::onHide()
{ 
}

void MenuBootVerification::onRender()
{
    if (spawning)
    {
        phone_spawn.render();
    }
    else
    {
        if (connected)
        {
            phone_connected.render();
        }
        else
        {
            phone_scan.render();
        }
    }
    qr_code.render();
}

void MenuBootVerification::onUpdate()
{
    if (spawning)
    {
        phone_spawn.update();
        triggerRender();
        if (phone_spawn.isFinished())
        {
            spawning = false;
            scan_delay = 1 * SCREEN_REFRESH_RATE; // 1s
        }
    }
    else
    {
        if (connected)
        {
            phone_connected.update();
            triggerRender();
        }
        else
        {
            if (scan_delay > 0)
            {
                scan_delay--;
                if (scan_delay == 0)
                {
                    phone_scan.setFrameIndex(0);
                }
            }
            else
            {
                phone_scan.update();
                triggerRender();
                if (phone_scan.isFinished())
                {
                    scan_delay = 3 * SCREEN_REFRESH_RATE; // 3s
                }
            }
        }
    }

    if (networkManager)
    {
        uint8_t nb_clients = networkManager->getWebSocket().getNbClients();
        if (nb_clients > 0)
        {
            if (!connected)
            {
                connected = true;
                
                // At this point, the client is on the web portal (diagnostic page).
                // We can assume he's gonna launch the diagnostic test, so clear the verification step in NVS to allow normal boot next time.
                // Everything is setup, skip verification on next boots
                NVS::Handle* nvsHandle;
                if (Status err = NVS::Open("boot", &nvsHandle); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Error opening NVS 'boot' namespace : %s", ErrorToString(err));
                    return;
                }
                bool skip_verif = true;
                if (Status err = nvsHandle->set<bool>("skip_verif", skip_verif); err != Status::Ok)
                {
                    // LOG_ERROR ...
                }
                NVS::Close(nvsHandle);
            }
        }
    }
}
