#include "dht22.h"

// DMA buffer to store input capture timestamps from DHT22 signal
volatile uint32_t captureBuffer[DHT_CAPTURE_COUNT];

// Holds the most recent decoded sensor data
static DHT22_Data latestData = {0};

// Default configuration for DHT22 sensor (timer, GPIO port/pin, channel)
const DHT22_Config SensorConfig_Default = {
    .dhtTimer = &htim1,
    .dhtPort = GPIOA,
    .dhtPin = GPIO_PIN_8,
    .dhtIcChannel = TIM_CHANNEL_1
};

// Active configuration used by the driver (can be overridden via DHT22_Init)
DHT22_Config config = SensorConfig_Default;

// Initializes the DHT22 configuration with user-specified hardware settings
void DHT22_Init(TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin, uint16_t channel) {
    config.dhtTimer = htim;
    config.dhtPort = port;
    config.dhtPin = pin;
    config.dhtIcChannel = channel;
}

// Starts a DHT22 measurement and waits for data to be captured and decoded
DHT22_Data DHT22_Read(void)
{
    latestData.ready = 0;

    // Reset timer and start input capture via DMA
    __HAL_TIM_SET_COUNTER(config.dhtTimer, 0);
    HAL_TIM_IC_Start_DMA(config.dhtTimer, config.dhtIcChannel, (uint32_t*)captureBuffer, DHT_CAPTURE_COUNT);

    // Send start signal to DHT22: pull pin low, then high
    DHT22_SetPinOutput();
    HAL_GPIO_WritePin(config.dhtPort, config.dhtPin, GPIO_PIN_RESET);
    delay_us(DHT_START_LOW_US);
    HAL_GPIO_WritePin(config.dhtPort, config.dhtPin, GPIO_PIN_SET);
    delay_us(DHT_START_HIGH_US);
    DHT22_SetPinInput();  // Switch pin to input to receive data

    // Wait for DMA capture to complete (max 10 ms)
    uint32_t timeout = HAL_GetTick() + 10;
    while (!latestData.ready && HAL_GetTick() < timeout) {
        // Waiting for DHT22_ProcessCapture() to set latestData.ready = 1
    }

    // If no data was received, mark status as timeout
    if (!latestData.ready) {
        latestData.status = 2;
    } else {
        latestData.ready = 0;  // Clear ready flag for next read
    }

    return latestData;
}

// Configures GPIO pin as push-pull output (used to send start signal)
void DHT22_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = config.dhtPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(config.dhtPort, &GPIO_InitStruct);
}

// Configures GPIO pin as input with pull-up (used to receive data)
void DHT22_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = config.dhtPin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(config.dhtPort, &GPIO_InitStruct);
}

// Microsecond delay using hardware timer
void delay_us(uint16_t delay)
{
    __HAL_TIM_SET_COUNTER(config.dhtTimer, 0);
    while (__HAL_TIM_GET_COUNTER(config.dhtTimer) < delay);
}

// Decodes the captured signal into temperature and humidity values
DHT22_Data DHT22_Decode(void)
{
    // Step 1: Skip handshake edges and extract timestamps
    int edgeCount = DHT_EDGE_COUNT;
    uint16_t timestamps[edgeCount];
    int idx = 0;
    for (int i = DHT_EDGE_SKIP; i < DHT_CAPTURE_COUNT; i++) {
        timestamps[idx++] = captureBuffer[i];
    }

    // Step 2: Calculate pulse widths between edges
    uint16_t pulseWidths[edgeCount - 1];
    for (int i = 0; i < edgeCount - 1; i++) {
        pulseWidths[i] = timestamps[i + 1] - timestamps[i];
    }

    // Step 3: Convert pulse widths to bits (1 or 0 based on threshold)
    uint8_t bits[TOTAL_BITS];
    for (int i = 0; i < TOTAL_BITS; i++) {
        bits[i] = (pulseWidths[i] >= THRESHOLD_US) ? 1 : 0;
    }

    // Step 4: Pack bits into 5 bytes (40 bits total)
    uint8_t data[5] = {0};
    for (int i = 0; i < TOTAL_BITS; i++) {
        data[i / 8] <<= 1;
        data[i / 8] |= bits[i];
    }

    // Step 5: Verify checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    latestData.status = 1;  // Assume error by default

    if (checksum == data[4]) {
        // Decode humidity
        latestData.humidity = (float)((data[0] << 8) | data[1]) / 10;

        // Decode temperature (handle negative values)
        if (data[2] & 0x80) {
            latestData.temperature = -(float)(((data[2] & 0x7F) << 8) | data[3]) / 10;
        } else {
            latestData.temperature = (float)((data[2] << 8) | data[3]) / 10;
        }

        latestData.status = 0;  // Success
    }

    return latestData;
}

// Called from input capture callback to decode and finalize data
void DHT22_ProcessCapture(void)
{
    latestData = DHT22_Decode();       // Decode signal into usable data
    latestData.ready = 1;              // Mark data as ready for retrieval
    HAL_TIM_Base_Stop(config.dhtTimer);
    HAL_TIM_IC_Stop(config.dhtTimer, config.dhtIcChannel);
}
