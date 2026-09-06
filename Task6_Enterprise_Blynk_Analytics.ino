#define BLYNK_TEMPLATE_ID "TMPL3RvbjIhYD"
#define BLYNK_TEMPLATE_NAME "EV BMS"
#define BLYNK_AUTH_TOKEN "DoMf6AhtkzEzt_urdJOPIJMJqNCK43ZF"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <math.h>

// =========================================================
// TASK 6
// ENTERPRISE BLYNK ANALYTICS AND DECISION DASHBOARD
// =========================================================

#define CELL_COUNT 4

// =========================================================
// WIFI
// =========================================================

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const unsigned long WIFI_CHECK_INTERVAL = 250;
const unsigned long WIFI_RETRY_INTERVAL = 5000;
const unsigned long BLYNK_RETRY_INTERVAL = 5000;

unsigned long lastWiFiCheck = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastBlynkAttempt = 0;

// =========================================================
// ANALYTICS
// =========================================================

const unsigned long ANALYTICS_INTERVAL = 1000;
unsigned long lastAnalyticsUpdate = 0;

const int MAX_FAULT_HISTORY = 10;

int faultCount = 0;
int stateTransitionCount = 0;

unsigned long systemStartTime = 0;
unsigned long uptimeSeconds = 0;

// =========================================================
// BATTERY DATA
// =========================================================

float cellVoltage[CELL_COUNT] =
{
  3.82,
  3.76,
  3.88,
  3.65
};

float soc = 82.0;

bool relayState = true;

String systemState = "NORMAL";
String faultState = "NO_FAULT";
String faultSource = "NONE";

// =========================================================
// RISK ANALYSIS
// =========================================================

float imbalance = 0.0;
float imbalanceTrend = 0.0;
float riskScore = 0.0;

String riskLevel = "LOW";
String recommendation = "Battery operating normally";

int previousWeakest = -1;
int previousStrongest = -1;

// =========================================================
// FAULT HISTORY
// =========================================================

struct FaultRecord
{
  unsigned long timestamp;
  int faultId;
  String source;
  String fault;
  String state;
};

FaultRecord faultHistory[MAX_FAULT_HISTORY];

int faultHistoryIndex = 0;

// =========================================================
// STATE TRANSITION HISTORY
// =========================================================

struct StateTransition
{
  unsigned long timestamp;
  String previousState;
  String newState;
  String fault;
};

StateTransition transitionHistory[MAX_FAULT_HISTORY];

int transitionHistoryIndex = 0;

// =========================================================
// DEMO CONTROL
// =========================================================

bool demoNetworkOutage = false;
bool demoFault = false;

// =========================================================
// WIFI STATE MACHINE
// =========================================================

enum WiFiState
{
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};

WiFiState wifiState = WIFI_DISCONNECTED;

// =========================================================
// FUNCTION DECLARATIONS
// =========================================================

void startWiFiConnection();
void updateWiFiStateMachine();
void updateBlynkConnection();

void updateBatterySimulation();

void injectFault();
void clearFault();

int getWeakestCell();
int getStrongestCell();

void calculateAnalytics();
void calculateRiskScore();
void generateRecommendation();

void updateStateMachine();
void logStateTransition(String oldState,
                        String newState,
                        String fault);

void logFault(String source,
              String fault);

void updateDashboard();

void updateFaultHistoryDisplay();
void updateTransitionDisplay();

void processSerialCommands();

// =========================================================
// SETUP
// =========================================================

void setup()
{
  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 EV BMS");
  Serial.println("TASK 6");
  Serial.println("ENTERPRISE BLYNK ANALYTICS");
  Serial.println("========================================");

  Serial.println();

  systemStartTime = millis();

  Blynk.config(BLYNK_AUTH_TOKEN);

  startWiFiConnection();

  Serial.println("[SYSTEM] Analytics engine ready");

  Serial.println();

  Serial.println("COMMANDS:");
  Serial.println("N = Network outage");
  Serial.println("R = Restore network");
  Serial.println("F = Inject battery fault");
  Serial.println("C = Clear fault");

  Serial.println();
}

// =========================================================
// MAIN LOOP
// =========================================================

