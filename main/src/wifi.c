/**
 * @file wifi.c
 * @brief Wi-Fi Station (STA) mode connection logic for ESP32.
 *
 * This file implements Wi-Fi initialization and connection handling.
 * It connects the ESP32 to a predefined access point using credentials provided
 * in wifi_credentials.h, and manages reconnection automatically upon disconnection.
 */

#include "wifi.h"                // Header for public Wi-Fi API
#include "wifi_credentials.h"    // Wi-Fi SSID and password (hidden from Git)
#include "esp_wifi.h"             // ESP-IDF Wi-Fi API
#include "esp_event.h"            // ESP-IDF Event loop API
#include "esp_log.h"              // ESP-IDF logging
#include "nvs_flash.h"            // Non-volatile storage (Wi-Fi calibration data)
#include "lwip/err.h"             // lwIP error codes
#include "lwip/sys.h"             // lwIP system functions

// Bit used to indicate a successful Wi-Fi connection (internal)
#define WIFI_CONNECTED_BIT BIT0

// Event group to signal when connected
static EventGroupHandle_t wifi_event_group;

// Tag used for logging
static const char *TAG = "wifi_station";

/**
 * @brief Wi-Fi event handler.
 *
 * This function is called automatically when Wi-Fi-related events occur.
 * It handles connecting, disconnection retries, and IP acquisition events.
 *
 * @param arg Unused user argument.
 * @param event_base The base ID of the event (WIFI_EVENT or IP_EVENT).
 * @param event_id The specific event ID.
 * @param event_data Event-specific data.
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Wi-Fi station started; attempt to connect to AP
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Wi-Fi disconnected; attempt to reconnect
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying connection to the AP...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Successfully got an IP address; set the connection bit
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Got IP Address");
    }
}

/**
 * @brief Initialize Wi-Fi in Station mode and connect to an AP.
 *
 * This function initializes the TCP/IP stack, Wi-Fi driver, event loop,
 * configures Wi-Fi with SSID and password, and starts the Wi-Fi connection.
 */
void wifi_init_sta(void) {
    // Create an event group to manage Wi-Fi connection state
    wifi_event_group = xEventGroupCreate();

    // Initialize network interface and event loop
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // Initialize Wi-Fi with default configurations
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Register event handlers for Wi-Fi and IP events
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &event_handler,
                                        NULL,
                                        NULL);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &event_handler,
                                        NULL,
                                        NULL);

    // Set Wi-Fi configuration (SSID and password from credentials header)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Set Wi-Fi mode to Station (client)
    esp_wifi_set_mode(WIFI_MODE_STA);
    // Apply Wi-Fi configuration
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // Start Wi-Fi
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished. Wi-Fi initialization complete.");
}

/**
 * @brief Get the handle to the Wi-Fi event group.
 *
 * Other modules can use this function to access the event group and wait
 * for Wi-Fi connection status (WIFI_CONNECTED_BIT).
 *
 * @return EventGroupHandle_t Handle to the Wi-Fi event group.
 */
EventGroupHandle_t get_wifi_event_group(void) {
    return wifi_event_group;
}
