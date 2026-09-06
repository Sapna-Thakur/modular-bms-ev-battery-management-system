#define BLYNK_TEMPLATE_ID "TMPL3RvbjIhYD"
#define BLYNK_TEMPLATE_NAME "EV BMS"
#define BLYNK_AUTH_TOKEN "DoMf6AhtkzEzt_urdJOPIJMJqNCK43ZF"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ============================================================
// FINAL EV BMS - TASK 1 TO TASK 6
// ============================================================

// ---------------- CELL CONFIGURATION ----------------
#define CELL_COUNT 4

const int CELL_PINS[CELL_COUNT] = {32, 33, 34, 35};

// ---------------- HARDWARE PINS ----------------
#define GREEN_LED   25
#define RELAY_PIN   26
#define YELLOW_LED  27
#define RED_LED     14
#define BUZZER_PIN  13

#define ONE_WIRE_PIN 4

#define LCD_SDA 21
#define LCD_SCL 22

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- TEMPERATURE ----------------
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature tempSensor(&oneWire);

float temperatureC = 25.0;

// ---------------- WIFI ----------------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

enum WiFiState {
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};

WiFiState wifiState = WIFI_DISCONNECTED;

unsigned long lastWiFiCheck = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastBlynkAttempt = 0;

const unsigned long WIFI_CHECK_INTERVAL = 250;
const unsigned long WIFI_RETRY_INTERVAL = 5000;
const unsigned long BLYNK_RETRY_INTERVAL = 5000;

// ============================================================
// TASK 1 - BATTERY DATA
// ============================================================

float cellVoltage[CELL_COUNT] = {
  3.82,
  3.76,
  3.88,
  3.65
};

float soc = 82.0;

float minimumVoltage = 0;
float maximumVoltage = 0;

float imbalance = 0;
float previousImbalance = 0;
float imbalanceTrend = 0;

int weakestCell = 1;
int strongestCell = 1;

// Adaptive threshold
float adaptiveThreshold = 0.10;

// ============================================================
// TASK 2 - PROTECTION
// ============================================================

bool relayState = true;

bool faultActive = false;

const float UNDER_VOLTAGE = 3.20;
const float OVER_VOLTAGE = 4.20;

const float UNDER_VOLTAGE_RECOVERY = 3.35;
const float OVER_VOLTAGE_RECOVERY = 4.10;

const unsigned long FAULT_DEBOUNCE_TIME = 1000;
const unsigned long RECOVERY_TIME = 3000;

unsigned long faultStartTime = 0;
unsigned long recoveryStartTime = 0;

bool recoveryRunning = false;

// ============================================================
// TASK 4 - STATE MACHINE
// ============================================================

enum BMSState {
  NORMAL,
  DEGRADED,
  FAILSAFE,
  SHUTDOWN
};

BMSState systemState = NORMAL;

String stateName(BMSState state) {

  switch (state) {

    case NORMAL:
      return "NORMAL";

    case DEGRADED:
      return "DEGRADED";

    case FAILSAFE:
      return "FAILSAFE";

    case SHUTDOWN:
      return "SHUTDOWN";
  }

  return "UNKNOWN";
}

String faultSource = "NONE";
String faultState = "NO_FAULT";

int faultCount = 0;
int stateTransitionCount = 0;
int faultID = 0;

// ============================================================
// TASK 6 - ANALYTICS
// ============================================================

float riskScore = 0;
String riskLevel = "LOW";

float batteryHealth = 100;

String recommendation = "Battery operating normally";

unsigned long systemStartTime = 0;
unsigned long uptimeSeconds = 0;

// ============================================================
// TASK 5 - TELEMETRY
// ============================================================

struct TelemetryEvent {

  unsigned long timestamp;

  float imbalance;
  float soc;

  int weakest;
  int strongest;

  bool relay;

  String fault;
  String state;

  int rssi;
};

const int QUEUE_SIZE = 12;

TelemetryEvent telemetryQueue[QUEUE_SIZE];

int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// ============================================================
// DEMO CONTROLS
// ============================================================

