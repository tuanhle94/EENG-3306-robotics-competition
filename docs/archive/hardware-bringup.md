# First hardware test: motion-disabled diagnostics

This is the first test to perform on the physical robot. It identifies the IMU,
checks the button, reads analog pin A1, and displays all six floor-sensor
readings. The diagnostic continuously commands both motors to zero and does not
accept commands from the computer.

## Safety setup

1. Put the robot's power switch in the `OFF` position.
2. Place the robot securely on a box or stand so both tracks are in the air.
3. Make sure the tracks cannot touch clothing, cables, or the table.
4. Do not connect an external potentiometer yet.
5. Connect the Arduino Uno to the computer with its USB cable.

The batteries may remain installed, but leave the robot switch `OFF` for the
first test.

## Upload and open the monitor

Open PowerShell and enter:

```powershell
cd "C:\Users\Hang Minh Nguyen\source\repos\EENG-3306-robotics-competition"
powershell -ExecutionPolicy Bypass -File scripts\run-hardware-diagnostics.ps1
```

PlatformIO should build the program, locate the Uno, upload it, and open a
serial monitor. Press `Ctrl+C` to close the monitor.

If PlatformIO reports more than one serial device, find the Uno's port in
Windows Device Manager under **Ports (COM & LPT)**, then specify it. Replace
`COM4` (this robot's current port) with the port shown on your computer:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-hardware-diagnostics.ps1 -Port COM4
```

Windows can assign a new port number after reconnection or a USB-port change.
If the error says `could not open port` and `The system cannot find the file
specified`, check the current port in Device Manager and rerun with that
number. Reconnect USB if the Arduino is not listed. Do not erase calibration
or change firmware code to fix a missing port.

## Expected output

The first two lines should be similar to:

```text
ZUMO SAFE HARDWARE DIAGNOSTIC
Motors are locked at zero. Baud rate: 115200.
```

The program then prints a line twice per second:

```text
DIAG imu=v1.3-LSM6DS33+LIS3MDL imu_ok=yes button=released A1=123 sensors=10,20,30,40,50,60 emitter_mode=always-on
```

The numbers will differ on the real robot.

This robot does not have the optional LEDON control header installed. Its
infrared sensor emitters are therefore always powered while the board is on.
Infrared light is normally invisible to human eyes, so no visible red light is
required for this test.

## Three small checks

### 1. IMU

Pass: `imu_ok=yes` and the IMU name is not `unknown`.

Fail: `imu_ok=NO`, `imu=unknown`, or the LED flashes very quickly. Leave the
power switch off and save the complete output.

### 2. User button

Watch `button=released`, press and hold the Zumo user button, and verify that it
changes to `button=PRESSED`. Release it and verify that it returns to
`button=released`.

### 3. Floor sensors

Place plain white paper a few millimeters below the sensor array and record one
line of output. Then place a wide strip of matte black electrical tape below
all six sensors and record another line. The six `sensors` values should
change clearly between white and black.

Do not worry about exact sensor numbers yet. Calibration comes later.

## What to send back

Copy and send:

1. One complete `DIAG` line with the button released.
2. One complete line while holding the button.
3. One line over white paper.
4. One line over black tape.
5. Any red error text printed by PlatformIO.

Do not test the tracks or turn the robot power switch on yet. The next checkpoint
will interpret these readings and test whether A1 is connected to the optional
battery divider.

## Recorded result: 2026-10-19

This robot passed the first diagnostic checkpoint:

- IMU: `v1.3-LSM6DS33+LIS3MDL`, `imu_ok=yes`.
- User button: changes between `released` and `PRESSED`.
- White surface: representative readings
  `1376,860,604,736,860,1244`.
- Matte black surface: `2000,2000,2000,2000,2000,2000`.

All six floor-sensor channels respond and distinguish white from black. The A1
number is not a battery measurement: a controlled test produced the same
approximately 429-446 range with switched battery power on and off. This means
the battery-monitor jumper is open and A1 is floating until an external speed
potentiometer is connected.
