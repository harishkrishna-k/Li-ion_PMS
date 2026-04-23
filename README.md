# Li-ion Battery Performance Monitoring System (Li-ion_PMS)

A comprehensive IoT-based battery monitoring system built with ESP32, featuring professional hardware design, cloud integration, and advanced signal processing.

![ESP32](https://img.shields.io/badge/ESP32-NodeMCU-red)
![Supabase](https://img.shields.io/badge/Supabase-PostgreSQL-blue)
![License](https://img.shields.io/badge/License-MIT-green)
![Arduino](https://img.shields.io/badge/Arduino-ESP32-red)

## 🎯 Project Overview

This is an engineering case study demonstrating a complete IoT solution:

- **Embedded Systems**: ESP32 firmware with non-blocking architecture, signal processing, and PID control
- **Hardware Design**: Professional PCB specifications, thermal management, and component integration
- **Cloud Infrastructure**: Supabase PostgreSQL with time-series optimization and analytics
- **Portfolio Presentation**: Professional case study website with live data visualization

## ✨ Key Features

### Hardware
- **Controller**: NodeMCU ESP32 (WiFi + Bluetooth)
- **Sensors**: LM35 (temperature), ACS712-5A (current), voltage divider
- **Display**: 16×2 LCD (I2C interface)
- **Control**: Relay module + BLDC fan with PID control

### Firmware (v2.0)
- **Non-blocking**: millis() timers instead of delay()
- **Signal Processing**: 32× oversampling + 10-sample moving average
- **PID Control**: Precision temperature management
- **WiFi Manager**: Web portal configuration
- **OTA Updates**: Over-The-Air firmware updates
- **Deep Sleep**: Power saving modes

### Cloud
- **Database**: Supabase PostgreSQL
- **Analytics**: Hourly/daily aggregates, SOH calculation
- **Retention**: Automatic 90-day data pruning
- **Security**: Row Level Security (RLS)

## 📊 Live Dashboard

Visit the live dashboard: **[harishkrishna-k.github.io/Li-ion_PMS](https://harishkrishna-k.github.io/Li-ion_PMS)**

The dashboard displays:
- Real-time voltage, current, temperature
- State of Charge (SOC) percentage
- Historical data charts
- Connection status indicators

## 🚀 Getting Started

### Hardware Setup

| GPIO | Component |
|------|-----------|
| 34 | LM35 Temperature |
| 36 | ACS712 Current |
| 39 | Voltage Divider |
| 32 | Charger Detect |
| 27 | Relay Control |
| 26 | Fan PWM |
| 21 | LCD SDA |
| 22 | LCD SCL |

See [hardware/pinout.md](hardware/pinout.md) for complete wiring.

### Software Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/harishkrishna-k/Li-ion_PMS.git
   cd Li-ion_PMS
   ```

2. **Configure Supabase**
   - Create a Supabase project
   - Run `database/schema.sql` in SQL Editor
   - Update credentials in `src/esp32/main.ino`

3. **Upload to ESP32**
   - Select Board: "ESP32 Dev Module"
   - Upload using Arduino IDE or PlatformIO

### Configuration

Update WiFi and Supabase credentials:

```cpp
// In main.ino
const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";
```

Or use WiFi Manager (access point mode) at `http://192.168.4.1`

## 📁 Project Structure

```
Li-ion_PMS/
├── README.md                    # This file
├── config.env.example         # Configuration template
├── LICENSE                   # MIT License
├── .gitignore
├── src/esp32/
│   ├── main.ino              # Main ESP32 sketch
│   ├── sensors.cpp/h          # Signal processing
│   ├── lcd_display.cpp/h     # LCD interface
│   ├── relay_control.cpp/h   # PID fan control
│   └── database.cpp/h        # Supabase client
├── docs/                     # Portfolio website
│   ├── index.html           # Case study site
│   ├── css/style.css        # Styles
│   ├── js/script.js        # Dashboard JS
│   └── assets/             # Images
├── hardware/
│   ├── pinout.md           # Pin configuration
│   ├── pcb-design.md      # PCB specs
│   └── schematic.md       # Circuits
└── database/
    └── schema.sql         # PostgreSQL schema
```

## 📈 Technical Specifications

| Parameter | Value |
|-----------|-------|
| Operating Voltage | 8.0V - 12.9V |
| Current Range | 0 - 5A |
| Temperature Range | -55°C to +150°C |
| ADC Resolution | 12-bit (ESP32) |
| Sensor Rate | 1 Hz |
| Cloud Sync | 5 seconds |
| Power (Active) | ~630mA |
| Power (Sleep) | 10µA |

## 🧪 Testing Results

| Test | Result |
|------|--------|
| Voltage Accuracy | ±0.05V ✓ |
| Current Accuracy | ±0.08A ✓ |
| Temperature Accuracy | ±0.3°C ✓ |
| PID Response | 60% overshoot reduction ✓ |
| Deep Sleep | 99% power reduction ✓ |

## 📚 Documentation

- [Hardware Pinout](hardware/pinout.md)
- [PCB Design Specs](hardware/pcb-design.md)
- [Circuit Schematic](hardware/schematic.md)
- [Database Schema](database/schema.sql)

## 👥 Project Team

- **Harish Krishna K** - Hardware & Firmware Development
- **Jayaram H** - CAD Design & Modeling  
- **Navin Y** - Testing & Validation
- **Guide**: Dr. M. Chandrasekar, Anna University

## 📄 License

MIT License - See [LICENSE](LICENSE)

---

<p align="center">Built with ESP32 & Supabase | Engineering Case Study 2023</p>