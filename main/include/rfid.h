#ifndef RFID_READER_H
#define RFID_READER_H

// Include the ESP-IDF error codes header.
// esp_err_t is the return type for functions that report success or standard error codes.
#include "esp_err.h"  // For esp_err_t

// Define the GPIO pin numbers for the SPI connection to the RC522 RFID scanner.
// These constants make the code more readable and easier to maintain.
#define RC522_SPI_BUS_GPIO_MISO 19    // SPI Master-In-Slave-Out (MISO) pin
#define RC522_SPI_BUS_GPIO_MOSI 23    // SPI Master-Out-Slave-In (MOSI) pin
#define RC522_SPI_BUS_GPIO_SCLK 18    // SPI Serial Clock (SCLK) pin
#define RC522_SPI_SCANNER_GPIO_SDA 22 // SPI Chip Select (SDA) pin for RC522
#define RC522_SCANNER_GPIO_RST 21     // Reset (RST) pin for RC522

// Declare the initialization function for the RFID reader.
// This function sets up and starts the RFID reader, returning ESP_OK on success or an error code.
esp_err_t rfid_reader_init(void);

// End of include guard
#endif // RFID_READER_H
