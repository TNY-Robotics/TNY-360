#pragma once
#include "utils.hpp"

namespace Button
{
    using Callback = void(*)();

    typedef struct CallbackSet
    {
        Callback onPressed[2];
        Callback onReleased[2];
        Callback onLongPressed[2];
    } CallbackSet;

    /**
    * @brief Initializes the GPIO module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
     * @brief Sets the callback functions for button events.
     * @param callbacks The set of callback functions to be assigned.
     * @return Error code indicating success or failure.
     */
    Error SetCallbacks(const CallbackSet& callbacks);
}