#pragma once
/**
 * @file wifi.h
 * @brief Wi-Fi station mode interface for ESP32.
 *
 * This header declares functions for initializing Wi-Fi in station mode
 * and managing Wi-Fi connection status using FreeRTOS event groups.
 */

#include "freertos/FreeRTOS.h"    // FreeRTOS core definitions
#include "freertos/event_groups.h" // FreeRTOS event group API

/**
 * @brief Bit used to indicate a successful Wi-Fi connection.
 *
 * This bit will be set in the event group when the ESP32 connects to the Wi-Fi access point
 * and receives an IP address. Other parts of the program can wait on this bit to detect
 * when Wi-Fi is ready.
 */
#define WIFI_CONNECTED_BIT BIT0

/**
 * @brief Initialize Wi-Fi in station mode (STA).
 *
 * This function:
 * - Initializes the TCP/IP stack.
 * - Configures the Wi-Fi driver in station mode.
 * - Connects to the specified access point.
 * - Registers event handlers to automatically reconnect if disconnected.
 */
void wifi_init_sta(void);

/**
 * @brief Get the Wi-Fi event group handle.
 *
 * This function returns the event group that contains Wi-Fi connection status bits.
 * Other tasks can use this event group to wait for the Wi-Fi connection to be established.
 *
 * @return EventGroupHandle_t Handle to the Wi-Fi event group.
 */
EventGroupHandle_t get_wifi_event_group(void);
