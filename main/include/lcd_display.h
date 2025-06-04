#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

// Standard integer types (uint16_t, etc.)
#include <stdint.h>

// ESP-IDF headers for SPI and GPIO control
#include "driver/spi_master.h"
#include "driver/gpio.h"

// FreeRTOS headers for tasks (used for delays etc.)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// --- LCD SPI Configuration ---

// SPI Host to use for the LCD
#define LCD_HOST    SPI2_HOST

// GPIO pin assignments for the LCD SPI interface
#define PIN_NUM_MOSI 12   // Master Out Slave In
#define PIN_NUM_CLK  14   // SPI Clock
#define PIN_NUM_CS   15   // Chip Select
#define PIN_NUM_DC   27   // Data/Command control pin
#define PIN_NUM_RST  26   // Reset pin

// LCD Resolution
#define LCD_H_RES   128   // Horizontal resolution (pixels)
#define LCD_V_RES   160   // Vertical resolution (pixels)

// Number of rows to update at once (buffered)
#define BUF_HEIGHT  40

// --- RGB565 Color Definitions (16-bit) ---

#define RGB565_GRAY   0x8410  // Gray color in RGB565 format
#define RGB565_GREEN  0x07E0  // Green color in RGB565 format
#define RGB565_RED    0xF800  // Red color in RGB565 format

// --- Card Status Enumeration ---
// Represents the card detection state
typedef enum {
    COLOR_WAITING = 0, // Gray background (waiting for card)
    COLOR_CARD,        // Green background (card detected)
    COLOR_CHIP         // Red background (chip detected)
} CardColor;

// --- Color Mapping Array ---
// Maps CardColor enum to actual RGB565 color values
static const uint16_t card_colors[] = {
    [COLOR_WAITING] = RGB565_GRAY,
    [COLOR_CARD]    = RGB565_GREEN,
    [COLOR_CHIP]    = RGB565_RED
};

// --- LCD API Declarations ---

/**
 * @brief Initialize the LCD screen (SPI bus, pins, and LCD controller).
 */
void lcd_init(void);

/**
 * @brief Fill the entire LCD screen with a specified color.
 *
 * @param color RGB565 color value to fill the screen with.
 */
void fill_screen(uint16_t color);

/**
 * @brief Get the RGB565 color associated with a CardColor state.
 *
 * @param color CardColor enum value.
 * @return Corresponding RGB565 color value.
 */
uint16_t get_color_for_card(CardColor color);

#endif // LCD_DISPLAY_H
