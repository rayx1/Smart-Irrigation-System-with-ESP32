#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Hardware Configuration
#define RELAY_PIN 23  // Changed from D1 to GPIO 23
#define MOISTURE_SENSOR_PIN 34  // Changed from A0 to GPIO 34 (ADC1_CH6)

// System Settings
int moistureThreshold = 40;       // Default moisture threshold (%)
bool autoMode = true;            // Start in automatic mode
unsigned long wateringDuration = 5000; // Watering duration in ms

// WiFi Configuration
const char* wifiSSID = "WIFI";
const char* wifiPassword = "12345678";

// Web Server Objects
WebServer server(80);
WebSocketsServer webSocket(81);

// System Variables
bool pumpStatus = false;
unsigned long pumpStartTime = 0;

// Embedded HTML with JavaScript (same as before)
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Smart Irrigation</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .card { background: white; border-radius: 8px; padding: 15px; margin-bottom: 15px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .status { display: flex; justify-content: space-between; margin: 10px 0; }
        .status-label { font-weight: bold; }
        .pump-status { font-weight: bold; color: #e74c3c; }
        .pump-status.on { color: #2ecc71; }
        button { width: 100%; padding: 10px; margin: 5px 0; border: none; border-radius: 4px; font-size: 16px; }
        .btn-on { background: #2ecc71; color: white; }
        .btn-off { background: #e74c3c; color: white; }
        .btn-auto { background: #3498db; color: white; }
        .slider-container { margin: 15px 0; }
        .threshold-value { text-align: center; font-size: 18px; margin: 5px 0; }
    </style>
</head>
<body>
    <div class="card">
        <h1>Smart Irrigation System</h1>
        <div class="status">
            <span class="status-label">Soil Moisture:</span>
            <span id="moistureValue">--%</span>
        </div>
        <div class="status">
            <span class="status-label">Pump Status:</span>
            <span class="pump-status" id="pumpStatus">OFF</span>
        </div>
        <div class="status">
            <span class="status-label">Mode:</span>
            <span id="modeStatus">Auto</span>
        </div>
    </div>

    <div class="card">
        <h2>Controls</h2>
        <button id="btnPumpOn" class="btn-on">Turn Pump ON</button>
        <button id="btnPumpOff" class="btn-off">Turn Pump OFF</button>
        <button id="btnAutoMode" class="btn-auto">Toggle Auto Mode</button>
    </div>

    <div class="card">
        <h2>Settings</h2>
        <div class="slider-container">
            <div class="status-label">Moisture Threshold:</div>
            <input type="range" id="thresholdSlider" min="0" max="100" value="40">
            <div class="threshold-value" id="thresholdValue">40%</div>
        </div>
    </div>

    <script>
        var socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        
        socket.onmessage = function(event) {
            var data = JSON.parse(event.data);
            document.getElementById('moistureValue').textContent = data.moisture + '%';
            
            var pumpStatus = document.getElementById('pumpStatus');
            pumpStatus.textContent = data.pumpStatus ? 'ON' : 'OFF';
            pumpStatus.className = 'pump-status' + (data.pumpStatus ? ' on' : '');
            
            document.getElementById('modeStatus').textContent = data.autoMode ? 'Auto' : 'Manual';
            document.getElementById('thresholdSlider').value = data.threshold;
            document.getElementById('thresholdValue').textContent = data.threshold + '%';
        };
        
        document.getElementById('btnPumpOn').addEventListener('click', function() {
            socket.send(JSON.stringify({command: 'pump_on'}));
        });
        
        document.getElementById('btnPumpOff').addEventListener('click', function() {
            socket.send(JSON.stringify({command: 'pump_off'}));
        });
        
        document.getElementById('btnAutoMode').addEventListener('click', function() {
            var mode = document.getElementById('modeStatus').textContent === 'Auto';
            socket.send(JSON.stringify({command: mode ? 'auto_off' : 'auto_on'}));
        });
        
        document.getElementById('thresholdSlider').addEventListener('input', function() {
            var value = this.value;
            document.getElementById('thresholdValue').textContent = value + '%';
            socket.send(JSON.stringify({threshold: parseInt(value)}));
        });
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize hardware
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Start with pump off

  // Connect to WiFi
  WiFi.begin(wifiSSID, wifiPassword);
  
  Serial.println("\nConnecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Configure web server
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  server.on("/status", HTTP_GET, []() {
    StaticJsonDocument<200> doc;
    doc["moisture"] = readMoisture();
    doc["pumpStatus"] = pumpStatus;
    doc["autoMode"] = autoMode;
    doc["threshold"] = moistureThreshold;
    
    String jsonString;
    serializeJson(doc, jsonString);
    server.send(200, "application/json", jsonString);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  // Automatic watering logic
  if (autoMode) {
    int currentMoisture = readMoisture();
    
    if (pumpStatus) {
      // Check if watering duration has elapsed
      if (millis() - pumpStartTime >= wateringDuration) {
        stopPump();
      }
    } else {
      // Check if soil is too dry
      if (currentMoisture < moistureThreshold) {
        startPump();
      }
    }
  }
  
  // Broadcast status every 2 seconds
  static unsigned long lastBroadcast = 0;
  if (millis() - lastBroadcast > 2000) {
    broadcastStatus();
    lastBroadcast = millis();
  }
}

void startPump() {
  digitalWrite(RELAY_PIN, LOW);
  pumpStatus = true;
  pumpStartTime = millis();
  Serial.println("Pump STARTED");
}

void stopPump() {
  digitalWrite(RELAY_PIN, HIGH);
  pumpStatus = false;
  Serial.println("Pump STOPPED");
}

int readMoisture() {
  int sensorValue = analogRead(MOISTURE_SENSOR_PIN);
  // ESP32 has 12-bit ADC (0-4095) instead of ESP8266's 10-bit (0-1023)
  int moisturePercent = map(sensorValue, 4095, 1200, 0, 100); // Adjusted for ESP32's ADC range
  moisturePercent = constrain(moisturePercent, 0, 100);
  return moisturePercent;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        broadcastStatus();
      }
      break;
    case WStype_TEXT:
      handleWebSocketMessage((char*)payload);
      break;
  }
}

void handleWebSocketMessage(char* payload) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print("WebSocket error: ");
    Serial.println(error.c_str());
    return;
  }
  
  if (doc.containsKey("command")) {
    String command = doc["command"];
    if (command == "pump_on") startPump();
    else if (command == "pump_off") stopPump();
    else if (command == "auto_on") autoMode = true;
    else if (command == "auto_off") autoMode = false;
  }
  
  if (doc.containsKey("threshold")) {
    moistureThreshold = doc["threshold"];
    Serial.print("Threshold set to: ");
    Serial.println(moistureThreshold);
  }
  
  broadcastStatus();
}

void broadcastStatus() {
  StaticJsonDocument<200> doc;
  doc["moisture"] = readMoisture();
  doc["pumpStatus"] = pumpStatus;
  doc["autoMode"] = autoMode;
  doc["threshold"] = moistureThreshold;
  
  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.broadcastTXT(jsonString);
}
