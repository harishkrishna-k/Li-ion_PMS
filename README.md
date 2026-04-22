# Li-ion Battery Performance Monitoring System (Li-ion_PMS)

A comprehensive IoT-based battery monitoring system built with ESP32 and Supabase for real-time lithium-ion battery parameter monitoring and remote data storage.

![ESP32](https://img.shields.io/badge/ESP32-NodeMCU-red)
![Supabase](https://img.shields.io/badge/Supabase-PostgreSQL-blue)
![License](https://img.shields.io/badge/License-MIT-green)

## Features

- **Real-time Monitoring**: Voltage, current, and temperature sensors
- **16x2 LCD Display**: Local status display with I2C interface
- **Automatic Cooling**: Temperature-based fan control
- **Remote Storage**: Supabase PostgreSQL cloud database
- **WiFi Connectivity**: NodeMCU ESP32 with built-in WiFi
- **Portfolio Dashboard**: Responsive web interface

## Hardware Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | NodeMCU ESP32 | Main controller |
| Temperature | LM35DZ | Battery temp sensing |
| Current | ACS712-5A | Current measurement |
| Voltage | Voltage Divider | Battery voltage |
| Display | 16x2 LCD I2C | Status display |
| Relay | 1-Channel 5V | Charge control |
| Cooling | BLDC Fan | Heat dissipation |

## Pin Configuration

```
GPIO 34 → LM35 Temperature Sensor
GPIO 36 → ACS712 Current Sensor
GPIO 39 → Voltage Divider
GPIO 32 → Charger Detection
GPIO 27 → Relay Control
GPIO 26 → BLDC Fan
GPIO 21 → LCD SDA (I2C)
GPIO 22 → LCD SCL (I2C)
```

## Getting Started

### Prerequisites

- NodeMCU ESP32
- Arduino IDE with ESP32 board support
- Supabase account (free tier)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/harishkrishna-k/Li-ion_PMS.git
   cd Li-ion_PMS
   ```

2. **Install dependencies**
   - Arduino IDE
   - ESP32 board package
   - Required libraries:
     - `LiquidCrystal_I2C`
     - `HTTPClient`
     - `WiFi`

3. **Configure Supabase**
   - Create a new Supabase project
   - Run `database/schema.sql` in SQL Editor
   - Update credentials in `src/esp32/main.ino`

4. **Upload code**
   ```bash
   # In Arduino IDE
   # Select Board: NodeMCU-ESP32
   # Upload via USB
   ```

### Configuration

Update these values in `src/esp32/main.ino`:

```cpp
const char* WIFI_SSID = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";
```

Or use `config.env.example` as reference.

## Project Structure

```
Li-ion_PMS/
├── README.md                 # This file
├── config.env.example        # Configuration template
├── .gitignore               # Git ignore rules
├── src/esp32/
│   ├── main.ino             # Main ESP32 sketch
│   ├── sensors.cpp/h        # Sensor functions
│   ├── lcd_display.cpp/h    # LCD interface
│   ├── relay_control.cpp/h # Relay & fan
│   └── database.cpp/h      # Supabase client
├── docs/                   # Portfolio website
│   ├── index.html
│   ├── css/style.css
│   └── js/script.js
├── hardware/
│   └── pinout.md           # Pin configuration
└── database/
    └── schema.sql          # Supabase schema
```

## Database Schema

The `sensor_readings` table stores:

| Column | Type | Description |
|--------|------|-------------|
| id | BIGSERIAL | Primary key |
| device_name | TEXT | Device identifier |
| voltage | REAL | Battery voltage (V) |
| current | REAL | Battery current (A) |
| temperature | REAL | Temperature (°C) |
| battery_status | TEXT | charging/discharging/full/low |
| created_at | TIMESTAMPTZ | Timestamp |

## Demo Dashboard

The `docs/` folder contains a portfolio-ready dashboard:

```bash
# Open in browser
docs/index.html
```

Or deploy to GitHub Pages for live demo.

## Tech Stack

- **Hardware**: NodeMCU ESP32, LM35, ACS712, 16x2 LCD
- **Database**: Supabase PostgreSQL
- **Frontend**: HTML, CSS, JavaScript
- **Deployment**: GitHub Pages

## Future Enhancements

- [ ] Add Bluetooth connectivity
- [ ] Implement OTA updates
- [ ] Add push notifications
- [ ] Mobile app integration
- [ ] Data analytics dashboard

## License

MIT License - See [LICENSE](LICENSE) for details.

## Authors

- [Harish Krishna K](https://github.com/harishkrishna-k)
- [Jayaram H](https://github.com)
- [Navin Y](https://github.com)

## Acknowledgments

- University College of Engineering
- Department of Mechanical Engineering
- Project Guide: Dr. M. Chandrasekar

---

<p align="center">Built with ESP32 & Supabase</p>