bool demoFault = false;
bool demoNetworkOutage = false;

// ============================================================
// TASK 3 - LCD
// ============================================================

unsigned long lastLCDUpdate = 0;
unsigned long lastPageChange = 0;

const unsigned long LCD_UPDATE_INTERVAL = 500;
const unsigned long LCD_PAGE_INTERVAL = 3000;

int currentPage = 0;

String lcdLine1 = "";
String lcdLine2 = "";

// ============================================================
// TIMERS
// ============================================================

unsigned long lastBatteryUpdate = 0;
unsigned long lastAnalyticsUpdate = 0;
unsigned long lastTelemetryUpdate = 0;

const unsigned long BATTERY_INTERVAL = 1000;
const unsigned long ANALYTICS_INTERVAL = 1000;
const unsigned long TELEMETRY_INTERVAL = 1000;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

void startWiFiConnection();
void updateWiFiStateMachine();
void updateBlynkConnection();

void readBattery();
void analyzeBattery();

void calculateRisk();
void updateStateMachine();

void protectionSystem();
void setRelay(bool state);
void updateIndicators();

void updateLCD();
void writeLCD(String line1, String line2);

void addTelemetryEvent();
void transmitQueuedEvents();

void updateBlynkDashboard();

void injectFault();
void clearFault();

void processSerialCommands();

void printExecutiveSummary();

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(LCD_SDA, LCD_SCL);

  lcd.init();
  lcd.backlight();

  tempSensor.begin();

  systemStartTime = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EV BMS");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  Blynk.config(BLYNK_AUTH_TOKEN);

  startWiFiConnection();

  Serial.println();
  Serial.println("========================================");
  Serial.println("        ESP32 EV BMS");
  Serial.println("        FINAL SYSTEM");
  Serial.println("        TASK 1 - TASK 6");
  Serial.println("========================================");

  Serial.println();
  Serial.println("COMMANDS:");
  Serial.println("F = Inject battery fault");
  Serial.println("C = Clear fault");
  Serial.println("N = Network outage");
  Serial.println("R = Restore network");
  Serial.println();

  setRelay(true);
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  unsigned long now = millis();

  // ---------------- WIFI ----------------

  if (now - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {

    lastWiFiCheck = now;

    updateWiFiStateMachine();
  }

  // ---------------- BLYNK ----------------

  if (wifiState == WIFI_CONNECTED) {

    Blynk.run();

    updateBlynkConnection();
  }

  // ---------------- BATTERY ----------------

  if (now - lastBatteryUpdate >= BATTERY_INTERVAL) {

    lastBatteryUpdate = now;

    readBattery();
  }

  // ---------------- ANALYTICS ----------------

  if (now - lastAnalyticsUpdate >= ANALYTICS_INTERVAL) {

    lastAnalyticsUpdate = now;

    analyzeBattery();

    calculateRisk();

    updateStateMachine();

    protectionSystem();

    updateIndicators();

    printExecutiveSummary();
  }

  // ---------------- LCD ----------------

  updateLCD();

  // ---------------- TELEMETRY ----------------

  if (now - lastTelemetryUpdate >= TELEMETRY_INTERVAL) {

    lastTelemetryUpdate = now;

    addTelemetryEvent();

    transmitQueuedEvents();

    updateBlynkDashboard();
  }

  // ---------------- SERIAL ----------------

  processSerialCommands();
}

// ============================================================
// WIFI
// ============================================================

void startWiFiConnection() {

  if (wifiState == WIFI_CONNECTING)
    return;

  Serial.println("[WIFI] Starting connection...");

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  wifiState = WIFI_CONNECTING;

  lastWiFiAttempt = millis();
}

// ============================================================

