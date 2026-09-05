#define BLYNK_TEMPLATE_ID "TMPL3RvbjIhYD"
#define BLYNK_TEMPLATE_NAME "EV BMS"
#define BLYNK_AUTH_TOKEN "DoMf6AhtkzEzt_urdJOPIJMJqNCK43ZF"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <math.h>

// =========================================================
// TASK 5
// EVENT-DRIVEN TELEMETRY + LIVE BLYNK DASHBOARD
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
// TELEMETRY
// =========================================================

const unsigned long TELEMETRY_RETRY_INTERVAL = 500;

const float VOLTAGE_EVENT_DELTA = 0.02;
const int RSSI_EVENT_DELTA = 5;

unsigned long lastTelemetryTime = 0;

unsigned long eventCounter = 0;

// =========================================================
// OFFLINE QUEUE
// =========================================================

#define QUEUE_SIZE 12

struct TelemetryEvent
{
  unsigned long timestamp;

  float cellVoltage[CELL_COUNT];

  int weakestCell;
  int strongestCell;

  bool relayState;

  String faultState;
  String systemState;

  int rssi;

  bool wifiConnected;
  bool blynkConnected;
};

TelemetryEvent offlineQueue[QUEUE_SIZE];

int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

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
// BMS DATA
// =========================================================

float cellVoltage[CELL_COUNT] =
{
  3.82,
  3.76,
  3.88,
  3.65
};

bool relayState = true;

String systemState = "NORMAL";
String faultState = "NO_FAULT";

// =========================================================
// DEMO CONTROL
// =========================================================

// Serial commands:
// N = simulate network outage
// R = restore network
// F = inject battery fault
// C = clear fault

bool demoNetworkOutage = false;
bool demoFault = false;

// =========================================================
// LAST SENT SNAPSHOT
// =========================================================

float lastSentVoltage[CELL_COUNT] =
{
  3.82,
  3.76,
  3.88,
  3.65
};

int lastSentWeakest = -1;
int lastSentStrongest = -1;

bool lastSentRelay = true;

String lastSentFault = "";
String lastSentState = "";

int lastSentRSSI = -100;

bool lastSentWiFi = false;
bool lastSentBlynk = false;

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

bool meaningfulChange();

TelemetryEvent createTelemetryEvent();

void enqueueEvent(TelemetryEvent event);
bool dequeueEvent(TelemetryEvent &event);

void processTelemetry();
void sendTelemetryEvent(TelemetryEvent event, bool queued);

void printTelemetryEvent(TelemetryEvent event);

void printQueueStatus();

void processSerialCommands();

void updateDashboardStatus();

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
  Serial.println("TASK 5");
  Serial.println("EVENT-DRIVEN TELEMETRY");
  Serial.println("========================================");

  Serial.println();
  Serial.println("[SYSTEM] Initializing...");

  // Configure Blynk without blocking WiFi connection
  Blynk.config(BLYNK_AUTH_TOKEN);

  // Start WiFi state machine
  startWiFiConnection();

  Serial.println("[SYSTEM] Telemetry engine ready");

  Serial.println();
  Serial.println("COMMANDS:");
  Serial.println("N = Network outage");
  Serial.println("R = Restore network");
  Serial.println("F = Inject fault");
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
  // NON-BLOCKING WIFI
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
  // SERIAL COMMANDS
  // -------------------------------------------------------

  processSerialCommands();

  // -------------------------------------------------------
  // EVENT-DRIVEN TELEMETRY
  // -------------------------------------------------------

  processTelemetry();
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
  // SIMULATED NETWORK OUTAGE
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

      Serial.print("[WIFI] IP: ");
      Serial.println(WiFi.localIP());

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

      return;
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

      // Zero timeout keeps this attempt short/non-blocking
      Blynk.connect(0);
    }
  }
}

// =========================================================
// BLYNK CONNECTED CALLBACK
// =========================================================

