# Full-system simulator

Status: Implemented (2026-10-18)

## Purpose

The simulator tests firmware decisions on a Windows computer before the robot
arrives. It uses deterministic virtual inputs, so the same test produces the
same result every time. It does not contain the official maze layout.

"Deterministic" means a scenario always provides the same inputs in the same
order. This makes failures repeatable and easier to debug.

Run it from the project directory:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run-simulator.ps1
```

A passing run ends with:

```text
=== full two-round virtual robot ===
calibration=pass learning=pass persistence=pass replay=pass safe-stop=pass

=== virtual fault injection ===
missing-imu=pass bad-calibration=pass corrupt-route=pass
replay-mismatch=pass timeout=pass motors-stop-on-fault=pass

SIMULATION PASSED
```

The command returns exit code `0` on success and a nonzero exit code on any
build or simulation failure. This makes it suitable for automated testing.

An **exit code** is the number a program returns to PowerShell when it ends.
Zero conventionally means success.

## What is simulated

| Behavior | Requirement | Simulation |
|---|---|---|
| Motors disabled during startup and idle | F-01 | Mission state prohibits motion |
| Reflectance calibration span | F-02 | Valid and flat sensor samples |
| Missing IMU | F-04 | Injected startup failure |
| Potentiometer speed limit | F-10, F-13 | Synthetic ADC value and clamped commands |
| PD line correction | F-12 | Synthetic left/center/right line positions |
| Line and junction observations | F-20 | Synthetic six-sensor patterns |
| Runtime maze learning | F-29 through F-33 | Branch, dead-end, and loop mazes |
| Route persistence | F-34 | Fixed EEPROM-format image and simulated power cycle |
| Round-two replay | F-35 | Stored directions replayed against hidden maze |
| Replay contradiction | F-36 | Missing expected exit produces a safe fault |
| Timeout and fault stop | S-01, S-02 | Injected timeout; motor permission becomes false |

The full successful scenario executes this lifecycle:

```text
BOOT
  -> SAFE_IDLE
  -> CALIBRATING
  -> READY_TO_LEARN
  -> LEARNING
  -> ROUND_ONE_COMPLETE
  -> simulated power cycle
  -> SAFE_IDLE
  -> CALIBRATING
  -> READY_TO_REPLAY
  -> REPLAYING
  -> COMPLETE
```

The route image contains a magic byte, format version, route length, up to 32
directions, and a checksum. Changing a stored byte without updating the
checksum causes startup to enter the fault state.

Here, a **route image** is simply the fixed group of bytes that will eventually
be written to the Arduino's EEPROM. The **magic byte** identifies those bytes
as one of our saved routes. The **checksum** helps detect accidental damage.

## What software cannot prove

The following still require the physical robot and a practice course:

- track traction, drivetrain friction, and motor polarity;
- real sensor ranges, lighting effects, line width, and electrical noise;
- gyro accuracy and physical 90-degree/180-degree turns;
- junction debounce distances and line-reacquisition timing;
- the official FINISH marker;
- safe minimum and maximum speeds;
- the Arduino EEPROM hardware adapter and actual power-cycle behavior;
- five-minute completion on the physical course.

A simulated pass means the decision logic behaves correctly for the modeled
inputs. It is not evidence that the physical robot is competition-ready.

## Main simulator files

| File | Responsibility |
|---|---|
| `src/full_robot_simulation.cpp` | Runs the two-round mission and fault injection |
| `src/grid_maze.cpp` | Supplies hidden virtual maze observations and movement |
| `src/simulator_main.cpp` | Defines scenarios and determines overall pass/fail |
| `src/mission_controller.cpp` | Enforces lifecycle transitions and motor permission |
| `src/sensor_calibration.cpp` | Tracks six sensor minima, maxima, and spans |
| `src/route_storage.cpp` | Encodes and validates the persistent route image |

