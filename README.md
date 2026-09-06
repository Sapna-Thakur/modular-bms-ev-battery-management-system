# EV Battery Management System using ESP32

# EV Battery Management System (BMS) using ESP32

## Introduction

This project is an ESP32-based **real-time Electric Vehicle Battery Management System (BMS)** designed to monitor, protect, analyze, and communicate battery conditions. It combines battery monitoring, safety protection, fault handling, LCD visualization, event-driven telemetry, and a live Blynk analytics dashboard into one integrated system.

## What This Project Does

The system continuously monitors multiple battery cells and identifies the **weakest and strongest cells, voltage imbalance, battery SoC, faults, system state, and communication health**. It uses non-blocking logic to respond to abnormal conditions without interrupting normal operation.

The project is divided into **six development tasks**, progressing from modular battery analysis to advanced cloud-based analytics and decision support.

## Key Capabilities

* 🔋 **Battery Monitoring** – Cell voltage, SoC, imbalance, weakest and strongest cells
* 🛡️ **Safety Protection** – Fault detection, relay protection, debounce and recovery
* 📟 **Smart LCD Display** – Flicker-free pages with critical fault override
* ⚙️ **Fault State Machine** – NORMAL, DEGRADED, FAILSAFE and SHUTDOWN states
* 📡 **Live Telemetry** – Event-driven data transmission with offline queue support
* 📊 **Blynk Analytics** – Risk score, battery health, fault history and operator recommendations

## Why It Is Useful

A BMS is essential in EVs because battery cells must operate within safe electrical and operational limits. This project demonstrates how an embedded controller can **detect abnormal battery conditions, protect the system, maintain reliable communication, and provide meaningful information to an operator in real time**.

## Benefits

* Improves battery safety and reliability
* Detects cell imbalance and abnormal conditions early
* Reduces unnecessary data transmission
* Provides reliable operation during network failures
* Enables real-time remote monitoring
* Converts raw battery data into useful health and risk information
* Demonstrates a scalable architecture suitable for larger battery packs

## Technology Stack

**ESP32 • Arduino C++ • Blynk IoT • Wi-Fi • 16×2 I2C LCD • Wokwi**

## Conclusion

This project demonstrates a complete **embedded EV-BMS workflow**, from cell-level monitoring and protection to fault management, telemetry, and cloud-based analytics. The modular and non-blocking architecture makes the system easier to scale, maintain, and extend for future EV battery applications.

---

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

https://wokwi.com/projects/474042704490273793

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

https://wokwi.com/projects/474042704490273793

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
---

# Task 4 – Fault State Machine with Structured Recovery

This project implements a deterministic fault state machine for an ESP32-based EV Battery Management System (BMS).

The system uses four operating states: NORMAL, DEGRADED, FAILSAFE, and SHUTDOWN. It identifies different fault sources and provides structured fault recovery.

## Features

- Four operating states using enum
- Clearly defined state transition logic
- Battery cell fault identification
- Relay mismatch detection
- Communication fault identification
- ADC fault identification
- Frozen ADC value detection
- Fault source tracking
- Structured state transition logging
- Timestamp logging
- Previous state and new state logging
- Fault ID logging
- FAILSAFE recovery verification
- Latched SHUTDOWN state
- Deterministic and deadlock-free operation

## Working Principle

The system continuously monitors battery cells, relay status, communication conditions, and ADC values.

When a fault is detected, the system identifies the fault source and assigns a fault ID. The state machine then performs the appropriate state transition according to the defined transition logic.

Frozen ADC values and relay command/feedback mismatches are detected to identify abnormal system conditions.

Every state transition is logged with a timestamp, previous state, new state, and fault ID in a structured format.

Recovery from FAILSAFE is performed only after verifying that the fault condition has been cleared and the required safety conditions are satisfied.

Critical faults can move the system to the SHUTDOWN state. The SHUTDOWN state is latched to prevent unsafe automatic restart.

## Technology Used

- ESP32
- Arduino C++
- Wokwi Simulator
- GitHub

## Wokwi Simulation

https://wokwi.com/projects/474042704490273793

## Current Configuration

The current implementation uses 4 simulated battery cells.

The fault state machine monitors battery cells, relay status, communication conditions, and ADC values.

---

# Task 5 – Event-Driven Telemetry and Live Blynk Dashboard

