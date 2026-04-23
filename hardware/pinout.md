# Hardware Pinout Configuration - v2.0 (ESP32)

## NodeMCU ESP32 Pin Mapping (Final)

| GPIO | Function | Component | Notes |
|------|----------|-----------|-------|
| 36 | ADC1_0 | ACS712 Current | VP (input only) |
| 34 | ADC1_6 | LM35 Temp Sensor | Input only |
| 39 | ADC1_3 | Voltage Divider | Input only |
| 32 | GPIO32 | Charger Detect | Internal pull-up |
| 27 | GPIO27 | Relay Module | Digital output |
| 26 | GPIO26 | BLDC Fan PWM | PWM output |
| 21 | GPIO21 | LCD I2C SDA | I2C data |
| 22 | GPIO22 | LCD I2C SCL | I2C clock |

## Circuit Diagram (v2.0)

```
                    NodeMCU ESP32
                       ┌─────┐
                       │     │
            ┌──────────┤ 36  ├──────────┐
            │         │     │          │
           GND        │     │         GND
                      │ 34  │
          LM35 VCC ────┤     │
                      │     │
          LM35 OUT ───┤ 39  ├──────────
                      │     │
                      │     │
         Voltage Div ───┤     │
                      │     │
            ┌──────────┤ 32  ├──────────┐
           VCC        │     │          │
          10kΩ       ├─────┤         GND
           ├─────────┤ 27  ├──────────┐
           │ Relay   │     │          │
           │IN      │     │          │
           ├────────┤ 26  ├──────────┐
           │FanPWM  │     │          │
   LCD SDA │        │     │          │
  ────────┼────────┤ 21  ├─────────┼── LCD
  LCD SCL │        │     │          │
  ────────┴────────┴─────┴──────────┴──
```

## Sensor Connections (v2.0)

### LM35 Temperature Sensor
| LM35 Pin | ESP32 GPIO | Description |
|---------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| OUT | GPIO 34 | Analog output |

### ACS712 Current Sensor (5A model)
| ACS712 Pin | ESP32 GPIO | Description |
|------------|-----------|-------------|
| VCC | 5V | Power supply |
| GND | GND | Ground |
| OUT | GPIO 36 | Analog output |
| VIN+ | Battery+ | Current input |
| VIN- | Battery- | Current output |

### Voltage Divider
| Component | Connection |
|-----------|------------|
| R1 (30kΩ) | Battery+ → GPIO 39 |
| R2 (7.5kΩ) | GPIO 39 → GND |
| D1 (3.3V Zener) | GPIO 39 → GND (clamp) |

### 16x2 LCD (I2C)
| LCD Pin | ESP32 GPIO | Description |
|---------|-----------|-------------|
| VCC | 5V | Power supply |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C data |
| SCL | GPIO 22 | I2C clock |

### Relay Module
| Pin | ESP32 GPIO | Description |
|-----|-----------|-------------|
| VCC | 5V | Power supply |
| GND | GND | Ground |
| IN | GPIO 27 | Control signal |

### BLDC Fan (PWM controlled)
| Pin | ESP32 GPIO | Description |
|-----|-----------|-------------|
| VCC | 12V (external) | Power supply |
| GND | GND | Ground |
| IN | GPIO 26 | PWM control |

## I2C Address

- LCD I2C Address: `0x27` (standard)
- Detection code:
  ```cpp
  Wire.beginTransmission(0x27);
  if(Wire.endTransmission() == 0) { Serial.println("LCD Found"); }
  ```

## Power Supply Requirements

| Component | Voltage | Current |
|-----------|---------|---------|
| ESP32 | 5V (USB) | 500mA |
| LCD | 5V | 50mA |
| Relay | 5V | 70mA |
| ACS712 | 5V | 10mA |
| LM35 | 3.3V | 10µA |
| Fan | 12V ext | 200mA |

**Recommended:** 5V/2A USB power adapter + 12V/1A fan supply

## Assembly Notes (v2.0)

1. **ESP32 ADC Range:** 0-3.3V (12-bit, 4095 levels)
2. **Voltage Divider:** Scales 0-14.4V to 0-3.3V
3. **ACS712:** Bidirectional (2.5V offset at zero current)
4. **I2C:** Use level shifter if 5V LCD
5. **Flyback Diode:** Place across relay coil (1N4007)
6. **Zener Clamp:** Protect ADC from voltage spikes
7. **Decoupling:** 100nF ceramic near ESP32 VCC

## Initial vs v2.0 Changes

| Feature | Original | v2.0 |
|---------|----------|------|
| Controller | Arduino Uno | ESP32 |
| Interface | Parallel LCD | I2C LCD |
| Connectivity | Serial only | WiFi + Cloud |
| Power | Always on | Deep Sleep |
| Fan Control | ON/OFF | PID PWM |
| Data | Local only | + Supabase |
| Updates | USB only | OTA |
| Filters | None | Oversampling + Avg |
| Configuration | Hardcoded | WiFi Manager |