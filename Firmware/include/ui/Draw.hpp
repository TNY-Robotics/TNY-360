#pragma once
#include "ui/Screen.hpp"
#include <cstdlib>

namespace Draw
{
    template <bool SafeMode = false>
    void Pixel(uint16_t x, uint16_t y, Screen::Color c = Screen::COLOR_WHITE)
    {
        if constexpr (SafeMode)
        {
            if (x >= Screen::GetInstance()->getInfo().width || y >= Screen::GetInstance()->getInfo().height) return;
        }
        Screen::GetInstance()->getInfo().data[y * Screen::GetInstance()->getInfo().width + x] = c;
    }

    template <bool SafeMode = false>
    void Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Screen::Color c = Screen::COLOR_WHITE)
    {
        int16_t dx = std::abs(x2 - x1);
        int16_t dy = std::abs(y2 - y1);
        
        int16_t sx = (x1 < x2) ? 1 : -1;
        int32_t sy = (y1 < y2) ? Screen::GetInstance()->getInfo().width : -Screen::GetInstance()->getInfo().width;
        
        int16_t err = dx - dy;

        auto* ptr = &Screen::GetInstance()->getInfo().data[y1 * Screen::GetInstance()->getInfo().width + x1];

        while (true)
        {
            if constexpr (SafeMode)
            {
                if (x1 < Screen::GetInstance()->getInfo().width && y1 < Screen::GetInstance()->getInfo().height)
                {
                    *ptr = c;
                }
            }
            else *ptr = c;

            if (x1 == x2 && y1 == y2) break;

            int16_t e2 = 2 * err;
            
            if (e2 > -dy) { 
                err -= dy; 
                x1 += (x1 < x2 ? 1 : -1);
                ptr += sx;
            }
            if (e2 < dx) { 
                err += dx; 
                y1 += (y1 < y2 ? 1 : -1);
                ptr += sy;
            }
        }
    }

    template<bool SafeMode = false>
    void LineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t w, Screen::Color c = Screen::COLOR_WHITE)
    {
        if (w <= 1) {
            Line(x1, y1, x2, y2, c);
            return;
        }
        
        CircleFilled<SafeMode>(x1, y1, w / 2, c);
        CircleFilled<SafeMode>(x2, y2, w / 2, c);

        int16_t dx = std::abs(x2 - x1);
        int16_t dy = std::abs(y2 - y1);
        
        int16_t sx = (x1 < x2) ? 1 : -1;
        int16_t sy = (y1 < y2) ? 1 : -1;
        
        int16_t err = dx - dy;

        int16_t t_offset = w / 2;

        while (true)
        {
            if (dx > dy) {
                int16_t y_start = y1 - t_offset;
                if (y_start < 0) y_start = 0; 
                if (y_start + w <= Screen::GetInstance()->getInfo().height) {
                    Vline<SafeMode>(x1, y_start, w, c);
                }
            } else {
                int16_t x_start = x1 - t_offset;
                if (x_start < 0) x_start = 0;
                if (x_start + w <= Screen::GetInstance()->getInfo().width) {
                    Hline<SafeMode>(x_start, y1, w, c);
                }
            }

            if (x1 == x2 && y1 == y2) break;
            
            int16_t e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x1 += sx; }
            if (e2 < dx) { err += dx; y1 += sy; }
        }
    }

    template <bool SafeMode = false>
    void Hline(uint16_t x, uint16_t y, uint16_t l, Screen::Color c = Screen::COLOR_WHITE)
    {
        if constexpr (SafeMode)
        {
            if (x >= Screen::GetInstance()->getInfo().width || y >= Screen::GetInstance()->getInfo().height) return;
            if (x + l > Screen::GetInstance()->getInfo().width) l = Screen::GetInstance()->getInfo().width - x;
        }

        bool* ptr = &Screen::GetInstance()->getInfo().data[y * Screen::GetInstance()->getInfo().width + x];
        memset(ptr, c, l);
    }

    template <bool SafeMode = false>
    void Vline(uint16_t x, uint16_t y, uint16_t l, Screen::Color c = Screen::COLOR_WHITE)
    {
        if constexpr (SafeMode)
        {
            if (x >= Screen::GetInstance()->getInfo().width || y >= Screen::GetInstance()->getInfo().height) return;
            if (y + l > Screen::GetInstance()->getInfo().height) l = Screen::GetInstance()->getInfo().height - y;
        }

        for (uint16_t i = 0; i < l; i++)
        {
            Screen::GetInstance()->getInfo().data[(y + i) * Screen::GetInstance()->getInfo().width + x] = c;
        }
    }

    template <bool SafeMode = false>
    void RectFilled(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Screen::Color c = Screen::COLOR_WHITE)
    {
        if constexpr (SafeMode)
        {
            if (x > Screen::GetInstance()->getInfo().width || y > Screen::GetInstance()->getInfo().height) return;
            if (x + w > Screen::GetInstance()->getInfo().width) w = Screen::GetInstance()->getInfo().width - x;
            if (y + h > Screen::GetInstance()->getInfo().height) h = Screen::GetInstance()->getInfo().height - y;
            if (w == 0 || h == 0) return;
        }

        for (uint16_t i = 0; i < h; i++)
        {
            Hline(x, y + i, w, c);
        }
    }

    template <bool SafeMode = false>
    void RectRounded(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, Screen::Color c = Screen::COLOR_WHITE)
    {
        if (r > w / 2) r = w / 2;
        if (r > h / 2) r = h / 2;

        for (int16_t i = 0; i < h - 2 * r; i++) {
            Hline<SafeMode>(x, y + r + i, w, c);
        }

        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x_off = 0;
        int16_t y_off = r;

        while (x_off < y_off)
        {
            if (f >= 0)
            {
                y_off--;
                ddF_y += 2;
                f += ddF_y;
            }
            x_off++;
            ddF_x += 2;
            f += ddF_x;

            Hline<SafeMode>(x + r - x_off, y + r - y_off, w - 2 * r + 2 * x_off, c);
            Hline<SafeMode>(x + r - x_off, y + h - r + y_off - 1, w - 2 * r + 2 * x_off, c);
            Hline<SafeMode>(x + r - y_off, y + r - x_off, w - 2 * r + 2 * y_off, c);
            Hline<SafeMode>(x + r - y_off, y + h - r + x_off - 1, w - 2 * r + 2 * y_off, c);
        }
    }

    template <bool SafeMode = false>
    void CircleFilled(uint16_t x0, uint16_t y0, uint16_t r, Screen::Color c = Screen::COLOR_WHITE)
    {
        int16_t x = 0;
        int16_t y = r;
        int16_t d = 3 - 2 * r;

        while (y >= x)
        {
            Hline<SafeMode>(x0 - y, y0 - x, 2 * y + 1, c);
            Hline<SafeMode>(x0 - y, y0 + x, 2 * y + 1, c);
            Hline<SafeMode>(x0 - x, y0 - y, 2 * x + 1, c);
            Hline<SafeMode>(x0 - x, y0 + y, 2 * x + 1, c);

            x++;
            if (d > 0)
            {
                y--;
                d = d + 4 * (x - y) + 10;
            }
            else
            {
                d = d + 4 * x + 6;
            }
        }
    }

    template <bool SafeMode = false>
    void TriangleFilled(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Screen::Color c = Screen::COLOR_WHITE)
    {
        if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
        if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
        if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

        if (y2 < 0 || y0 >= Screen::GetInstance()->getInfo().height) return;
        
        int32_t dx01 = 0, dx02 = 0, dx12 = 0;

        if (y1 > y0) dx01 = ((int32_t)(x1 - x0) << 16) / (y1 - y0);
        if (y2 > y0) dx02 = ((int32_t)(x2 - x0) << 16) / (y2 - y0);
        if (y2 > y1) dx12 = ((int32_t)(x2 - x1) << 16) / (y2 - y1);

        int32_t xa = (int32_t)x0 << 16;
        int32_t xb = xa;

        for (int16_t y = y0; y < y1; y++)
        {
            if (y >= 0 && y < Screen::GetInstance()->getInfo().height) {
                int16_t x_start = xa >> 16;
                int16_t x_end   = xb >> 16;
                
                if (x_start > x_end) std::swap(x_start, x_end);
                
                if (x_start < 0) x_start = 0;
                if (x_end >= Screen::GetInstance()->getInfo().width) x_end = Screen::GetInstance()->getInfo().width - 1;

                if (x_end >= x_start) {
                    Hline<SafeMode>(x_start, y, x_end - x_start + 1, c);
                }
            }
            xa += dx02;
            xb += dx01;
        }
        xb = (int32_t)x1 << 16;

        for (int16_t y = y1; y <= y2; y++)
        {
             if (y >= 0 && y < Screen::GetInstance()->getInfo().height) {
                int16_t x_start = xa >> 16;
                int16_t x_end   = xb >> 16;
                
                if (x_start > x_end) std::swap(x_start, x_end);
                
                if (x_start < 0) x_start = 0;
                if (x_end >= Screen::GetInstance()->getInfo().width) x_end = Screen::GetInstance()->getInfo().width - 1;

                if (x_end >= x_start) {
                    Hline<SafeMode>(x_start, y, x_end - x_start + 1, c);
                }
            }
            xa += dx02;
            xb += dx12;
        }
    }

    template <bool SafeMode = false>
    void Blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* bitmap, Screen::Color color = Screen::COLOR_WHITE, bool transparent_bg = false)
    {
        for (uint16_t j = 0; j < h; j++)
        {
            for (uint16_t i = 0; i < w; i++)
            {
                uint8_t byte = bitmap[(j * w + i) / 8];
                uint8_t bit = 1 << (7 - ((j * w + i) % 8));
                if (byte & bit)
                {
                    Pixel<SafeMode>(x + i, y + j, color);
                }
                else if (!transparent_bg)
                {
                    Pixel<SafeMode>(x + i, y + j, !color);
                }
            }
        }
    }

    template <bool SafeMode = false>
    void Text(uint16_t x, uint16_t y, char* text, Screen::Color color = Screen::COLOR_WHITE, bool transparent_bg = false)
    {
        uint16_t x_coord = x;
        while (*text)
        {
            uint8_t c = *text++;

            if constexpr (SafeMode)
            {
                for (uint16_t j = 0; j < 8; j++) {
                    for (uint16_t k = 0; k < 8; k++) {
                        bool active = (screen_font[c][j] & (1 << k));
                        Pixel<true>(x_coord + k, y + j, active ? color : !color);
                    }
                }
            }
            else 
            {
                auto* draw_ptr = &Screen::GetInstance()->getInfo().data[y * Screen::GetInstance()->getInfo().width + x_coord];

                for (uint16_t j = 0; j < 8; j++)
                {
                    uint8_t row_bits = screen_font[c][j];
                    for (uint16_t k = 0; k < 8; k++) 
                    {
                        if (!transparent_bg || (row_bits & 1)) {
                            draw_ptr[k] = (row_bits & 1) ? color : !color;
                        }
                        row_bits >>= 1;
                    }
                    draw_ptr += Screen::GetInstance()->getInfo().width;
                }
            }

            x_coord += 8;
            if (x_coord >= Screen::GetInstance()->getInfo().width)
            {
                x_coord = x;
                y += 8+2; // 8 for font height + 2 for line spacing
            }
        }
    }
    template <bool SafeMode = false>
    void Text(uint16_t x, uint16_t y, const char* text, Screen::Color color = Screen::COLOR_WHITE, bool transparent_bg = false)
    {
        Text<SafeMode>(x, y, (char*) text, color, transparent_bg);
    }

    uint16_t GetTextWidth(const char* text);
}