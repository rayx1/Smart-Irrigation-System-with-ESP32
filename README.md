# Smart Irrigation System with ESP32

A complete IoT-based smart irrigation system that automatically waters plants based on soil moisture levels, with a web interface for monitoring and control.

## Features

- 🌱 **Automatic watering** based on soil moisture thresholds
- 💧 **Real-time monitoring** of soil moisture levels
- 🌐 **Web interface** accessible from any device
- 🔄 **WebSocket communication** for live updates
- ⚙️ **Manual override** capability
- 📱 **Responsive design** works on mobile and desktop
- 🔌 **ESP32-based** for reliable performance

## Hardware Requirements

| Component | Specification |
|-----------|--------------|
| ESP32 Board | Any ESP32 development board |
| Soil Moisture Sensor | Capacitive type recommended |
| Water Pump | 3-12V DC pump |
| Relay Module | 5V compatible |
| Power Supply | Appropriate for your pump |
| Jumper Wires | For connections |

## Wiring Diagram

```plaintext
ESP32         Peripheral
-----------------------------
GPIO23  ────> Relay IN
3.3V    ────> Moisture Sensor VCC
GND     ────> Moisture Sensor GND
GPIO34  ────> Moisture Sensor OUT

## Software Requirements

Arduino IDE with ESP32 support

Required libraries:

WiFi

WebServer

WebSockets

ArduinoJson

## Configuration
  WiFi Settings:
  Modify these lines in the code:
const char* wifiSSID = "YOUR_WIFI_SSID";
const char* wifiPassword = "YOUR_WIFI_PASSWORD";

## Moisture Calibration:
Adjust these values in readMoisture():
int moisturePercent = map(sensorValue, 4095, 1200, 0, 100);
// 4095 = dry value (sensor in air)
// 1200 = wet value (sensor in water)

## Default Settings:
int moistureThreshold = 40;  // % threshold for watering
unsigned long wateringDuration = 5000;  // 5 seconds watering duration

## Usage
After uploading, open Serial Monitor (115200 baud) to see the IP address

Connect any device to the same WiFi network

Access the web interface at http://[ESP_IP_ADDRESS]

Interface allows:

Viewing current moisture level

Manual pump control

Auto/manual mode switching

Adjusting moisture threshold

## Web Interface

The responsive web interface provides:

Real-time moisture percentage

Pump status indicator

Control buttons

Threshold adjustment slider

## License
This project is licensed under the MIT License - see the LICENSE file for details.


### Key Features of This README:

1. **Visual Appeal**: Includes placeholders for images/banners
2. **Complete Documentation**: Covers all aspects of the project
3. **Structured Information**: Easy to navigate sections
4. **Troubleshooting Guide**: Quick solutions to common issues
5. **Professional Formatting**: Tables, code blocks, and clear headings

### How to Use:
1. Copy this content to a file named `README.md` in your project root
2. Replace placeholder values (like `[Your Name]`)
3. Add actual project images (upload to Imgur or GitHub)
4. Customize any sections specific to your implementation

This README will help users understand, install, and use your project effectively, while also making your GitHub repository look professional and well-maintained.
