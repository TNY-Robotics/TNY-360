#pragma once
#include "common/utils.hpp"
#include <memory.h>
#include <utility>

extern char (*screen_font)[8];

class Screen
{
public:
    static constexpr const char* TAG = "Screen";

    /**
     * Color type
     * - `COLOR_BLACK`: black color
     * - `COLOR_WHITE`: white color
     */
    typedef bool Color;

    static constexpr Color COLOR_BLACK = false;
    static constexpr Color COLOR_WHITE = true;

    /**
     * Screen information structure
     * - `data`: the screen buffer data
     * - `width`: the screen width in pixels
     * - `height`: the screen height in pixels
     */
    typedef struct
    {
        /** the screen buffer data, in row-major order, with origin on the top-left corner */
        Color* data;
        /** the screen width in pixels */
        uint16_t width;
        /** the screen height in pixels */
        uint16_t height;
    } Info;

    Screen() = default;

    virtual ~Screen() = default;

    /**
     * @brief Initialize the screen module.
     * @return Error code indicating success or failure.
     */
    virtual Status init() = 0;

    /**
     * @brief Deinitialize the screen module.
     * @return Error code indicating success or failure.
     */
    virtual Status deinit() = 0;

    /**
     * @brief Clear the screen buffer.
     * @return Error code indicating success or failure.
     */
    virtual Status clear() = 0;

    /**
     * @brief Uploads the screen buffer to the physical screen.
     * @return Error code indicating success or failure.
     */
    virtual Status upload() = 0;

    /**
     * @brief Get the screen information.
     * @return The screen information structure.
     */
    Info& getInfo() { return info; }

    /**
     * @brief Get the singleton instance of the screen.
     * @return The singleton instance of the screen.
     */
    static Screen* GetInstance() { return Instance; }

protected:
    Info info;

    /**
     * @brief Set the singleton instance of the screen.
     * @param instance The singleton instance of the screen.
     */
    static void setInstance(Screen* instance) { Instance = instance; }

private:
    inline static Screen* Instance = nullptr;
};
