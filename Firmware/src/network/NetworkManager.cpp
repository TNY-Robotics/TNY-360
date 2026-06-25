#include "network/NetworkManager.hpp"
#include "network/protocol/Protocol.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

NetworkManager::NetworkManager()
{
}

Status NetworkManager::init()
{
    LOG_SCOPE(TAG, "NetworkManager::init");

    // Initialize the WiFi manager
    if (Status err = wifi_manager.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the web interface
    if (Status err = web_interface.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the websocket server
    if (Status err = web_socket.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the update manager
    if (Status err = update_manager.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the protocol system
    if (Status err = Protocol::Init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the camera server
    // FIXME : The method name shouldn't be start but something like initServer
    if (Status err = Robot::GetInstance().getUIManager().getCamera().start(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status NetworkManager::deinit()
{
    // Deinitialize the protocol system
    if (Status err = Protocol::Deinit(); err != Status::Ok)
    {
        return err;
    }

    // Denitialize the update manager
    if (Status err = update_manager.deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the websocket server
    if (Status err = web_socket.deinit(); err != Status::Ok)
    {
        return err;
    }
    
    // Deinitialize the web interface
    if (Status err = web_interface.deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the WiFi manager
    if (Status err = wifi_manager.deinit(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}