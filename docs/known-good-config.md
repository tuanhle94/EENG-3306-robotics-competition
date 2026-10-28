# Known-good configurations

Restore one of these whenever a change makes the robot worse. Change one
setting at a time from the newest one.

## Competition version: both rounds without the cable (newest)

On 2026-10-23 this build completed **round one and round two without the
USB cable** on the competition maze: the full competition flow.

- **Git tag:** `competition-ready-2026-10-23` (same code as
  `round-one-turn-fix-2026-10-23`, commit `92c5b7f`).
- **Knob:** 369 of 1023, speed **188**, fresh batteries, the same for both
  rounds. With older batteries a higher setting (about 200) matches.
- **Round one:** 38 junctions, FINISH, 23-move path
  `LLRRLRLSRSLRLLRLLSRRSSL` (details below).
- **Round two:** drove the saved path to FINISH, reported by the operator.
  The robot's own record of this run is not yet saved.

To restore it:

```powershell
git checkout competition-ready-2026-10-23
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -Pololu
git checkout main
```

## Round one with the turn fix, without the cable

On 2026-10-23 round one completed **without the cable** with the turn fix:
a turn first leaves any line already under its watched sensor, so it can no
longer stop after a few degrees.

- **Git tag:** `round-one-turn-fix-2026-10-23`.
- **Knob:** 369 of 1023, speed **188**, fresh batteries. Keep this setting
  for round two.
- **Result:** 38 junctions, FINISH at the 38th, 23-move path
  `LLRRLRLSRSLRLLRLLSRRSSL`; supply 5080 mV; weakest calibrated sensor
  range 1328.
- **Log:** [runs/2026-10-23-round-one-turn-fix-188.log](runs/2026-10-23-round-one-turn-fix-188.log).
- Round two with this path: completed without the cable (above).

## Round one without the cable at 188 with fresh batteries

On 2026-10-23, after the day's fixes (calibration check, line search after
turns, round-two exit check and fallback, straight-only junctions ignored),
round one completed **without the cable** with fresh batteries.

- **Git tag:** `round-one-untethered-188-2026-10-23`.
- **Knob:** 378 of 1023, speed **188**. Fresh batteries make every knob
  setting faster, so this lower setting matched the earlier behaviour at
  200; use the same setting for round two.
- **Result:** 39 junctions, FINISH at the 39th, 24-move path
  `LLLRRLRLSRSLRLLRLLSRRSSL`; supply 5080 mV throughout; weakest calibrated
  sensor range 1664.
- **Log:** [runs/2026-10-23-untethered-round-one-speed-188.log](runs/2026-10-23-untethered-round-one-speed-188.log).
- Round two with this build: not yet run.

To restore it: `git checkout round-one-untethered-188-2026-10-23`, upload with
`-Pololu`, then `git checkout competition-two-round-run`.

## Proof of concept: round one without the cable

On 2026-10-23 the robot completed **round one without the USB cable** on the
competition maze, and the same build completed round one with the cable just
before. This is the proof of concept: autonomous, battery-only, exploring
the full maze and saving the round-two path.

- **Git tag:** `poc-untethered-2026-10-23`.
- **Build:** the two-round version below plus one change: after the
  calibration sweep the robot turns slowly until the line is under its middle
  sensors (within 500 of 2500), pauses 100 ms, checks again, then drives. If
  it finds no line within 3 s it stops (NO_LINE_AT_START).
- **Speed:** the knob was not fitted, so A1 floated; the robot ran at 164
  from that noise. Later builds fix the speed at 200 until the knob is fitted.
- **Logs:** [untethered round one record](runs/2026-10-23-untethered-round-one.log),
  [tethered round one, same build](runs/2026-10-23-tethered-round-one-centering.log).

| Run | Cable | Speed | After sweep / after centering | Result |
|---|---|---|---|---|
| Round one | Yes | 164 | 2043 / 2158 | 49 features, FINISH, 26-move path saved |
| Round one | **No** | 164 | 2040 / 2140 | 49 features, FINISH, the same 26-move path |

- Learned path both times: `RLSSSRRLRLSRSLRLLRLLSRRSSL`, identical to the
  first two-round success, so exploration is repeatable.
- Round two without the cable is not yet recorded for this build.
- Before this build, untethered starts failed: rocking timeouts, a circling
  run that saved 34 left turns, and a mid-run restart. Low random speeds
  (160-171) from the unconnected knob pin and battery supply sag are the
  suspects; the next build records the supply voltage.

To put this exact version back on the robot:

```powershell
git checkout poc-untethered-2026-10-23
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -Pololu
git checkout competition-two-round-run
```

