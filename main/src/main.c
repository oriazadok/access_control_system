/**
 * @file main.c
 * @brief Main application entry point for the Access Control System.
 *
 * This file initializes the system:
 * - Initializes LCD display.
 * - Connects to Wi-Fi.
 * - Authenticates to Firebase.
 * - Synchronizes system time via NTP.
 * - Initializes the RFID reader.
 * 
 * After initialization, the system enters a ready state to read RFID cards.
 */

#include "nvs_flash.h"    // Non-volatile storage (Wi-Fi, system calibration data)
#include "wifi.h"         // Wi-Fi connection setup
#include "firebase.h"     // Firebase sign-in and logging
#include "lcd_display.h"  // LCD display driver
#include "rfid.h"         // RFID reader driver
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"      // Logging
#include "esp_sntp.h"     // SNTP (Simple Network Time Protocol) for time sync
#include <time.h>         // Time functions (standard C library)

// Tag used for logging time synchronization events
static const char *TIME_TAG = "time_sync";

/**
 * @brief Initialize SNTP (Simple Network Time Protocol) for time synchronization.
 *
 * This function configures the ESP32 to synchronize its internal clock
 * using the NTP server "pool.ntp.org".
 */
void initialize_sntp(void) {
    ESP_LOGI(TIME_TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);   // Set SNTP to poll mode
    esp_sntp_setservername(0, "pool.ntp.org");     // Set default NTP server
    esp_sntp_init();                               // Initialize SNTP
}

/**
 * @brief Wait until system time is synchronized.
 *
 * This function blocks execution until the system clock is set.
 * It retries several times (up to retry_count) and waits between attempts.
 */
void wait_for_time_sync(void) {
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10; // Max retries

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TIME_TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds between retries
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == retry_count) {
        ESP_LOGW(TIME_TAG, "Failed to synchronize time after %d attempts", retry_count);
    } else {
        ESP_LOGI(TIME_TAG, "Time synchronized successfully");
    }
}

/**
 * @brief Main application entry point.
 *
 * This function initializes all peripherals and services needed for the Access Control System:
 * - LCD display
 * - Wi-Fi connection
 * - Firebase sign-in
 * - SNTP time synchronization
 * - RFID reader
 *
 * Once initialized, it sets the LCD to a "waiting for card" color.
 */
void app_main(void) {
    
    lcd_init();                 // Initialize LCD display
    srand(time(NULL));           // Seed random number generator (for random colors, IDs, etc.)

    ESP_ERROR_CHECK(nvs_flash_init()); // Initialize NVS for Wi-Fi and other system data
    wifi_init_sta();                    // Initialize Wi-Fi in Station mode

    // Wait for Wi-Fi connection
    EventBits_t bits = xEventGroupWaitBits(get_wifi_event_group(),
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdTRUE,
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("main", "Wi-Fi Connected, proceeding to Firebase...");
    }

    ESP_ERROR_CHECK(firebase_sign_in()); // Sign-in to Firebase to obtain ID token

    initialize_sntp();       // Start SNTP time sync
    wait_for_time_sync();    // Block until system time is synchronized

    ESP_ERROR_CHECK(rfid_reader_init()); // Initialize RFID reader

    fill_screen(get_color_for_card(COLOR_WAITING)); // Set LCD to "waiting" color
}
