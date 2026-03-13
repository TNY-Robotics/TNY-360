#include "Robot.hpp"
#include "common/Log.hpp"

Robot* Robot::instance = nullptr;

Robot::Robot()
{
    instance = this;
}

Error Robot::init()
{
    if (Error err = body.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = network_manager.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = ui_manager.init(); err != Error::None)
    {
        return err;
    }

    if (Error err = audio_manager.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the decision loop
    if (Error err = decision_loop.init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Robot::start()
{
    // Start the decision loop
    if (Error err = decision_loop.start(); err != Error::None)
    {
        return err;
    }

    // Start the control loop
    if (Error err = body.getControlLoop().start(); err != Error::None)
    {
        return err;
    }

    // NOTE : THIS DELAY IS IMPORTANT!
    // allow some time for systems to stabilize before enabling motors and everything
    // (to get motor feedback data and all that stuff)
    vTaskDelay(pdMS_TO_TICKS(500));

    return Error::None;
}

Error Robot::stop()
{
    // Stop the control loop
    if (Error err = body.getControlLoop().stop(); err != Error::None)
    {
        return err;
    }

    // Stop the decision loop
    if (Error err = decision_loop.stop(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Robot::deinit()
{
    // Deinitialize the decision loop
    if (Error err = decision_loop.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = audio_manager.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = ui_manager.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = network_manager.deinit(); err != Error::None)
    {
        return err;
    }

    if (Error err = body.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}