#include "Robot.hpp"
#include "common/Log.hpp"

Robot* Robot::instance = nullptr;

Robot::Robot()
{
    instance = this;
}

Error Robot::init()
{
    Error err;

    if ((err = update_manager.init()) != Error::None)
    {
        return err;
    }

    if ((err = body.init()) != Error::None)
    {
        return err;
    }

    if ((err = network_manager.init()) != Error::None)
    {
        return err;
    }

    if ((err = timer.init()) != Error::None)
    {
        return err;
    }

    if ((err = audio_manager.init()) != Error::None)
    {
        return err;
    }

    // THIS DELAY IS IMPORTANT!
    // allow some time for systems to stabilize before enabling motors and everything
    vTaskDelay(pdMS_TO_TICKS(500));

    return Error::None;
}

Error Robot::deinit()
{
    Error err;

    if ((err = body.deinit()) != Error::None)
    {
        return err;
    }

    if ((err = network_manager.deinit()) != Error::None)
    {
        return err;
    }

    if ((err = timer.deinit()) != Error::None)
    {
        return err;
    }

    if ((err = audio_manager.deinit()) != Error::None)
    {
        return err;
    }

    return Error::None;
}