void updateWiFiStateMachine() {

  unsigned long now = millis();

  // Simulated outage

  if (demoNetworkOutage) {

    if (wifiState != WIFI_DISCONNECTED) {

      Serial.println();
      Serial.println(">>> NETWORK OUTAGE SIMULATED <<<");

      Blynk.disconnect();

      WiFi.disconnect(false);

      wifiState = WIFI_DISCONNECTED;
    }

    return;
  }

  // DISCONNECTED

  if (wifiState == WIFI_DISCONNECTED) {

    if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {

      startWiFiConnection();
    }

    return;
  }

  // CONNECTING

  if (wifiState == WIFI_CONNECTING) {

    if (WiFi.status() == WL_CONNECTED) {

      Serial.println();
      Serial.println(">>> WIFI CONNECTED <<<");

      Serial.print("[WIFI] RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");

      wifiState = WIFI_CONNECTED;

      lastBlynkAttempt = 0;
    }

    else if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {

      Serial.println("[WIFI] Retry...");

      WiFi.disconnect();

      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      lastWiFiAttempt = now;
    }

    return;
  }

  // CONNECTED

  if (wifiState == WIFI_CONNECTED) {

    if (WiFi.status() != WL_CONNECTED) {

      Serial.println();
      Serial.println(">>> WIFI CONNECTION LOST <<<");

      Blynk.disconnect();

      wifiState = WIFI_DISCONNECTED;
    }
  }
}

// ============================================================
// BLYNK
// ============================================================

void updateBlynkConnection() {

  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED)
    return;

  if (!Blynk.connected()) {

    if (now - lastBlynkAttempt >= BLYNK_RETRY_INTERVAL) {

      lastBlynkAttempt = now;

      Serial.println("[BLYNK] Connection attempt...");

      Blynk.connect(0);
    }
  }
}

// ============================================================

BLYNK_CONNECTED() {

  Serial.println();
  Serial.println(">>> BLYNK CONNECTED <<<");

  updateBlynkDashboard();
}

// ============================================================
// BATTERY READING
// ============================================================

void readBattery() {

  if (demoFault) {

    cellVoltage[3] = 3.05;

  } else {

    cellVoltage[0] -= 0.001;
    cellVoltage[1] -= 0.001;
    cellVoltage[2] += 0.001;
    cellVoltage[3] += 0.001;

    if (cellVoltage[0] < 3.50)
      cellVoltage[0] = 3.82;

    if (cellVoltage[1] < 3.50)
      cellVoltage[1] = 3.76;

    if (cellVoltage[2] > 4.05)
      cellVoltage[2] = 3.88;

    if (cellVoltage[3] > 3.90)
      cellVoltage[3] = 3.65;
  }

  soc -= 0.01;

  if (soc < 70)
    soc = 82;

  // Temperature

  tempSensor.requestTemperatures();

  float t = tempSensor.getTempCByIndex(0);

  if (t > -50 && t < 125)
    temperatureC = t;
}

// ============================================================
// TASK 1 - BATTERY ANALYSIS
// ============================================================

void analyzeBattery() {

  minimumVoltage = cellVoltage[0];
  maximumVoltage = cellVoltage[0];

  weakestCell = 1;
  strongestCell = 1;

  for (int i = 1; i < CELL_COUNT; i++) {

    if (cellVoltage[i] < minimumVoltage) {

      minimumVoltage = cellVoltage[i];

      weakestCell = i + 1;
    }

    if (cellVoltage[i] > maximumVoltage) {

      maximumVoltage = cellVoltage[i];

      strongestCell = i + 1;
    }
  }

  previousImbalance = imbalance;

  imbalance =
    maximumVoltage - minimumVoltage;

  imbalanceTrend =
    imbalance - previousImbalance;

  // Adaptive threshold based on SoC

  if (soc >= 80) {

    adaptiveThreshold = 0.08;

  } else if (soc >= 50) {

    adaptiveThreshold = 0.10;

  } else {

    adaptiveThreshold = 0.12;
  }

  uptimeSeconds =
    (millis() - systemStartTime) / 1000;

  Serial.print("[BMS] Cells: ");

  for (int i = 0; i < CELL_COUNT; i++) {

    Serial.print(cellVoltage[i], 2);

    if (i < CELL_COUNT - 1)
      Serial.print(" | ");
  }

  Serial.println();

  Serial.print("[BMS] Weakest: Cell ");
  Serial.println(weakestCell);

  Serial.print("[BMS] Strongest: Cell ");
  Serial.println(strongestCell);

  Serial.print("[BMS] Imbalance: ");
  Serial.print(imbalance, 3);
  Serial.println(" V");

  Serial.print("[BMS] Adaptive Threshold: ");
  Serial.print(adaptiveThreshold, 3);
  Serial.println(" V");

  Serial.print("[BMS] Trend: ");

  if (imbalanceTrend > 0)
    Serial.println("INCREASING");

  else if (imbalanceTrend < 0)
    Serial.println("DECREASING");

  else
    Serial.println("STABLE");
}

