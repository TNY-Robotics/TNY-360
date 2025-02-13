#pragma once
#include <stdint.h>
#include "screen.hpp"

void draw_pixel(screen_info_t* screen_info, uint16_t x, uint16_t y, color_t color);
void draw_line(screen_info_t* screen_info, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color);
void draw_hline(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t length, color_t color);
void draw_vline(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t length, color_t color);
void draw_rect(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color);
void draw_circle(screen_info_t* screen_info, uint16_t x, uint16_t y, uint16_t radius, color_t color);
void draw_blit(screen_info_t* screen_info, uint16_t x, uint16_t y, screen_info_t* blit_info);
void draw_text(screen_info_t* screen_info, uint16_t x, uint16_t y, const char* text, color_t color);
uint16_t draw_text_width(const char* text);