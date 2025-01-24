# Dynamic Leader Election for ESP32 Devices

This project implements a dynamic leader election protocol for multiple ESP32 devices using ESP-NOW. 
Upon boot, devices attempt to find an existing leader, and if no leader is found, the device assumes leadership.

## Features
- **Leader Election**: Devices check for an existing leader and elect one if none is found.
- **ESP-NOW Communication**: Devices use ESP-NOW to send and receive leader status and other messages.
- **Automatic Leader Handling**: If a leader is elected, other devices adjust their behavior accordingly.
