# HoverChair Project - Copilot Instructions

## Project Overview

This is a motorized computer chair project using salvaged hoverboard motors and controllers. An ESP32 serves as the main controller, receiving input from a phone app (BLE/WiFi) or joystick and sending commands to the hoverboard motor boards.

## Hardware Architecture

### Components
- **2x Hoverboard Controller Boards** (GD32F130C8, ARM Cortex-M3)
  - MASTER board: Left wheel, receives steering commands, controls slave
  - SLAVE board: Right wheel, receives PWM from master
- **1x ESP32 DOIT DevKit v1** (30-pin, ESP-WROOM-32)
  - Main controller for user input and command generation
- **42V Li-Ion Battery** (10S, 36V nominal)
- **2-axis Analog Joystick** with button
- **Piezo Buzzer** for melodies/alerts
- **USB-TTL with FT232RNL** for debugging/flashing

### Communication
- ESP32 → MASTER: UART at 26300 baud (GPIO17 TX → MASTER PB7 RX)
- MASTER ↔ SLAVE: UART at 26300 baud (PA2/PA3)

## ESP32 Pin Assignments

| Function | GPIO | Notes |
|----------|------|-------|
| UART TX (to MASTER) | GPIO17 | UART2 TX |
| UART RX (from MASTER) | GPIO16 | UART2 RX (optional telemetry) |
| Joystick X | GPIO34 | ADC1_CH6 |
| Joystick Y | GPIO35 | ADC1_CH7 |
| Joystick Button | GPIO32 | Internal pullup |
| Buzzer | GPIO25 | DAC1 |
| Status LED | GPIO2 | Onboard LED |

## Steering Protocol (ESP32 → MASTER)

8-byte frame at 26300 baud:
```
Byte 0:     '/'           (0x2F) - Start
Byte 1-2:   Speed         (int16_t, big-endian) -1000 to +1000
Byte 3-4:   Steer         (int16_t, big-endian) -1000 to +1000  
Byte 5-6:   CRC16         (big-endian)
Byte 7:     '\n'          (0x0A) - Stop
```

## CRC Calculation

Reference implementation from hoverboard firmware:
```c
uint16_t CalcCRC(uint8_t *ptr, int count) {
    uint16_t crc = 0;
    while (count--) {
        crc ^= *ptr++ << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}
```

## Key Files

- `ARCHITECTURE.md` - Full system diagrams and wiring
- `src/` - ESP32 firmware (PlatformIO)
- `../HoverBoardGigaDevice/` - Original hoverboard firmware (reference only, do not modify)

## Development Notes

- Use PlatformIO with Arduino framework for ESP32
- Hoverboard firmware compiled with Keil µVision (reference in parent repo)
- MASTER/SLAVE selection via `#define MASTER` or `#define SLAVE` in hoverboard `config.h`
- 2000ms timeout on hoverboard - motors stop if no commands received

## Safety

- Always test with wheels elevated
- TIMEOUT_MS = 2000 provides emergency stop
- Start with low speed values (±100) before full range
- Keep emergency power-off accessible

## Repository

- GitHub: https://github.com/wyofalcon/HoverChair
- Parent reference repo: Hoverboard-Firmware-Hack-Gen2 (local only)