void loop()
{
  unsigned long now = millis();

  // -------------------------------------------------------
  // WIFI
  // -------------------------------------------------------

  if (now - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
  {
    lastWiFiCheck = now;

    updateWiFiStateMachine();
  }

  // -------------------------------------------------------
  // BLYNK
  // -------------------------------------------------------

  if (wifiState == WIFI_CONNECTED)
  {
    Blynk.run();

    updateBlynkConnection();
  }

  // -------------------------------------------------------
  // BATTERY
  // -------------------------------------------------------

  updateBatterySimulation();

  // -------------------------------------------------------
  // SERIAL
  // -------------------------------------------------------

  processSerialCommands();

  // -------------------------------------------------------
  // ANALYTICS
  // -------------------------------------------------------

  if (now - lastAnalyticsUpdate >= ANALYTICS_INTERVAL)
  {
    lastAnalyticsUpdate = now;

    calculateAnalytics();

    calculateRiskScore();

    generateRecommendation();

    updateStateMachine();

    updateDashboard();
  }
}

// =========================================================
// WIFI START
// =========================================================

void startWiFiConnection()
{
  if (wifiState == WIFI_CONNECTING)
    return;

  Serial.println("[WIFI] Starting connection...");

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  wifiState = WIFI_CONNECTING;

  lastWiFiAttempt = millis();
}

// =========================================================
// WIFI STATE MACHINE
// =========================================================

void updateWiFiStateMachine()
{
  unsigned long now = millis();

  // -------------------------------------------------------
  // SIMULATED OUTAGE
  // -------------------------------------------------------

  if (demoNetworkOutage)
  {
    if (wifiState != WIFI_DISCONNECTED)
    {
      Serial.println();
      Serial.println(">>> NETWORK OUTAGE SIMULATED <<<");

      Blynk.disconnect();

      WiFi.disconnect(false);

      wifiState = WIFI_DISCONNECTED;
    }

    return;
  }

  // -------------------------------------------------------
  // DISCONNECTED
  // -------------------------------------------------------

  if (wifiState == WIFI_DISCONNECTED)
  {
    if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL)
    {
      startWiFiConnection();
    }

    return;
  }

  // -------------------------------------------------------
  // CONNECTING
  // -------------------------------------------------------

  if (wifiState == WIFI_CONNECTING)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.println(">>> WIFI CONNECTED <<<");

      Serial.print("[WIFI] RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");

      wifiState = WIFI_CONNECTED;

      lastBlynkAttempt = 0;
    }
    else
    {
      if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL)
      {
        Serial.println("[WIFI] Retry...");

        WiFi.disconnect();

        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        lastWiFiAttempt = now;
      }
    }

    return;
  }

  // -------------------------------------------------------
  // CONNECTED
  // -------------------------------------------------------

  if (wifiState == WIFI_CONNECTED)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println();
      Serial.println(">>> WIFI CONNECTION LOST <<<");

      Blynk.disconnect();

      wifiState = WIFI_DISCONNECTED;
    }
  }
}

// =========================================================
// BLYNK CONNECTION
// =========================================================

void updateBlynkConnection()
{
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED)
    return;

  if (!Blynk.connected())
  {
    if (now - lastBlynkAttempt >= BLYNK_RETRY_INTERVAL)
    {
      lastBlynkAttempt = now;

      Serial.println("[BLYNK] Connection attempt...");

      Blynk.connect(0);
    }
  }
}

// =========================================================
// BLYNK CONNECTED
// =========================================================

BLYNK_CONNECTED()
{
  Serial.println();
  Serial.println(">>> BLYNK CONNECTED <<<");

  updateDashboard();
}

// =========================================================
// BATTERY SIMULATION
// =========================================================

void updateBatterySimulation()
{
  static unsigned long lastUpdate = 0;

  unsigned long now = millis();

  if (now - lastUpdate < 1000)
    return;

  lastUpdate = now;

  // -------------------------------------------------------
  // FAULT MODE
  // -------------------------------------------------------

  if (demoFault)
  {
    cellVoltage[3] = 3.05;

    faultState = "UNDER_VOLTAGE";
    faultSource = "BATTERY";

    return;
  }

  // -------------------------------------------------------
  // NORMAL SIMULATION
  // -------------------------------------------------------

  cellVoltage[0] -= 0.001;
  cellVoltage[1] -= 0.001;
  cellVoltage[2] += 0.001;
  cellVoltage[3] += 0.001;

  // Simulated SoC variation
  soc -= 0.01;

  if (soc < 70)
    soc = 82;

  // Keep voltage safe
  for (int i = 0; i < CELL_COUNT; i++)
  {
    if (cellVoltage[i] < 3.50)
      cellVoltage[i] = 3.82;

    if (cellVoltage[i] > 4.05)
      cellVoltage[i] = 3.82;
  }

  faultState = "NO_FAULT";
  faultSource = "NONE";
}