## First two-round success (with the cable)

On 2026-10-23 the robot completed **round one and round two** on the
competition maze with this configuration. It is the baseline for speeding up
and optimizing: change one setting at a time, and if a change makes things
worse, return to this one.

- **Git tag:** `known-good-2026-10-23` (commit `1e90283`; the competition
  program last changed in `ea6cd83`).
- **Full serial log:** [runs/2026-10-23-two-round-success.log](runs/2026-10-23-two-round-success.log).

To put this exact version back on the robot:

```powershell
git checkout known-good-2026-10-23
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -Pololu
```

Then return to the working branch with `git checkout competition-two-round-run`.

### Result

| Round | A1 reading | Motor speed (of 400) | Result |
|---|---|---|---|
| One | 298 of 1023 | 166 | Explored 48 junctions and dead ends, found FINISH at the 49th feature, saved a 26-move path |
| Two | 370 of 1023 | 177 | Drove the 26 saved moves and stopped on FINISH |

- The A1 readings came from an unconnected pin (no knob fitted), so the
  speeds 166 and 177 were set by noise, not chosen.
- Learned path: `RLSSSRRLRLSRSLRLLRLLSRRSSL` (26 moves).
- Round one: 5 dead ends (U-turns), every turn between 387 and 587 ms, U-turns
  1121 to 1552 ms.
- Round two: turns 348 to 463 ms.
- FINISH read all six sensors at 1000 at the check point, so it was not
  confused with a crossing.
- Times from start to FINISH were not measured; time them with a stopwatch
  next.
- Before this run, two round-one starts at speed 160 and 163 timed out during
  the start-up rocking (calibration). The cause was not confirmed (battery
  switch, battery level or placement); the next start, at 166, worked.

### Program and build

| Item | Value |
|---|---|
| Source | `src/pololu_maze_firmware.cpp` |
| Build | `uno_pololu_maze` (`platformio.ini`), ZumoShield library 2.1.0 |
| Upload | `scripts\run-competition-firmware.ps1 -Port COM4 -Pololu` |
| Program memory | 39% of 32 KB; working memory 22% of 2 KB |

### Settings

| Setting | Value | Meaning |
|---|---|---|
| `SENSOR_THRESHOLD` | 300 (of 1000) | Calibrated reading above this counts as black |
| `SPEED_MIN`, `SPEED_MAX` | 120, 280 | Speed range of the knob on A1 |
| Speed equation | speed = 120 + reading x 160 / 1023 | Driving and turning both use it; knob read at power on and at every start |
| `LINE_THICKNESS` | 1.0 inch | Tape width on the competition maze |
| `CHECK_DISTANCE` | 1.5 inches | How far past a crossing's first edge it checks for a straight exit and FINISH; round two drives the same distance before turning |
| `INCHES_TO_ZUNITS` | 17142 | Pololu's distance-per-time constant; drive-in time = 17142 x 1.5 / speed, which is 155 ms at 166 and 145 ms at 177 |
| Line following | steering = (position - 2500) / 3, capped at the speed; neither motor reverses | Pololu's MazeSolver controller, unchanged |
| Turns | Spin at the speed until sensor 1 (left, U-turn) or sensor 4 (right) crosses the line | Pololu's MazeSolver turn, unchanged |
| `kTurnTimeoutMs` | 3000 ms | A turn or calibration sweep longer than this stops the run |
| Calibration | Pololu's sweep: rock left and right twice over the START line, then turn left onto it | Every start |
| Strategy | Left-hand rule; path shortened by removing each U-turn | Pololu's MazeSolver, unchanged |
| Start button | Under 2 s starts; hold 10 s erases the path | |
| Hands-off delay | 1000 ms | Between the press and the sweep |
| Path storage | EEPROM 400 to 503 | Survives power off |
| Run record | EEPROM 512 to 1023 | Printed at the next power on |

### Course and robot as tested

| Item | Value |
|---|---|
| Maze | Competition maze, Houston Engineering Center |
| Black tape | 1 inch wide |
| FINISH | Black rectangle about 4 x 2 inches |
| Light non-path tape | Reads under 100 of 1000 after calibration (ignored) |
| Robot | Arduino Uno, Zumo Shield, 75:1 gearmotors, 6-sensor array, 4 AA batteries |
| Speed knob | **Not fitted.** A1 was unconnected, so its readings (and these speeds) were electrical noise. Later builds use a fixed 200 until the knob is fitted. |
| Cable | USB connected during this run |

### Not yet tested with this configuration

- Both rounds without the USB cable.
- A second repeat, to show it is repeatable.
- Speeds above 177.
