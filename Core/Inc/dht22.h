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
#include "ssd1306.h"


typedef struct {
	float temperature;
	float humidity;
    uint8_t status;  // 0 = OK, 1 = Checksum error, 2 = Timeout
    uint8_t ready;
} DHT22_Data;










#define DHT_CAPTURE_COUNT 43
#define THRESHOLD_US 100
#define TOTAL_BITS 40
#define MAX_BITS 64
#define DHT_START_LOW_US 1200
#define DHT_START_HIGH_US 20
#define DHT_EDGE_SKIP 2
#define DHT_EDGE_COUNT 41

extern volatile uint32_t captureBuffer[DHT_CAPTURE_COUNT];
extern volatile uint8_t dataReady;

DHT22_Data DHT22_Read();

void DHT22_SetPinOutput();


void DHT22_SetPinInput();

DHT22_Data DHT22_Decode(void);

void delay_us(uint16_t delay);


void DHT22_Init(TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin);


void DHT22_ProcessCapture(void);

#endif /* INC_DHT22_H_ */
