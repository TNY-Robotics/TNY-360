#pragma once
#include "locomotion/Body.hpp"
#include "network/NetworkManager.hpp"
#include "locomotion/Timer.hpp"
#include "network/Protocol.hpp"

class Robot
{
public:
    static Robot& GetInstance()
    {
        return *instance;
    }

    Robot();

    /**
     * @brief Initialize the robot.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the robot.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Get the robot's body.
     * @return Reference to the Body.
     */
    Body& getBody() { return body; }

    /**
     * @brief Get the robot's network manager.
     * @return Reference to the NetworkManager.
     */
    NetworkManager& getNetworkManager() { return network_manager; }

    /**
     * @brief Get the robot's protocol object.
     * @return Reference to the Protocol.
     */
    Protocol& getProtocol() { return protocol; }

private:
    static Robot* instance;

    Body body;
    NetworkManager network_manager;
    Timer timer;
    Protocol protocol;
};