// ============================================================
// TASK 6 - RISK
// ============================================================

void calculateRisk() {

  float imbalanceRisk =
    (imbalance / 0.30) * 40.0;

  if (imbalanceRisk > 40)
    imbalanceRisk = 40;

  float faultRisk =
    faultCount * 10.0;

  if (faultRisk > 30)
    faultRisk = 30;

  float socRisk = 0;

  if (soc < 30)
    socRisk = 30;

  else if (soc < 50)
    socRisk = 20;

  else if (soc < 70)
    socRisk = 10;

  riskScore =
    imbalanceRisk +
    faultRisk +
    socRisk;

  if (riskScore > 100)
    riskScore = 100;

  // Risk level

  if (riskScore < 25)

    riskLevel = "LOW";

  else if (riskScore < 50)

    riskLevel = "MEDIUM";

  else if (riskScore < 75)

    riskLevel = "HIGH";

  else

    riskLevel = "CRITICAL";

  batteryHealth =
    100.0 - riskScore;

  if (batteryHealth < 0)
    batteryHealth = 0;

  // Recommendation

  if (systemState == SHUTDOWN)

    recommendation =
      "Immediate shutdown inspection";

  else if (systemState == FAILSAFE)

    recommendation =
      "Verify battery safety";

  else if (faultActive)

    recommendation =
      "Inspect affected battery cell";

  else if (imbalance >= adaptiveThreshold)

    recommendation =
      "Check cell balancing";

  else if (riskLevel == "MEDIUM")

    recommendation =
      "Monitor battery condition";

  else

    recommendation =
      "Battery operating normally";
}

// ============================================================
// TASK 4 - STATE MACHINE
// ============================================================

void updateStateMachine() {

  BMSState oldState = systemState;

  if (faultActive) {

    if (riskScore >= 75)

      systemState = SHUTDOWN;

    else if (riskScore >= 50)

      systemState = FAILSAFE;

    else

      systemState = DEGRADED;
  }

  else {

    if (riskScore >= 75)

      systemState = FAILSAFE;

    else if (riskScore >= 50)

      systemState = DEGRADED;

    else

      systemState = NORMAL;
  }

  if (oldState != systemState) {

    stateTransitionCount++;

    Serial.println();
    Serial.println("========== STATE TRANSITION ==========");

    Serial.print("Timestamp: ");
    Serial.println(millis());

    Serial.print("Previous: ");
    Serial.println(stateName(oldState));

    Serial.print("New: ");
    Serial.println(stateName(systemState));

    Serial.print("Fault ID: ");
    Serial.println(faultID);

    Serial.println("======================================");
  }
}

// ============================================================
// TASK 2 - PROTECTION SYSTEM
// ============================================================

