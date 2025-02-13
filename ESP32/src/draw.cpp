#include <stdint.h>
#include <string.h>
#include "draw.hpp"
#include "font8x8_basic.h"

void draw_pixel(screen_info_t* screen_info, uint16_t x, uint16_t y, color_t color)
{
    screen_info->data[y * screen_info->width + x] = color;
}

void draw_line(screen_info_t* screen_info, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color)
{
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    int16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    if (dx < 0)
    {
        dx1 = -1;
    } else if (dx > 0)
    {
        dx1 = 1;
    }
    if (dy < 0)
    {
        dy1 = -1;
    } else if (dy > 0)
    {
        dy1 = 1;
    }
    if (dx < 0)
    {
        dx2 = -1;
    } else if (dx > 0)
    {
        dx2 = 1;
    }
    int16_t longest = abs(dx);
    int16_t shortest = abs(dy);
    if (!(longest > shortest))
    {
        longest = abs(dy);
        shortest = abs(dx);
        if (dy < 0)
        {
            dy2 = -1;
        } else if (dy > 0)
        {
            dy2 = 1;
        }
        dx2 = 0;
    }
    int16_t numerator = longest >> 1;
    for (int16_t i = 0; i <= longest; i++)
    {
        draw_pixel(screen_info, x1, y1, color);
        numerator += shortest;
        if (!(numerator < longest))
        {
            numerator -= longest;
            x1 += dx1;
            y1 += dy1;
        } else
        {
            x1 += dx2;
            y1 += dy2;
        }
    }
}

void draw_hline(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t length, color_t color)
{
    for (uint16_t i = 0; i < length; i++)
    {
        draw_pixel(screen_info, x + i, y, color);
    }
}

void draw_vline(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t length, color_t color)
{
    for (uint16_t i = 0; i < length; i++)
    {
        draw_pixel(screen_info, x, y + i, color);
    }
}

void draw_rect(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color)
{
    for (uint16_t i = 0; i < height; i++)
    {
        for (uint16_t j = 0; j < width; j++)
        {
            screen_info->data[(y + i) * screen_info->width + (x + j)] = color;
        }
    }
}

void draw_circle(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t radius, color_t color)
{
    uint16_t x_min = x - radius;
    uint16_t x_max = x + radius;
    uint16_t y_min = y - radius;
    uint16_t y_max = y + radius;
    
    for (uint16_t i = y_min; i <= y_max; i++)
    {
        for (uint16_t j = x_min; j <= x_max; j++)
        {
            if ((i - y) * (i - y) + (j - x) * (j - x) <= radius * radius)
            {
                screen_info->data[i * screen_info->width + j] = color;
            }
        }
    }
}

void draw_blit(screen_info_t* screen_info, uint16_t x, uint16_t y, screen_info_t* blit_info)
{
    for (uint16_t i = 0; i < blit_info->height; i++)
    {
        for (uint16_t j = 0; j < blit_info->width; j++)
        {
            screen_info->data[(y + i) * screen_info->width + (x + j)] = blit_info->data[i * blit_info->width + j];
        }
    }
}

void draw_text(screen_info_t* screen_info, uint16_t x, uint16_t y, const char* text, color_t color)
{
    uint16_t text_length = strlen(text);
    for (uint16_t i = 0; i < text_length; i++)
    {
        uint8_t c = text[i];
        for (uint16_t j = 0; j < 8; j++)
        {
            for (uint16_t k = 0; k < 8; k++)
            {
                screen_info->data[(y + j) * screen_info->width + (x + i * 8 + k)] = (font8x8_basic[c][j] & (1 << k)) ? color : !color;
            }
        }
    }
}

uint16_t draw_text_width(const char* text)
{
    return strlen(text) * 8;
}
