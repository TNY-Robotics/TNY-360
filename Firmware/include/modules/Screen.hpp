#pragma once
#include "utils.hpp"

namespace Screen
{
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

namespace Draw
{
    void Pixel(Screen::Info* screen_info, uint16_t x, uint16_t y, Screen::Color c);
    void Line(Screen::Info* screen_info, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Screen::Color c);
    void LineThick(Screen::Info* screen_info, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t w, Screen::Color c);
    void Hline(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t l, Screen::Color c);
    void Vline(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t l, Screen::Color c);
    void RectFilled(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t w, uint16_t h, Screen::Color c);
    void RectRounded(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, Screen::Color c);
    void CircleFilled(Screen::Info* screen_info, uint16_t x, uint16_t y, uint16_t r, Screen::Color c);
    void TriangleFilled(Screen::Info* screen_info, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Screen::Color c);
    void Blit(Screen::Info* screen_info, uint16_t x, uint16_t y, Screen::Info* blit_info);
    void Text(Screen::Info* screen_info, uint16_t x, uint16_t y, const char* t, Screen::Color c);
    uint16_t GetTextWidth(const char* text);
}