// =========================================================
// FAULT INJECTION
// =========================================================

void injectFault()
{
  if (!demoFault)
  {
    demoFault = true;

    faultCount++;

    logFault(
      "BATTERY",
      "UNDER_VOLTAGE"
    );

    Serial.println();
    Serial.println("========================================");
    Serial.println(">>> FAULT INJECTION <<<");
    Serial.println("Cell 4 UNDER_VOLTAGE");
    Serial.println("========================================");
  }
}

// =========================================================
// CLEAR FAULT
// =========================================================

void clearFault()
{
  demoFault = false;

  cellVoltage[3] = 3.65;

  faultState = "NO_FAULT";
  faultSource = "NONE";

  Serial.println();
  Serial.println("========================================");
  Serial.println(">>> FAULT CLEARED <<<");
  Serial.println("========================================");
}

// =========================================================
// WEAKEST CELL
// =========================================================

int getWeakestCell()
{
  int weakest = 0;

  for (int i = 1; i < CELL_COUNT; i++)
  {
    if (cellVoltage[i] < cellVoltage[weakest])
    {
      weakest = i;
    }
  }

  return weakest + 1;
}

// =========================================================
// STRONGEST CELL
// =========================================================

int getStrongestCell()
{
  int strongest = 0;

  for (int i = 1; i < CELL_COUNT; i++)
  {
    if (cellVoltage[i] > cellVoltage[strongest])
    {
      strongest = i;
    }
  }

  return strongest + 1;
}

// =========================================================
// ANALYTICS
// =========================================================

void calculateAnalytics()
{
  float minimum = cellVoltage[0];
  float maximum = cellVoltage[0];

  for (int i = 1; i < CELL_COUNT; i++)
  {
    if (cellVoltage[i] < minimum)
      minimum = cellVoltage[i];

    if (cellVoltage[i] > maximum)
      maximum = cellVoltage[i];
  }

  imbalance = maximum - minimum;

  static float previousImbalance = 0;

  imbalanceTrend =
    imbalance - previousImbalance;

  previousImbalance = imbalance;

  uptimeSeconds =
    (millis() - systemStartTime) / 1000;

  Serial.print("[ANALYTICS] Imbalance: ");
  Serial.print(imbalance, 3);

  Serial.print(" V | SoC: ");
  Serial.print(soc, 1);

  Serial.print("% | Faults: ");
  Serial.println(faultCount);
}

// =========================================================
// RISK SCORE
// =========================================================

void calculateRiskScore()
{
  // -------------------------------------------------------
  // Imbalance contribution
  // -------------------------------------------------------

  float imbalanceRisk =
    (imbalance / 0.30) * 40.0;

  if (imbalanceRisk > 40)
    imbalanceRisk = 40;

  // -------------------------------------------------------
  // Fault frequency contribution
  // -------------------------------------------------------

  float faultRisk =
    faultCount * 10.0;

  if (faultRisk > 30)
    faultRisk = 30;

  // -------------------------------------------------------
  // SoC contribution
  // -------------------------------------------------------

  float socRisk = 0;

  if (soc < 30)
    socRisk = 30;
  else if (soc < 50)
    socRisk = 20;
  else if (soc < 70)
    socRisk = 10;

  // -------------------------------------------------------
  // Composite score
  // -------------------------------------------------------

  riskScore =
    imbalanceRisk +
    faultRisk +
    socRisk;

  if (riskScore > 100)
    riskScore = 100;

  // -------------------------------------------------------
  // Risk level
  // -------------------------------------------------------

  if (riskScore < 25)
  {
    riskLevel = "LOW";
  }
  else if (riskScore < 50)
  {
    riskLevel = "MEDIUM";
  }
  else if (riskScore < 75)
  {
    riskLevel = "HIGH";
  }
  else
  {
    riskLevel = "CRITICAL";
  }
}

