#include "network/NetworkManager.hpp"
#include "network/protocol/Protocol.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

NetworkManager::NetworkManager()
{
}

Error NetworkManager::init()
{
    LOG_SCOPE(TAG, "NetworkManager::init");

    // Initialize the WiFi manager
    if (Error err = wifi_manager.init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::WiFiInitFailed);
        return err;
    }

    // Initialize the web interface
    if (Error err = web_interface.init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::WebInterfaceInitFailed);
        return err;
    }

    // Initialize the websocket server
    if (Error err = web_socket.init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::WebSocketInitFailed);
        return err;
    }

    // Initialize the update manager
    if (Error err = update_manager.init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::UpdateInitFailed);
        return err;
    }

    // Initialize the protocol system
    if (Error err = Protocol::Init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::ProtocolInitFailed);
        return err;
    }

    // Start the camera server
    if (Error err = Robot::GetInstance().getUIManager().getCamera().start(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error NetworkManager::deinit()
{
    // Deinitialize the protocol system
    if (Error err = Protocol::Deinit(); err != Error::None)
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