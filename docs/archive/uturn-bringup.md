# First gyro-controlled 180-degree U-turn test

This diagnostic tests the turn used when the robot reaches a dead end. It uses
the already verified gyroscope controller, but targets approximately 180
degrees instead of 90 degrees. It does not require a black line.

## Safety limits

- Keep the battery off during upload and gyro calibration.
- Keep the robot completely still until gyro calibration passes.
- Motion begins only after one button press and a three-second countdown.
- The robot turns at command 200, slows to 150 after 150 degrees, and targets
  178 degrees to allow for a small amount of physical coasting.
- The motors stop automatically after at most 3.5 seconds.
- Pressing the button during motion is an emergency stop.

## Run the test

1. Turn the battery switch **OFF**.
2. Stop the old Serial Monitor with `Ctrl+C`.
3. Place the robot flat and keep it still.
4. Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-uturn-diagnostic.ps1
```

5. Wait for `GYRO calibration=PASS` and `TURN ready direction=LEFT`.
6. Put the robot in a flat, open area. No line is needed.
7. Turn the battery switch **ON** and press the Zumo button once.
8. After it stops, turn the battery switch **OFF**.

A successful test reports `TURN result=ANGLE_REACHED` near 178 degrees and
physically looks like a half-turn: the blade finishes pointing in the opposite
direction.

## Verified result

The first physical LEFT U-turn slowed at 150 degrees and stopped automatically
at 178.1 degrees with `ANGLE_REACHED`. The production design will use a LEFT
U-turn at dead ends.
