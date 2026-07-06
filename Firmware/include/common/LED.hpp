#pragma once
#include "common/utils.hpp"
#include "common/Error.hpp"

namespace LED
{
    typedef struct Color
    {
        uint8_t r; // Red component (0-255)
        uint8_t g; // Green component (0-255)
        uint8_t b; // Blue component (0-255)
        Color() : r(0), g(0), b(0) {}
        Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    } Color;
    
    using Id = uint8_t; // LED identifier type

    static constexpr Id LED_COUNT = 1; // Number of LEDs in the strip (only one for status, body leds idea has been abandonned)

    /**
     * @brief Initializes the LED module.
     * @return Error code indicating success or failure.
     */
    Status Init();

    /**
     * @brief Deinitializes the LED module.
     * @return Error code indicating success or failure.
     */
    Status Deinit();

    /**
     * @brief Sets the color of a specific LED.
     * @param id The LED identifier.
     * @param color The color to set.
     * @return Error code.
     */
    Status SetColor(Id id, Color color);

    /**
     * @brief Sets the color of a specific LED for a duration.
     * @param id The LED identifier.
     * @param color The color to set.
     * @param duration_s Duration in seconds.
     * @return Error code.
     */
    Status SetColor(Id id, Color color, float duration_s);

    /**
     * @brief Sets colors for all LEDs.
     * @param colors Array of colors.
     * @return Error code.
     */
    Status SetColors(const Color colors[]);

    /**
     * @brief Sets colors for all LEDs with durations.
     * @param colors Array of colors.
     * @param durations_s Array of durations in seconds.
     * @return Error code.
     */
    Status SetColors(const Color colors[], const float durations_s[]);

    /**
     * @brief Sets colors for specific LEDs.
     * @param ids Array of LED identifiers.
     * @param colors Array of colors.
     * @return Error code.
     */
    Status SetColors(const Id ids[], const Color colors[], uint8_t count);

    /**
     * @brief Sets colors for specific LEDs with durations.
     * @param ids Array of LED identifiers.
     * @param colors Array of colors.
     * @param durations_s Array of durations in seconds.
     * @return Error code.
     */
    Status SetColors(const Id ids[], const Color colors[], const float durations_s[], uint8_t count);

    /**
     * @brief Gets the color of a specific LED.
     * @param id The LED identifier.
     * @param outColor Pointer to output color.
     * @return Error code.
     */
    Status GetColor(Id id, Color* outColor);

    /**
     * @brief Gets colors of all LEDs.
     * @param outColors Pointer to array of output colors.
     * @return Error code.
     */
    Status GetColors(Color* outColors);

    /**
     * @brief Gets colors of specific LEDs.
     * @param ids Array of LED identifiers.
     * @param outColors Pointer to array of output colors.
     * @return Error code.
     */
    Status GetColors(const Id ids[], Color* outColors, uint8_t count);

    /**
     * @brief Show error code on first LED as a color.
     * @param errCode Error code to display.
     * @note This function launches a task in an infinite loop, displaying the error code.
     * @return void.
     */
    void LoopErrorCode(uint8_t errCode);

    /**
     * @brief Clear any looping error code displayed on the LED
     */
    void clearErrorCode();
}