BLYNK_CONNECTED()
{
  Serial.println();
  Serial.println(">>> BLYNK CONNECTED <<<");

  Serial.println("[TELEMETRY] Restoring offline events...");

  // Immediately show connection health
  updateDashboardStatus();

  // Offline events will be transmitted
  // in FIFO order by processTelemetry()
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
  // FAULT INJECTION
  // -------------------------------------------------------

  if (demoFault)
  {
    cellVoltage[3] = 3.05;
    systemState = "DEGRADED";
    faultState = "UNDER_VOLTAGE";

    return;
  }

  // -------------------------------------------------------
  // NORMAL SMALL CHANGES
  // -------------------------------------------------------

  cellVoltage[0] -= 0.001;
  cellVoltage[1] -= 0.001;
  cellVoltage[2] += 0.001;
  cellVoltage[3] += 0.001;

  // Keep values inside safe range
  for (int i = 0; i < CELL_COUNT; i++)
  {
    if (cellVoltage[i] < 3.50)
      cellVoltage[i] = 3.82;

    if (cellVoltage[i] > 4.05)
      cellVoltage[i] = 3.82;
  }

  systemState = "NORMAL";
  faultState = "NO_FAULT";
}

// =========================================================
// FAULT INJECTION
// =========================================================

void injectFault()
{
  demoFault = true;

  Serial.println();
  Serial.println("========================================");
  Serial.println(">>> FAULT INJECTION <<<");
  Serial.println("Cell 4 UNDER_VOLTAGE");
  Serial.println("========================================");
}

// =========================================================
// CLEAR FAULT
// =========================================================

