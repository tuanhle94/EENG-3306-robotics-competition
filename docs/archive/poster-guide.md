# Poster content guide

Status: Working draft, updated 2026-10-19

This guide gives the presentation team accurate, plain-language material for a
college engineering poster. Update the evidence table before printing the
poster. A planned or simulated feature must not be described as physically
verified.

## Suggested title

**Autonomous Maze Learning with an Arduino Uno and Pololu Zumo Robot**

Suggested subtitle:

**A two-round robot that explores from sensor observations, builds a map, and
replays a shortest learned route**

## Thirty-second explanation

The robot follows a black-line maze using six downward-facing light sensors.
During round one, it identifies available paths at each junction and builds a
small graph in the Arduino Uno's limited memory. When it reaches FINISH, it
uses breadth-first search to calculate a shortest route through the paths it
actually observed. It saves that route for round two, when it can replay the
solution without being given the maze in advance.

## The engineering problem

- The robot does not know the maze before round one.
- It must distinguish ordinary line segments, turns, branches, dead ends, and
  the FINISH marker using only six floor sensors.
- An Arduino Uno has only 2 KB of SRAM, so the map cannot use dynamic,
  computer-sized data structures.
- The robot has no wheel encoders, so location and turning must combine sensor
  observations, gyro-relative heading, and calibrated travel behavior.
- The learned route must survive the transition between competition rounds.

## System overview

```text
Black line
    |
    v
Six reflectance sensors ---> Feature detection ---> Maze graph
                                  |                    |
                                  v                    v
                              Line control       Shortest route
                                  |                    |
                                  +------> Mission controller
                                               |
                                               v
                                      Left/right motors

Gyroscope ----------------> Turn and heading feedback
Speed knob ----------------> Adjustable speed limit
EEPROM <-------------------> Saved round-two route
```

## How learning works

1. Read the six reflectance sensors and estimate the line position.
2. Follow the line with a proportional-derivative controller.
3. At a junction, observe which exits exist: left, straight, and right.
4. Store the junction as a node and traveled paths as graph edges.
5. Choose an observed exit that has not been explored.
6. Mark each traversed edge from both directions so loops do not cause endless
   exploration.
7. At FINISH, run breadth-first search over the learned graph.
8. Save the resulting route with integrity data for round two.

## Why use a graph?

A graph represents junctions as **nodes** and paths as **edges**. This allows
the robot to represent branches and loops without storing a picture of the
course. Breadth-first search finds a route with the fewest graph edges when all
edges are treated equally.

## Why simulate before driving?

The simulator lets the team test decisions that are difficult to reproduce by
hand. It contains a branch/dead-end maze, a loop maze, a full learn-save-reset-
replay sequence, and injected failures. Simulation verifies software logic,
but it does not replace testing real motors, surfaces, lighting, or batteries.

## Evidence available today

| Claim | Evidence | Status as of 2026-10-19 |
|---|---|---|
| Fixed-memory graph and shortest-path logic work | Native unit tests | Implemented and passing |
| Branches, dead ends, and loops can be explored | Deterministic simulator | Simulated and passing |
| Full learn/save/reset/replay lifecycle works | Full-system simulator | Simulated and passing |
| Software failures stop commanded motion | Fault-injection simulator tests | Simulated and passing |
| Arduino Uno build fits the board | PlatformIO AVR build | Implemented; build passes |
| IMU and user button work | Physical diagnostic | Hardware verified |
| Six sensors distinguish white and black | White/black physical readings | Hardware verified |
| All six sensors meet the calibration span | 3567-sample manual sweep; minimum span 712 | Hardware verified |
| Both tracks drive the robot straight | 400 ms command at speed 100 | Hardware verified |
| Closed-loop control follows a straight black line | One smooth three-second run at command 100; automatic stop | Initial hardware pass |
| Autonomous line following works on the course | Repeated course runs | Not yet verified |
| Physical turns and junction detection work | Repeated course runs | Not yet verified |
| EEPROM retains the route on the Uno | Hardware power-cycle test | Not yet verified |
| Complete two-round maze run works | End-to-end physical trial | Not yet verified |

Current automated result: **30 of 30 native tests pass**. Do not replace that
number without rerunning the suite.

## Hardware facts for the poster

| Component | Project configuration |
|---|---|
| Robot | Pololu Zumo Robot for Arduino, item #2510 |
| Motors | Two 75:1 HP micro metal gearmotors |
| Controller | Arduino Uno / ATmega328P |
| Motor driver | DRV8835 dual motor driver on the Zumo Shield |
| Floor sensing | Six-channel Zumo reflectance sensor array |
| Motion sensing | Zumo Shield v1.3: LSM6DS33 accelerometer/gyro and LIS3MDL magnetometer |
| Batteries tested | Four non-rechargeable AA alkaline cells, 6 V nominal |
| Software tools | C++, PlatformIO, native unit tests, deterministic simulator |

## Suggested three-column poster layout

### Left column: problem and hardware

- Competition goal and two-round challenge
- Labeled photograph of the robot
- Small hardware table
- Six-sensor explanation

### Center column: algorithm

- System overview diagram
- Round-one learning steps
- Small example maze converted into nodes and edges
- Breadth-first shortest-route illustration

### Right column: verification and results

- Difference between simulation and hardware testing
- Evidence/status table
- Sensor white-versus-black readings
- Test counts and final course results
- Remaining limitations and future work

## Recommended visuals

- Top-view robot photograph with Arduino, sensor array, blade, and tracks labeled
- Bottom-view photograph showing the six reflectance sensors
- A small maze next to its graph representation
- Round-one exploration route versus round-two shortest route
- Bar chart comparing six sensor readings on white and black
- Final photograph of the competition course after physical validation

## Claims the poster team should avoid for now

Do not currently say:

- "The robot has completed the competition maze."
- "All autonomous features are hardware verified."
- "The physical robot saves and restores its route after power loss."
- "The robot always finds the globally shortest physical-distance route."

Acceptable current wording includes:

- "The complete two-round mission passes in deterministic simulation."
- "Core hardware components have passed initial bench tests."
- "The robot completed its first smooth, timed straight-line following test."
- "The graph search finds a shortest route through the learned graph."
- "Full physical integration and course tuning are in progress."

## Final poster update checklist

Before printing:

- Replace every `Not yet verified` row that has new evidence.
- Add the final number of successful full-course trials.
- Add measured battery voltage, line width, speed range, and controller gains.
- Add the final firmware commit identifier.
- Confirm that photographs match the actual competition configuration.
- Have the code owner review every technical claim.
