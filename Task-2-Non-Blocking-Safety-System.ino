/*
   =========================================================
   TASK 2
   NON-BLOCKING PROTECTION RELAY AND SAFETY SYSTEM
   ESP32 + Wokwi Simulation
   =========================================================
*/

#define CELL_COUNT 4

// =========================================================
// TIMING SETTINGS
// =========================================================

const unsigned long SAMPLE_INTERVAL = 500;
const unsigned long DEBOUNCE_TIME   = 1500;
const unsigned long RECOVERY_TIME   = 5000;
const unsigned long FROZEN_TIME     = 3000;


// =========================================================
// VOLTAGE LIMITS
// =========================================================

// Fault trip thresholds
const float UNDER_VOLTAGE_TRIP = 3.20;
const float OVER_VOLTAGE_TRIP  = 4.25;

// Recovery thresholds
const float UNDER_VOLTAGE_RECOVER = 3.35;
const float OVER_VOLTAGE_RECOVER  = 4.15;

// Sensor valid range
const float SENSOR_MIN = 2.50;
const float SENSOR_MAX = 4.35;

// Unrealistic voltage jump
const float MAX_VOLTAGE_JUMP = 0.35;

// Frozen sensor tolerance
const float FROZEN_TOLERANCE = 0.0001;


// =========================================================
// ENUMS
// =========================================================

enum SafetyState
{
  NORMAL,
  DEBOUNCING,
  FAULT_ACTIVE,
  RECOVERY_VERIFYING
};


enum FaultType
{
  NO_FAULT,
  UNDER_VOLTAGE,
  OVER_VOLTAGE,
  FROZEN_SENSOR,
  UNREALISTIC_JUMP,
  OUT_OF_RANGE
};


// =========================================================
// GLOBAL VARIABLES
// =========================================================

float cellVoltage[CELL_COUNT] =
{
  3.820,
  3.740,
  3.890,
  3.610
};


float previousVoltage[CELL_COUNT];

unsigned long frozenStartTime[CELL_COUNT];

unsigned long previousSampleTime = 0;
unsigned long debounceStartTime = 0;
unsigned long recoveryStartTime = 0;


SafetyState safetyState = NORMAL;

FaultType activeFault = NO_FAULT;
FaultType candidateFault = NO_FAULT;


bool relayState = false;

int simulationStep = 0;


// =========================================================
// FUNCTION DECLARATIONS
// =========================================================

void updateBatterySimulation();

FaultType detectFault();

bool safeForRecovery();

void updateSafetySystem();

void setRelay(bool state);

const char* getFaultName(FaultType fault);

const char* getStateName(SafetyState state);

void printSystemStatus();


// =========================================================
// SETUP
// =========================================================

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < CELL_COUNT; i++)
  {
    previousVoltage[i] = cellVoltage[i];

    // Frozen timer starts from zero
    frozenStartTime[i] = 0;
  }

  Serial.println();
  Serial.println("==========================================");
  Serial.println("STARTING TASK 2 SAFETY SYSTEM");
  Serial.println("NON-BLOCKING PROTECTION RELAY");
  Serial.println("==========================================");
  Serial.println();

  setRelay(true);
}


// =========================================================
// MAIN LOOP
// =========================================================

void loop()
{
  unsigned long currentTime = millis();

  if (currentTime - previousSampleTime >= SAMPLE_INTERVAL)
  {
    previousSampleTime = currentTime;

    // Update simulated battery
    updateBatterySimulation();

    // Run safety logic
    updateSafetySystem();

    // Print status
    printSystemStatus();
  }
}


// =========================================================
// BATTERY SIMULATION
// =========================================================

