# What we can reuse from other Zumo projects

## Short answer

Use the examples to understand the driving sequence, not as replacement
firmware. Our robot is an Arduino Uno with a Zumo Shield. The Zumo 32U4 is a
different robot, and its own library README says it does not support this
shield.

The most useful idea is to treat an intersection as three separate jobs:

1. Detect a possible branch.
2. Drive far enough to put the robot's pivot point at the intersection,
   watching both sides for other branches along the way.
3. Turn, then confirm the outgoing line before following it.

The pivot point is approximately the center of the rectangle between the two
tracks. It is behind the front sensors. Detecting tape at the front does not
mean the pivot point has reached that tape yet.

## Zumo 32U4 library review

Reviewed the local repository's README, `examples/MazeSolver/MazeSolver.ino`,
`GridMovement.h`, `TurnSensor.h`, `examples/LineFollower/LineFollower.ino`, and
the turn-sensing configuration in `src/Zumo32U4IMU.cpp`.

| Example | Useful idea | What we should not copy directly |
|---|---|---|
| `GridMovement.h`: `driveToIntersectionCenter` | Keep checking left/right branches during the advance; check straight at the end. | Its 130 ms setting was chosen for that robot, motors, and batteries. It is not a measurement for ours. Its scan also estimates time from sensor-read counts rather than elapsed milliseconds. |
| `GridMovement.h`: `turn` | Combine approximate gyro rotation with final line-sensor alignment. | It can spin indefinitely while waiting for a sensor. Its 45-degree handoff is not evidence that our chassis is aligned. Keep our timeouts and measured 88/178-degree turns for this experiment. |
| `TurnSensor.h` | Measure angle by adding rotation rate multiplied by elapsed time, after subtracting the stationary offset. | Its gyro setup uses 0.07 degrees/second per count. Our current shield setup and angle tracker use 0.00875. Those settings must stay paired. |
| `LineFollower.ino` | Steer using line position and its change between readings. | Five-sensor center position 2000, controller gains, speed 400, pin assignments, and display calls do not transfer unchanged to our six-sensor Uno setup. |
| `MazeSolver.ino` | Separate learning and replay; bound the route buffer. | The example explicitly assumes no maze loops. Keep our graph-based learner and EEPROM storage. |

The MazeSolver example itself does not use wheel encoders. Its center advance
is still time-based, so it does not give us a way to measure travel distance
on this Uno robot.

## Relationship to the current timing experiment

The earlier `Complete Source Code` download uses a longer advance than the
original Zumo Shield example: about 257 ms at motor command 200 for its assumed
0.75-inch tape. Scaling that reference to our command 150 gives the trial
setting of 343 ms. Scaling motor commands this way is only an estimate, not a
measurement of ground speed.

The 32U4 example supports separating advance from turn and collecting branch
evidence during that advance. It does **not** establish that 343 ms is correct.
Do not combine its 130 ms with 343 ms, and do not change the turn angle at the
same time.

Our competition candidate now:

- starts its total advance clock at the first confirmed outer-sensor reading;
- remembers left and right branches seen during that advance;
- does not restart the clock while an outer sensor remains over tape;
- treats an inner-side sensor as a possible corner, not a confirmed side exit;
- stops with `JUNCTION_SCAN_TIMEOUT` if the scan reaches 800 ms;
- keeps the official Zumo Shield motor driver, EEPROM formats, gyro turn
  settings, emergency button, and other motion timeouts.

The standalone junction diagnostic retains its earlier scan behavior. Use
the competition first-turn inspection to evaluate this new timing.

## What still needs the real robot

Computer tests can verify timing, branch memory, and timeout decisions. They
cannot prove that the chassis reaches the correct pivot position. The next
check is one stopped first turn, with its log and an overhead photo. Follow
[corner-inspection.md](corner-inspection.md); a full-maze run comes later.
