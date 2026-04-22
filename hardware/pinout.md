# Hardware Pinout Configuration

## NodeMCU ESP32 Pin Mapping

| GPIO | Function | Component | Notes |
|------|----------|-----------|-------|
| 36 | ADC1_0 | ACS712 Current | Vp (input only) |
| 34 | ADC1_6 | LM35 Temp Sensor | Input only |
| 39 | ADC1_3 | Voltage Divider | Input only |
| 32 | GPIO32 | Charger Detect | Internal pull-up |
| 27 | GPIO27 | Relay Module | Digital output |
| 26 | GPIO26 | BLDC Fan | Digital output |
| 21 | GPIO21 | LCD I2C SDA | I2C data |
| 22 | GPIO22 | LCD I2C SCL | I2C clock |

## Circuit Diagram

```
                    NodeMCU ESP32
                       ┌─────┐
                       │     │
            ┌──────────┤ 36  ├──────────┐ ACS712 VCC (+5V)
            │         │     │          │
           GND        │     │         GND
                      │ 34  │
         LM35 VCC ────┤     │
                      │     │
         LM35 OUT ────┤ 39  ├────────── LM35 OUT
                      │     │
         Voltage Div ──┤     │
                      │     │
           ┌──────────┤ 32  ├──────────┐ Charger Detect
           │         │     │          │ (Pulled LOW when
          VCC        │     │         GND  charging)
           │         ├─────┤
           │    10KΩ  │ 27  │
           └─────────┤     ├──────────┐ relay IN
           ⚡        │     │          │
           │         │ 26  │          │
           └─────────┤     ├──────────┐ Fan IN
   LCD SDA  │        │     │          │
  ─────────┤────────┤ 21  ├─────────┼── LCD VCC
  LCD SCL  │        │     │          │
  ─────────┴────────┴─────┴──────────┴── LCD GND
```

## Sensor Connections

### LM35 Temperature Sensor
| LM35 Pin | ESP32 GPIO | Description |
|---------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| OUT | GPIO 34 | Analog output |

### ACS712 Current Sensor
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
| Junction | ADC Input |

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

### BLDC Fan
| Pin | ESP32 GPIO | Description |
|-----|-----------|-------------|
| VCC | 12V (external) | Power supply |
| GND | GND | Ground |
| IN | GPIO 26 | PWM control |

## I2C Address

- LCD I2C Address: `0x27` (standard)
- If LCD not responding, scan with:
  ```cpp
  Wire.beginTransmission(0x27);
  if(Wire.endTransmission() == 0) { /* Found */ }
  ```

## Power Supply Requirements

| Component | Voltage | Current |
|-----------|---------|---------|
| ESP32 | 5V | 500mA |
| LCD | 5V | 50mA |
| Relay | 5V | 70mA |
| ACS712 | 5V | 10mA |
| LM35 | 3.3V | 10µA |

**Recommended:** 5V/2A power adapter

## Assembly Notes

1. **ESP32 ADC Range:** 0-3.3V (12-bit, 4095 levels)
2. **Voltage Divider:** Scale 0-14.4V to 0-3.3V
3. **ACS712:** Bidirectional (2.5V offset at zero current)
4. **I2C:** Use level shifter if 5V LCD