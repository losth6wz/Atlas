# Atlas

Atlas is a small STM32 mission-computer project built around the **STM32F401RE**. It brings together motion sensing, serial telemetry, command handling, GPS input, SD-card storage, and FreeRTOS in one approachable embedded system.

The project is currently at **Atlas v0.8** and is under active development. The core IMU and telemetry flow is working, while GPS reception and the final logging task integration are still being refined.

## What Atlas can do

- Read acceleration data from an I²C IMU at address `0x68`
- Calculate roll and pitch
- Calibrate the IMU from the board button or a serial command
- Send orientation and status telemetry over UART
- Accept simple `PING` and `CALIBRATE` commands
- Run application work through FreeRTOS/CMSIS-RTOS v2
- Communicate with a GPS module over USART1
- Provide an SPI and FatFs foundation for SD-card logging
- Protect shared RTOS data with queues and mutexes

## Current status

| Area | Status | Notes |
| --- | --- | --- |
| STM32 HAL setup | Ready | Clock, GPIO, I²C, SPI, and UART configured |
| IMU | Working | Readout, calibration, roll, and pitch |
| Telemetry | Working | Thread-safe UART output |
| Command protocol | Working | `PING` and `CALIBRATE` |
| FreeRTOS | Working | Atlas, IMU, and logger tasks configured |
| SD / FatFs | In progress | Driver and storage layers are present |
| GPS | In progress | USART1 receive path is being debugged |

## Hardware and connections

The CubeMX configuration targets an **STM32F401RETx** device in an LQFP64 package.

| Function | Peripheral | Pins / settings |
| --- | --- | --- |
| Debug telemetry | USART2 | PA2 TX, PA3 RX, 115200 baud |
| GPS | USART1 | PA9 TX, PA10 RX, 115200 baud |
| IMU | I²C1 | PB8 SCL, PB9 SDA |
| SD card | SPI1 | PA5 SCK, PA6 MISO, PA7 MOSI |
| SD chip select | GPIO | PB6 |
| Calibration button | GPIO | PC13 / B1 |

> Check the voltage and logic-level requirements of each breakout board before connecting it. GPS and SD modules vary, even when they use the same pin labels.

## Serial commands

Send commands to the telemetry UART as plain text:

```text
PING
CALIBRATE
```

`PING` returns `PONG`. `CALIBRATE` starts IMU calibration; the B1 button performs the same action.

## Project structure

```text
Atlas_V0/
├── Core/
│   ├── Inc/               Application headers
│   └── Src/               Atlas, IMU, GPS, storage, and telemetry logic
├── Drivers/               STM32 HAL and CMSIS
├── FATFS/                 FatFs application and target glue
├── Middlewares/           FreeRTOS and FatFs sources
├── Atlas_V0.ioc           STM32CubeMX configuration
└── STM32F401RETX_*.ld     Linker scripts
```

The application uses a small state machine:

```text
BOOT → READY → CALIBRATING → RUNNING
                         ↘ ERROR
```

## Build and flash

1. Clone this repository.
2. Open **STM32CubeIDE**.
3. Choose **File → Import → Existing Projects into Workspace**.
4. Select the cloned repository folder.
5. Build the project, connect the STM32 board, and run or debug it.

The generated HAL, CMSIS, FreeRTOS, and FatFs sources needed by the project are included in the repository. Build output is intentionally excluded from version control.

## Roadmap

- Finish validating GPS UART output with a known-good GNSS module
- Parse NMEA sentences into structured position data
- Connect the IMU producer and logger consumer tasks end to end
- Finalize periodic CSV logging and synchronization
- Add clearer fault reporting and recovery behavior
- Document a tested wiring diagram

## A note for contributors

Atlas is an evolving learning and engineering project. Small, focused improvements are welcome—especially around reliability, documentation, GPS parsing, and embedded testing.

If you build on it, please test carefully on your own hardware before relying on it in a vehicle or mission-critical system.
