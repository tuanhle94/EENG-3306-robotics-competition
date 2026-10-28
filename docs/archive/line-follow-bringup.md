# First timed line-following test

This test is the first time the floor sensors control the motors. It follows
one simple black line at motor command 100 and stops automatically after three
seconds. It does not navigate intersections or solve a maze.

## Safety limits

- The robot must pass an eight-second manual sensor calibration first.
- A separate button press and three-second countdown start motion.
- The speed limit is 100 out of 400.
- The run stops after three seconds.
- The motors stop if the line is missing for 150 milliseconds.
- A stronger correction may briefly reverse one track at no more than command
  100 during a curve.
- At a large position error of 1600 or more, the recovery command pivots at
  `100,-100` or `-100,100`. Once started, the pivot continues until the error
  falls below 800; this prevents rapid switching near the entry boundary.
- The robot stops with `EDGE_NOT_RECOVERED` if the line remains at an extreme
  sensor edge for 750 milliseconds.
- Pressing the Zumo button during the countdown or run stops the test.

## Prepare

1. Make a straight matte-black line on a flat white surface on the floor.
2. Leave open space around the entire test area. Do not test on a table.
3. Point the robot away from stairs, people, pets, and fragile objects.
4. Leave enough slack in the USB cable for a short movement.
5. Start with the battery switch `OFF`.

## Upload

Close any existing Serial Monitor with `Ctrl+C`, then run:

```powershell
cd "C:\Users\Hang Minh Nguyen\source\repos\EENG-3306-robotics-competition"
powershell -ExecutionPolicy Bypass -File scripts\run-line-follow-diagnostic.ps1
```

## Calibrate

1. Keep the battery switch `OFF`.
2. Press and release the physical Zumo user button.
3. For eight seconds, slide the robot left and right by hand so every sensor
   crosses both white and black several times.
4. Wait for `CAL result=PASS` and `LINE ready`.

If calibration fails, repeat it before attempting motion.

## Follow the line

1. Center the front sensor array over the black line.
2. Aim the robot along the line with clear floor ahead.
3. Turn the battery switch `ON`.
4. Press and release the Zumo user button once.
5. Move your hands away during the three-second countdown.
6. Watch the robot; it should steer along the black line and stop after three
   seconds.
7. Turn the battery switch `OFF` after it stops.

If motion becomes unsafe, switch the battery power off. The robot might move
only a short distance at this low speed, which is acceptable for the first run.

## Report

Send:

```text
CAL result=...
LINE result=...
Robot behavior: followed / drifted left / drifted right / oscillated / did not move
Did it remain over the line: yes / no
```

Also copy several `LINE position=... motors=...` lines if available.

## Verified checkpoint

On 2026-10-19, this procedure passed once at motor command 100. The firmware
reported `LINE result=COMPLETED` with `peak_error=508`, and the tester observed
that the robot stayed on the black line and moved smoothly. This verifies the
short straight-line diagnostic only; curves, intersections, and repeated runs
still require testing.

The first gentle-curve attempt stayed over the line but reached position 5000
with motor commands saturated at `100,0`, so it did not complete the curve.
That attempt is recorded as a failed curve test even though the older
diagnostic printed `COMPLETED` when its timer expired.

The current firmware reports `TIME_LIMIT_REACHED` when the three-second timer
expires. This describes why the motors stopped; it does not claim that the
robot completed the physical curve.

A later retry exposed a threshold problem: `0,0,0,130,0,0` was rejected by the
old 200-count line-presence threshold even though the remaining signal followed
the physical line. The threshold is now 100, with automated coverage for both
that measured case and lower background noise.
