# HoverChair

This repository contains the complete, consolidated source code and documentation for the "HoverChair" project, a dual-brain electric vehicle built on a Generation 2 Hoverboard.

## Project Structure

*   **`Controller_ESP32/`**: The "Brain" of the operation. This contains the PlatformIO project for the ESP32 microcontroller, which handles the user interface (Joystick/Web) and sends serial commands to the Hoverboard.
*   **`Firmware_GD32/`**: The "Muscle". This contains the original Keil/C project for the GD32F130C8 microcontrollers inside the Hoverboard (Master and Slave sensorboards).
*   **`Binaries/`**: Pre-compiled, tested `.bin` files (`hover_master.bin`, `hover_slave.bin`) ready to be flashed to the GD32 chips.
*   **`Docs/`**: Architecture diagrams, pinout documentation, and original schematics.
*   **`Tools/`**: Useful utilities, such as the OpenOCD configuration file (`hover_swd.cfg`) for flashing the GD32 chips via a Raspberry Pi using SWD.

## Getting Started

1.  **Flash the Hoverboard (Muscle):** Use the binaries in `Binaries/` and the tools in `Tools/` to flash your Master and Slave sensorboards.
2.  **Flash the ESP32 (Brain):** Open `Controller_ESP32/` in PlatformIO (VS Code) and upload to your ESP32.
3.  **Wire it up:** Consult `Docs/ARCHITECTURE.md` for the wiring guide.