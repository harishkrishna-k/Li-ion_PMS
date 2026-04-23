# Li-ion Battery Performance Monitoring System (Li-ion_PMS)

## ⚠️ IMPORTANT DISCLAIMERS - READ BEFORE USE

### 1. Blocking Operations
The HTTP calls to Supabase are **BLOCKING**. The `http.POST()` function will freeze the ESP32 during network operations. This means:
- Temperature monitoring stops while uploading to cloud
- WiFi latency affects responsiveness

**For production use:** Implement FreeRTOS tasks or use async HTTP client.

### 2. Mock Data Dashboard
The portfolio dashboard (`/docs`) displays **MOCK DATA**, not real cloud data. The JavaScript generates random values for visual demonstration only.

To see real data:
1. Set up Supabase
2. Configure credentials in firmware
3. Implement frontend API fetching using `@supabase/supabase-js`

### 3. Memory Considerations
This code uses simple arrays and the Arduino `String` class. For long-running deployments:
- Consider using ESP-IDF's filter component
- Buffer overflow is possible with large payloads

### 4. Sensor Calibration
Calibration offsets are set to 0.0 by default. Run the calibration function with no load for accurate readings.

---

## Features

### Hardware
- **Controller**: NodeMCU ESP32 (WiFi + Bluetooth)
- **Sensors**: LM35 (temperature), ACS712-5A (current), voltage divider
- **Display**: 16×2 LCD (I2C interface)
- **Control**: Relay module + BLDC fan

### Firmware (v2.1)
- Simple averaging filters (not "advanced signal processing")
- Basic threshold-based fan control (not true PID)
- Hardcoded credentials (not fully dynamic)
- Known blocking HTTP calls

---

## Known Limitations

| Issue | Description | Fix Required |
|-------|-------------|---------------|
| Blocking HTTP | http.POST() freezes ESP32 | Use async HTTP or FreeRTOS |
| Mock Dashboard | Frontend shows random data | Implement real API fetch |
| Memory | String concatenation fragments heap | Use static buffers/ArduinoJson |
| Database | Raw queries on large tables | Add materialized views |
| Architecture | Global variables everywhere | Refactor to proper state machine |

---

## Getting Started

### Hardware Setup
See [hardware/pinout.md](hardware/pinout.md) for wiring.

### Software Setup
1. Install ESP32 board support in Arduino IDE
2. Update WiFi credentials in `main.ino`:
   ```cpp
   const char* WIFI_SSID = "YourNetwork";
   const char* WIFI_PASSWORD = "YourPassword";
   const char* SUPABASE_URL = "your-project.supabase.co";
   const char* SUPABASE_ANON_KEY = "your-key";
   ```
3. Upload to ESP32

### Configuration
Update Supabase credentials directly in the firmware (hardcoded for simplicity):

```cpp
// In main.ino
const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";
```

---

## Project Structure

```
Li-ion_PMS/
├── README.md                    # This file
├── config.env.example
├── LICENSE
├── .gitignore
├── src/esp32/
│   ├── main.ino                 # Main sketch
│   ├── sensors.cpp/h            # Sensor processing
│   ├── lcd_display.cpp/h
│   ├── relay_control.cpp/h
│   └── database.cpp/h           # Note: Uses blocking HTTP
├── docs/
│   ├── index.html              # Case study (with mock data)
│   ├── css/style.css
│   └── js/script.js             # Generates MOCK data
├── hardware/
│   ├── pinout.md
│   ├── pcb-design.md
│   └── schematic.md
└── database/
    └── schema.sql               # Requires materialized views for performance
```

---

## Technical Specs

| Parameter | Value |
|-----------|-------|
| Operating Voltage | 8.0V - 12.9V |
| Current Range | 0 - 5A |
| Temperature Range | -55°C to +150°C |
| Data Rate | 1 Hz sensor, 5 sec cloud |
| Power (Active) | ~630mA |
| **Known Issues** | Blocking calls, mock dashboard |

---

## Testing Results

| Test | Result |
|------|--------|
| Voltage Accuracy | ±0.05V |
| Current Accuracy | ±0.08A |
| Temperature Accuracy | ±0.3°C |

---

## Contributors

- **Harish Krishna K** - Hardware & Firmware
- **Jayaram H** - CAD Design
- **Navin Y** - Testing & Validation
- **Guide**: Dr. M. Chandrasekar, Anna University

---

## License

MIT License - See [LICENSE](LICENSE)

---

## Summary

This is a **student project** with documented limitations. It demonstrates:
- Basic IoT system design
- ESP32 programming
- Supabase integration (basic)
- Hardware prototyping

It is **NOT** production-ready. See "Known Limitations" table above.

For a portfolio, present it as a learning project with clear acknowledgments of what works and what needs improvement.