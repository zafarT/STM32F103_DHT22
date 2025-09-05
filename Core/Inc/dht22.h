/*
 * dht22.h
 *
 *  Created on: Aug 31, 2025
 *      Author: zafar
 */

#ifndef INC_DHT22_H_
#define INC_DHT22_H_

#include "main.h"
#include "stm32f1xx_it.h"
#include "stdio.h"

// -----------------------------
// DHT22 Timing and Signal Macros
// -----------------------------

#define DHT_CAPTURE_COUNT   43     // Total number of edges to capture via DMA
#define THRESHOLD_US        100    // Pulse width threshold to distinguish between 0 and 1
#define TOTAL_BITS          40     // Total bits transmitted by DHT22 (5 bytes)
#define MAX_BITS            64     // Max bits for buffer safety (not used directly)
#define DHT_START_LOW_US    1200   // Duration to pull data line low to start communication
#define DHT_START_HIGH_US   20     // Duration to pull data line high before releasing
#define DHT_EDGE_SKIP       2      // Number of initial edges to skip (handshake)
#define DHT_EDGE_COUNT      41     // Number of edges used to decode 40 bits

// -----------------------------
// DHT22 Sensor Data Structure
// -----------------------------

typedef struct {
    float temperature;     // Temperature in °C
    float humidity;        // Humidity in %
    uint8_t status;        // 0 = OK, 1 = Checksum error, 2 = Timeout
    uint8_t ready;         // Flag indicating if new data is available
} DHT22_Data;

// -----------------------------
// DHT22 Configuration Structure
// -----------------------------

typedef struct {
    TIM_HandleTypeDef *dhtTimer;   // Timer used for input capture and microsecond delay
    GPIO_TypeDef      *dhtPort;    // GPIO port connected to DHT22 data line
    uint16_t           dhtPin;     // GPIO pin connected to DHT22 data line
    uint8_t            dhtIcChannel; // Timer input capture channel
} DHT22_Config;

// -----------------------------
// Global Variables
// -----------------------------

extern volatile uint32_t captureBuffer[DHT_CAPTURE_COUNT];  // DMA buffer for edge timestamps
extern volatile uint8_t dataReady;                          // Optional flag (unused in current version)
extern const DHT22_Config SensorConfig_Default;             // Default configuration for DHT22
extern DHT22_Config config;                                 // Active configuration used by driver

// -----------------------------
// Public API Functions
// -----------------------------

/**
 * @brief Initializes the DHT22 configuration with custom hardware settings.
 */
void DHT22_Init(TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin, uint16_t channel);

/**
 * @brief Starts a DHT22 measurement and returns the decoded temperature and humidity.
 */
DHT22_Data DHT22_Read(void);

/**
 * @brief Configures the DHT22 GPIO pin as push-pull output.
 */
void DHT22_SetPinOutput(void);

/**
 * @brief Configures the DHT22 GPIO pin as input with pull-up.
 */
void DHT22_SetPinInput(void);

/**
 * @brief Decodes the captured signal into temperature and humidity values.
 */
DHT22_Data DHT22_Decode(void);

/**
 * @brief Delays for a specified number of microseconds using the configured timer.
 */
void delay_us(uint16_t delay);

/**
 * @brief Called from input capture callback to decode and finalize sensor data.
 */
void DHT22_ProcessCapture(void);

#endif /* INC_DHT22_H_ */
