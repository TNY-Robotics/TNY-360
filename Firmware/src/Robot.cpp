#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"

Robot* Robot::instance = nullptr;

Robot::Robot()
{
    instance = this;
}

Status Robot::init()
{
    LOG_SCOPE(TAG, "Robot::init");

    // Initialize the LED module to diplay status
    if (Status err = LED::Init(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Couldn't initialize LED module");
        return err;
    }

    // Set yellow color (init color)
    LED::SetColor(0, LED::Color(16, 16, 0), 0.1f);

    if (Status err = network_manager.init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = ui_manager.init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = audio_manager.init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = body.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the decision loop
    if (Status err = decision_loop.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the control loop
    if (Status err = control_loop.init(); err != Status::Ok)
    {
        return err;
    }

    // all good, turn orange for startup
    LED::SetColor(0, LED::Color(16, 8, 0), 0.1f);

    return Status::Ok;
}

Status Robot::start()
{
    // Set default standing pose to avoid any weird position error or anything
    decision_loop.askBodyPosition(0, 0, DEFAULT_BODY_HEIGHT_M);
    decision_loop.askBodyRotation(0, 0, 0);
    decision_loop.askBodyVelocity(0, 0, 0);

    // Start the decision loop
    if (Status err = decision_loop.start(); err != Status::Ok)
    {
        return err;
    }

    // Start the control loop
    if (Status err = control_loop.start(); err != Status::Ok)
    {
        return err;
    }

    // NOTE : THIS DELAY IS IMPORTANT!
    // allow some time for systems to stabilize before enabling motors and everything
    // (to get motor feedback data and all that stuff)
    vTaskDelay(pdMS_TO_TICKS(500));

    // robot is ready ! Turn green (low intensity to avoid using power for nothing)
    LED::SetColor(0, LED::Color(0, 1, 0), 0.1f);

    // Set the menu to face (face only displays when everything is working)
    Menus::SetCurrentMenu(Menus::GetMenuFace());

    return Status::Ok;
}

Status Robot::stop()
{
    // Stop the control loop
    if (Status err = control_loop.stop(); err != Status::Ok)
    {
        return err;
    }

    // Stop the decision loop
    if (Status err = decision_loop.stop(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status Robot::deinit()
{
    // Deinitialize the control loop
    if (Status err = control_loop.deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the decision loop
    if (Status err = decision_loop.deinit(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = body.deinit(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = network_manager.deinit(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = audio_manager.deinit(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = ui_manager.deinit(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}