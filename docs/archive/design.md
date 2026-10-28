# Robot and maze-simulator design

Status: Draft v0.1 (2026-10-18)

## 1. Purpose

This project controls a Pololu Zumo Robot for Arduino (#2510) using an Arduino
Uno. The competition has two rounds:

1. **Learn:** the robot drives through an unknown line maze and builds its own
   map from sensor observations.
2. **Replay:** the robot follows a shortest route calculated from the map it
   learned in round one.

The maze is not stored in the program ahead of time. Only the START location is
known when learning begins.

This document explains how the software is organized, how information moves
through it, and which parts are implemented today. Exact behavioral
requirements are maintained in [specification.md](specification.md), while
unknowns and validation work are tracked in [blockers.md](blockers.md).

No advanced robotics background is required. New terms are explained when they
first appear, and the shared glossary is in [README.md](../README.md).

## 2. Design goals

The design favors these qualities:

- **Autonomous:** no remote steering or manually entered route.
- **Testable:** maze decisions can run on a computer without the robot.
- **Safe:** invalid states, timeouts, and memory limits stop the motors.
- **Small:** all production structures fit within the Arduino Uno's 2 KB SRAM.
- **Explainable:** team members should be able to describe every major decision
  during judging.
- **Configurable:** course-dependent thresholds and timing are tuned without
  hard-coding the official maze.

## 3. System overview

The software is divided into hardware-independent logic and a thin hardware
adapter:

A **hardware adapter** is a small layer of code that translates general
commands such as "read the line sensors" or "set motor speeds" into calls to
the Pololu Arduino library. Keeping this layer small lets the simulator replace
the real hardware without replacing the maze algorithm.

```text
                    Arduino Uno + Zumo hardware
                  +-----------------------------+
Black line ------>| Six reflectance sensors     |
Potentiometer --->| A1 speed input              |
                  | IMU + button + motor driver |
                  +--------------+--------------+
                                 |
                                 v
                  +-----------------------------+
                  | Hardware adapter            |
                  | Read sensors / drive motors |
                  +--------------+--------------+
                                 |
                                 v
                  +-----------------------------+
                  | Autonomous state machine    |
                  | Calibrate, learn, replay    |
                  +-------+-------------+-------+
                          |             |
                          v             v
                  +-------------+  +------------+
                  | Line and    |  | Maze       |
                  | turn control|  | learner    |
                  +-------------+  +------+-----+
                                           |
                                           v
                                    +-------------+
                                    | Maze graph  |
                                    | + route     |
                                    +-------------+
```

The computer simulator replaces the hardware adapter with a virtual maze. It
uses the same maze learner and graph code intended for the Arduino.

## 4. Hardware model

Confirmed equipment:

| Part | Project use |
|---|---|
| Arduino Uno | Runs the firmware |
| Pololu Zumo #2510 | Chassis, 75:1 HP motors, motor drivers, sensors, and IMU |
| Six reflectance sensors | Measure the black line and detect maze features |
| External potentiometer on A1 | Sets the maximum permitted motor speed |
| Zumo user button on D12 | Local start acknowledgement |
| Four AA batteries | Power source |

The detailed connections are in [hardware-pin-map.md](../hardware-pin-map.md).

## 5. Important concepts

### 5.1 Heading

The robot uses four absolute directions:

```text
       NORTH
         ^
         |
WEST <---+---> EAST
         |
         v
       SOUTH
```

At a junction, the sensors describe exits relative to the robot: left,
straight, right, and back. The learner converts those observations into
absolute directions using the current heading.

Example: if the robot faces EAST, its left exit points NORTH.

### 5.2 Node

A node represents an important maze location, normally an intersection,
branch, dead end, START, or FINISH. Each node stores:

- estimated grid coordinates `(x, y)`;
- which north/east/south/west exits have been observed;
- which exits have already been traversed;
- neighboring node IDs for known connections.

### 5.3 Edge

An edge is a line segment connecting two nodes. Connecting an edge is
bidirectional: if node A connects EAST to node B, node B connects WEST to node
A. Marking the edge explored updates both ends.

### 5.4 Graph

The complete learned map is a graph: nodes are locations and edges are line
segments. A graph is required because the competition maze can contain loops.
A simple list such as `LEFT, RIGHT, STRAIGHT` cannot reliably represent a loop
or recognize that two routes return to the same intersection.

## 6. Round-one learning algorithm

Round one follows this cycle:

```text
Read local exits
       |
       v
Update the current graph node
       |
       v
Is this confirmed FINISH? -- yes --> Calculate shortest learned route --> Stop
       |
       no
       v
Choose an observed, untraversed exit
       |
       +-- one exists --> Turn/move --> Add or revisit node --> Repeat
       |
       +-- none exists --> Find nearest known node with an unexplored exit
                              |
                              +--> Move one known edge toward it --> Repeat
```

The current exit priority is left, straight, right, then back. This priority
does not supply maze knowledge; it only makes the exploration deterministic so
the same observations produce the same choices.

When the robot reaches a coordinate already in the graph, it reuses the node
instead of creating a duplicate. That action closes a loop in the learned map.

### 6.1 What "learns itself" means

The program does not contain:

- the official maze layout;
- a precomputed sequence of turns;
- preloaded intersection coordinates;
- a route entered between rounds.

Every non-START node and every edge must result from a runtime observation and
movement during round one.

## 7. Shortest-route calculation

When FINISH is confirmed, the graph uses breadth-first search (BFS) from START
to FINISH.

Breadth-first search is a standard graph algorithm. Imagine drawing rings
outward from START: first it checks every location one edge away, then every
location two edges away, and so on until it reaches FINISH.

BFS examines routes in increasing edge count. Therefore, for a graph whose
edges are treated as equal length, the first route found uses the fewest edges.
The result is stored as absolute directions such as:

```text
NORTH, NORTH, WEST, SOUTH
```

If physical segments have substantially different lengths, fewest edges might
not equal shortest distance or fastest time. Weighted path planning can be
added later if the final course requires it.

## 8. Round-two replay

The planned replay process is:

1. Load the saved route and checksum from EEPROM.
2. Wait in safe idle for the local start action.
3. Follow the line normally between junctions.
4. At each junction, validate that the required exit is physically present.
5. Execute the stored turn using sensor feedback.
6. Stop at confirmed FINISH.

If an expected exit is missing or the route data is invalid, the robot stops
instead of guessing.

The checksummed route format and simulated power cycle are implemented. The
code that reads and writes the Arduino's physical EEPROM, plus real hardware
replay, is not implemented yet.

## 9. Line following

After calibration, `readLine()` returns a position from 0 to 5000. A centered
line is approximately 2500.

```text
sensor:      0      1      2      3      4      5
position:    0    1000   2000   3000   4000   5000
                              ^
                       center is 2500
```

The controller calculates:

```text
error = measured position - 2500
correction = proportional term + derivative term
left speed  = speed limit + correction
right speed = speed limit - correction
```

The proportional term reacts to the current error. The derivative term reacts
to how quickly the error changes and helps reduce overshoot. Both motor
commands are clamped to the limit selected by the external potentiometer.

The initial gains come from Pololu's 75:1-motor example. They are starting
values and must be tuned on the physical course.

## 10. Junction and FINISH detection

Calibrated readings are converted into a six-bit mask. A bit is set when its
sensor sees dark line material.

```text
bit:       0  1  2  3  4  5
location:  L  L  C  C  R  R
```

- Left sensors indicate a possible left exit.
- Center sensors indicate a possible straight exit.
- Right sensors indicate a possible right exit.
- No active sensors indicates possible line loss or a dead end.
- All active sensors indicates a FINISH candidate or a wide intersection.

A single reading is not enough to declare FINISH. The working assumption is a
sustained all-dark observation after junction probing. Its duration remains a
configuration value until physical testing.

## 11. Turning

Planned turns use both orientation and line feedback:

1. Begin rotating in the requested direction.
2. Use the IMU to track relative rotation when a gyro is available.
3. Confirm that the old line has left the selected sensor region.
4. Slow near the expected 90-degree or 180-degree heading.
5. Stop rotating when the center sensors reacquire the new line.
6. Abort and stop both motors if the maneuver times out.

The IMU and sensor APIs compile, but this maneuver controller is not yet
implemented.

## 12. Simulator design

The simulator contains a hidden `GridMaze`. The learner does not receive that
maze directly. On every step it receives only:

- the exits at its current location, relative to its heading; and
- whether its current location is FINISH.

The simulator then applies the chosen direction, moves to the neighboring
virtual node, and repeats. At FINISH, it replays the learned route against the
hidden maze and verifies that the route ends at the correct location.

The simulator includes two maze shapes, a complete two-round mission, and a
fault-injection suite. The two maze shapes are:

### Branch and dead end

```text
          FINISH
             |
dead end -- junction -- dead end
             |
           START
```

The robot enters one dead end, returns, reaches FINISH, and produces a two-edge
replay route.

### Loop

```text
FINISH -- B ---- A
          |      |
          C -- START
```

The starting heading forces the robot around the cycle. When it returns to
START, the learned coordinates match the existing START node, closing the loop.
It then travels through known edges to the remaining unexplored branch and
reaches FINISH.

Run the simulator with:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run-simulator.ps1
```

## 13. Fixed-memory strategy

The Arduino Uno has limited SRAM, so the project avoids dynamic containers such
as `std::vector` in production code.

SRAM is temporary working memory. A `std::vector` can grow while a program is
running, which makes its memory use less predictable. Fixed-size arrays reserve
a known amount of space when the program is built.

- Maximum graph size: 32 nodes.
- Neighbor IDs and masks use eight-bit values.
- Routes use fixed-size arrays.
- Overflow returns a failure instead of writing outside an array.
- The Zumo library's calibration allocation is the documented initialization
  exception.

## 14. Failure behavior

The intended safe response to any of these conditions is to stop both motors
and signal an error:

- sensor or IMU initialization failure;
- invalid node or edge;
- graph or route capacity exceeded;
- impossible coordinate connection;
- missing expected replay exit;
- turn, segment, or complete-attempt timeout;
- corrupted EEPROM data.

The simulator reports logic faults, while the final robot will use the onboard
LED and buzzer for local status.

## 15. Implementation status

| Component | Status |
|---|---|
| Potentiometer speed mapping | Implemented and unit tested |
| Six-sensor calibration validation | Implemented and unit tested |
| PD line-control calculation | Implemented and unit tested |
| Six-sensor mask classification | Implemented and unit tested |
| Fixed-memory graph | Implemented and unit tested |
| Exploration choice | Implemented and unit tested |
| Online maze learner | Implemented and simulator tested |
| BFS shortest route | Implemented and unit/simulator tested |
| Branch/dead-end simulation | Implemented and passing |
| Loop simulation | Implemented and passing under the regular-grid assumption |
| Mission lifecycle and motor permission | Integrated competition candidate compiles; physical test remains |
| Checksummed route image | EEPROM save, read, checksum, and power-cycle flow implemented; physical test remains |
| Full learn/reset/replay simulation | Implemented and passing |
| Safety fault injection | Implemented and passing |
| Uno hardware initialization | Implemented and hardware verified |
| Arduino calibration workflow | Integrated with checksummed EEPROM persistence; manual capture hardware verified, persistence test remains |
| Junction debounce/probing | Implemented and individually hardware verified |
| Sensor/IMU turn controller | Implemented; 90-degree and 180-degree turns individually hardware verified |
| Arduino EEPROM read/write adapter | Implemented and compiled; physical power-cycle test remains |
| Round-two hardware replay | Implemented and compiled; physical end-to-end test remains |
| Complete autonomous hardware state machine | Candidate implemented and compiled; physical end-to-end test remains |

## 16. Verification approach

Development proceeds in layers:

1. **Native unit tests:** arithmetic, masks, graph operations, and decisions.
2. **Software simulation:** complete learning and replay using hidden mazes.
3. **Arduino build:** compatibility and flash/SRAM checks.
4. **Bench tests:** potentiometer, sensors, IMU, and lifted tracks.
5. **Feature-course tests:** line following, turns, branches, and dead ends.
6. **Two-round practice:** full autonomous learning and replay.

Current automated verification is documented in
[verification-matrix.md](verification-matrix.md).

## 17. Main source files

| File | Responsibility |
|---|---|
| `src/speed_control.cpp` | Maps potentiometer input and clamps motor commands |
| `src/line_controller.cpp` | Calculates PD line-following motor speeds |
| `src/line_observation.cpp` | Classifies the six reflectance readings |
| `src/exploration_policy.cpp` | Chooses an observed, untraversed exit |
| `src/maze_graph.cpp` | Stores nodes/edges and finds paths |
| `src/maze_learner.cpp` | Coordinates observation, exploration, mapping, and finish routing |
| `src/grid_maze.cpp` | Implements the hidden virtual maze |
| `src/simulator_main.cpp` | Defines and runs example simulation scenarios |
| `src/main.cpp` | Arduino hardware entry point; currently motion-disabled |

