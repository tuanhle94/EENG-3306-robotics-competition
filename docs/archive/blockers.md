# Open questions and evidence checklist

Last reviewed: 2026-10-19

This file is the single tracking list for missing decisions and evidence. The
competition PDF is the only organizer specification currently available. An
unknown therefore does **not** stop software development unless safe progress is
impossible. We adopt a documented, configurable default and continue; later
course information or bench evidence can replace that default.

For definitions of common robotics and Arduino terms, see
[README.md](../README.md).

A blocker is closed only when its resolution and supporting evidence are
recorded here. Do not delete closed items; change their status so the decision
history remains available.

In this document, **blocker** means "something that must be resolved before the
robot can be called competition-ready." It does not always mean that all work
must stop. For example, hardware checks remain open because the robot has not
arrived, but software development and simulation can continue.

## What the team needs to do now

- Continue implementing and simulating the firmware.
- Complete hardware check B-13 before enabling normal competition movement.
- During practice-course testing, record evidence for B-20 through B-26 in
  [tuning-log.md](../tuning-log.md).
- Revisit B-01 through B-03 if the instructor supplies additional course
  details.

## Status definitions

- `Open`: information or a decision is still required.
- `In progress`: an owner is actively obtaining or validating it.
- `Resolved`: the resolution and evidence are recorded.
- `Accepted risk`: the team knowingly proceeds with the documented fallback.

An accepted risk is not the same as a solved problem. It means the team has a
reasonable temporary assumption and a safe response if that assumption is
wrong.

## Working policy while organizer details are unavailable

- Build and simulate against the assumptions below.
- Keep course-dependent thresholds and timing in configuration rather than
  hard-coding an official maze layout.
- Stop safely when observations contradict an assumption.
- Update this register if the instructor publishes a final course specification.

## Competition and course blockers

| ID | Priority | Missing information | Why it matters | Owner/source | Working decision or required result | Status |
|---|---|---|---|---|---|---|
| B-01 | Critical | Exact FINISH marker and detection condition | A large black marker can look like a four-way intersection. Incorrect detection could end round one early. | Competition PDF plus Pololu reference behavior | Assume a broad black area detected across all six sensors for a sustained, configurable interval after junction probing | Accepted risk |
| B-02 | Critical | Course geometry: line width, minimum segment length, intersection spacing, and whether it is a regular grid | The robot has no wheel encoders. Geometry affects time-based position estimates. | Competition PDF | Assume paths meet at 90-degree angles on a regular grid; keep width, spacing, and travel-time values configurable and calibrate them on the practice course | Accepted risk |
| B-03 | Critical | Method for recognizing revisited intersections | Six reflectance values do not uniquely identify identical junctions. | Engineering decision | Develop gyro-relative heading plus calibrated segment-travel coordinates; verify in simulation before hardware use | In progress |
| B-04 | High | Round transition procedure | The rules allow judge-approved reset/restart but do not promise that RAM remains powered. | Competition PDF | Persist the learned route with checksum in EEPROM; normal button press starts the valid stored round-two route | Resolved |
| B-05 | High | Official START procedure | The PDF refers to an official start signal without defining an electrical interface. | Competition PDF | Use the local Zumo button as the start acknowledgement; no remote-control input exists | Resolved |
| B-06 | High | Maximum maze complexity | The official upper bound is not supplied. | Engineering decision | Support 32 nodes initially, detect overflow, stop safely, and increase only if SRAM measurements allow | Accepted risk |
| B-07 | High | Eligibility/scoring treatment of assembled Pololu item #2510 | The written rules name kit #2509, while the team has identified #2510 as the exact assigned robot. | Team confirmation | Treat #2510 as authoritative for implementation; retain the discrepancy for poster/judging awareness | Resolved |
| B-08 | Medium | Intersection and path feature set | Exact layouts are withheld. | Competition PDF | Support straight segments, 90-degree turns, intersections, branches, dead ends, and loops; reject unsupported geometry safely | Resolved |
| B-09 | Critical | Whether round one ends on the first FINISH encounter | Continuing after reaching FINISH is not explicitly authorized. | Competition PDF | Stop at the first confirmed FINISH and compute the shortest route in the graph learned up to that point | Resolved |

