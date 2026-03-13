#pragma once
#include "locomotion/Body.hpp"
#include "decision/DecisionLoop.hpp"
#include "network/NetworkManager.hpp"
#include "audio/AudioManager.hpp"
#include "ui/UIManager.hpp"

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
     * @brief Starts the robot main loops
     * @note : This starts loops on both cores : BRAIN + REFLEX
     */
    Error start();

    /**
     * @brief Stops the robot main loops
     * @note : This stops loops on both cores : BRAIN + REFLEX
     */
    Error stop();

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
     * @brief Get the robot's audio manager.
     * @return Reference to the AudioManager.
     */
    AudioManager& getAudioManager() { return audio_manager; }

    /**
     * @brief Get the robot's ui manager.
     * @return Reference to the UIManager.
     */
    UIManager& getUIManager() { return ui_manager; }

    /**
     * @brief Get the robot's decision loop object.
     * @return Reference to the decision loop object.
     */
    DecisionLoop& getDecisionLoop() { return decision_loop; }

private:
    static Robot* instance;

    Body body;
    NetworkManager network_manager;
    AudioManager audio_manager;
    UIManager ui_manager;
    DecisionLoop decision_loop;
};