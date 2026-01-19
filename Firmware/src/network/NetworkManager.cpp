#include "network/NetworkManager.hpp"

NetworkManager::NetworkManager()
{
}

Error NetworkManager::init()
{
    // Initialize WiFiManager
    if (Error err = wifi_manager.init(); err != Error::None)
    {
        return err;
    }

    // Initialize WebInterface
    if (Error err = web_interface.init(); err != Error::None)
    {
        return err;
    }

    // Initialize WebSocket
    if (Error err = web_socket.init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error NetworkManager::deinit()
{
    // Deinitialize WiFiManager
    if (Error err = wifi_manager.deinit(); err != Error::None)
    {
        return err;
    }
    
    // Deinitialize WebInterface
    if (Error err = web_interface.deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize WebSocket
    if (Error err = web_socket.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}