#pragma once
#include "driver/i2c_master.h"

#define COLOR_BLACK false
#define COLOR_WHITE true

/**
 * Color type
 * - `COLOR_BLACK`: black color
 * - `COLOR_WHITE`: white color
 */
typedef bool color_t;

/**
 * Screen information structure
 * - `data`: the screen buffer data
 * - `width`: the screen width in pixels
 * - `height`: the screen height in pixels
 */
typedef struct
{
    /** the screen buffer data, in row-major order, with origin on the top-left corner */
    color_t* data;
    /** the screen width in pixels */
    uint16_t width;
    /** the screen height in pixels */
    uint16_t height;
} screen_info_t;

/** 
 * Initializes the screen buffer and the i2c communication handle
 * 
 * Creates a new communication handle for the screen over i2c and
 * creates a new screen buffer accessible through `get_screen_info()`
 * @param i2c_bus_handle: the i2c bus handle to use for communication
 */
void screen_init(const i2c_master_bus_handle_t* i2c_bus_handle);

/**
 * Sets all pixels in the screen buffer to `COLOR_BLACK`
 */
void screen_clear();

/** 
 * Transfers the entire screen buffer to the i2c screen
 */
void screen_upload();

/**
 * Returns the screen information, such as data buffer, width, height, etc.
 * This method is used by the draw functions to draw on the screen buffer.
 * @return screen buffer information as `screen_info_t`
 */
screen_info_t* get_screen_info();