void protectionSystem() {

  bool unsafe = false;

  for (int i = 0; i < CELL_COUNT; i++) {

    if (cellVoltage[i] < UNDER_VOLTAGE) {

      unsafe = true;

      faultSource = "BATTERY";
      faultState = "UNDER_VOLTAGE";
    }

    if (cellVoltage[i] > OVER_VOLTAGE) {

      unsafe = true;

      faultSource = "BATTERY";
      faultState = "OVER_VOLTAGE";
    }
  }

  if (demoFault) {

    unsafe = true;

    faultSource = "BATTERY";
    faultState = "UNDER_VOLTAGE";
  }

  // Fault debounce

  if (unsafe) {

    recoveryRunning = false;

    if (faultStartTime == 0)
      faultStartTime = millis();

    if (millis() - faultStartTime >=
        FAULT_DEBOUNCE_TIME) {

      if (!faultActive) {

        faultActive = true;

        faultCount++;

        faultID++;

        Serial.println();
        Serial.println(">>> PROTECTION FAULT CONFIRMED <<<");

        Serial.print("Fault ID: ");
        Serial.println(faultID);

        Serial.print("Source: ");
        Serial.println(faultSource);

        Serial.print("Fault: ");
        Serial.println(faultState);
      }

      setRelay(false);
    }

    return;
  }

  faultStartTime = 0;

  // Recovery

  if (faultActive) {

    if (!recoveryRunning) {

      recoveryRunning = true;

      recoveryStartTime = millis();

      Serial.println(
        "[RECOVERY] Verification started"
      );
    }

    if (millis() - recoveryStartTime >=
        RECOVERY_TIME) {

      faultActive = false;

      recoveryRunning = false;

      faultState = "NO_FAULT";
      faultSource = "NONE";

      setRelay(true);

      Serial.println(
        "[RECOVERY] Safety verified - Relay ON"
      );
    }

    return;
  }

  setRelay(true);
}

// ============================================================
// RELAY
// ============================================================

void setRelay(bool state) {

  relayState = state;

  digitalWrite(
    RELAY_PIN,
    relayState ? HIGH : LOW
  );
}

// ============================================================
// INDICATORS
// ============================================================

void updateIndicators() {

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  if (systemState == NORMAL) {

    digitalWrite(GREEN_LED, HIGH);
  }

  else if (systemState == DEGRADED) {

    digitalWrite(YELLOW_LED, HIGH);
  }

  else {

    digitalWrite(RED_LED, HIGH);
  }

  // Buzzer for active critical fault

  if (faultActive &&
      systemState != NORMAL) {

    tone(BUZZER_PIN, 2000, 100);
  }
}

// ============================================================
// TASK 3 - LCD
// ============================================================

void updateLCD() {

  unsigned long now = millis();

  if (now - lastPageChange >= LCD_PAGE_INTERVAL) {

    lastPageChange = now;

    currentPage++;

    if (currentPage >= 3)
      currentPage = 0;
  }

  if (now - lastLCDUpdate <
      LCD_UPDATE_INTERVAL)

    return;

  lastLCDUpdate = now;

  // Critical fault override

  if (faultActive) {

    writeLCD(
      "FAULT:",
      faultState
    );

    return;
  }

  if (currentPage == 0) {

    String line1 =
      "C1:" +
      String(cellVoltage[0], 2) +
      " C2:" +
      String(cellVoltage[1], 2);

    String line2 =
      "C3:" +
      String(cellVoltage[2], 2) +
      " C4:" +
      String(cellVoltage[3], 2);

    writeLCD(line1, line2);
  }

  else if (currentPage == 1) {

    String line1 =
      "STATE:" +
      stateName(systemState);

    String line2 =
      "Relay:" +
      String(relayState ? "ON" : "OFF");

    writeLCD(line1, line2);
  }

  else {

    String line1 =
      "SOC:" +
      String(soc, 0) +
      "% R:" +
      String(riskScore, 0);

    String line2 =
      "T:" +
      String(temperatureC, 1) +
      "C";

    writeLCD(line1, line2);
  }
}

// ============================================================
// FLICKER-FREE LCD WRITE
// ============================================================

void writeLCD(String line1, String line2) {

  while (line1.length() < 16)
    line1 += " ";

  while (line2.length() < 16)
    line2 += " ";

  if (line1 != lcdLine1) {

    lcd.setCursor(0, 0);

    lcd.print(line1.substring(0, 16));

    lcdLine1 =
      line1.substring(0, 16);
  }

  if (line2 != lcdLine2) {

    lcd.setCursor(0, 1);

    lcd.print(line2.substring(0, 16));

    lcdLine2 =
      line2.substring(0, 16);
  }
}

// ============================================================
// TASK 5 - TELEMETRY QUEUE
// ============================================================

