# 🌡️ DHT22 Sensor Driver for STM32 (HAL-based)

This is a lightweight and reliable driver for reading temperature and humidity from a **DHT22 sensor** using **STM32 microcontrollers** and the **HAL (Hardware Abstraction Layer)**. It uses **Timer Input Capture with DMA** for precise signal decoding.

---

## 📦 Features

- Accurate decoding of DHT22 signal using timer input capture
- Microsecond-level timing via hardware timer
- Configurable GPIO and timer settings
- Handles negative temperatures and checksum validation
- Clean modular design with easy integration

---

## 🛠️ Requirements

- STM32 microcontroller (tested on STM32F1 series)
- STM32CubeIDE or STM32CubeMX
- HAL drivers enabled for:
  - Timer (TIMx) with Input Capture and DMA
  - GPIO

---

## ⚙️ Setup Instructions

### 1. 🧰 Hardware Wiring

DHT22 Sensor:

VCC → 3.3V or 5V

GND → GND

DATA → STM32 GPIO (e.g. PA8)

4.7kΩ pull-up resistor between DATA and VCC


---

### 2. 🧩 CubeMX Configuration

- Enable **TIMx** (e.g. TIM1) with **Input Capture** on a channel (e.g. CH1)
- Enable **DMA** for TIMx Input Capture
- Set timer clock to allow **1 µs resolution** (e.g. 1 MHz)
- Enable GPIO pin as **Alternate Function Input**

---

### 3. 📁 Add Library Files

Copy these files into your project:

Core/Inc/dht22.h 
Core/Src/dht22.c

Include the header in your main file:

```c
#include "dht22.h"


4. ⚙️ Initialize the Driver
In main.c or your setup function:


DHT22_Init(&htim1, GPIOA, GPIO_PIN_8, TIM_CHANNEL_1);
```

🚀 How to Use
1. Start a Measurement
```c
DHT22_Data data = DHT22_Read();

if (data.status == 0) {
    // ✅ Success
    float temperature = data.temperature;
    float humidity = data.humidity;
} else if (data.status == 1) {
    // ❌ Checksum error
} else if (data.status == 2) {
    // ⏱️ Timeout (no response from sensor)
}
```
2. Input Capture Callback
In stm32f1xx_it.c or wherever your TIM IC callback is defined:
```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        DHT22_ProcessCapture();
    }
}
```
# 🤝 Contributing to DHT22 STM32 Driver

Thank you for considering contributing to this project! Whether you're fixing bugs, improving documentation, or adding new features — your help is appreciated.

---

## 🧰 Ways to Contribute

- 🐞 Report bugs or issues
- 💡 Suggest new features or improvements
- 🛠️ Submit pull requests with code changes
- 📚 Improve documentation or examples

---

## 🚀 Getting Started

### 1. Fork the Repository

Click the **Fork** button at the top right of this page to create your own copy.

### 2. Clone Your Fork

```bash
git clone https://github.com/your-username/dht22-stm32-driver.git
cd dht22-stm32-driver
```
