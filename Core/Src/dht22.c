
#include "dht22.h"


uint32_t captureBuffer[DHT_CAPTURE_COUNT];



static TIM_HandleTypeDef *dhtTimer;
static GPIO_TypeDef *dhtPort;
static uint16_t dhtPin;

void DHT22_Init(TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin) {
    dhtTimer = htim;
    dhtPort = port;
    dhtPin = pin;
}




void DHT22_StartMeasurement(void) {


	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_IC_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)captureBuffer, DHT_CAPTURE_COUNT);

    DHT22_SetPinOutput();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    delay_us(DHT_START_LOW_US );
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    delay_us(DHT_START_HIGH_US );
    DHT22_SetPinInput();



}


void DHT22_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void DHT22_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


void delay_us(uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}


void DHT22_Decode(void) {
	DHT22_Data result = {0};
    // Step 1: Extract non-zero timestamps
    int edgeCount = DHT_EDGE_COUNT;
    uint16_t timestamps[edgeCount];
    int idx = 0;
    for (int i = DHT_EDGE_SKIP; i < DHT_CAPTURE_COUNT; i += 1) // to skip handshake of dht22
    {
        timestamps[idx++] = captureBuffer[i];
    }

    // Step 2: Calculate pulse widths
    uint16_t pulseWidths[edgeCount - 1];
    for (int i = 0; i < edgeCount - 1; i++) {
        pulseWidths[i] = timestamps[i + 1] - timestamps[i];
    }

    // Step 3:
    int start = 0;
    uint8_t bits[TOTAL_BITS];
    for (int i = 0; i < TOTAL_BITS; i++) {
        bits[i] = (pulseWidths[start + i] >= THRESHOLD_US) ? 1 : 0;
    }

    // Step 4: Convert bits to bytes
    uint8_t data[5] = {0};
    for (int i = 0; i < TOTAL_BITS; i++) {
        data[i / 8] <<= 1;
        data[i / 8] |= bits[i];
    }

    // Step 5: Verify checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

    if(checksum == data[4])
    {
    	result.humidity = ((data[0] << 8) | data[1]) / 10.0f;
    	result.temperature = ((data[2] << 8) | data[3]) / 10.0f;
    	result.status = 0;
    }
    else
    {
    	result.status = 1;
    }





	HAL_TIM_Base_Stop(&htim1);
	HAL_TIM_IC_Stop(&htim1, TIM_CHANNEL_1);

}