// =========================================================
// OPERATOR RECOMMENDATION
// =========================================================

void generateRecommendation()
{
  if (systemState == "SHUTDOWN")
  {
    recommendation =
      "Immediate shutdown inspection required";
  }
  else if (systemState == "FAILSAFE")
  {
    recommendation =
      "Verify battery safety before recovery";
  }
  else if (faultState != "NO_FAULT")
  {
    recommendation =
      "Inspect affected battery cell";
  }
  else if (imbalance >= 0.20)
  {
    recommendation =
      "Check cell balancing and battery health";
  }
  else if (riskLevel == "MEDIUM")
  {
    recommendation =
      "Monitor battery condition closely";
  }
  else
  {
    recommendation =
      "Battery operating normally";
  }
}

// =========================================================
// STATE MACHINE
// =========================================================

void updateStateMachine()
{
  String oldState = systemState;

  // -------------------------------------------------------
  // CRITICAL FAULT
  // -------------------------------------------------------

  if (faultState != "NO_FAULT")
  {
    if (riskScore >= 75)
      systemState = "SHUTDOWN";
    else if (riskScore >= 50)
      systemState = "FAILSAFE";
    else
      systemState = "DEGRADED";
  }

  // -------------------------------------------------------
  // NO FAULT
  // -------------------------------------------------------

  else
  {
    if (riskScore >= 75)
      systemState = "FAILSAFE";
    else if (riskScore >= 50)
      systemState = "DEGRADED";
    else
      systemState = "NORMAL";
  }

  // -------------------------------------------------------
  // LOG TRANSITION
  // -------------------------------------------------------

  if (oldState != systemState)
  {
    logStateTransition(
      oldState,
      systemState,
      faultState
    );
  }
}

// =========================================================
// STATE TRANSITION LOG
// =========================================================

void logStateTransition(
  String oldState,
  String newState,
  String fault
)
{
  transitionHistory[
    transitionHistoryIndex
  ].timestamp = millis();

  transitionHistory[
    transitionHistoryIndex
  ].previousState = oldState;

  transitionHistory[
    transitionHistoryIndex
  ].newState = newState;

  transitionHistory[
    transitionHistoryIndex
  ].fault = fault;

  transitionHistoryIndex++;

  if (transitionHistoryIndex >= MAX_FAULT_HISTORY)
    transitionHistoryIndex = 0;

  stateTransitionCount++;

  Serial.println();
  Serial.println("========== STATE TRANSITION ==========");

  Serial.print("Timestamp: ");
  Serial.println(millis());

  Serial.print("Previous: ");
  Serial.println(oldState);

  Serial.print("New: ");
  Serial.println(newState);

  Serial.print("Fault: ");
  Serial.println(fault);

  Serial.println("======================================");
}

// =========================================================
// FAULT HISTORY LOG
// =========================================================

void logFault(
  String source,
  String fault
)
{
  faultHistory[
    faultHistoryIndex
  ].timestamp = millis();

  faultHistory[
    faultHistoryIndex
  ].faultId = faultCount;

  faultHistory[
    faultHistoryIndex
  ].source = source;

  faultHistory[
    faultHistoryIndex
  ].fault = fault;

  faultHistory[
    faultHistoryIndex
  ].state = systemState;

  faultHistoryIndex++;

  if (faultHistoryIndex >= MAX_FAULT_HISTORY)
    faultHistoryIndex = 0;

  Serial.println();
  Serial.println("========== FAULT HISTORY ==========");

  Serial.print("Fault ID: ");
  Serial.println(faultCount);

  Serial.print("Source: ");
  Serial.println(source);

  Serial.print("Fault: ");
  Serial.println(fault);

  Serial.println("===================================");
}

// =========================================================
// DASHBOARD
// =========================================================

