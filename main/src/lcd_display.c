#include "lcd_display.h"

// SPI device handle for the LCD
static spi_device_handle_t spi;

/**
 * @brief Send a command byte to the LCD.
 *
 * @param cmd The command byte to send.
 */
static void lcd_cmd(const uint8_t cmd) {
    gpio_set_level(PIN_NUM_DC, 0); // Set DC low to indicate command mode
    spi_transaction_t t = {
        .length = 8,               // 8 bits = 1 byte
        .tx_buffer = &cmd,          // Command data
    };
    spi_device_transmit(spi, &t);   // Transmit command
}

/**
 * @brief Send data bytes to the LCD.
 *
 * @param data Pointer to the data bytes to send.
 * @param len Number of bytes to send.
 */
static void lcd_data(const uint8_t *data, int len) {

    gpio_set_level(PIN_NUM_DC, 1); // Set DC high to indicate data mode
    spi_transaction_t t = {
        .length = len * 8,           // Total length in bits
        .tx_buffer = data,           // Data buffer
    };
    spi_device_transmit(spi, &t);    // Transmit data
}

/**
 * @brief Initialize the ST7735 LCD display controller.
 *
 * Sends the initialization commands to bring the LCD out of sleep
 * and set up basic parameters (frame rate, pixel format, etc.).
 */
static void st7735_init(void) {

    // Hardware reset sequence
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Software reset
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(150));
    // Sleep out
    lcd_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(150));

    // Frame rate control
    uint8_t data1[] = {0x05, 0x3C, 0x3C};
    lcd_cmd(0xB1);
    lcd_data(data1, 3);

    // Display function control
    uint8_t data2[] = {0x03};
    lcd_cmd(0xB6);
    lcd_data(data2, 1);

    // Interface pixel format (16 bits/pixel)
    uint8_t data3[] = {0x55};
    lcd_cmd(0x3A);
    lcd_data(data3, 1);

    // Set column address range
    uint8_t data4[] = {0x00, 0x00, 0x00, 0x00};
    lcd_cmd(0x2A);
    lcd_data(data4, 4);

    // Set row address range
    lcd_cmd(0x2B);
    lcd_data(data4, 4);

    // Normal display mode on
    lcd_cmd(0x13);
    // Display ON
    lcd_cmd(0x29);
}

/**
 * @brief Initialize the LCD display.
 *
 * Sets up the SPI bus, SPI device, and GPIO pins used by the LCD,
 * then runs the LCD initialization sequence.
 */
void lcd_init(void) {

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,         // No MISO (read) needed
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,       // Not using Quad SPI
        .quadhd_io_num = -1,       // Not using Quad SPI
    };
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Configure SPI device for the LCD
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000, // 26 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = PIN_NUM_CS,         // Chip select pin
        .queue_size = 7,                    // Transaction queue size
    };
    spi_bus_add_device(LCD_HOST, &devcfg, &spi);

    // Configure DC and RST pins as output
    gpio_reset_pin(PIN_NUM_DC);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_NUM_RST);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);

    // Initialize LCD controller
    st7735_init();
}

/**
 * @brief Get the 16-bit RGB565 color value for a given card color.
 *
 * @param color Enum value representing the card color.
 * @return 16-bit RGB565 color.
 */
uint16_t get_color_for_card(CardColor color) {
    return card_colors[color];
}

/**
 * @brief Fill the entire LCD screen with a single color.
 *
 * @param color 16-bit RGB565 color to fill the screen with.
 */
void fill_screen(uint16_t color) {
    
    // Swap bytes for LCD endian format (little-endian to big-endian)
    uint16_t color_swapped = (color >> 8) | (color << 8);

    // Static buffer to hold one chunk of pixel data
    static uint16_t color_buf[LCD_H_RES * BUF_HEIGHT];

    // Fill the buffer with the chosen color
    for (int i = 0; i < LCD_H_RES * BUF_HEIGHT; i++) {
        color_buf[i] = color_swapped;
    }

    // Send buffer chunk by chunk to fill the screen
    for (int y = 0; y < LCD_V_RES; y += BUF_HEIGHT) {
        int draw_height = (y + BUF_HEIGHT <= LCD_V_RES) ? BUF_HEIGHT : (LCD_V_RES - y);

        // Set column address range
        uint8_t col_data[] = {0x00, 0x00, 0x00, LCD_H_RES - 1};
        lcd_cmd(0x2A);
        lcd_data(col_data, 4);

        // Set row address range
        uint8_t row_data[] = {
            (uint8_t)(y >> 8), (uint8_t)y,
            (uint8_t)((y + draw_height - 1) >> 8), (uint8_t)(y + draw_height - 1)
        };
        lcd_cmd(0x2B);
        lcd_data(row_data, 4);

        // Write memory command
        lcd_cmd(0x2C);

        gpio_set_level(PIN_NUM_DC, 1); // Data mode

        int pixels_to_send = LCD_H_RES * draw_height;
        int max_pixels_per_transfer = 1024; // Send in chunks to avoid SPI buffer overflow

        int sent = 0;
        while (sent < pixels_to_send) {
            int chunk_size = (pixels_to_send - sent) > max_pixels_per_transfer
                             ? max_pixels_per_transfer
                             : (pixels_to_send - sent);

            spi_transaction_t t = {
                .length = chunk_size * 16,        // 16 bits per pixel
                .tx_buffer = &color_buf[sent],    // Pointer to start of the chunk
                .flags = 0,
            };
            spi_device_transmit(spi, &t);         // Send chunk
            sent += chunk_size;
        }
    }
}
