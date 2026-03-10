#pragma once
#include "common/utils.hpp"
#include <memory.h>
#include <utility>

extern char (*screen_font)[8];

namespace Screen
{
    constexpr const char* TAG = "Screen";

    /**
     * Color type
     * - `COLOR_BLACK`: black color
     * - `COLOR_WHITE`: white color
     */
    typedef bool Color;

    constexpr Color COLOR_BLACK = false;
    constexpr Color COLOR_WHITE = true;

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

    extern Info info;

    Error Init();

    Error Clear();

    Error Upload();
}
