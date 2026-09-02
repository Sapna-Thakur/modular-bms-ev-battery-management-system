#include <Arduino.h>

// =====================================================
// TASK 1: MODULAR BATTERY MANAGEMENT ENGINE
// ESP32 + Wokwi Simulation
// =====================================================

// Change only this constant to scale the system
#define CELL_COUNT 4

// Battery information structure
struct BatteryData {
  float voltage[CELL_COUNT];

  int weakestCell;
  int strongestCell;

  float weakestVoltage;
  float strongestVoltage;

  float imbalance;
  float previousImbalance;

  float averageSoC;
  float adaptiveThreshold;

  String trend;
  String status;
};

BatteryData battery;

// Simulation timer
unsigned long previousUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2000;

// -----------------------------------------------------
// Initialize battery cells
// -----------------------------------------------------
void initializeBattery() {

  battery.voltage[0] = 3.82;
  battery.voltage[1] = 3.74;
  battery.voltage[2] = 3.89;
  battery.voltage[3] = 3.61;

  battery.previousImbalance = 0.0;
}

// -----------------------------------------------------
// Simulate battery voltage changes
// -----------------------------------------------------
void simulateBatteryChange() {

  // Simulated discharge / imbalance changes
  battery.voltage[0] -= 0.002;
  battery.voltage[1] -= 0.003;
  battery.voltage[2] -= 0.001;
  battery.voltage[3] -= 0.004;

  // Prevent unrealistic values
  for (int i = 0; i < CELL_COUNT; i++) {

    if (battery.voltage[i] < 3.20) {
      battery.voltage[i] = 3.80;
    }
  }
}

// -----------------------------------------------------
// Calculate average State of Charge
// Simple simulation model
// -----------------------------------------------------
float calculateAverageSoC() {

  float totalSoC = 0;

  for (int i = 0; i < CELL_COUNT; i++) {

    float soc = ((battery.voltage[i] - 3.0) / 1.2) * 100;

    soc = constrain(soc, 0, 100);

    totalSoC += soc;
  }

  return totalSoC / CELL_COUNT;
}

// -----------------------------------------------------
// Adaptive imbalance threshold
// Threshold changes according to battery SoC
// -----------------------------------------------------
float calculateAdaptiveThreshold(float averageSoC) {

  if (averageSoC > 80) {
    return 0.08;
  }
  else if (averageSoC > 50) {
    return 0.12;
  }
  else {
    return 0.18;
  }
}

// -----------------------------------------------------
// Analyze battery cells
// -----------------------------------------------------
void analyzeBattery() {

  battery.weakestCell = 0;
  battery.strongestCell = 0;

  battery.weakestVoltage = battery.voltage[0];
  battery.strongestVoltage = battery.voltage[0];

  // Find weakest and strongest cell
  for (int i = 1; i < CELL_COUNT; i++) {

    if (battery.voltage[i] < battery.weakestVoltage) {

      battery.weakestVoltage = battery.voltage[i];
      battery.weakestCell = i;
    }

    if (battery.voltage[i] > battery.strongestVoltage) {

      battery.strongestVoltage = battery.voltage[i];
      battery.strongestCell = i;
    }
  }

  // Calculate voltage imbalance
  battery.imbalance =
      battery.strongestVoltage - battery.weakestVoltage;

  // Calculate average SoC
  battery.averageSoC = calculateAverageSoC();

  // Calculate adaptive threshold
  battery.adaptiveThreshold =
      calculateAdaptiveThreshold(battery.averageSoC);

  // Detect imbalance trend
  if (battery.imbalance > battery.previousImbalance + 0.005) {

    battery.trend = "INCREASING";
  }
  else if (battery.imbalance < battery.previousImbalance - 0.005) {

    battery.trend = "DECREASING";
  }
  else {

    battery.trend = "STABLE";
  }

  // Determine BMS status
  if (battery.imbalance > battery.adaptiveThreshold) {

    battery.status = "IMBALANCE WARNING";
  }
  else {

    battery.status = "NORMAL";
  }

  // Store current imbalance for next comparison
  battery.previousImbalance = battery.imbalance;
}

// -----------------------------------------------------
// Display battery information
// -----------------------------------------------------
void printBatteryStatus() {

  Serial.println();
  Serial.println("==========================================");
  Serial.println("     MODULAR EV BATTERY MANAGEMENT");
  Serial.println("==========================================");

  Serial.println();

  Serial.print("Configured Cells: ");
  Serial.println(CELL_COUNT);

  Serial.println();
  Serial.println("CELL VOLTAGES:");

  for (int i = 0; i < CELL_COUNT; i++) {

    Serial.print("Cell ");
    Serial.print(i + 1);

    Serial.print(": ");
    Serial.print(battery.voltage[i], 3);

    Serial.println(" V");
  }

  Serial.println();

  Serial.print("Weakest Cell: Cell ");
  Serial.print(battery.weakestCell + 1);

  Serial.print(" (");
  Serial.print(battery.weakestVoltage, 3);

  Serial.println(" V)");

  Serial.print("Strongest Cell: Cell ");
  Serial.print(battery.strongestCell + 1);

  Serial.print(" (");
  Serial.print(battery.strongestVoltage, 3);

  Serial.println(" V)");

  Serial.println();

  Serial.print("Voltage Imbalance: ");
  Serial.print(battery.imbalance, 3);
  Serial.println(" V");

  Serial.print("Average SoC: ");
  Serial.print(battery.averageSoC, 1);
  Serial.println("%");

  Serial.print("Adaptive Threshold: ");
  Serial.print(battery.adaptiveThreshold, 3);
  Serial.println(" V");

  Serial.print("Imbalance Trend: ");
  Serial.println(battery.trend);

  Serial.print("BMS Status: ");
  Serial.println(battery.status);

  Serial.println("==========================================");
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println("Starting Modular BMS Engine...");

  initializeBattery();

  analyzeBattery();

  printBatteryStatus();
}

// =====================================================
// MAIN LOOP - NON-BLOCKING
// =====================================================
void loop() {

  unsigned long currentTime = millis();

  if (currentTime - previousUpdate >= UPDATE_INTERVAL) {

    previousUpdate = currentTime;

    simulateBatteryChange();

    analyzeBattery();

    printBatteryStatus();
  }
}