void addTelemetryEvent() {

  bool connected =
    WiFi.status() == WL_CONNECTED &&
    Blynk.connected();

  // Only meaningful data is queued

  static float lastImbalance = -1;
  static float lastSoc = -1;
  static String lastState = "";
  static String lastFault = "";

  bool meaningfulChange = false;

  if (abs(imbalance - lastImbalance) >= 0.005)
    meaningfulChange = true;

  if (abs(soc - lastSoc) >= 1.0)
    meaningfulChange = true;

  if (stateName(systemState) != lastState)
    meaningfulChange = true;

  if (faultState != lastFault)
    meaningfulChange = true;

  if (!meaningfulChange)
    return;

  lastImbalance = imbalance;
  lastSoc = soc;
  lastState = stateName(systemState);
  lastFault = faultState;

  TelemetryEvent event;

  event.timestamp = millis();

  event.imbalance = imbalance;
  event.soc = soc;

  event.weakest = weakestCell;
  event.strongest = strongestCell;

  event.relay = relayState;

  event.fault = faultState;
  event.state = stateName(systemState);

  event.rssi =
    WiFi.status() == WL_CONNECTED
      ? WiFi.RSSI()
      : -100;

  if (queueCount < QUEUE_SIZE) {

    telemetryQueue[queueTail] = event;

    queueTail++;

    if (queueTail >= QUEUE_SIZE)
      queueTail = 0;

    queueCount++;

    Serial.print("[TELEMETRY] ");

    if (connected)
      Serial.println("LIVE EVENT");

    else
      Serial.println("QUEUED EVENT");
  }

  else {

    Serial.println(
      "[TELEMETRY] Queue FULL"
    );
  }
}

// ============================================================
// FIFO TRANSMISSION
// ============================================================

void transmitQueuedEvents() {

  if (WiFi.status() != WL_CONNECTED)
    return;

  if (!Blynk.connected())
    return;

  if (queueCount == 0)
    return;

  TelemetryEvent event =
    telemetryQueue[queueHead];

  Blynk.virtualWrite(V0, event.imbalance);
  Blynk.virtualWrite(V1, event.soc);
  Blynk.virtualWrite(V2, event.weakest);
  Blynk.virtualWrite(V3, event.strongest);
  Blynk.virtualWrite(V8, event.relay ? 1 : 0);
  Blynk.virtualWrite(V9, event.fault);
  Blynk.virtualWrite(V10, "CONNECTED");
  Blynk.virtualWrite(V11, event.rssi);
  Blynk.virtualWrite(V12, queueCount - 1);
  Blynk.virtualWrite(V13, "QUEUED");
  Blynk.virtualWrite(V15, event.state);

  queueHead++;

  if (queueHead >= QUEUE_SIZE)
    queueHead = 0;

  queueCount--;

  Serial.println(
    "[TELEMETRY] Queued event transmitted"
  );
}

// ============================================================
// TASK 6 - BLYNK DASHBOARD
// ============================================================

void updateBlynkDashboard() {

  if (!Blynk.connected())
    return;

  // Cell information

  Blynk.virtualWrite(V0, cellVoltage[0]);
  Blynk.virtualWrite(V1, cellVoltage[1]);
  Blynk.virtualWrite(V2, cellVoltage[2]);
  Blynk.virtualWrite(V3, cellVoltage[3]);

  // Cell analysis

  Blynk.virtualWrite(V4, weakestCell);
  Blynk.virtualWrite(V5, strongestCell);

  // Relay

  Blynk.virtualWrite(
    V8,
    relayState ? 1 : 0
  );

  // Fault

  Blynk.virtualWrite(V9, faultState);

  // WiFi

  Blynk.virtualWrite(
    V10,
    WiFi.status() == WL_CONNECTED
      ? "CONNECTED"
      : "OFFLINE"
  );

  // RSSI

  Blynk.virtualWrite(
    V11,
    WiFi.status() == WL_CONNECTED
      ? WiFi.RSSI()
      : -100
  );

  // Queue

  Blynk.virtualWrite(V12, queueCount);

  // Telemetry mode

  Blynk.virtualWrite(
    V13,
    queueCount > 0
      ? "QUEUED"
      : "LIVE"
  );

  // Fault count

  Blynk.virtualWrite(V14, faultCount);

  // State

  Blynk.virtualWrite(
    V15,
    stateName(systemState)
  );

  // Analytics

  Blynk.virtualWrite(V16, soc);
  Blynk.virtualWrite(V17, imbalance);
  Blynk.virtualWrite(V18, riskScore);
  Blynk.virtualWrite(V19, riskLevel);
  Blynk.virtualWrite(V20, batteryHealth);
  Blynk.virtualWrite(V21, uptimeSeconds);
  Blynk.virtualWrite(V22, faultCount);
  Blynk.virtualWrite(V23, stateTransitionCount);
  Blynk.virtualWrite(V24, recommendation);
  Blynk.virtualWrite(V25, imbalanceTrend);
  Blynk.virtualWrite(V26, faultSource);
  Blynk.virtualWrite(V27, stateName(systemState));
}

