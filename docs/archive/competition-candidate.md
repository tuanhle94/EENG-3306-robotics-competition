# Autonomous competition firmware candidate

Status: Implemented and compiled; physical end-to-end testing is still required.

This build combines the separately tested behaviors. It follows the line,
identifies maze features, chooses routes, turns with the gyroscope, finds the
outgoing line, learns a map, saves a shortest route, and replays that route.

It is a **candidate** because compiling does not prove the complete physical
maze will work. The team must test it before competition.

## Upload

Connect USB, close any other Serial Monitor, and run:

```powershell
.\scripts\run-competition-firmware.ps1
```

The robot currently connects on `COM4`. If automatic port selection does not
work, run:

```powershell
.\scripts\run-competition-firmware.ps1 -Port COM4
```

The port number can change when you reconnect the robot or use another USB
port. If uploading says it cannot find the port, check the Arduino entry in
Windows Device Manager under **Ports (COM & LPT)** and use that number instead.
This error means uploading could not start; it does not mean the code failed
to compile. You do not need to erase calibration to fix a port-number error.

## Start a new learning run

1. Switch battery power **OFF**.
2. Hold the Zumo button while resetting or powering the Arduino.
3. Release it within two seconds. This clears the old route but keeps the saved
   sensor calibration.
4. Keep the robot still while gyro calibration runs.
5. Confirm `EEPROM calibration=VALID`, `CAL using stored values`, and
   `mode=LEARN`.
6. Put the robot at START, centered and facing into the maze.
7. Switch battery power **ON**, press the button once, and move your hands away.

Before the motors move, the firmware checks that an inner sensor sees a strong
black line. The Serial Monitor will show one of these results:

- `MISSION start=ACCEPTED inner_peak=...`: the robot starts the run.
- `MISSION start=REJECTED ... required_inner_peak=500`: the robot stays safely
  stopped. Move it slightly so the front sensor board is centered over solid
  black tape, then press the button again. Re-uploading is not required.

The button is an emergency stop while the robot moves. A successful learning
run prints `MISSION result=ROUND_ONE_COMPLETE` and saves its route in EEPROM.

## Replay the saved route

1. Reset normally; do **not** hold the button during startup.
2. Keep the robot still during gyro calibration. The saved sensor calibration
   loads automatically; no manual sweep is required.
3. Confirm `EEPROM calibration=VALID` and `mode=REPLAY`.
4. Return the robot to the identical START position and direction.
5. Switch battery power on and press the button once.

A successful replay prints `MISSION result=ROUND_TWO_COMPLETE`.

## Save or replace sensor calibration

Normal competition startup never requires picking up the robot for a sensor
sweep. Calibration is a separate setup operation that is saved in EEPROM.

1. Keep the Zumo button held for more than two seconds during reset or power-on.
2. Release it after `EEPROM calibration=CLEARED` appears.
3. Keep battery power off, then press and release the button once.
4. Sweep every sensor over both white and black for eight seconds.
5. Confirm `CAL result=PASS` and `EEPROM calibration=SAVED`.

Do this during practice or an allowed setup period, not after an official run
has started. Stored calibration remains available after reset and power loss.

## Current assumptions

- START points in the map's north direction.
- Paths use 90-degree turns and form a regular grid.
- FINISH keeps all six sensors over black for at least 400 milliseconds. The
  organizers did not provide a more exact marker definition.
- Normal line following uses command `150`, with a minimum forward-track
  command of `100`. Gyro turns use `200`, slowing to `150`; no speed knob is
  fitted. These are motor commands, not measured travel speeds.
- Junction centering currently tries a total advance of `343` milliseconds
  from the first confirmed outer-sensor reading. Both branches are monitored
  during this time; the entire scan is limited to `800` milliseconds. This
  timing is experimental and needs a physical positioning check.
- The map supports at most 32 nodes.

If testing fails, copy the complete output from `MISSION start_raw=...` through
the final `MISSION result=...` line, and note which maze feature the robot had
just reached. The log includes raw and normalized sensors, signal strength,
line-loss time, motor commands, turn angle, and gyro readings. During a junction
scan it also reports `outer_confirmed`, `scan_ms` (time since probing began),
and `advance_ms` (time since the first confirmed outer reading).

Current physical testing is focused on the first turn's heading and pivot
placement. See [corner-inspection.md](corner-inspection.md) for the
`-InspectFirstTurn` checkpoint. This remains a candidate; successful individual
tests and compilation do not establish complete-maze reliability.

The useful ideas and hardware differences in the downloaded examples are
explained in [reference-code-review.md](reference-code-review.md).
