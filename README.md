# 2026 Ratliff Relays robot

Arduino firmware for a Pololu Zumo robot with an Arduino Uno that explores a
line maze in round one and drives the shortest learned route in round two.

Start with [`docs/team-handout.md`](docs/team-handout.md): how the robot works,
the competition rules, how to run it and how to troubleshoot it. The other
documents are listed in [`docs/README.md`](docs/README.md).

## Competition program

`src/pololu_maze_firmware.cpp` (build `uno_pololu_maze`, the only build) is
based on the MazeSolver example in Pololu's ZumoShield library. It adds a
speed knob on A1, settings for 1-inch tape, a calibration check, centering on
the line, turns that cannot stop early, a line search after turns, a round-two
junction check with a fallback, a path saved in EEPROM for round two, a
10-second erase, a stop button, time limits and a run record printed at the
next power on.

Upload it (battery OFF, USB in) and open the serial monitor:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4
```

Read the last run's record without uploading:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -MonitorOnly
```

`-Pololu` is still accepted, so older instructions keep working.

## Build

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html), then run:

```text
pio run
```

## History

Our first program (gyro turns and a grid map with shortest-path search) and
its unit tests, bench tests and simulator were removed on 2026-10-24. They
remain in git history (for example tag `known-good-2026-10-23`), and their
documents are in [`docs/archive/`](docs/archive/).