## Hardware checks

| ID | Priority | Missing information | Why it matters | Owner/source | Required result | Status |
|---|---|---|---|---|---|---|
| B-10 | High | Physical Zumo Shield revision | The original shield has no gyro; v1.2 and v1.3 have different gyro hardware. | Team inspection plus firmware diagnostic | Diagnostic repeatedly reports `v1.3-LSM6DS33+LIS3MDL` and `imu_ok=yes` | Resolved |
| B-11 | High | Reflectance emitter configuration | Firmware must match whether Arduino D2 can control the emitters. | Team inspection plus white/black test | Optional LEDON control header is not installed, so emitters are always on; firmware uses `QTR_NO_EMITTER_PIN` | Resolved |
| B-12 | High | A1 battery-monitor jumper state | A1 cannot read both the battery divider and external speed potentiometer. | Controlled switch test | A1 remained in the same approximately 429-446 range with battery power switched on and off, so it is floating and the battery-monitor jumper is open | Resolved |
| B-13 | High | External potentiometer selection and mounting | Electrical range, stable ADC readings, insulation, and competition adjustability depend on the physical component. | Team/instructor | Record resistance, wiring, mounting method, and approval if not using the default implementation | Open |
| B-14 | Medium | Battery chemistry and operating voltage | Alkaline versus NiMH cells change motor behavior and tuning. | Team | Four non-rechargeable AA alkaline cells are installed: 6 V nominal; replace rather than recharge | Resolved |
| B-15 | Medium | Motor polarity and drivetrain condition | Reversed polarity or unequal friction invalidates steering assumptions. | Team bench test | Equal positive commands at speed 100 moved the robot straight toward the blade for 400 ms; no library flip is required | Resolved |

## Calibration evidence still required

These items do not prevent implementation or simulator development. They are
required only before declaring the firmware competition-ready.

Useful terms in the tables:

- **Gyro:** the rotation sensor inside the IMU.
- **Wheel encoder:** a sensor that measures wheel rotation; this robot does not
  have one.
- **ADC sweep:** slowly turn the potentiometer and record its readings from 0
  through 1023.
- **Stiction:** the friction that must be overcome before a stopped motor begins
  moving.
- **EEPROM:** memory that keeps the saved route after a reset or power loss.
- **Monotonic response:** turning the speed knob in one direction always moves
  the requested speed in one direction; it does not jump backward unexpectedly.
- **Motor polarity:** which electrical direction makes a motor drive its track
  forward.
- **Drivetrain:** the motors, gears, wheels, and tracks that move the robot.

| ID | Priority | Evidence needed | Pass condition | Status |
|---|---|---|---|---|
| B-20 | High | Six-sensor calibration ranges | Every sensor observes both line and background with an accepted minimum span | Resolved: all six moving-calibration spans exceed 500 counts |
| B-21 | High | Potentiometer ADC sweep | Stable usable range with monotonic motor-speed response | Open |
| B-22 | High | Minimum and maximum safe speeds | Minimum overcomes stiction; maximum completes the practice course reliably | Open |
| B-23 | High | PD line-controller tuning | Repeated line following at low, medium, and high pot settings without line loss | In progress: first three-second straight-line run passed smoothly at fixed command 100 |
| B-24 | High | Turn and reversal tuning | Ten consecutive successful left, right, and 180-degree controlled turns | In progress: command 100 produced only a small physical pivot; the next bounded test uses command 200 for at most two seconds |
| B-25 | High | Junction/dead-end/FINISH classification | Ten consecutive correct classifications for every official feature type | Open |
| B-26 | High | End-to-end two-round trial | Explore, retain, reset as permitted, and replay the shortest learned route without intervention | Open |

## Resolved decisions