// ============================================================
// FAULT INJECTION
// ============================================================

void injectFault() {

  if (!demoFault) {

    demoFault = true;

    Serial.println();
    Serial.println(
      ">>> BATTERY FAULT INJECTION <<<"
    );

    Serial.println(
      "Cell 4 = 3.05 V"
    );
  }
}

// ============================================================
// CLEAR FAULT
// ============================================================

void clearFault() {

  demoFault = false;

  cellVoltage[3] = 3.65;

  Serial.println();
  Serial.println(
    ">>> FAULT CLEAR REQUESTED <<<"
  );

  Serial.println(
    "[RECOVERY] Safety verification active"
  );
}

// ============================================================
// SERIAL COMMANDS
// ============================================================

void processSerialCommands() {

  if (!Serial.available())
    return;

  char command =
    Serial.read();

  if (command == 'F' ||
      command == 'f') {

    injectFault();
  }

  else if (command == 'C' ||
           command == 'c') {

    clearFault();
  }

  else if (command == 'N' ||
           command == 'n') {

    demoNetworkOutage = true;

    Serial.println();
    Serial.println(
      ">>> OFFLINE MODE ENABLED <<<"
    );
  }

  else if (command == 'R' ||
           command == 'r') {

    demoNetworkOutage = false;

    lastWiFiAttempt =
      millis() - WIFI_RETRY_INTERVAL;

    Serial.println();
    Serial.println(
      ">>> NETWORK RESTORE REQUESTED <<<"
    );
  }
}

// ============================================================
// SERIAL EXECUTIVE SUMMARY
// ============================================================

void printExecutiveSummary() {

  Serial.println();
  Serial.println(
    "========== EXECUTIVE SUMMARY =========="
  );

  Serial.print("Battery Health: ");
  Serial.print(batteryHealth, 1);
  Serial.println("%");

  Serial.print("SoC: ");
  Serial.print(soc, 1);
  Serial.println("%");

  Serial.print("Temperature: ");
  Serial.print(temperatureC, 1);
  Serial.println(" C");

  Serial.print("Weakest Cell: Cell ");
  Serial.println(weakestCell);

  Serial.print("Strongest Cell: Cell ");
  Serial.println(strongestCell);

  Serial.print("Imbalance: ");
  Serial.print(imbalance, 3);
  Serial.println(" V");

  Serial.print("Risk Score: ");
  Serial.print(riskScore, 1);
  Serial.println("/100");

  Serial.print("Risk Level: ");
  Serial.println(riskLevel);

  Serial.print("Relay: ");
  Serial.println(
    relayState ? "ON" : "OFF"
  );

  Serial.print("Fault: ");
  Serial.println(faultState);

  Serial.print("Fault Source: ");
  Serial.println(faultSource);

  Serial.print("Current State: ");
  Serial.println(
    stateName(systemState)
  );

  Serial.print("Queue Depth: ");
  Serial.println(queueCount);

  Serial.print("Uptime: ");
  Serial.print(uptimeSeconds);
  Serial.println(" sec");

  Serial.print("Recommendation: ");
  Serial.println(recommendation);

  Serial.println(
    "========================================"
  );
}
