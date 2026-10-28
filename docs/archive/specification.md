# Robot firmware specification

Status: Draft v0.2 (2026-10-18)

## How to read this document

This file is the project's technical agreement: it says exactly what the
firmware must do and how the team will decide whether each behavior works.

- **Requirement:** the behavior that must exist.
- **Acceptance criterion:** the observable result required to mark it passing.
- `C` IDs come from competition constraints.
- `F` IDs describe firmware behavior.
- `S` IDs describe safety and memory limits.
- `O` IDs identify assumptions or open questions.

Words such as **shall** and **must** mean the item is required. A requirement is
not complete merely because its code compiles; its acceptance criterion must
also pass at the stated test level.

## 1. Goal and scope

The robot shall autonomously explore a black-line maze on a light background,
retain what it learned, and then traverse a shortest learned route. The target
platform is the assembled Pololu Zumo Robot for Arduino (#2510) with its 75:1
HP gearmotors, DRV8835 dual motor drivers, mounted six-sensor reflectance array,
Zumo Shield IMU, a separately supplied Arduino-compatible controller, four AA
batteries, and a team-installed external speed potentiometer.

The part names identify the actual hardware, but the overall arrangement is
simple: the Uno reads six floor sensors and one speed knob, uses the IMU to help
measure turns, and independently commands the left and right motors.

This specification covers firmware. Mechanical work, soldering workmanship,
the poster, and competition administration are documented project obligations
but are not software behaviors.

For a general glossary, see [README.md](../README.md). Important terms are also
explained near the sections where they are used.

## 2. External constraints from the competition rules

| ID | Requirement | Verification |
|---|---|---|
| C-01 | After the official start, navigation shall be autonomous: no remote commands, steering, repositioning, or physical assistance. | Inspection and official run |
| C-02 | The robot shall read the six reflectance sensors and estimate line position. | Sensor telemetry and track test |
| C-03 | Left and right motors shall be independently controlled. | Lifted-track test |
| C-04 | The robot shall detect turns, branches, intersections, and dead ends and execute controlled turns. | Feature test course |
| C-05 | Round one shall retain enough robot-observed information to generate a usable round-two route. | Power-cycle/restart test |
| C-06 | Round two shall use information obtained during round one; no manually entered route is allowed. | Code inspection and run log |
| C-07 | The learned route shall remove dead-end travel, reversals, and redundant loop traversal where the explored graph permits it. | Graph tests and course test |
| C-08 | A user-adjustable external circuit shall measurably change the motor-speed limit without a software edit. | Potentiometer sweep test |
| C-09 | An official attempt shall stop safely or declare failure before five minutes. | Timed test |
| C-10 | The maze layout shall not be hard-coded. | Code inspection |

## 3. Assumed hardware profile

The robot model is confirmed as Pololu item #2510 and the controller is an
Arduino Uno with an ATmega328P. The physical shield revision still requires
inspection.

- Base robot: assembled item #2510 with two 75:1 HP 6 V micro metal gearmotors,
  six-sensor array, and no wheel encoders.
- Shield: probably v1.3 for a current-production unit; verify the marking on the
  team's physical shield. v1.3 provides an LSM6DS33 accelerometer/gyro and an
  LIS3MDL magnetometer.
- Controller: Arduino Uno/ATmega328P.
- Sensor array: installed by Pololu; use its default pinout and the LEDON jumper
  connected to D2 for the Uno.
- Speed potentiometer: wiper on A1, with the shield's optional battery-level
  jumper left open (it is disconnected by default); outer legs connect to 5 V
  and GND.
- Start input: Zumo user pushbutton on digital pin 12.
- Motor outputs: Zumo Shield library (`ZumoMotors`).
- Orientation: Zumo Shield library (`ZumoIMU`); prefer gyro-relative turns over
  magnetometer absolute heading because motor current and nearby metal distort
  the magnetometer.
- Fixed-size data structures only; normal operation performs no heap allocation.
  The ZumoShield library's one-time sensor-calibration allocation is an explicit
  initialization exception and must succeed before motion is enabled.

`ATmega328P` is the processor on the Uno. **Heap allocation** means requesting
working memory while the program is running. The project normally avoids it so
memory use remains predictable.

## 4. Functional behavior

### 4.1 Startup and calibration

| ID | Requirement | Acceptance criterion |
|---|---|---|
| F-01 | Startup shall leave both motors stopped until an explicit local start action. | Motors remain at zero through boot and idle. |
| F-02 | A calibration mode shall collect reflectance minima/maxima over both line and background before a run. | Each sensor has a non-trivial calibrated span; otherwise start is rejected. |
| F-03 | Firmware shall report its mode and calibration outcome using only local indicators during an official run. Serial output may exist for practice but shall not accept control. | No serial command path can alter motion. |
| F-04 | Firmware shall detect the installed IMU through `ZumoIMU::init()` and keep motors stopped if it is unrecognized or unavailable. | Disconnect/failure test produces a local error indication and zero motor commands. |

### 4.2 Speed control and line following

| ID | Requirement | Acceptance criterion |
|---|---|---|
| F-10 | The potentiometer reading 0..1023 shall map monotonically to a configurable safe motor limit. | Unit tests cover endpoints, midpoint, monotonicity, and clamping. |
| F-11 | The configured minimum shall be high enough to overcome drivetrain stiction; the maximum shall remain within the library range -400..400. | Bench characterization recorded in `docs/tuning-log.md`. |
| F-12 | A closed-loop controller shall keep the estimated line position near the six-sensor center (2500). | No line loss on the agreed straight/curve practice course at three pot settings. |
| F-13 | Every commanded left/right speed shall be saturated to the current external speed limit. | Unit test plus lifted-track test. |

Initial line-controller baseline: fixed-point PD control derived from Pololu's
75:1-motor example, using line center 2500, proportional gain 1/4, derivative
gain 6, and no integral term. These values are starting points, not accepted
course tuning.

The first physical speed-100 trial showed that those example gains were too
aggressive at low speed: a 500-count error stopped one track completely. The
next low-speed diagnostic candidate is proportional gain 1/12 and derivative
gain 1, with the controller initialized at the measured starting position to
avoid a derivative kick. These remain experimental until repeated runs pass.

In plain language, the PD controller changes the two motor speeds to steer back
toward the line. "Fixed-point" means it uses integer arithmetic, which is less
expensive on an Arduino Uno than decimal arithmetic.

### 4.3 Feature detection and turns

| ID | Requirement | Acceptance criterion |
|---|---|---|
| F-20 | Sensor classification shall expose left, straight, and right line evidence plus no-line and all-dark conditions. | Unit tests cover representative masks. |
| F-21 | Junction detection shall be debounced spatially/temporally so one physical junction creates one decision event. | Ten repeated runs produce exactly one event per feature. |
| F-22 | A turn shall use sensor feedback to leave the old line and reacquire the selected line; it shall not rely only on a fixed delay. | 10/10 controlled 90-degree turns in both directions. |
| F-23 | Loss of all line evidence beyond a configured interval shall be treated as a dead end and cause a controlled reversal. | Dead-end test succeeds 10/10. |
| F-24 | An all-dark reading is only a FINISH candidate; intersection and FINISH discrimination shall use the final course marker specification. | Pending organizer definition (O-01). |
| F-25 | Competition junction advance shall use a configurable total duration from the first confirmed outer-sensor detection. Continued tape readings shall not restart that duration. Inner-only evidence shall not authorize a turn, and an incomplete scan shall stop on timeout. | Sequence tests cover tape remaining under outer sensors, a delayed opposite branch, inner-only probing, an 800 ms deadline, and timer wraparound. Hardware centering remains pending. |

Current candidate for F-25: 343 ms at motor command 150, derived from the
downloaded MazeSolver's four-times-longer reference and assumed 0.75-inch tape.
This is a trial setting. Motor command is not measured speed, and this value
does not replace a physical positioning check. The existing diagnostic uses
its previous scan behavior; the competition and first-turn inspection builds
use the new duration.

### 4.4 Learning, loops, and replay

| ID | Requirement | Acceptance criterion |
|---|---|---|
| F-29 | Round one shall begin with no preloaded route, maze graph, junction coordinates, or hard-coded turn sequence. The START node is the only initial map state. | Simulator inspection confirms every non-START node and edge originates from a runtime observation. |
| F-30 | The learned representation shall be a bounded graph of junction nodes and directional edges, not only a turn string. | Unit tests construct a cyclic graph. |
| F-31 | Each traversed edge shall connect two localized nodes and be marked explored in both directions. | Unit tests verify symmetric connection state. |
| F-32 | Exploration shall select among exits observed by the robot, prefer an untraversed available edge, and backtrack only when no untraversed edge remains. | Unit tests verify relative left/straight/right/back priority; simulator tests cover branches, dead ends, and a loop. |
| F-33 | On FINISH, the robot shall compute a shortest route over known edges from START to FINISH. | Unit tests cover competing routes and a cycle. |
| F-34 | The round-two route and integrity metadata shall be saved in EEPROM before indicating round-one completion. | Corruption and power-cycle test. |
| F-35 | Round two shall replay the saved route while continuing normal line control and feature validation. | End-to-end two-round test. |
| F-36 | Route replay shall fail safe if observed junction features contradict the stored route. | Mismatch test stops motors and signals an error. |

## 5. Safety and resource requirements

The Arduino Uno has only 2 KB of SRAM, which is its temporary working memory.
The program therefore uses fixed-size arrays and must detect when they are full
instead of writing beyond them.

| ID | Requirement | Acceptance criterion |
|---|---|---|
| S-01 | Any timeout, invalid state, route overflow, or localization contradiction shall stop both motors. | Fault-injection tests. |
| S-02 | No control loop operation shall block indefinitely. | Static review; every maneuver has a timeout. |
| S-03 | The graph shall reject overflow without corrupting existing data. | Capacity unit tests. |
| S-04 | The final Uno build shall leave unused working memory for safe operation. | The build-size report shows at least 25% of SRAM remains free. |

## 6. Course-dependent assumptions and tracking

The competition PDF is the only organizer specification currently available.
The active tracking source is [`blockers.md`](blockers.md). Missing final-course
details do not stop implementation: the project uses documented, configurable
defaults and replaces them when course information or test evidence becomes
available.

| ID | Item | Working decision | Status |
|---|---|---|---|
| O-01 | FINISH marker | The team reports a large black dot at the end of a short branch, far from START. Treat sensors 1-4 all black for 200 ms as FINISH (Pololu MazeSolver uses sensors 1-4); crossings measured at most two 50 ms samples. Dot size not yet measured. | Accepted risk |
| O-02 | Physical Zumo Shield revision | Detect through `ZumoIMU`; reject gyro-dependent motion if unavailable. | Hardware check |
| O-03 | Revisited-junction localization | Implement gyro-relative heading plus calibrated segment-travel coordinates. | In progress |
| O-04 | Course dimensions | Keep thresholds, line width, spacing, and travel calibration configurable. | Accepted risk |
| O-05 | A1 battery-level jumper | Verify open before connecting the potentiometer. | Hardware check |

Candidate resolution for O-03: use gyro-relative heading plus calibrated segment
travel to quantize junctions onto a coordinate grid. Item #2510 has no wheel
encoders, so segment distance must be inferred from travel time and detected
course features. This must be validated on the actual competition-style course
before it becomes a requirement.

In simpler terms, the robot will estimate which grid intersection it reached
from its direction and travel between detected features. Without wheel
encoders, this estimate can drift, so simulation alone cannot approve it.

## 7. Definition of done

The firmware is competition-ready only when all C, F, and S requirements are
either passing or explicitly accepted by the team/instructor; required hardware
checks and calibration evidence in `blockers.md` are complete; native tests
pass; the exact target compiles; and the complete two-round hardware test
succeeds repeatedly without intervention.