| ID | Decision | Resolution | Evidence | Status |
|---|---|---|---|---|
| R-01 | Base robot | Pololu Zumo Robot for Arduino, assembled item #2510 with 75:1 HP motors | Team-provided product identification | Resolved |
| R-02 | Controller | Arduino Uno/ATmega328P | Team confirmation; Uno build succeeds | Resolved |
| R-03 | Software library | Pololu `ZumoShield` 2.1.0 | Supplied source matches inspected PlatformIO package files byte-for-byte | Resolved |
| R-04 | Host-side testing | PlatformIO native tests plus a deterministic full-mission simulator | Twenty-two native tests and all simulator scenarios pass as of 2026-10-19 | Resolved |

## Hardware evidence recorded on 2026-10-19

- B-10: the diagnostic repeatedly identified the IMU as
  `v1.3-LSM6DS33+LIS3MDL` with `imu_ok=yes`.
- B-11: photographs show the reflectance board installed without the optional
  LEDON control header. The nearly identical former `sensors_on` and
  `sensors_off` readings confirm that D2 does not switch the emitters.
- B-14: a photograph shows four AA alkaline cells marked non-rechargeable.
- B-12: with no external potentiometer attached, A1 stayed in the same range
  when switched battery voltage was turned on and off. The official divider
  uses switched `VBAT`, so a connected divider would have produced a clear
  change. A1 is available for the external speed potentiometer.
- B-20 preliminary result: representative white readings were
  `1376,860,604,736,860,1244`; matte black produced
  `2000,2000,2000,2000,2000,2000`. All six channels distinguish the two
  surfaces. This is a connectivity/contrast pass, not the final moving
  calibration.
- B-15: the gated motor diagnostic commanded both tracks forward at 100 for
  400 ms. The robot moved straight and stopped automatically at the timeout.
  Positive left and right commands therefore match physical forward motion;
  no polarity flip is configured.
- B-20: the eight-second manual moving calibration collected 3567 samples.
  Minimums were `1288,1208,1000,1208,1132,1208`, maximums were all `2000`,
  and spans were `712,792,1000,792,868,792`. Every channel exceeded the
  required 500-count span, so the diagnostic reported `CAL result=PASS`.
- B-23 partial evidence: after reducing the diagnostic controller to
  proportional 1/12 and derivative 1, the robot completed one three-second
  straight-line run at command 100. It stayed on the black line, moved
  smoothly, reported `peak_error=508`, and stopped automatically. B-23 remains
  open until repeated, curved-line, and multiple-speed trials pass.
- B-23 curve evidence: the first gentle-curve attempt did not complete the
  curve. The line estimate reached 5000 while commands saturated at `100,0`.
  The next diagnostic permits bounded reverse correction and detects a
  persistent extreme-edge condition instead of treating timeout as success.
- B-23 threshold evidence: a curve retry stopped on normalized readings
  `0,0,0,130,0,0` because the original 200-count presence threshold was too
  high. The revised threshold accepts 100 or greater and has a regression test
  for the measured 130-count signal.
- B-20 follow-up: hard-floor readings confirmed the Pololu pin order but showed
  a weakest outside-sensor span of 156 counts on the current paper. The active
  diagnostic minimum is now 150 counts, backed by a regression test using the
  measured six-channel values. The earlier 500-count sweep remains recorded as
  historical evidence from a different placement/surface condition.
- B-23 curve retry: bounded reverse commands around `100,-53` turned the robot
  only slightly while position remained near 4345. The next controller uses a
  full bounded pivot at absolute error 1600 and stops after 750 ms if that
  recovery does not move the line away from the edge.
- B-23 full-pivot follow-up: the controller repeatedly exited recovery as soon
  as position moved just below the entry boundary, so the robot still turned
  only slightly. Recovery now uses an 800-count exit boundary and derivative
  gain zero to prevent threshold chatter and near-center command spikes.
- B-24 initial evidence: both dedicated LEFT and RIGHT pivot attempts at motor
  command 100 timed out with zero sensor transitions, and the robot visibly
  rotated only a little. The next turn-only diagnostic uses command 200 with a
  two-second automatic stop; line-following speed remains 100.
