#include <freertos/FreeRTOS.h>
#include "modules/Log.hpp"
#include "modules/WiFi.hpp"
#include "modules/Network.hpp"
#include "modules/Lang.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, "Starting robot ...");
    
    // Initialize WiFi module
    Log::Add(Log::Level::Info, "Initializing WiFi...");
    if (Error err = WiFi::Init(); err == Error::Ok)
    {
        Log::Add(Log::Level::Info, "WiFi initialized, connecting to AP...");
        // Connect to AP, if error, start AP mode
        WiFi::Connect(
            []() { // on success, start web server and websocket
                Log::Add(Log::Level::Info, "Connected to WiFi, starting webserver...");
                if (Network::SetupSTA() != Error::Ok) {
                    Log::Add(Log::Level::Error, "Network SetupSTA failed");
                    return;
                }
                Log::Add(Log::Level::Info, "WebServer and websocket started");
            },
            [](Error err) { // on failure, start AP
                Log::Add(Log::Level::Info, "Failed to connect to Wi-Fi, starting AP...");
                WiFi::StartAP(WIFI_AP_SSID, WIFI_AP_PASSWORD,
                    []() { // on success, start captive portal
                        Log::Add(Log::Level::Info, "AP started");
                        if (Network::SetupAP() != Error::Ok) {
                            Log::Add(Log::Level::Error, "Network SetupAP failed");
                            return;
                        }
                        Log::Add(Log::Level::Info, "Captive portal ready");
                    },
                    [](Error err) { // on failure, log error
                        Log::Add(Log::Level::Error, "Failed to start AP, this might be a serious issue.");
                    }
                );
            }
        );
    }
    else Log::Add(Log::Level::Error, "Wi-Fi Init failed with error: %d", static_cast<uint8_t>(err));
}
#ifdef __cplusplus
}
#endif