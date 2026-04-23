# Circuit Schematic (ASCII Diagram)

## Main Power Circuit

```
                    +------------------+
                    |     Battery      |
                    |  11.1V (4x3)    |
                    |     11.76Ah     |
                    +--------+--------+
                             |
                        J1   |
                   +--------+|+--------+
                   | VBUS+          |
                   +--------+--------+
                             |
                        F1 (3A PTC)
                             |
                        +----+----+
                        |        |
                       D1      K1 (NO)
                   Zener   Relay
                   3.3V   Coil
                        |        |
                        +----+----+
                             |
                        J2-A (LOAD+)
                             |
                         [LOAD]
                         Fan/Motor
                             |
                        +----+----+
                        |        |
                        J2-B    |
                   VBUS-      |
                        +----+----+
                        |        |
                        K1 (COM)
                        +--------+
                             |
                   +--------+--------+
                   | J1 (VBUS-)    |
                   +------------------+
```

## Sensor Circuits

### Voltage Divider (GPIO 39)
```
GPIO39--[R1=30k]--+
                 |
                +--[R2=7.5k]--GND
                 |
                +--[D1=Zener3.3v]--GND (Protection)
```

### Temperature Sensor (LM35 - GPIO 34)
```
+3.3V--+----[LM35]----+
       |      |       |
      OUT    GND     |
       |      |       |
      34-----+--------+--GND
```

### Current Sensor (ACS712 - GPIO 36)
```
+5V----+-------+
       |       |
      IP+    |
       |       |
       |      +--[VCC]--+
       |      |          |
      OUT     GND      |
       |      |       |
      36-----+--------+--GND
```

## Relay Driver Circuit (GPIO 27)
```
GPIO27--[R3=330]--Base
                   |
                  Q1 (NPN)
                   |
                  GND
                   |
              Relay Coil
                   |
               +--[D2=Flyback]--GND
```

## LCD I2C Interface (GPIO 21, 22)
```
+5V--+--[I2C LCD]----+
     |     |         |
    SDA   SCL      GND
      |     |         |
     21    22--------+--GND
```

## Complete Pinout Summary

```
ESP32 Pin Connections:
+-------------------+-----------------------+
| GPIO  | Component                |
+-------------------+-----------------------+
| 3.3V | LM35 VCC, ESP32 Power    |
| 5V   | ACS712 VCC, Relay VCC   |
| GND   | All grounds            |
| 34   | LM35 OUT              |
| 36   | ACS712 OUT            |
| 39   | Voltage Divider       |
| 32   | Charger Detect (pullup)|
| 27   | Relay IN              |
| 26   | Fan PWM               |
| 21   | LCD SDA (I2C)         |
| 22   | LCD SCL (I2C)         |
+-------------------+-----------------------+
```

## Power Budget

| Component | Voltage | Current (max) | Power |
|-----------|---------|---------------|-------|
| ESP32 | 3.3V | 500mA | 1.65W |
| LCD Backlight | 5V | 50mA | 0.25W |
| Relay Coil | 5V | 70mA | 0.35W |
| ACS712 | 5V | 10mA | 0.05W |
| LM35 | 3.3V | 10µA | negligible |
| Fan | 12V | 200mA | 2.4W (external) |
| **Total** | | **~630mA** | **~4.7W** |

> Note: Fan uses external 12V supply, not from USB