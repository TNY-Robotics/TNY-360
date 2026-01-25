#pragma once
#include "locomotion/Body.hpp"
#include "network/NetworkManager.hpp"
#include "locomotion/Timer.hpp"
#include "network/Protocol.hpp"
#include "network/UpdateManager.hpp"
#include "audio/AudioManager.hpp"

class Robot
{
public:
    constexpr static const char* TAG = "Robot";

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

    /**
     * @brief Get the robot's audio manager.
     * @return Reference to the AudioManager.
     */
    AudioManager& getAudioManager() { return audio_manager; }

    /**
     * @brief Get the robot's update manager.
     * @return Reference to the UpdateManager.
     */
    UpdateManager& getUpdateManager() { return update_manager; }

private:
    static Robot* instance;

    Body body;
    NetworkManager network_manager;
    UpdateManager update_manager;
    Timer timer;
    Protocol protocol;
    AudioManager audio_manager;
};