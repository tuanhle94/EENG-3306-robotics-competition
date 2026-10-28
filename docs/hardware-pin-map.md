# Hardware pin map

Use this document when connecting hardware or diagnosing a pin conflict. A pin
is a numbered electrical connection on the Arduino. `D` means digital pin and
`A` means analog-input pin. The same physical pin must not be assigned two
incompatible jobs.

This map is derived from the local `ZumoShield` 2.1.0 source and applies to the
Pololu Zumo Robot for Arduino (#2510) with the team's confirmed Arduino Uno.
The Leonardo column is retained only as a warning against using the wrong
jumper position or build target.

The project uses the **Arduino Uno column**. Do not copy the Leonardo settings.

| Function | Arduino Uno / ATmega328P | Leonardo / ATmega32U4 | Notes |
|---|---:|---:|---|
| Left motor direction | D8 | D8 | Zumo Shield fixed connection |
| Left motor PWM | D10 | D10 | Timer1 is used by `ZumoMotors` |
| Right motor direction | D7 | D7 | Zumo Shield fixed connection |
| Right motor PWM | D9 | D9 | Timer1 is used by `ZumoMotors` |
| User button | D12 | D12 | Press pulls low through 1 kohm |
| User LED | D13 | D13 | High turns LED on |
| Reflectance sensors 0..5 | D4, A3, D11, A0, A2, D5 | Same logical pins | Library default order |
| Reflectance emitter control | D2 | A4 | LEDON jumper must match controller |
| IMU I2C | A4/A5 | D2/D3 | Do not attach the potentiometer here |
| External speed potentiometer | A1 | A1 | Battery-monitor jumper must remain open |

The battery-monitor jumper is disconnected by default. Verify it visually with
power removed before connecting the potentiometer wiper to A1.

## Terms used in the table

- **PWM:** a rapid on/off signal used to control motor power.
- **Timer1:** a timing circuit inside the Uno that produces the motor PWM
  signals. Avoid assigning another library that also requires Timer1.
- **Pulls low:** pressing the button changes its input from a high voltage to a
  low voltage. The program therefore treats `LOW` as pressed.
- **I2C:** the two-wire communication link used by the Arduino and IMU.
- **Emitter:** the infrared LEDs that shine light onto the floor for the
  reflectance sensors.
- **Jumper:** a small solder or wire connection that selects a hardware option.
- **Wiper:** the adjustable middle connection of a potentiometer.
- **1 kohm:** 1 kilo-ohm, or 1,000 ohms of electrical resistance.

## Library provenance

This section records where the pin information came from. It is included so a
future student can repeat the check instead of trusting an unexplained table.

The supplied local repository at
`C:\Users\Hang Minh Nguyen\source\repos\zumo-shield-arduino-library-master`
identifies itself as version 2.1.0. SHA-256 comparison of its manifest, umbrella
header, IMU implementation, reflectance-array header, and motor implementation
matches the PlatformIO package used by this project byte-for-byte.

