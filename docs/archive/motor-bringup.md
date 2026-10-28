# Lifted-track motor test

This is the first test that can move the tracks. It verifies that both motors
work, identifies their forward directions, and checks for obvious drivetrain
problems. The robot must remain raised for the entire test.

## What makes this test safer

- The program starts disarmed and commands both motors to zero.
- Entering `A` only arms the test; it does not move anything.
- `L`, `R`, or `B` produces one 400 ms pulse at 25% motor command.
- Every pulse stops automatically.
- `X`, the Zumo user button, or a 30-second arming timeout stops and disarms it.
- Uploading the ordinary `uno` environment still produces the motion-disabled
  diagnostic. Motion is compiled only into `uno_motor_test`.

## Prepare the robot

1. Set the battery switch to `OFF`.
2. Put the robot on a sturdy box or stand.
3. Confirm that both tracks are completely in the air.
4. Keep hair, sleeves, fingers, cables, and loose objects away from the tracks.
5. Connect the Arduino USB cable.

## Upload the motor diagnostic

Open PowerShell and run:

```powershell
cd "C:\Users\Hang Minh Nguyen\source\repos\EENG-3306-robotics-competition"
powershell -ExecutionPolicy Bypass -File scripts\run-motor-diagnostic.ps1
```

If the script asks for a port, find the Arduino under **Device Manager > Ports
(COM & LPT)** and replace `COM4` below with its port:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-motor-diagnostic.ps1 -Port COM4
```

The Serial Monitor should show:

```text
ZUMO LIFTED-TRACK MOTOR DIAGNOSTIC
Starts disarmed. Commands: A=arm L=left R=right B=both X=stop.
```

## Run one track at a time

1. Recheck that both tracks are clear and raised.
2. Turn the battery switch to `ON`. Nothing should move.
3. In the Serial Monitor, enter `A` and press Enter.
4. Enter `L` and press Enter. Only the left track should move briefly.
5. Record whether it moved toward the robot's forward direction (toward the
   metal blade) or backward.
6. Enter `R` and press Enter. Only the right track should move briefly.
7. Record its direction and whether either track sounds rough, sticks, or rubs.
8. Do not use `B` yet. First report the left and right results.
9. Enter `X`, press Enter, and turn the battery switch to `OFF`.

If anything unexpected happens, switch the battery power off. The Arduino may
remain connected by USB.

## Report the result

Send these four observations:

```text
LEFT: moved / did not move
LEFT DIRECTION: forward / backward / unsure
RIGHT: moved / did not move
RIGHT DIRECTION: forward / backward / unsure
```

Also report grinding, rubbing, large speed differences, or red PlatformIO
errors. Do not place the moving robot on the floor yet.
