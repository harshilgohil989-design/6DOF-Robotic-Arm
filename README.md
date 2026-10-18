# 6-DOF Robotic Arm — Vision-Based Automatic Pick and Place

An ESP32-based robotic arm project that combines computer vision, UART communication, and servo motor control to perform colour-based object sorting.

## Project Overview

This project uses two ESP32 boards to implement a vision-guided robotic arm. The ESP32-CAM captures images and identifies coloured objects, while the ESP32-WROOM-32 receives detection commands over UART and controls six servo motors to perform pick-and-place operations.

The project is designed to demonstrate embedded firmware development, serial communication, camera interfacing, and robotic motion control.

## System Architecture

```text
           Coloured Object
                  |
                  v
          +----------------+
          |   ESP32-CAM    |
          | Camera Module  |
          | Colour Detection|
          +----------------+
                  |
             UART Serial
          TX -> RX | GND
                  |
                  v
          +----------------+
          | ESP32-WROOM-32 |
          | Command Parser |
          | Servo Control  |
          +----------------+
                  |
                  v
          +----------------+
          | 6-DOF Robotic  |
          |      Arm       |
          +----------------+
                  |
                  v
           Pick and Place
```

## Hardware Components

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32-WROOM-32 | 1 | Main controller |
| ESP32-CAM with OV2640 | 1 | Image capture and colour detection |
| MG995 servo motors | 3 | High-torque joint movement |
| SG90 servo motors | 3 | Lightweight joint movement |
| External regulated servo power supply | 1 | Servo power |
| Jumper wires and connecting hardware | As required | Interconnections |

## Features

- Camera-based red, blue, and black colour detection.
- UART serial communication between two ESP32 boards.
- Command-based robotic arm control.
- Six servo motor channels.
- Configurable home, pick, and drop positions.
- Modular firmware for vision and motion control.
- Serial debugging for development and testing.

## Communication Protocol

The ESP32-CAM sends newline-terminated text commands to the ESP32-WROOM-32 at 9600 baud.

| Command | Description |
|---|---|
| `COLOR:RED` | Select the red-object sorting sequence |
| `COLOR:BLUE` | Select the blue-object sorting sequence |
| `COLOR:BLACK` | Select the black-object sorting sequence |
| `HOME` | Return the arm to its configured home pose |
| `START` | Enable command processing |
| `STOP` | Disable acceptance of new commands |

## UART Wiring

| ESP32-CAM | ESP32-WROOM-32 |
|---|---|
| GPIO13 (TX) | GPIO16 (RX2) |
| GPIO14 (RX) | GPIO17 (TX2), for two-way communication |
| GND | GND |

The UART pins are configurable. GPIO13 and GPIO14 share functions with the ESP32-CAM microSD interface, so the microSD card must not be used with this configuration.

## Software and Tools

- Arduino IDE
- ESP32 Arduino Core
- ESP32Servo library
- ESP32 camera driver (`esp_camera.h`)
- GitHub Desktop
- Visual Studio Code

## Repository Structure

```text
6DOF-Robotic-Arm/
├── ESP32_Controller/
│   └── ESP32_Controller.ino
├── ESP32_CAM_Vision/
│   └── ESP32_CAM_Vision.ino
├── Hardware_Photos/
└── README.md
```

## Setup Instructions

1. Install the ESP32 board support package in Arduino IDE.
2. Install the ESP32Servo library for the controller firmware.
3. Open `ESP32_Controller/ESP32_Controller.ino` and select the appropriate ESP32 board.
4. Open `ESP32_CAM_Vision/ESP32_CAM_Vision.ino` and configure it for the supported ESP32-CAM board variant.
5. Verify the GPIO assignments and UART connections before powering the hardware.
6. Connect the ESP32-CAM TX to the ESP32-WROOM RX and connect both grounds.
7. Configure the servo positions according to the arm's mechanical geometry.
8. Compile and test each board independently before testing the complete system.

## Safety and Calibration

- Power the servo motors using a suitable external regulated supply. Do not power six servos from the ESP32 3.3 V pin.
- Connect the external supply ground to the ESP32 ground.
- Check servo direction, mechanical limits, and joint clearance before movement.
- Calibrate the pick, drop, and home positions before automatic operation.
- Colour detection thresholds may require adjustment for ambient lighting, object size, and camera exposure.
- Test UART communication before enabling automatic motion.

## Current Development Status

The repository contains the initial firmware implementation for vision-based colour detection, UART command transfer, and servo control.

**Validation status:** Hardware operation, colour-detection accuracy, UART reliability, and robotic motion require testing on the assembled system. The configured GPIO pins and motion angles must be verified against the actual wiring and mechanical design.

## Future Improvements

- Add inverse kinematics for improved end-effector positioning.
- Implement non-blocking servo motion and safer emergency-stop handling.
- Improve colour detection using HSV-based image processing.
- Add camera-to-controller acknowledgements and communication error handling.
- Record sorting results and performance measurements.
- Add hardware diagrams, calibration instructions, and demonstration videos.

## Author

**Harshil Gohil**

Robotics and Automation Engineering Student

Interested in Embedded Systems, Firmware Development, PCB Design, and Industrial Automation.
