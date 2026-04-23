# Hardware Engineering Documentation

## PCB Design Specifications

### Board Dimensions
- **Size:** 70mm × 50mm (2-layer PCB)
- **Thickness:** 1.6mm FR4
- **Finish:** HASL (Lead-free)
- **Silkscreen:** White

### Layer Stackup
```
Top Layer: Signal + Power
Core: FR4 1.6mm
Bottom Layer: Ground Plane
```

### Power Trace Widths (Based on 2A current)
| Net | Width | Notes |
|-----|-------|-------|
| VBUS+ | 3.0mm | Battery positive |
| VBUS- | 3.0mm | Battery negative |
| 5V Rail | 1.5mm | ESP32 power |
| 3.3V Rail | 1.0mm | Sensor power |
| Signal | 0.3mm | GPIO traces |

---

## Circuit Protection

### Voltage Divider Protection
```
R1 (30kΩ, 1%)
R2 (7.5kΩ, 1%)
D1: 3.3V Zener Diode (BZX84C3V3)
```
- Zener clamps voltage to 3.3V max
- Protects ESP32 ADC pin

### Relay Protection
```
D2: 1N4007 Flyback Diode
Parallel across relay coil
```
- Catches inductive spike when relay de-energizes

### ESP32 Power Filtering
```
C1: 100nF Ceramic (VCC to GND)
C2: 10µF Electrolytic (VCC to GND)
```
- Placed within 5mm of ESP32 VCC pin

---

## Pin Configuration (ESP32 DevKit)

| GPIO | Function | Component | Notes |
|------|-----------|-----------|-------|
| 36 | ADC1_0 | ACS712 OUT | VP (input only) |
| 34 | ADC1_6 | LM35 OUT | Input only |
| 39 | ADC1_3 | Voltage Div | Input only |
| 32 | GPIO32 | Charger Detect | Pull-up |
| 27 | GPIO27 | Relay IN | Digital out |
| 26 | GPIO26 | Fan PWM | PWM output |
| 21 | GPIO21 | LCD SDA | I2C |
| 22 | GPIO22 | LCD SCL | I2C |

---

## Test Points

| TP | Signal | Purpose |
|----|--------|---------|
| TP1 | VBUS | Battery voltage check |
| TP2 | 5V | 5V rail check |
| TP3 | 3.3V | 3.3V rail check |
| TP4 | LM35 | Temp sensor out |
| TP5 | ACS | Current sensor out |
| TP6 | GND | Common ground |

---

## Screw Terminals

| Terminal | Net | Current Rating |
|----------|-----|--------------|
| J1-A | VBUS+ | 5A |
| J1-B | VBUS- | 5A |
| J2-A | RELAY_NO | 3A |
| J2-B | LOAD+ | 3A |

---

## Component Placement (Top View)

```
+------------------------------------------+
|  [ESP32]    [Voltage Divider]  [J1]  |
|  Module         [R1][R2]      BAT   |
|                                          |
|  [LM35]     [ACS712]    [Relay]  [J2]  |
|                        [D2]     LOAD |
|                                          |
|  [C1][C2]       [LCD HDR]           |
|                                          |
|  [LED1] LED2                    [TEST]|
+------------------------------------------+
```

---

## BOM (Bill of Materials)

| Ref | Part | Value | Package | Qty | Notes |
|-----|------|-------|--------|-----|-------|
| U1 | ESP32 DevKit | - | Module | 1 | NodeMCU |
| U2 | LM35DZ | - | TO-92 | 1 | Temp sensor |
| U3 | ACS712 | 5A | SO-8 | 1 | Current sensor |
| K1 | HK4100F | 5V | Relay | 1 | Single channel |
| LCD1 | LCD1602 | I2C | I2C | 1 | 16x2 with backpack |
| R1 | MF Resistor | 30kΩ | 0805 | 1 | 1% metal film |
| R2 | MF Resistor | 7.5kΩ | 0805 | 1 | 1% metal film |
| R3 | MF Resistor | 10kΩ | 0805 | 1 | Pull-up |
| R4 | MF Resistor | 330Ω | 0805 | 1 | LED current |
| C1 | Ceramic | 100nF | 0805 | 1 | Decoupling |
| C2 | Electrolytic | 10µF | 0805 | 1 | Decoupling |
| D1 | Zener | 3.3V | SMD | 1 | Voltage clamp |
| D2 | Diode | 1N4007 | DO-41 | 1 | Flyback |
| LED1 | LED | Red | 0805 | 1 | Power indicator |
| LED2 | LED | Green | 0805 | 1 | Status |
| J1 | Terminal | 2P | 5.08mm | 1 | Battery input |
| J2 | Terminal | 2P | 5.08mm | 1 | Load output |
| F1 | Fuse | 3A | Resettable | 1 | PTC resettable |

---

## Estimated Costs

| Component | Unit Price (₹) | Qty | Total (₹) |
|-----------|-----------------|-----|------------|
| ESP32 DevKit | 350 | 1 | 350 |
| LM35DZ | 50 | 1 | 50 |
| ACS712-5A | 120 | 1 | 120 |
| Relay 5V | 40 | 1 | 40 |
| LCD I2C | 150 | 1 | 150 |
| PCB (JLCPCB) | 100 | 1 | 100 |
| Components | 150 | 1 | 150 |
| Enclosure | 200 | 1 | 200 |
| **Total** | | | | **1,160** |

---

## 3D Enclosure Design

### Dimensions
- **Material:** PETG or PLA (3D printable)
- **Size:** 100mm × 80mm × 35mm
- **Wall thickness:** 2mm

### Features
- LCD window cutout (aligned with display)
- Ventilation slots (top and sides)
- PCB mounting holes (M3 standoffs)
- Cable entry points (3mm diameter)
- Access holes for programming

### Design Notes
- Use FDM printing with 20% infill
- Add draft or sloped walls for easier printing
- Include snap-fit for battery compartment

---

## Thermal Analysis

### Heat Sources
1. **ESP32:** ~100mA max, generates heat during WiFi
2. **Relay coil:** ~70mA when energized
3. **Linear regulator:** If using 5V input, may need heatsink

### Cooling Strategy
1. **Passive:** PCB copper pour on bottom layer (ground plane)
2. **Active:** BLDC fan with PID control
3. **Interface:** Thermal paste between heat sources and enclosure

### Temperature Zones
- Safe: < 50°C
- Warning: 50-60°C
- Critical: > 60°C (triggers protection)