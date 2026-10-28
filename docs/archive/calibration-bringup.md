# Manual reflectance-sensor calibration

This procedure teaches the software the real light and dark ranges of all six
floor sensors. The motors remain locked at zero throughout the test.

## Prepare the surface

1. Put a strip or area of matte black tape on a flat white surface.
2. Make the black region wide enough that each of the six sensors can pass over
   it as you move the robot from side to side.
3. Turn the robot's battery switch `ON`. The motors stay locked at zero.
   With the battery off the IR emitters are too dim: on 2026-10-21 white
   read 1212-1720 battery-off and 128-212 battery-on.
4. Connect the Arduino to the computer by USB.

## Upload the calibration diagnostic

Open PowerShell and run:

```powershell
cd "C:\Users\Hang Minh Nguyen\source\repos\EENG-3306-robotics-competition"
powershell -ExecutionPolicy Bypass -File scripts\run-calibration-diagnostic.ps1
```

If necessary, specify the Arduino's port, replacing `COM4` with the port shown
in Windows Device Manager:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-calibration-diagnostic.ps1 -Port COM4
```

## Collect the readings

1. Put the robot flat on the white part of the test surface.
2. Press and release the physical Zumo user button.
3. For eight seconds, slowly slide the whole robot left and right by hand.
4. Keep it flat and make sure every sensor crosses both white and black several
   times. Do not lift or tilt the front sensor board.
5. Stop moving when the Serial Monitor prints the result.

The output ends with lines similar to:

```text
CAL min=500,300,250,300,300,500
CAL max=2000,2000,2000,2000,2000,2000
CAL span=1500,1700,1750,1700,1700,1500
CAL result=PASS
```

`PASS` means every sensor observed at least 150 counts of difference between
its lightest and darkest samples. This threshold comes from the weakest
repeatable outside-sensor span measured on the team's hard-floor test surface;
it is not supplied by the competition rules. `FAIL` identifies a sensor that
did not cross both surfaces clearly. Press the Zumo button to repeat after
correcting the surface or movement.

## What to report

Send the five final lines beginning with:

```text
CAL samples=
CAL min=
CAL max=
CAL span=
CAL result=
```

Keep the battery switch on during this calibration; a battery-off sweep
records dim readings that hide partly covered tape.
