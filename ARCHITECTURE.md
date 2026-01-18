# Hoverboard Chair Controller - ESP32 Project

## Project Overview

This project converts two hoverboard motor controller boards into a motorized computer chair, controlled by an ESP32 Dev Board v2 with phone app and joystick input.

---

## System Architecture

```
                           ┌──────────────────────────────────────────────────────────────────────────┐
                           │                        42V Li-Ion Battery Pack                           │
                           │                    (10S, 36V nominal / 42V charged)                      │
                           └────────────────────────────────┬─────────────────────────────────────────┘
                                                            │
                              ┌─────────────────────────────┼─────────────────────────────────────────┐
                              │                             │                                         │
                              │                      ┌──────▼──────┐                                  │
                              │                      │ Charge Port │                                  │
                              │                      │  (42V DC)   │                                  │
                              │                      └─────────────┘                                  │
                              │                                                                       │
                       ┌──────▼──────┐                                                                │
                       │Power Button │ (directly wired to MASTER board)                               │
                       │   (PC15)    │                                                                │
                       └──────┬──────┘                                                                │
                              │                                                                       │
         ┌────────────────────┴────────────────────┐                                                  │
         │                                         │                                                  │
         │  SELF_HOLD (PB2) latches power on       │                                                  │
         │                                         │                                                  │
   ┌─────▼─────────────────────────┐       ┌───────▼───────────────────────┐                          │
   │     Hoverboard MASTER         │       │     Hoverboard SLAVE          │                          │
   │        (GD32F130C8)           │       │        (GD32F130C8)           │                          │
   │                               │       │                               │                          │
   │ PWR IN ◄──────────────────────┼───────┼──◄────────────────────────────┼──────────────────────────┘
   │                               │       │                               │       (Battery → both boards)
   │ VBATT ADC (PA4) ──────────────┼───────┼───────────────────────────────│
   │   (monitors battery voltage)  │       │                               │
   │                               │       │                               │
   │ CHARGE_STATE (PF0) ◄──────────┼───────┼─── Charger detection          │
   │                               │       │                               │
   │ BUZZER (PB10) ────────────────┼───┐   │                               │
   │                               │   │   │                               │
   │ USART1 TX/RX (PA2/PA3) ◄──────┼───┼───┼► USART1 TX/RX (PA2/PA3)       │
   │   (Master↔Slave comms)        │   │   │   (Master↔Slave comms)        │
   │                               │   │   │                               │
   │ USART0 RX (PB7) ◄─────────────┼───┼───┼─────────────────────────┐     │
   │   (Steering input from ESP32) │   │   │                         │     │
   │                               │   │   │                         │     │
   │ MOTOR ────────────────────────┼─► │   │ MOTOR ──────────────────┼─►   │
   │   (Left Wheel)                │   │   │   (Right Wheel)         │     │
   └───────────────────────────────┘   │   └───────────────────────────────┘
                                       │                             │
                                       │                             │
   ┌───────────────────────────────────┼─────────────────────────────┼─────────────────────────────────┐
   │                              ESP32 Dev Board v2                                                   │
   │                                   │                             │                                 │
   │  ┌──────────────┐  ┌──────────────┤  ┌──────────────┐           │                                 │
   │  │ Phone App    │  │ Joystick     │  │ Speaker      │◄──────────┘ (optional buzzer passthrough)   │
   │  │ (BLE/WiFi)   │  │ (Analog)     │  │ + Amp        │                                             │
   │  └──────┬───────┘  └──────┬───────┘  └──────────────┘                                             │
   │         └────────────┬────┘                                                                       │
   │                      │                                                                            │
   │              ┌───────▼───────┐                                                                    │
   │              │ Motor Control │                                                                    │
   │              │   Mixer       │                                                                    │
   │              └───────┬───────┘                                                                    │
   │                      │                                                                            │
   │              ┌───────▼───────┐                                                                    │
   │              │   UART TX     │────────────────────────► To MASTER PB7 (Steering input)            │
   │              │  (26300 baud) │                                                                    │
   │              └───────────────┘                                                                    │
   │                                                                                                   │
   │  Power: 5V from MASTER board's 5V breakout header (or USB during dev)                             │
   └───────────────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Power Distribution

| Component | Power Source | Voltage | Notes |
|-----------|--------------|---------|-------|
| Battery | 10S Li-Ion Pack | 42V max / 36V nom | Standard hoverboard battery |
| MASTER board | Battery direct | 42V → 3.3V/5V onboard | Has voltage regulators |
| SLAVE board | Battery direct | 42V → 3.3V/5V onboard | Has voltage regulators |
| ESP32 | MASTER 5V breakout | 5V | Or USB during development |
| Charge Port | External charger | 42V 2A typical | Charges battery |

---

## Communication Protocol

### ESP32 → MASTER Board (Steering Commands)

- **UART**: USART0 on MASTER board
- **Pins**: ESP32 TX → MASTER PB7 (RX)
- **Baud Rate**: 26300
- **Frame Format**: 8 bytes total

```
Byte 0:     '/'           (0x2F) - Start character
Byte 1-2:   Speed         (int16_t, big-endian) - Range: -1000 to +1000
Byte 3-4:   Steer         (int16_t, big-endian) - Range: -1000 to +1000
Byte 5-6:   CRC16         (big-endian)
Byte 7:     '\n'          (0x0A) - Stop character
```

### MASTER → SLAVE Board (Internal)

- **UART**: USART1 on both boards
- **Pins**: PA2 (TX) / PA3 (RX)
- **Baud Rate**: 26300
- **Frame Format**: 10 bytes from MASTER, 5 bytes from SLAVE

---

## Hoverboard Board Pin Reference

### MASTER Board Key Pins

| Function | Pin | Notes |
|----------|-----|-------|
| Steering RX | PB7 | USART0 - ESP32 connects here |
| Steering TX | PB6 | USART0 - Not used for receive-only |
| Slave TX | PA2 | USART1 - To SLAVE |
| Slave RX | PA3 | USART1 - From SLAVE |
| Buzzer | PB10 | GPIO toggle for tones |
| Power Button | PC15 | Input, active low |
| Self Hold | PB2 | Output, keeps power on |
| Battery Voltage | PA4 | ADC input |
| Charge State | PF0 | Input, charger detection |

### SLAVE Board Key Pins

| Function | Pin | Notes |
|----------|-----|-------|
| Master TX | PA2 | USART1 - To MASTER |
| Master RX | PA3 | USART1 - From MASTER |

---

## ESP32 Dev Board v2 Pin Assignments

> **TODO**: Update with actual ESP32 pinout once provided

| Function | ESP32 Pin | Notes |
|----------|-----------|-------|
| UART TX (to MASTER) | TBD | Connect to MASTER PB7 |
| Joystick X | TBD | Analog input |
| Joystick Y | TBD | Analog input |
| Buzzer/Speaker | TBD | PWM output |
| BLE/WiFi | Built-in | Phone app control |

---

## Speaker/Buzzer Options

### Option 1: Simple Piezo Buzzer (Recommended for Phase 1)
- No amplifier needed
- Wiring: `ESP32 GPIO → 100-330Ω resistor → Piezo (+) → GND`
- Uses `tone()` function for melodies

### Option 2: Speaker + PAM8403 Amp (Future upgrade)
- Better sound quality for melodies
- Wiring: `ESP32 DAC/PWM → PAM8403 input → 4-8Ω speaker`
- Can play more complex audio

---

## Firmware Configuration

### Hoverboard Boards (GD32F130C8)

The original firmware in `../HoverBoardGigaDevice/` needs:

1. **MASTER board**: Compile with `#define MASTER` in `config.h`
2. **SLAVE board**: Compile with `#define SLAVE` in `config.h` (comment out MASTER)

