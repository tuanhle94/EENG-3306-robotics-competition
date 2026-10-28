# Inspect turn placement

The robot currently connects on `COM4`. All commands below use that port.
Windows can assign a different number after reconnection or a USB-port change.
If uploading says the port cannot be found, check the Arduino entry in Device
Manager under **Ports (COM & LPT)** and replace `COM4` with that number.

## Current next test: one left turn that finds the outgoing line

Goal: one left turn that reliably ends with a center sensor on the outgoing
line. Maze learning and route replay wait until this passes repeatedly.

### Step 1: recalibrate (once)

The calibration saved on 2026-10-21 never saw white, so partly covered tape
read as white. The firmware now rejects it at startup with
`EEPROM calibration=REJECTED reason=WHITE_OR_BLACK_NOT_SEEN` and asks for a
new sweep. You do not need to erase anything.

1. Battery **ON** (motors stay stopped during calibration), USB connected.
   Put the robot **flat** on the white board. A battery-off sweep fails:
   the IR emitters are too dim without the battery.
2. Press the button, then for 8 seconds slide the robot slowly left and right
   so every sensor crosses the tape several times. Do not lift or tilt it.
3. Expect `CAL min=` values around 150-350 and `CAL max=` values near 2000.
   `FAIL` now means a sensor never saw white (min above 600) or black (max
   below 1500).

### Step 2: run the alignment test

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -InspectFirstAlignment
```

1. Confirm `INSPECT mode=FIRST_ALIGNMENT stop_at_next_feature=yes`.
2. Place the robot on the same straight line as before, with the outer
   sensors on white. A start with an outer sensor on tape is now rejected
   (`OUTER_SENSOR_ON_TAPE`).
3. Battery ON, press the button once, and do not touch the robot during the
   3-second countdown; the gyro offset is re-measured then.
4. The robot drives to the intersection, turns, and reports
   `INSPECT first_turn_aligned=PASS` once a center sensor holds the new
   line. It then follows that line and stops at the next maze feature:
   - `follow_result=NEXT_FEATURE`: it continued along the new path. Pass.
   - `follow_result=LINE_LOST`: it lost the new line. Fail.
   - `follow_result=TIME_LIMIT`: it followed for 2 seconds without reaching
     a feature; check `line_seen=`.
   A `LINE_REACQUIRE_TIMEOUT` fault is still possible; it stops the motors
   safely.
5. Send the whole log from `MISSION start_raw` to the end. Repeat 5 times.

### How to read the result

The `MISSION turn_summary` and `MISSION turn_end` lines separate the two
possible causes:

- **Sensor interpretation:** `turn_end raw=` shows a sensor clearly above
  white (for example 800-1500) while `normalized=` shows 0. With a good
  calibration this should no longer happen.
- **Positioning:** `turn_end raw=` is all near white (about 150-350). Then
  `first_tape_deg=` shows when each sensor first saw tape. Tape reaching the
  center sensors (2 and 3) well before 88 degrees means the turn passed the
  line (pivot short of the intersection); tape never reaching them means the
  turn stopped short of it. `expected_line=` shows which way the search will
  look first.
- `gyro_peak=` at 32767 means the gyro reading clipped and the turn angle is
  under-measured.
- `GYRO countdown_offset=` compared with `boot_offset=` shows how far the
  gyro zero moves once the battery is on.

Only after five logs agree should the 343 ms advance or the 88-degree target
change, one at a time.

## Earlier test: try the configurable advance

The competition candidate now uses a trial total advance of 343 milliseconds
at motor command 150. The clock starts when an outer sensor first confirms a
branch. Continued tape readings do not restart it. Both side sensors are
checked throughout; an incomplete scan stops after 800 milliseconds.

This setting comes from the longer-advance downloaded example, not from a
measured speed on our board. The Zumo 32U4 example supports this sequence of
steps but uses a different timing value. See
[reference-code-review.md](reference-code-review.md). The gyro targets remain
88 degrees for a quarter-turn and 178 degrees for a U-turn.

Do one stopped first turn before another whole-maze attempt:

1. Close the monitor with `Ctrl+C`, keep USB connected, and switch battery OFF.
2. From the project folder, run:

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -InspectFirstTurn
   ```

3. Leave the user button released during startup. Keep the robot still while
   the gyro calibrates. Confirm `INSPECT mode=FIRST_TURN` and valid saved
   sensor calibration.
4. Put the robot at the same START, centered and facing into the maze. Turn
   battery ON and press and release the button once.
5. Confirm the log reports `junction_advance_ms=343`. The robot should stop
   after its first gyro turn, before searching or driving away:

   ```text
   INSPECT stopped_after_first_gyro_turn action=LEFT ... motors=0,0
   ```

6. Turn battery OFF without moving it. Send the log from `MISSION start_raw`
   through the stop message, plus a photo from directly above.

