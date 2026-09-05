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
# Task 3 – Flicker-Free LCD Display Engine

This project implements a flicker-free LCD display engine using ESP32 and Wokwi simulation.

The system displays battery and safety information on a 16x2 I2C LCD while updating only the portions of the display whose values have changed.

## Features

* Flicker-free LCD updates
* Updates only changed display content
* Battery status display
* System state display
* Telemetry information display
* Automatic page rotation
* Critical fault screen override
* Non-blocking display refresh using `millis()`
* Non-blocking page rotation using `millis()`

## Working Principle

The LCD display engine stores the previously displayed content and compares it with the current display values.

Only the LCD lines whose content has changed are updated. This eliminates unnecessary screen clearing and reduces visible flickering.

The display automatically rotates between three information pages using a non-blocking timer.

The information pages include Battery Status, System State, and Telemetry Information.

During an active critical fault, the LCD immediately overrides the normal page rotation and displays a dedicated fault screen showing the active fault.

The normal information pages return only after the fault is cleared.

The LCD refresh interval is selected to provide responsive information updates while reducing unnecessary I2C communication and display flickering.

## Technology Used

* ESP32
* Arduino C++
* 16x2 I2C LCD
* Wokwi Simulator
* GitHub

## Wokwi Simulation

https://wokwi.com/projects/474042704490273793

## Current Configuration

The current implementation uses a 16x2 I2C LCD connected to the ESP32.

The LCD automatically rotates between three information pages:

* Battery Status
* System State
* Telemetry Information

The LCD uses a non-blocking refresh and page rotation system based on `millis()`.

During a critical fault, the display immediately switches to a dedicated fault screen until the fault is cleared.

# Task 4 – Fault State Machine with Structured Recovery

This project implements a deterministic fault state machine for an ESP32-based EV Battery Management System (BMS).

The system uses four operating states: NORMAL, DEGRADED, FAILSAFE, and SHUTDOWN. The state machine manages different fault conditions and provides structured recovery handling.

**## Features**

- Four operating states using enum
- Clearly defined state transition logic
- Battery cell fault detection
- Relay mismatch detection
- Communication fault detection
- ADC fault detection
- Frozen ADC value detection
- Fault source identification and tracking
- Structured state transition logging
- Timestamp logging
- Previous and new state logging
- Fault ID logging
- Verified FAILSAFE recovery
- Latched SHUTDOWN state
- Deterministic and deadlock-free operation

**## Working Principle**

The system continuously monitors battery cells, relay status, communication conditions, and ADC values.

When a fault is detected, the system identifies the fault source and assigns a fault ID. The state machine then transitions to the appropriate operating state according to the defined transition logic.

The system detects conditions such as frozen ADC values and relay command/feedback mismatches.

Every state transition is recorded with a timestamp, previous state, new state, and fault ID in a structured format.

Recovery from FAILSAFE is performed only after verifying that the fault condition has been cleared and the required safety conditions are satisfied. The system does not immediately return to NORMAL without verification.

Critical faults can move the system to the SHUTDOWN state. The SHUTDOWN state is latched to prevent unsafe automatic restart.

**## Fault Sources**

- Battery Cells
- Relay
- Communication
- ADC

**## Operating States**

- NORMAL
- DEGRADED
- FAILSAFE
- SHUTDOWN

**## Technology Used**

- ESP32

- Arduino C++

- Wokwi Simulator

- GitHub

**## Wokwi Simulation**

[Open Task 4 Wokwi Simulation]https://wokwi.com/projects/474042704490273793