void updateBatterySimulation()
{
  simulationStep++;


  // =======================================================
  // NORMAL BATTERY BEHAVIOUR
  // =======================================================

  // Cell 1
  cellVoltage[0] -= 0.002;

  // Cell 2
  cellVoltage[1] -= 0.003;

  // Cell 3
  cellVoltage[2] -= 0.001;

  // Cell 4
  cellVoltage[3] -= 0.002;


  // =======================================================
  // TEST 1 : UNDER VOLTAGE
  // =======================================================

  if (simulationStep >= 12 &&
      simulationStep < 18)
  {
    if (simulationStep == 12)
    {
      Serial.println();
      Serial.println(
        ">>> FAULT INJECTION: CELL 4 UNDER VOLTAGE <<<"
      );
    }

    cellVoltage[3] = 3.10;
  }


  // =======================================================
  // CLEAR UNDER VOLTAGE
  // =======================================================

  if (simulationStep >= 18 &&
      simulationStep < 32)
  {
    if (simulationStep == 18)
    {
      Serial.println();
      Serial.println(
        ">>> UNDER VOLTAGE CLEARED <<<"
      );
    }

    // Keep safely above recovery threshold
    cellVoltage[3] = 3.60;
  }


  // =======================================================
  // TEST 2 : OVER VOLTAGE
  // =======================================================

  if (simulationStep >= 32 &&
      simulationStep < 38)
  {
    if (simulationStep == 32)
    {
      Serial.println();
      Serial.println(
        ">>> FAULT INJECTION: CELL 3 OVER VOLTAGE <<<"
      );
    }

    cellVoltage[2] = 4.30;
  }


  // =======================================================
  // CLEAR OVER VOLTAGE
  // =======================================================

  if (simulationStep >= 38 &&
      simulationStep < 52)
  {
    if (simulationStep == 38)
    {
      Serial.println();
      Serial.println(
        ">>> OVER VOLTAGE CLEARED <<<"
      );
    }

    cellVoltage[2] = 4.00;
  }


  // =======================================================
  // TEST 3 : UNREALISTIC JUMP
  // =======================================================

  if (simulationStep == 52)
  {
    Serial.println();
    Serial.println(
      ">>> FAULT INJECTION: UNREALISTIC VOLTAGE JUMP <<<"
    );

    cellVoltage[1] += 0.60;
  }


  // =======================================================
  // CLEAR UNREALISTIC JUMP
  // =======================================================

  if (simulationStep > 52 &&
      simulationStep < 68)
  {
    if (simulationStep == 53)
    {
      Serial.println();
      Serial.println(
        ">>> UNREALISTIC JUMP CLEARED <<<"
      );
    }

    // Normal value
    cellVoltage[1] = 3.75;
  }


  // =======================================================
  // TEST 4 : FROZEN SENSOR
  // =======================================================

  if (simulationStep >= 68 &&
      simulationStep < 78)
  {
    if (simulationStep == 68)
    {
      Serial.println();
      Serial.println(
        ">>> FAULT INJECTION: FROZEN SENSOR CELL 1 <<<"
      );
    }

    // Freeze Cell 1
    // Keep exactly same voltage
    cellVoltage[0] = previousVoltage[0];
  }


  // =======================================================
  // CLEAR FROZEN SENSOR
  // =======================================================

  if (simulationStep == 78)
  {
    Serial.println();
    Serial.println(
      ">>> FROZEN SENSOR CLEARED <<<"
    );
  }


  // =======================================================
  // FINAL NORMAL OPERATION
  // =======================================================

  if (simulationStep >= 78)
  {
    // All cells continue changing normally
  }
}


// =========================================================
// FAULT DETECTION
// =========================================================

FaultType detectFault()
{
  unsigned long currentTime = millis();


  for (int i = 0; i < CELL_COUNT; i++)
  {
    float voltage = cellVoltage[i];

    float difference =
      fabs(voltage - previousVoltage[i]);


    // =====================================================
    // OUT OF RANGE
    // =====================================================

    if (voltage < SENSOR_MIN ||
        voltage > SENSOR_MAX)
    {
      return OUT_OF_RANGE;
    }


    // =====================================================
    // UNDER VOLTAGE
    // =====================================================

    if (voltage <= UNDER_VOLTAGE_TRIP)
    {
      return UNDER_VOLTAGE;
    }


    // =====================================================
    // OVER VOLTAGE
    // =====================================================

    if (voltage >= OVER_VOLTAGE_TRIP)
    {
      return OVER_VOLTAGE;
    }


    // =====================================================
    // UNREALISTIC JUMP
    // =====================================================

    if (difference > MAX_VOLTAGE_JUMP)
    {
      return UNREALISTIC_JUMP;
    }


    // =====================================================
    // FROZEN SENSOR
    // =====================================================

    if (difference <= FROZEN_TOLERANCE)
    {
      // Start timer when sensor first becomes frozen
      if (frozenStartTime[i] == 0)
      {
        frozenStartTime[i] = currentTime;
      }

      // Frozen for required time
      if (currentTime - frozenStartTime[i]
          >= FROZEN_TIME)
      {
        return FROZEN_SENSOR;
      }
    }
    else
    {
      // Sensor is changing normally
      frozenStartTime[i] = 0;
    }
  }


  return NO_FAULT;
}


