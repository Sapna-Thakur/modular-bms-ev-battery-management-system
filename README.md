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