Key config values in `config.h`:
```c
#define PWM_FREQ         16000    // PWM frequency in Hz
#define DC_CUR_LIMIT     15       // Motor current limit in amps
#define TIMEOUT_MS       2000     // Steering timeout before emergency stop
#define INACTIVITY_TIMEOUT 8      // Minutes until auto power-off
```

### ESP32 Firmware

Located in this folder - handles:
- Phone app (BLE/WiFi)
- Joystick input
- Mixing speed/steer values
- UART transmission to MASTER board
- Buzzer melodies

---

## Build Checklist

- [ ] Flash MASTER firmware to left wheel board
- [ ] Flash SLAVE firmware to right wheel board  
- [ ] Wire MASTER↔SLAVE USART connection (if not already)
- [ ] Wire ESP32 TX → MASTER PB7
- [ ] Wire ESP32 power from MASTER 5V header
- [ ] Connect joystick to ESP32
- [ ] Connect buzzer to ESP32
- [ ] Test with USB-TTL debugger
- [ ] Test phone app control

---

## Safety Notes

⚠️ **IMPORTANT**:
- Always test with wheels off the ground first
- The 42V battery can deliver dangerous current
- Use the `TIMEOUT_MS` safety feature - motors stop if no commands received
- Keep emergency power-off accessible
- Start with low speed values and gradually increase

---

## Files in This Project

```
ESP32_Chair_Controller/
├── ARCHITECTURE.md          <- This file
├── src/                     <- ESP32 firmware source (TODO)
├── include/                 <- Header files (TODO)
└── platformio.ini           <- PlatformIO config (TODO)
```

The original hoverboard firmware reference is in `../HoverBoardGigaDevice/`
