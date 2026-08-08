# ATLAS Mission Computer 🚀

ATLAS is a custom embedded mission computer built around the **STM32F401RE**.

The project started as a simple IMU-based orientation system and has evolved into a modular, FreeRTOS-based embedded platform with real-time sensing, telemetry, SD-card logging, command handling, and an expanding sensor stack.

The goal is to build ATLAS like an actual small mission computer rather than a collection of disconnected sensor demos.

---

## Current Version

**Firmware v0.8**

Current system status:

| System | Status |
|---|---|
| STM32 Core | ✅ Working |
| FreeRTOS | ✅ Working |
| IMU | ✅ Working |
| Orientation Estimation | ✅ Working |
| PC Telemetry | ✅ Working |
| Command Protocol | ✅ Working |
| Calibration | ✅ Working |
| SD Card / FatFS | ✅ Working |
| RTOS Logger | ✅ Working |
| CSV Mission Logging | ✅ Working |
| Queue Overflow Monitoring | ✅ Working |
| GPS | 🚧 In Development |
| Health Monitoring | 🚧 Planned |

---

## Hardware

### Main Controller

- STM32 NUCLEO-F401RE
- ARM Cortex-M4
- STM32F401RE MCU

### IMU

The IMU is connected over **I2C1** and currently provides:

- Accelerometer measurements
- Roll estimation
- Pitch estimation
- Calibration support

### Storage

A microSD card is connected using **SPI1**.

Storage uses:

- FatFS
- Custom SPI SD-card driver
- Persistent mission log file
- Batched writes
- Periodic synchronization

### GPS

GPS support is currently under development using **USART1**.

The intended architecture is:

```text
GPS
 │
 ▼
USART1
 │
 ▼
GPS Task
 │
 ▼
RTOS Data Layer
 ├── Telemetry
 └── Mission Logger