void clearFault()
{
  demoFault = false;

  cellVoltage[3] = 3.65;

  systemState = "NORMAL";
  faultState = "NO_FAULT";

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
// MEANINGFUL CHANGE DETECTION
// =========================================================

bool meaningfulChange()
{
  bool changed = false;

  // -------------------------------------------------------
  // Cell voltage
  // -------------------------------------------------------

  for (int i = 0; i < CELL_COUNT; i++)
  {
    if (
      fabs(cellVoltage[i] - lastSentVoltage[i])
      >= VOLTAGE_EVENT_DELTA
    )
    {
      changed = true;
    }
  }

  // -------------------------------------------------------
  // Weakest / strongest cell
  // -------------------------------------------------------

  if (getWeakestCell() != lastSentWeakest)
    changed = true;

  if (getStrongestCell() != lastSentStrongest)
    changed = true;

  // -------------------------------------------------------
  // Relay
  // -------------------------------------------------------

  if (relayState != lastSentRelay)
    changed = true;

  // -------------------------------------------------------
  // Fault
  // -------------------------------------------------------

  if (faultState != lastSentFault)
    changed = true;

  // -------------------------------------------------------
  // System state
  // -------------------------------------------------------

  if (systemState != lastSentState)
    changed = true;

  // -------------------------------------------------------
  // WiFi
  // -------------------------------------------------------

  bool currentWiFi =
    WiFi.status() == WL_CONNECTED;

  if (currentWiFi != lastSentWiFi)
    changed = true;

  // -------------------------------------------------------
  // Blynk
  // -------------------------------------------------------

  bool currentBlynk =
    Blynk.connected();

  if (currentBlynk != lastSentBlynk)
    changed = true;

  // -------------------------------------------------------
  // RSSI
  // -------------------------------------------------------

  if (currentWiFi)
  {
    int currentRSSI = WiFi.RSSI();

    if (
      abs(currentRSSI - lastSentRSSI)
      >= RSSI_EVENT_DELTA
    )
    {
      changed = true;
    }
  }

  return changed;
}

// =========================================================
// CREATE TELEMETRY EVENT
// =========================================================

TelemetryEvent createTelemetryEvent()
{
  TelemetryEvent event;

  event.timestamp = millis();

  for (int i = 0; i < CELL_COUNT; i++)
  {
    event.cellVoltage[i] =
      cellVoltage[i];
  }

  event.weakestCell =
    getWeakestCell();

  event.strongestCell =
    getStrongestCell();

  event.relayState =
    relayState;

  event.faultState =
    faultState;

  event.systemState =
    systemState;

  if (WiFi.status() == WL_CONNECTED)
  {
    event.rssi =
      WiFi.RSSI();

    event.wifiConnected = true;
  }
  else
  {
    event.rssi = -100;

    event.wifiConnected = false;
  }

  event.blynkConnected =
    Blynk.connected();

  return event;
}

// =========================================================
// ENQUEUE
// =========================================================

void enqueueEvent(TelemetryEvent event)
{
  // -------------------------------------------------------
  // Queue full
  // -------------------------------------------------------

  if (queueCount >= QUEUE_SIZE)
  {
    Serial.println("[QUEUE] FULL - dropping oldest event");

    queueHead++;

    if (queueHead >= QUEUE_SIZE)
      queueHead = 0;

    queueCount--;
  }

  offlineQueue[queueTail] =
    event;

  queueTail++;

  if (queueTail >= QUEUE_SIZE)
    queueTail = 0;

  queueCount++;

  Serial.print("[QUEUE] Event stored. Depth = ");
  Serial.println(queueCount);
}

// =========================================================
// DEQUEUE
// =========================================================

bool dequeueEvent(TelemetryEvent &event)
{
  if (queueCount == 0)
    return false;

  event =
    offlineQueue[queueHead];

  queueHead++;

  if (queueHead >= QUEUE_SIZE)
    queueHead = 0;

  queueCount--;

  return true;
}

// =========================================================
// TELEMETRY PROCESSOR
// =========================================================

void processTelemetry()
{
  unsigned long now = millis();

  if (now - lastTelemetryTime <
      TELEMETRY_RETRY_INTERVAL)
  {
    return;
  }

  lastTelemetryTime = now;

  bool networkAvailable =
    WiFi.status() == WL_CONNECTED &&
    Blynk.connected();

  // =======================================================
  // NETWORK AVAILABLE
  // =======================================================

  if (networkAvailable)
  {
    // -----------------------------------------------------
    // FIRST: transmit offline events in FIFO order
    // -----------------------------------------------------

    if (queueCount > 0)
    {
      TelemetryEvent queuedEvent;

      if (dequeueEvent(queuedEvent))
      {
        sendTelemetryEvent(
          queuedEvent,
          true
        );

        Serial.print(
          "[QUEUE] Sent queued event. Remaining = "
        );

        Serial.println(queueCount);

        return;
      }
    }

    // -----------------------------------------------------
    // THEN: send current event only if meaningful change
    // -----------------------------------------------------

    if (meaningfulChange())
    {
      TelemetryEvent liveEvent =
        createTelemetryEvent();

      sendTelemetryEvent(
        liveEvent,
        false
      );
    }
  }

  // =======================================================
  // NETWORK UNAVAILABLE
  // =======================================================

  else
  {
    if (meaningfulChange())
    {
      TelemetryEvent offlineEvent =
        createTelemetryEvent();

      enqueueEvent(
        offlineEvent
      );

      printQueueStatus();
    }
  }
}

// =========================================================
// SEND TELEMETRY
// =========================================================

void sendTelemetryEvent(
  TelemetryEvent event,
  bool queued
)
{
  eventCounter++;

  // -------------------------------------------------------
  // Dashboard data
  // -------------------------------------------------------

  Blynk.virtualWrite(
    V0,
    event.cellVoltage[0]
  );

  Blynk.virtualWrite(
    V1,
    event.cellVoltage[1]
  );

  Blynk.virtualWrite(
    V2,
    event.cellVoltage[2]
  );

  Blynk.virtualWrite(
    V3,
    event.cellVoltage[3]
  );

  Blynk.virtualWrite(
    V4,
    event.weakestCell
  );

  Blynk.virtualWrite(
    V5,
    event.strongestCell
  );

  Blynk.virtualWrite(
    V6,
    event.relayState
      ? "ON"
      : "OFF"
  );

  Blynk.virtualWrite(
    V7,
    event.faultState
  );

  Blynk.virtualWrite(
    V8,
    event.wifiConnected
      ? "CONNECTED"
      : "OFFLINE"
  );

  Blynk.virtualWrite(
    V9,
    event.rssi
  );

  Blynk.virtualWrite(
    V10,
    queueCount
  );

  Blynk.virtualWrite(
    V11,
    queued
      ? "QUEUED"
      : "LIVE"
  );

  Blynk.virtualWrite(
    V12,
    eventCounter
  );

  Blynk.virtualWrite(
    V13,
    event.systemState
  );

  // -------------------------------------------------------
  // Update last sent snapshot
  // -------------------------------------------------------

  for (int i = 0; i < CELL_COUNT; i++)
  {
    lastSentVoltage[i] =
      event.cellVoltage[i];
  }

  lastSentWeakest =
    event.weakestCell;

  lastSentStrongest =
    event.strongestCell;

  lastSentRelay =
    event.relayState;

  lastSentFault =
    event.faultState;

  lastSentState =
    event.systemState;

  lastSentRSSI =
    event.rssi;

  lastSentWiFi =
    event.wifiConnected;

  lastSentBlynk =
    event.blynkConnected;

  // -------------------------------------------------------
  // Serial log
  // -------------------------------------------------------

  printTelemetryEvent(event);
}

// =========================================================
// TELEMETRY LOG
// =========================================================

void printTelemetryEvent(
  TelemetryEvent event
)
{
  Serial.println();
  Serial.println(
    "---------- TELEMETRY EVENT ----------"
  );

  Serial.print("Event ID: ");
  Serial.println(eventCounter);

  Serial.print("Timestamp: ");
  Serial.println(event.timestamp);

  Serial.print("C1: ");
  Serial.println(event.cellVoltage[0], 3);

  Serial.print("C2: ");
  Serial.println(event.cellVoltage[1], 3);

  Serial.print("C3: ");
  Serial.println(event.cellVoltage[2], 3);

  Serial.print("C4: ");
  Serial.println(event.cellVoltage[3], 3);

  Serial.print("Weakest Cell: C");
  Serial.println(event.weakestCell);

  Serial.print("Strongest Cell: C");
  Serial.println(event.strongestCell);

  Serial.print("Relay: ");
  Serial.println(
    event.relayState
      ? "ON"
      : "OFF"
  );

  Serial.print("State: ");
  Serial.println(
    event.systemState
  );

  Serial.print("Fault: ");
  Serial.println(
    event.faultState
  );

  Serial.print("WiFi: ");
  Serial.println(
    event.wifiConnected
      ? "CONNECTED"
      : "OFFLINE"
  );

  Serial.print("RSSI: ");
  Serial.println(event.rssi);

  Serial.print("Queue Depth: ");
  Serial.println(queueCount);

  Serial.println(
    "--------------------------------------"
  );
}

// =========================================================
// QUEUE STATUS
// =========================================================

void printQueueStatus()
{
  Serial.print(
    "[TELEMETRY] Offline queue depth: "
  );

  Serial.println(queueCount);
}

// =========================================================
// DASHBOARD STATUS
// =========================================================

void updateDashboardStatus()
{
  if (!Blynk.connected())
    return;

  Blynk.virtualWrite(
    V8,
    WiFi.status() == WL_CONNECTED
      ? "CONNECTED"
      : "OFFLINE"
  );

  Blynk.virtualWrite(
    V9,
    WiFi.status() == WL_CONNECTED
      ? WiFi.RSSI()
      : -100
  );

  Blynk.virtualWrite(
    V10,
    queueCount
  );

  Blynk.virtualWrite(
    V11,
    queueCount > 0
      ? "QUEUED"
      : "LIVE"
  );
}

// =========================================================
// SERIAL COMMANDS
// =========================================================

void processSerialCommands()
{
  if (!Serial.available())
    return;

  char command =
    Serial.read();

  // -------------------------------------------------------
  // NETWORK OUTAGE
  // -------------------------------------------------------

  if (
    command == 'N' ||
    command == 'n'
  )
  {
    demoNetworkOutage = true;

    Serial.println();
    Serial.println(
      ">>> OFFLINE MODE ENABLED <<<"
    );

    Serial.println(
      "Telemetry events will be queued."
    );
  }

  // -------------------------------------------------------
  // RESTORE NETWORK
  // -------------------------------------------------------

  if (
    command == 'R' ||
    command == 'r'
  )
  {
    demoNetworkOutage = false;

    lastWiFiAttempt =
      millis() - WIFI_RETRY_INTERVAL;

    Serial.println();
    Serial.println(
      ">>> NETWORK RESTORE REQUESTED <<<"
    );
  }

  // -------------------------------------------------------
  // FAULT
  // -------------------------------------------------------

  if (
    command == 'F' ||
    command == 'f'
  )
  {
    injectFault();
  }

  // -------------------------------------------------------
  // CLEAR FAULT
  // -------------------------------------------------------

  if (
    command == 'C' ||
    command == 'c'
  )
  {
    clearFault();
  }
}
