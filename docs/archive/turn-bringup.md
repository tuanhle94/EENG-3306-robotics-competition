# First gyro-controlled 90-degree turn test

This diagnostic tests one turn at a time. It uses the Zumo Shield's gyroscope
to measure the angle directly. The robot does not need to be placed on a line
or corner for this test.

The robot turns quickly at first, slows near the target, and stops at a measured
88 degrees to allow for a small amount of physical coasting. A two-second
timeout stops the motors if the angle cannot be measured correctly.

## Safety limits

- The battery remains off while uploading and while the gyro calibrates.
- Keep the robot completely still during gyro calibration.
- The robot turns only after the button is pressed and a three-second countdown
  finishes.
- Button bounce is ignored during the countdown, so one physical press cannot
  accidentally cancel the turn.
- Fast turn speed is 200 out of 400; it slows to 150 after 70 degrees. The
  physical drivetrain could not continue a right pivot at command 100.
- Each attempt stops at the target angle or after two seconds.
- Pressing the button during motion stops the attempt.

## Upload

1. Turn the robot's battery switch **OFF**.
2. Stop an old Serial Monitor by pressing `Ctrl+C` in PowerShell.
3. Place the robot flat and do not move it.
4. Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-turn-diagnostic.ps1
```

5. Keep the robot still until these messages appear:

```text
GYRO calibration=PASS
TURN ready direction=LEFT
```

## Run one turn

1. Put the robot on a flat, open area with space on every side. A line is not
   required.
2. Turn the battery switch **ON**.
3. Press and release the Zumo button once.
4. Move your hands and the USB cable away during the three-second countdown.
5. The robot should rotate left approximately 90 degrees and stop.
6. Turn the battery switch **OFF**.

A successful attempt ends with output similar to:

```text
TURN result=ANGLE_REACHED direction=LEFT angle_deg=88.0
```

Report the visible angle and the final `TURN result` line. A `TIMEOUT` means the
gyro did not reach the target before the safety timer stopped the motors.