This project implements an event-driven telemetry system for an ESP32-based EV Battery Management System (BMS).

The system transmits telemetry data only when meaningful events or significant parameter changes occur. When Wi-Fi or Blynk connectivity is lost, telemetry events are stored in a fixed-size offline queue and transmitted in FIFO order after connectivity is restored.

## Features

- Event-driven telemetry
- Meaningful parameter change detection
- Real-time cell voltage monitoring
- Weakest cell identification
- Strongest cell identification
- Relay status monitoring
- Fault state monitoring
- Wi-Fi connectivity monitoring
- RSSI monitoring
- Fixed-size offline telemetry queue
- FIFO queued event transmission
- Queue full handling
- Non-blocking Wi-Fi reconnection state machine
- Live Blynk dashboard
- LIVE and QUEUED telemetry indication
- Network outage demonstration
- Fault injection demonstration

## Working Principle

The system continuously monitors battery parameters and communication status.

Telemetry events are generated only when meaningful changes occur, such as significant cell voltage changes, RSSI changes, relay changes, fault changes, system state changes, or connectivity changes.

When Wi-Fi or Blynk connectivity is unavailable, meaningful telemetry events are stored in a fixed-size offline queue.

After connectivity is restored, the queued events are transmitted in FIFO order before new live telemetry events.

The Wi-Fi connection is managed using a non-blocking state machine with DISCONNECTED, CONNECTING, and CONNECTED states.

The RSSI value is monitored to assess Wi-Fi communication quality.

## Technology Used

- ESP32
- Arduino C++
- Blynk IoT
- Wi-Fi
- Wokwi Simulator
- GitHub

## Wokwi Simulation

https://wokwi.com/projects/474042704490273793

## Current Configuration

The current implementation uses 4 simulated battery cells.

The telemetry system monitors cell voltages, weakest and strongest cells, relay status, fault state, Wi-Fi connectivity, RSSI, and offline queue depth.

The offline queue can store up to 12 telemetry events.

The Blynk dashboard displays real-time battery and communication information and distinguishes between LIVE and QUEUED telemetry.

---

# Enterprise Blynk Analytics and Decision Dashboard

## Task 6 – Enterprise Blynk Analytics and Decision Dashboard

This project implements an advanced analytics and decision dashboard for an ESP32-based EV Battery Management System (BMS) using Blynk IoT.

The system analyzes battery health, voltage imbalance, risk level, fault history, state transitions, and operating conditions to provide real-time analytics and operator recommendations.

## Features

* Real-time battery health monitoring
* State of Charge (SoC) monitoring
* Cell voltage imbalance analysis
* Imbalance trend monitoring
* Composite battery risk score
* Risk level classification
* Structured fault history
* Fault source identification
* State transition tracking
* Uptime monitoring
* Fault count monitoring
* Operator recommendations
* Current system state monitoring
* Historical Blynk data visualization
* Executive summary dashboard
* Non-blocking analytics updates using `millis()`

## Working Principle

The system continuously analyzes battery and system parameters received from the BMS engine.

A composite risk score is calculated using factors such as cell voltage imbalance, fault frequency, and battery SoC.

The calculated risk score is classified into four levels:

* LOW
* MEDIUM
* HIGH
* CRITICAL

Battery health is calculated from the overall risk condition. The system also records fault events and state transitions with timestamps, fault sources, and operating states.

Based on the current battery condition and system state, the dashboard generates human-readable operator recommendations such as battery monitoring, cell inspection, or immediate safety inspection.

The Blynk dashboard provides an executive summary containing battery health, SoC, risk score, risk level, uptime, fault count, current state, and operator recommendation.

## Technology Used

* ESP32
* Arduino C++
* Blynk IoT
* Wi-Fi
* Wokwi Simulator
* GitHub

## Wokwi Simulation

https://wokwi.com/projects/474042704490273793

## Current Configuration

The current implementation uses 4 simulated battery cells.

The analytics system monitors:

* Cell voltages
* Weakest and strongest cells
* SoC
* Voltage imbalance
* Imbalance trend
* Risk score
* Risk level
* Battery health
* Fault count
* State transition count
* Fault source
* Current system state
* Uptime
* Operator recommendation

The Blynk dashboard provides real-time analytics and an executive summary of the EV battery system.

The system also supports fault injection and network outage demonstrations to verify that the dashboard accurately reflects changes in the backend state.

