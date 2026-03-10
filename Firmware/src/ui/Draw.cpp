#include "ui/Draw.hpp"
#include "ui/font8x8_basic.h"

char (*screen_font)[8] = font8x8_basic;

namespace Draw
{
    uint16_t GetTextWidth(const char* text)
    {
        uint16_t width = 0;
        while (*text)
        {
            width += 8;
            text++;
        }
        return width;
    }
}