- B-24 command-200 evidence: the stronger pivot rotated about 180 degrees. The
  tracker was waiting to enter and then leave the new line, which was too late
  for the physical corner. It now stops after three consecutive dark readings
  on the new line; a physical 90-degree retest is still required.
- B-24 line-stop follow-up: stopping after stable new-line detection still
  produced about 180 degrees because the crossing point depended on physical
  placement. The isolated turn diagnostic now measures rotation directly with
  the working LSM6DS33 gyro, slows after 70 degrees, and targets 88 degrees.
  Physical gyro-turn verification remains required.
- B-24 first gyro result: the LEFT diagnostic slowed after 70 degrees and
  stopped automatically at a measured 88.0 degrees with `ANGLE_REACHED` before
  the two-second timeout. The tester confirmed that the physical motion was an
  approximately 90-degree quarter-turn. A RIGHT trial and repeated trials
  remain before B-24 can be resolved.
- B-24 first RIGHT gyro result: the turn reached 71.6 degrees, but command 100
  could not continue the in-place pivot. The safety timeout stopped it at 72.2
  degrees. The next bounded trial raises only the slow-turn command to 150.
- B-24 verified quarter-turn checkpoint: slow command 150 produced a LEFT stop
  at 88.1 degrees and a RIGHT stop at 88.0 degrees. Both were visually
  confirmed as correct quarter-turns. A separate bounded 180-degree diagnostic
  is the next physical test.
- B-24 first U-turn result: the LEFT dead-end reversal slowed at 150 degrees
  and stopped automatically at 178.1 degrees with `ANGLE_REACHED`. Production
  U-turns will consistently use LEFT. Repeated reliability trials remain before
  B-24 is fully resolved.
- B-25 implementation checkpoint: a bounded command-100 diagnostic now follows
  one incoming line, latches left/right branches, moves beyond the wide tape,
  checks for a straight continuation, reports exits, and stops. Its native
  sequence test passes and the Uno firmware compiles; physical classification
  evidence is still required.
- B-25 first physical attempt: the approach falsely reported `DEAD_END` when
  the narrow line briefly fell between sensors 3 and 2. The diagnostic now
  steers toward the estimator's last-known side during the existing 150 ms
  grace period instead of continuing straight. Physical retry pending.
- B-25 second physical attempt: a physically centered narrow line registered
  only on sensor 3, so estimated position 3000 caused a continuous `100,59`
  correction until the robot left the line. The junction diagnostic now maps
  its measured centered start to controller position 2500; physical retry is
  still required.
- B-25 four-way result: after start-position recentering, the robot approached
  and crossed the physical four-way junction, detected left, straight, and
  right exits, reported `LSR`, and stopped automatically. T-junction, one-side
  branch, dead-end, FINISH, and repeated classifications remain.
- B-25 T-junction result: the robot approached and crossed the physical top T,
  reported left and right exits with no straight continuation (`L-R`), and
  stopped automatically. One-side branch, dead-end, FINISH, and repeated
  classifications remain.
- B-25 dead-end result: the robot approached a physical tape endpoint, required
  persistent line loss through the recovery grace period, reported `DEAD_END`,
  and stopped automatically. One-side branch, FINISH, and repeated
  classifications remain.
- B-25 right-corner result: a physical one-sided corner correctly reported only
  the robot-relative right exit (`--R`) and stopped automatically. The mirrored
  left exit, FINISH, and repeated classifications remain.
- B-25 left-corner result: the mirrored physical corner correctly reported only
  the robot-relative left exit (`L--`) and stopped automatically. Four-way,
  T-junction, both one-sided directions, and dead-end now have initial physical
  passes. FINISH recognition and repeated trials remain.

## Update template

When resolving an item, update its status and append a short entry here:

```text
Date:
Blocker ID:
Resolution:
Evidence/location:
Decision made by:
Follow-up requirement or test:
```

