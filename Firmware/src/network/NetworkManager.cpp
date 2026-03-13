#include "network/NetworkManager.hpp"
#include "network/Protocol.hpp"

NetworkManager::NetworkManager()
{
}

Error NetworkManager::init()
{
    // Initialize the WiFi manager
    if (Error err = wifi_manager.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the web interface
    if (Error err = web_interface.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the websocket server
    if (Error err = web_socket.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the update manager
    if (Error err = update_manager.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the protocol layer
    if (Error err = protocol.init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error NetworkManager::deinit()
{
    // Deinitialize the protocol layer
    if (Error err = protocol.deinit(); err != Error::None)
    {
        return err;
    }

    // Denitialize the update manager
    if (Error err = update_manager.deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the websocket server
    if (Error err = web_socket.deinit(); err != Error::None)
    {
        return err;
    }
    
    // Deinitialize the web interface
    if (Error err = web_interface.deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the WiFi manager
    if (Error err = wifi_manager.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}