// =========================================================
// SAFE RECOVERY CHECK
// =========================================================

bool safeForRecovery()
{
  // =======================================================
  // CHECK ALL VOLTAGES
  // =======================================================

  for (int i = 0; i < CELL_COUNT; i++)
  {
    float voltage = cellVoltage[i];


    // Under-voltage recovery
    if (voltage < UNDER_VOLTAGE_RECOVER)
    {
      return false;
    }


    // Over-voltage recovery
    if (voltage > OVER_VOLTAGE_RECOVER)
    {
      return false;
    }
  }


  // =======================================================
  // CHECK CURRENT FAULT
  // =======================================================

  FaultType currentFault = detectFault();

  if (currentFault != NO_FAULT)
  {
    return false;
  }


  return true;
}


// =========================================================
// NON-BLOCKING SAFETY STATE MACHINE
// =========================================================

void updateSafetySystem()
{
  unsigned long currentTime = millis();

  FaultType detectedFault =
    detectFault();


  // =======================================================
  // NORMAL STATE
  // =======================================================

  if (safetyState == NORMAL)
  {
    if (detectedFault != NO_FAULT)
    {
      candidateFault =
        detectedFault;

      debounceStartTime =
        currentTime;

      safetyState =
        DEBOUNCING;


      Serial.println();

      Serial.print("[");
      Serial.print(currentTime);

      Serial.println(
        " ms] SAFETY: Potential fault detected"
      );


      Serial.print(
        "Fault Candidate: "
      );

      Serial.println(
        getFaultName(candidateFault)
      );
    }
  }


  // =======================================================
  // DEBOUNCING STATE
  // =======================================================

  else if (safetyState == DEBOUNCING)
  {
    // Fault disappeared
    if (detectedFault == NO_FAULT)
    {
      Serial.println(
        "[SAFETY] Fault disappeared - debounce cancelled"
      );

      candidateFault =
        NO_FAULT;

      safetyState =
        NORMAL;
    }


    // Different fault detected
    else if (detectedFault != candidateFault)
    {
      candidateFault =
        detectedFault;

      debounceStartTime =
        currentTime;


      Serial.println(
        "[SAFETY] Fault type changed - debounce restarted"
      );
    }


    // Fault confirmed
    else if (
      currentTime - debounceStartTime
      >= DEBOUNCE_TIME
    )
    {
      activeFault =
        candidateFault;

      safetyState =
        FAULT_ACTIVE;


      Serial.println();

      Serial.print("[");
      Serial.print(currentTime);

      Serial.print(
        " ms] FAULT CONFIRMED: "
      );

      Serial.println(
        getFaultName(activeFault)
      );


      setRelay(false);
    }
  }


  // =======================================================
  // FAULT ACTIVE
  // =======================================================

  else if (safetyState == FAULT_ACTIVE)
  {
    if (safeForRecovery())
    {
      recoveryStartTime =
        currentTime;

      safetyState =
        RECOVERY_VERIFYING;


      Serial.println();

      Serial.print("[");
      Serial.print(currentTime);

      Serial.println(
        " ms] RECOVERY: Safe condition detected - verification started"
      );
    }
  }


  // =======================================================
  // RECOVERY VERIFYING
  // =======================================================

  else if (
    safetyState == RECOVERY_VERIFYING
  )
  {
    // Fault returned
    if (!safeForRecovery())
    {
      safetyState =
        FAULT_ACTIVE;


      Serial.println();

      Serial.println(
        "[RECOVERY FAILED] Unsafe condition returned"
      );
    }


    // Recovery successful
    else if (
      currentTime - recoveryStartTime
      >= RECOVERY_TIME
    )
    {
      safetyState =
        NORMAL;

      activeFault =
        NO_FAULT;

      candidateFault =
        NO_FAULT;


      Serial.println();

      Serial.println(
        "[RECOVERY SUCCESS] System returned to NORMAL"
      );


      setRelay(true);
    }
  }


  // =======================================================
  // UPDATE PREVIOUS VOLTAGES
  // =======================================================

  for (int i = 0;
       i < CELL_COUNT;
       i++)
  {
    previousVoltage[i] =
      cellVoltage[i];
  }
}


// =========================================================
// RELAY CONTROL
// =========================================================

