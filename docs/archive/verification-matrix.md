# Verification matrix

This table connects every test to the requirements it supports. Look up a
requirement ID in [specification.md](specification.md), then find that ID in the
second column below. A requirement may need more than one test level; a passing
simulation does not replace a required physical test.

## Test ID prefixes

| Prefix | Meaning |
|---|---|
| `UT` | Unit test: tests one small piece of code on the computer |
| `SIM` | Simulator test: tests several pieces together with virtual inputs |
| `IT` | Integration test: tests software with the physical robot or course |
| `E2E` | End-to-end test: performs the complete two-round workflow |
| `FT` | Fault test: deliberately creates a problem and checks the safe response |

**Implemented** means the automated test exists and currently passes. **Not
started** means the test procedure or required hardware work is not complete.
**Awaiting hardware** means useful software testing is complete, but a real
robot or practice course is still required.

## Current bottom line

The unit tests and full simulator pass. Initial physical checks have verified
the IMU, user button, reflectance-sensor calibration, straight motor motion,
one smooth three-second straight-line following trial, and individual
gyro-controlled LEFT and RIGHT quarter-turns, and an individual LEFT U-turn.
Initial junction classifications and saved-calibration reloads have also been
observed. Repeatability, integrated turn placement, route persistence on the
robot, and the complete two-round run remain unfinished.

| Test ID | Requirements | Level | Status |
|---|---|---|---|
| UT-SPEED-01 | C-08, F-10 | Native unit | Implemented |
| UT-LINE-01 | C-02, F-20 | Native unit | Implemented |
| UT-JUNCTION-01 | C-04, F-21 | Native unit | Implemented; sequence distinguishes a four-way crossing from a dead end |
| UT-JUNCTION-TIMING-01 | F-25 | Native unit | Implemented; six tests cover first-outer timing, branch memory, both inner-corner directions, configurable duration, scan deadlines, reset, and timer wraparound |
| IT-JUNCTION-PLACEMENT-01 | C-04, F-21, F-22, F-25 | Competition first-turn inspection | Pending; trial 343 ms advance must place the sensors on the outgoing tape after the measured turn |
| UT-CONTROL-01 | F-12, F-13 | Native unit | Implemented |
| UT-GRAPH-01 | F-30, F-31, S-03 | Native unit | Implemented |
| UT-PATH-01 | C-07, F-33 | Native unit | Implemented |
| UT-EXPLORE-01 | C-10, F-29, F-32 | Native unit | Implemented |
| IT-CAL-01 | F-02 | Robot bench | Hardware verified; all six spans exceed 500 counts |
| IT-IMU-01 | F-04 | Robot bench | Hardware verified; v1.3 IMU detected successfully |
| IT-MOTOR-01 | C-03, F-13 | Robot bench | Hardware verified at command 100; equal commands moved straight |
| IT-LINE-01 | F-12, F-13 | Straight practice line | Initial hardware pass: 1/1 smooth three-second run at command 100; repeated and curved-line trials remain |
| IT-JUNCTION-01 | C-04, F-21 | Practice course | Initial physical passes: four-way `LSR`, T-junction `L-R`, right corner `--R`, and left corner `L--`; FINISH and repetitions remain |
| IT-TURN-01 | C-04, F-22 | Robot bench | Initial hardware pass: LEFT stopped at 88.1 degrees, RIGHT stopped at 88.0 degrees, and LEFT U-turn stopped at 178.1 degrees; repetition remains |
| IT-DEADEND-01 | C-04, F-23 | Practice course | Initial physical pass: one tape endpoint correctly reported `DEAD_END` and stopped automatically |
| SIM-BRANCH-01 | F-29, F-32, F-33 | Simulator | Implemented |
| SIM-LOOP-01 | C-07, F-30, F-32, F-33 | Simulator | Implemented under regular-grid assumption; hardware validation remains |
| SIM-MISSION-01 | F-01, F-02, F-10, F-12, F-13, F-20, F-29 through F-35 | Simulator | Implemented; full learn, persist, reset, and replay lifecycle passes |
| SIM-FAULT-01 | F-04, F-36, S-01, S-02 | Simulator | Implemented; IMU, calibration, corruption, mismatch, and timeout faults stop motion |
| IT-LOOP-01 | C-07, F-32 | Practice course | Awaiting hardware/course validation |
| IT-EEPROM-01 | C-05, F-34 | Robot bench | Calibration and learned route (length 7) both saved and reloaded after reset, 2026-10-21 |
| E2E-ROUNDS-01 | C-01, C-05, C-06, F-35 | Practice course | 2/2 complete two-round runs on two practice mazes (7- and 11-step routes), 2026-10-21 (USB tethered). First untethered round one (2026-10-22) stopped at a phantom right corner after a straight; fix (branch hold after straights) not yet run on the robot. Competition maze pending |
| IT-RECORDER-01 | F-36 | Practice course | Run recorder printed a complete untethered round one at the next power-up, 2026-10-22; `branch_trigger` lines not yet seen on the robot |
| IT-AUTOCAL-01 | F-02 | Practice course | Automatic calibration twist passed in the 2026-10-22 untethered run (smallest span 1590, ended 6 degrees off start) |
| FT-SAFE-01 | C-09, F-36, S-01, S-02 | Physical robot fault injection | Not started |

The first integrated physical run on 2026-10-20 followed into a four-way
feature, selected left, completed an 88.4-degree turn, reacquired the outgoing
line, detected a dead end, and completed a 178.2-degree reversal. The run then
stopped safely on a line-reacquisition timeout, so the complete maneuver remains
in progress rather than passed.

The `uno_competition` integrated candidate compiles for the Arduino Uno. Its
checksummed EEPROM formats cover both the learned route and reusable sensor
calibration. This confirms that the code fits the board, but it does not change
the physical end-to-end tests above from `Not started`.

2026-10-21 reference-timing check: all 42 native tests pass. Competition,
first-turn inspection, static-sensor inspection, and standalone junction
diagnostic builds compile successfully. The competition build uses 965 bytes
of static RAM and 22,422 bytes of flash. These checks do not measure travel
distance or establish physical maze success. The new test instructions are in
[corner-inspection.md](corner-inspection.md).

