# First junction-detection test

This diagnostic makes one short approach toward one maze feature. The robot
follows the incoming line, records left and right branches while crossing the
wide tape, checks whether the center line continues, prints the detected exits,
and stops. It does not choose a route or turn.

## Safety limits

- Calibration is performed with battery power off and motors stopped.
- Motion starts only after calibration passes, the button is pressed, and a
  three-second countdown finishes.
- Driving uses motor command 100.
- The robot stops after detecting one feature, after five seconds, or when the
  button is pressed during motion.

## Upload and calibrate

1. Turn the battery switch **OFF**.
2. Stop the old Serial Monitor with `Ctrl+C`.
3. Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-junction-diagnostic.ps1
```

4. Press the Zumo button once.
5. During the eight-second calibration, slide all six sensors repeatedly over
   both the white board and black tape.
6. Stop when `CAL result=PASS` and `JUNCTION ready` appear.

## Run one feature test

1. Choose one T-junction or four-way intersection with at least 10–15 cm of
   straight incoming line.
2. Place the robot centered on that incoming line, 10–15 cm before the feature,
   with the blade pointing toward it.
3. Keep the USB cable away from the tracks.
4. Turn the battery switch **ON**.
5. Press the Zumo button once and move your hands away.
6. Turn the battery switch **OFF** immediately after the robot stops.

The result uses three characters in left-straight-right order:

- `LSR` means left, straight, and right were detected.
- `LS-` means left and straight were detected.
- `L-R` means left and right were detected, but no straight continuation.
- `DEAD_END` means the incoming line ended without a side branch.

Always report both the printed result and the feature that was physically on
the board. A correct classification is required before route-selection code is
allowed to use it.

## Verified result

The first physical four-way test correctly reported
`JUNCTION result=FEATURE exits=LSR` and stopped automatically. Other feature
types still require physical verification. The first physical T-junction test
also passed, reporting `JUNCTION result=FEATURE exits=L-R`.
The first physical tape-end test also passed, reporting
`JUNCTION result=DEAD_END`.
The first one-sided right-corner test passed, reporting
`JUNCTION result=FEATURE exits=--R`.
The mirrored one-sided left-corner test passed, reporting
`JUNCTION result=FEATURE exits=L--`.