A good positioning result means the robot faces along the outgoing stripe
and a center sensor (index 2 or 3) sees that stripe. It does not prove the
whole maze works. If it stops with `JUNCTION_SCAN_TIMEOUT`, send that log;
do not increase the timeout to make the robot continue blindly.

## Optional measurement: measure the sideways correction

The first-turn checkpoint reached 88.2 degrees. The photo shows the chassis
approximately parallel to the left-going tape, but the center sensors still
saw white: final readings were `0,0,0,0,25,0`. This establishes a sensor-position
problem after the turn; it does not yet measure how much forward travel should
be added or removed before pivoting. The tape seen partway through the turn
was not proof of parallel alignment.

Leave the robot in that stopped position if possible, then:

1. Switch battery OFF, keep USB connected, and close the monitor with `Ctrl+C`.
2. Run:

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -SensorsOnly
   ```

3. Confirm `INSPECT mode=STATIC_SENSORS motors_disabled=yes` and valid stored
   calibration. This build cannot start driving, erase EEPROM, or recalibrate.
4. Record a sensor line before moving anything. Mark a track's position with
   a small removable marker or place a ruler alongside it.
5. Keeping the robot flat and facing exactly the same direction, slide it
   sideways toward the black stripe by a few millimeters at a time. Do not
   drive forward along the stripe or rotate it. Watch the sensor readings.
6. Find the nearest position where `center_line=yes` remains steady and
   `center_peak` is preferably at least 500. Note whether you slid it toward
   the TOP or BOTTOM of the board and approximately how many millimeters.
7. Send a sensor line before and after the move, the direction and distance,
   and an overhead photo. If no nearby position gives a strong center signal,
   report that instead; the raw readings help check calibration too.

The measurement separates where the pivot should be from how far the gyro
should rotate. It avoids estimating the correction from a perspective photo.
Turn the battery ON for valid sensor readings; the motors and user button are
disabled in this build.

## Earlier checkpoint: inspect the first turn

The first inspection photo shows the robot near the original intersection,
pointing diagonally across its outgoing line. The log declared the first turn
complete at 60.8 degrees. The supposed second corner was a false detection
caused by driving off the same line. The earlier interpretation that the robot
had reached the far-left corner was incorrect.

Early sensor-based completion has been removed: the chassis must reach the
measured 88-degree quarter-turn (178 degrees for a U-turn). This addresses the
premature turn; it does not yet verify the pivot position or subsequent search.

Use this checkpoint before another full maze attempt:

1. Close the monitor with `Ctrl+C`, leave USB connected, and switch battery off.
2. Run:

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -InspectFirstTurn
   ```

3. Confirm `INSPECT mode=FIRST_TURN stop_after_gyro=yes` and saved calibration.
   Leave the user button released during startup and keep the robot still.
4. Put the robot at the same START, turn battery on, and press the button once.
5. The robot stops immediately after its first measured 88-degree turn. It
   does not search for the line or drive away afterward. Expected message:

   ```text
   INSPECT stopped_after_first_gyro_turn action=LEFT ... motors=0,0
   ```

6. Switch battery off without moving it. Send a photo from above showing the
   robot and the intersection, along with the log.

The robot should be approximately parallel to the left-going tape. If it is
parallel but sideways from the tape, the next issue is its pivot location.
If it is still diagonal, inspect the approach heading and measured gyro turn.
This build stays stopped until reset and preserves the saved route.

## Earlier inspection: stop before the second turn

This temporary test answers one question: where is the robot when the code
decides to make its second turn? The latest log shows tape passing across the
center sensors at 27-39 degrees, well before the expected quarter-turn. A photo
before the turn helps distinguish poor corner placement from a wrong turn-stop
rule. We have not established which is the root cause yet.

1. Close the current Serial Monitor with `Ctrl+C`.
2. Keep USB connected and turn the battery switch off.
3. From the project folder, run:

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -InspectSecondTurn
   ```

4. Check for `INSPECT mode=CORNER stop_before_turn=2` and
   `EEPROM calibration=VALID`. Do not hold the user button during startup.
5. Put the robot at the same START position used for the failing run. Keep it
   still during gyro calibration, turn the battery on, then press and release
   the user button once.
6. It follows the first segment, makes the first turn, then stops **before
   rotating for the second turn**. Look for:

   ```text
   INSPECT stopped_before_turn=2 action=LEFT ... motors=0,0
   ```

7. Turn the battery off without moving the robot. Take a photo from directly
   above showing the entire robot, tape under it, and the nearby corner. Send
   the photo and the serial log.

The pivot is approximately halfway between the left and right tracks, halfway
along their contact length. It is behind the front metal blade. We need to see
whether this point is before, on, or beyond the tape corner.

After the inspection stop, pressing the button cannot restart motion. Reset
restarts setup. The test reads stored calibration, forces learning in memory,
and does not replace the saved route. Existing startup erase gestures still
apply, so leave the user button released while resetting or uploading.

This is a diagnostic build, not the competition build. Restore normal firmware
after inspection using the same command without `-InspectSecondTurn`.
