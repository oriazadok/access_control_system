/**
 * @file rfid.c
 * @brief RFID reader implementation using RC522 and ESP32.
 *
 * This module initializes the RC522 RFID scanner and listens for RFID tag detections.
 * When a valid tag is detected, it logs the UID and sends a timestamped event to Firebase.
 */

#include "rfid.h"              // Our public header
#include "firebase.h"          // For sending data to Firebase

#include "rc522.h"              // RC522 driver
#include "driver/rc522_spi.h"   // RC522 SPI interface
#include "picc/rc522_mifare.h"  // RC522 PICC (card) handling
#include "esp_log.h"            // ESP logging
#include "lcd_display.h"        // LCD display interface

#include <stdlib.h>             // For setenv()
#include <string.h>             // For memory functions
#include <time.h>               // For timestamp functions

// Tag used for logging
#define TAG "rfid_reader"

// RC522 driver handle
static rc522_driver_handle_t driver;
// RC522 scanner handle
static rc522_handle_t scanner;

// Predefined UID values for known cards
const uint8_t CARD_UID[4] = { 0x99, 0xB6, 0xB3, 0x02 };
const uint8_t CHIP_UID[4] = { 0x25, 0x0F, 0xC5, 0x01 };

/**
 * @brief Compare two UID arrays.
 *
 * @param uid1 Pointer to first UID array.
 * @param uid2 Pointer to second UID array.
 * @param size Number of bytes to compare.
 * @return true if UIDs are equal, false otherwise.
 */
static bool compare_uid(const uint8_t *uid1, const uint8_t *uid2, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (uid1[i] != uid2[i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Callback function called when the RFID card state changes.
 *
 * This function is triggered when a new RFID tag is detected.
 * It checks if the detected UID matches predefined known UIDs.
 * It logs the UID, updates the display color based on the UID,
 * generates the current timestamp, and sends the event to Firebase.
 *
 * @param arg Unused user argument.
 * @param base Event base (unused).
 * @param event_id Event ID (unused).
 * @param data Pointer to event data containing the detected PICC information.
 */
static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data) {

    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    // Only process active cards
    if (picc->state != RC522_PICC_STATE_ACTIVE) {
        return;
    }

    // Convert UID to string
    char uid_str[RC522_PICC_UID_STR_BUFFER_SIZE_MAX];
    rc522_picc_uid_to_str(&picc->uid, uid_str, sizeof(uid_str));
    ESP_LOGI(TAG, "UID: %s", uid_str);

    // Check if UID matches known cards and update the display accordingly
    if (picc->uid.length == 4) {
        if (compare_uid(picc->uid.value, CARD_UID, 4)) {
            fill_screen(get_color_for_card(COLOR_CARD));
        } else if (compare_uid(picc->uid.value, CHIP_UID, 4)) {
            fill_screen(get_color_for_card(COLOR_CHIP));
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); // Hold color for 3 seconds
        fill_screen(get_color_for_card(COLOR_WAITING)); // Reset display
    }

    // 🕒 Get current timestamp
    time_t now;
    struct tm timeinfo;
    char timestamp[25]; // Buffer for ISO8601 time format

    // Set timezone to Israel (UTC+2 / UTC+3 for DST)
    setenv("TZ", "IST-2IDT,M3.4.4/26,M10.5.0", 1);
    tzset(); // Apply timezone settings

    time(&now); // Get current time
    localtime_r(&now, &timeinfo); // Convert to local time

    // Format time as ISO8601
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

    // Send UID and timestamp to Firebase
    send_rfid_log_to_firebase(uid_str, timestamp);
}

/**
 * @brief Initialize the RFID reader.
 *
 * This function initializes the SPI driver and sets up the RC522 RFID reader.
 * It also registers the event handler for card detection events.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t rfid_reader_init(void) {

    // Configure RC522 SPI driver
    rc522_spi_config_t driver_config = {
        .host_id = SPI3_HOST, // SPI3 bus
        .bus_config = &(spi_bus_config_t){
            .miso_io_num = RC522_SPI_BUS_GPIO_MISO,
            .mosi_io_num = RC522_SPI_BUS_GPIO_MOSI,
            .sclk_io_num = RC522_SPI_BUS_GPIO_SCLK,
        },
        .dev_config = {
            .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA, // SPI chip select (SDA)
        },
        .rst_io_num = RC522_SCANNER_GPIO_RST, // Reset pin
    };

    // Create and install the SPI driver
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    // Configure the RC522 scanner
    rc522_config_t scanner_config = {
        .driver = driver,
    };

    // Create the scanner and register event handler
    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);

    // Start the scanner
    rc522_start(scanner);

    return ESP_OK;
}