void updateDashboard()
{
  if (!Blynk.connected())
    return;

  // -------------------------------------------------------
  // Existing Task 5 data
  // -------------------------------------------------------

  Blynk.virtualWrite(V0, cellVoltage[0]);
  Blynk.virtualWrite(V1, cellVoltage[1]);
  Blynk.virtualWrite(V2, cellVoltage[2]);
  Blynk.virtualWrite(V3, cellVoltage[3]);

  Blynk.virtualWrite(
    V4,
    getWeakestCell()
  );

  Blynk.virtualWrite(
    V5,
    getStrongestCell()
  );

  Blynk.virtualWrite(
    V8,
    relayState ? "ON" : "OFF"
  );

  Blynk.virtualWrite(
    V9,
    faultState
  );

  Blynk.virtualWrite(
    V10,
    WiFi.status() == WL_CONNECTED
      ? "CONNECTED"
      : "OFFLINE"
  );

  Blynk.virtualWrite(
    V11,
    WiFi.status() == WL_CONNECTED
      ? WiFi.RSSI()
      : -100
  );

  Blynk.virtualWrite(
    V12,
    0
  );

  Blynk.virtualWrite(
    V13,
    "LIVE"
  );

  Blynk.virtualWrite(
    V14,
    faultCount
  );

  Blynk.virtualWrite(
    V15,
    systemState
  );

  // =======================================================
  // TASK 6 ANALYTICS
  // =======================================================

  // V16 - SoC
  Blynk.virtualWrite(
    V16,
    soc
  );

  // V17 - Cell Imbalance
  Blynk.virtualWrite(
    V17,
    imbalance
  );

  // V18 - Risk Score
  Blynk.virtualWrite(
    V18,
    riskScore
  );

  // V19 - Risk Level
  Blynk.virtualWrite(
    V19,
    riskLevel
  );

  // V20 - Battery Health
  float batteryHealth =
    100.0 - riskScore;

  if (batteryHealth < 0)
    batteryHealth = 0;

  Blynk.virtualWrite(
    V20,
    batteryHealth
  );

  // V21 - Uptime
  Blynk.virtualWrite(
    V21,
    uptimeSeconds
  );

  // V22 - Fault Count
  Blynk.virtualWrite(
    V22,
    faultCount
  );

  // V23 - State Transition Count
  Blynk.virtualWrite(
    V23,
    stateTransitionCount
  );

  // V24 - Recommendation
  Blynk.virtualWrite(
    V24,
    recommendation
  );

  // V25 - Imbalance Trend
  Blynk.virtualWrite(
    V25,
    imbalanceTrend
  );

  // V26 - Fault Source
  Blynk.virtualWrite(
    V26,
    faultSource
  );

  // V27 - Current State
  Blynk.virtualWrite(
    V27,
    systemState
  );

  // -------------------------------------------------------
  // Serial Executive Summary
  // -------------------------------------------------------

  Serial.println();
  Serial.println("========== EXECUTIVE SUMMARY ==========");

  Serial.print("Battery Health: ");
  Serial.print(batteryHealth, 1);
  Serial.println("%");

  Serial.print("SoC: ");
  Serial.print(soc, 1);
  Serial.println("%");

  Serial.print("Risk Score: ");
  Serial.print(riskScore, 1);
  Serial.println("/100");

  Serial.print("Risk Level: ");
  Serial.println(riskLevel);

  Serial.print("Fault Count: ");
  Serial.println(faultCount);

  Serial.print("Current State: ");
  Serial.println(systemState);

  Serial.print("Uptime: ");
  Serial.print(uptimeSeconds);
  Serial.println(" sec");

  Serial.print("Recommendation: ");
  Serial.println(recommendation);

  Serial.println("========================================");
}

// =========================================================
// SERIAL COMMANDS
// =========================================================

void processSerialCommands()
{
  if (!Serial.available())
    return;

  char command = Serial.read();

  // -------------------------------------------------------
  // NETWORK OUTAGE
  // -------------------------------------------------------

  if (command == 'N' || command == 'n')
  {
    demoNetworkOutage = true;

    Serial.println();
    Serial.println(">>> OFFLINE MODE ENABLED <<<");
  }

  // -------------------------------------------------------
  // NETWORK RESTORE
  // -------------------------------------------------------

  if (command == 'R' || command == 'r')
  {
    demoNetworkOutage = false;

    lastWiFiAttempt =
      millis() - WIFI_RETRY_INTERVAL;

    Serial.println();
    Serial.println(">>> NETWORK RESTORE REQUESTED <<<");
  }

  // -------------------------------------------------------
  // FAULT
  // -------------------------------------------------------

  if (command == 'F' || command == 'f')
  {
    injectFault();
  }

  // -------------------------------------------------------
  // CLEAR
  // -------------------------------------------------------

  if (command == 'C' || command == 'c')
  {
    clearFault();
  }
}