void setRelay(bool state)
{
  // Anti-chatter
  if (relayState == state)
  {
    return;
  }


  relayState = state;


  Serial.print("[");
  Serial.print(millis());

  Serial.print(
    " ms] RELAY TRANSITION: "
  );


  if (relayState)
  {
    Serial.println(
      "ON - NORMAL OPERATION"
    );
  }
  else
  {
    Serial.println(
      "OFF - PROTECTION ACTIVE"
    );
  }
}


// =========================================================
// FAULT NAME
// =========================================================

const char*
getFaultName(FaultType fault)
{
  switch (fault)
  {
    case NO_FAULT:
      return "NORMAL";

    case UNDER_VOLTAGE:
      return "UNDER VOLTAGE";

    case OVER_VOLTAGE:
      return "OVER VOLTAGE";

    case FROZEN_SENSOR:
      return "FROZEN SENSOR";

    case UNREALISTIC_JUMP:
      return "UNREALISTIC JUMP";

    case OUT_OF_RANGE:
      return "OUT OF RANGE";

    default:
      return "UNKNOWN";
  }
}


// =========================================================
// STATE NAME
// =========================================================

const char*
getStateName(SafetyState state)
{
  switch (state)
  {
    case NORMAL:
      return "NORMAL";

    case DEBOUNCING:
      return "DEBOUNCING";

    case FAULT_ACTIVE:
      return "FAULT ACTIVE";

    case RECOVERY_VERIFYING:
      return "RECOVERY VERIFYING";

    default:
      return "UNKNOWN";
  }
}


// =========================================================
// SYSTEM STATUS
// =========================================================

void printSystemStatus()
{
  float minimumVoltage =
    cellVoltage[0];

  float maximumVoltage =
    cellVoltage[0];


  int weakestCell = 0;

  int strongestCell = 0;


  float totalVoltage = 0;


  // =======================================================
  // CALCULATIONS
  // =======================================================

  for (
    int i = 0;
    i < CELL_COUNT;
    i++
  )
  {
    totalVoltage +=
      cellVoltage[i];


    if (
      cellVoltage[i]
      < minimumVoltage
    )
    {
      minimumVoltage =
        cellVoltage[i];

      weakestCell = i;
    }


    if (
      cellVoltage[i]
      > maximumVoltage
    )
    {
      maximumVoltage =
        cellVoltage[i];

      strongestCell = i;
    }
  }


  float imbalance =
    maximumVoltage -
    minimumVoltage;


  float averageVoltage =
    totalVoltage /
    CELL_COUNT;


  // =======================================================
  // APPROXIMATE SOC
  // =======================================================

  float soc =
    (
      (averageVoltage - 3.0)
      /
      (4.2 - 3.0)
    )
    * 100;


  if (soc > 100)
  {
    soc = 100;
  }


  if (soc < 0)
  {
    soc = 0;
  }


  // =======================================================
  // PRINT STATUS
  // =======================================================

  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "TASK 2: NON-BLOCKING SAFETY SYSTEM"
  );

  Serial.println(
    "=========================================="
  );


  // Cell voltages
  for (
    int i = 0;
    i < CELL_COUNT;
    i++
  )
  {
    Serial.print(
      "Cell "
    );

    Serial.print(
      i + 1
    );

    Serial.print(
      ": "
    );

    Serial.print(
      cellVoltage[i],
      3
    );

    Serial.println(
      " V"
    );
  }


  Serial.println(
    "------------------------------------------"
  );


  Serial.print(
    "Weakest Cell: "
  );

  Serial.println(
    weakestCell + 1
  );


  Serial.print(
    "Strongest Cell: "
  );

  Serial.println(
    strongestCell + 1
  );


  Serial.print(
    "Voltage Imbalance: "
  );

  Serial.print(
    imbalance,
    3
  );

  Serial.println(
    " V"
  );


  Serial.print(
    "Average SoC: "
  );

  Serial.print(
    soc,
    1
  );

  Serial.println(
    "%"
  );


  Serial.print(
    "Relay: "
  );

  Serial.println(
    relayState
    ? "ON"
    : "OFF"
  );


  Serial.print(
    "Active Fault: "
  );

  Serial.println(
    getFaultName(activeFault)
  );


  Serial.print(
    "Safety State: "
  );

  Serial.println(
    getStateName(safetyState)
  );


  Serial.println(
    "=========================================="
  );
}
