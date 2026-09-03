# Modular BMS EV Battery Management

## Task 1 – Modular Battery Management Engine

This project implements a modular Battery Management System (BMS) engine using ESP32 and Wokwi simulation.

The system monitors multiple battery cells and analyzes their voltage condition to identify battery imbalance.

## Features

- Configurable battery cell count
- Weakest cell identification
- Strongest cell identification
- Voltage imbalance calculation
- Imbalance trend monitoring
- Average State of Charge (SoC) estimation
- Adaptive imbalance threshold
- Battery status monitoring
- Non-blocking periodic updates using `millis()`

## Working Principle

The BMS stores battery information using a structured data model. Cell voltages are monitored continuously, and the system identifies the minimum and maximum voltage values.

The voltage imbalance is calculated as:

`Voltage Imbalance = Strongest Cell Voltage - Weakest Cell Voltage`

The imbalance is compared with an adaptive threshold based on the average State of Charge.

## Technology Used

- ESP32
- Arduino C++
- Wokwi Simulator
- GitHub

## Wokwi Simulation

[Open Task 1 Wokwi Simulation](https://wokwi.com/projects/474042704490273793)

## Current Configuration

The current implementation uses 4 simulated battery cells.

The architecture is designed so that the cell count can be modified using:

```cpp
#define CELL_COUNT 4
```

---

# Task 2 – Non-Blocking Protection Relay and Safety System

This project implements a non-blocking protection relay and battery safety system using ESP32 and Wokwi simulation.

The system continuously monitors multiple battery cells and detects unsafe voltage and sensor conditions to protect the battery system.

## Features

- Under-voltage detection
- Over-voltage detection
- Frozen sensor detection
- Unrealistic voltage jump detection
- Out-of-range sensor detection
- Fault debouncing
- Protection relay control
- Automatic recovery verification
- Hysteresis-based recovery
- Non-blocking periodic updates using `millis()`

## Working Principle

The system monitors the voltage of multiple battery cells continuously.

When an unsafe condition is detected, the system verifies the fault using a debounce period. Once the fault is confirmed, the protection relay turns OFF to protect the battery.

After the battery returns to safe operating conditions, the system verifies the recovery condition before turning the relay ON again.

## Technology Used

- ESP32
- Arduino C++
- Wokwi Simulator
- GitHub

## Wokwi Simulation

[Open Task 2 Wokwi Simulation](https://wokwi.com/projects/474042704490273793)

## Current Configuration

The current implementation uses 4 simulated battery cells.

The safety system monitors the cells using:

```cpp
#define CELL_COUNT 4
```
