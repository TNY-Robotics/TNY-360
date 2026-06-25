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

    static inline Robot& GetInstance()
    {
        return *instance;
    }

    Robot();

    /**
     * @brief Initialize the robot.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the robot.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Starts the robot main loops
     * @note : This starts loops on both cores : BRAIN + REFLEX
     */
    Status start();

    /**
     * @brief Stops the robot main loops
     * @note : This stops loops on both cores : BRAIN + REFLEX
     */
    Status stop();

    /**
     * @brief Get the robot's body.
     * @return Reference to the Body.
     */
    inline Body& getBody() { return body; }

    /**
     * @brief Get the robot's network manager.
     * @return Reference to the NetworkManager.
     */
    inline NetworkManager& getNetworkManager() { return network_manager; }

    /**
     * @brief Get the robot's audio manager.
     * @return Reference to the AudioManager.
     */
    inline AudioManager& getAudioManager() { return audio_manager; }

    /**
     * @brief Get the robot's ui manager.
     * @return Reference to the UIManager.
     */
    inline UIManager& getUIManager() { return ui_manager; }

    /**
     * @brief Get the robot's DecisionLoop object.
     * @return Reference to the Decisionloop object.
     */
    inline DecisionLoop& getDecisionLoop() { return decision_loop; }

    /**
     * @brief Get the robot's ControlLoop object
     * @return Reference to ControlLoop object
     */
    inline ControlLoop& getControlLoop() { return control_loop; }

private:
    static Robot* instance;

    Body body;
    NetworkManager network_manager;
    AudioManager audio_manager;
    UIManager ui_manager;

    DecisionLoop decision_loop;
    ControlLoop control_loop;
};