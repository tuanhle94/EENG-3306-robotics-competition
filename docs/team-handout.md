# How our maze robot works

Team handout for the Ratliff Relays competition, Saturday 2026-10-24.
Written for teammates who do not code. Rule numbers refer to the official
_2026 Ratliff Relays Competition Guidelines_.

## In one paragraph

Our robot drives a maze of black tape by itself. In **round one** it explores
from START until it finds FINISH (a black rectangle about 4 x 2 inches),
writing down every turn. In **round two** it drives the shortest path it
learned straight to FINISH. The path is kept in the robot's permanent memory,
so switching it off between rounds is fine.

## Competition day at a glance

| Time        | What happens                                                                            |
| ----------- | --------------------------------------------------------------------------------------- |
| 10:00-11:00 | Sign in (Patriots Engage self check-in), T-shirts. Everyone must be present 10:00-3:00. |
| 11:00       | Welcome                                                                                 |
| 11:10-12:30 | Poster judging: be at the poster, ready for questions                                   |
| 12:15-1:15  | Lunch                                                                                   |
| 1:00-2:30   | Robot runs                                                                              |
| 2:30-3:00   | Awards                                                                                  |

**How we are scored (rule 4.9 and 6.1):** poster 30%, competition 70%. The 70
competition points are:

| Part                   | Points | What judges look at                                                                                   |
| ---------------------- | ------ | ----------------------------------------------------------------------------------------------------- |
| Assembly and soldering | 15     | Motors, solder joints, wiring, sensor array, tidy work                                                |
| External speed control | 15     | The speed knob: circuit, wiring, how its reading sets speed, and a demo that turning it changes speed |
| Maze performance       | 25     | Line following, junctions, turns, dead ends, reaching FINISH                                          |
| Completion time        | 25     | Faster successful runs score more; round two time matters most                                        |

**Runs (rule 4.8):** two official attempts, the better one counts, **5 minutes
maximum** per attempt. The timer runs from the start signal to FINISH. Judges
may end an attempt if the robot leaves the course, gets stuck, or is touched.

**Between rounds (rule 4.7):** only judge-approved actions: switching off and
on, checking connections, fresh batteries if allowed, restarting the program.
No reprogramming, no changing the saved path, no swapping parts. The speed
knob **may** be adjusted before each round.

## Quick answers for judges

| Question                              | Answer                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| ------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Left-hand or right-hand rule?         | **Left-hand rule.** At every junction it takes the exit furthest left: left, then straight, then right, then turn around.                                                                                                                                                                                                                                                                                                                                                                                                                  |
| Which library?                        | Pololu's **ZumoShield** Arduino library (version 2.1). Our program is built on its **MazeSolver** example.                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| How fast? | The speed knob sets it: motor command 160 to 240 out of a maximum of 400. With fresh batteries we run at about **188** (knob about 370); with older batteries about 200. Both rounds use the same setting. Faster made the robot miss branches and once restarted the Arduino on a battery dip. |
| Arduino IDE?                          | The code is normal Arduino C++. We build and upload it with **PlatformIO in VS Code**, which uses the same Arduino tools and library and also runs our automated tests. We tested Pololu's original example in the **Arduino IDE** too.                                                                                                                                                                                                                                                                                                    |
| What did you add to Pololu's example? | The speed knob; settings for this maze's 1-inch tape and finish rectangle; a calibration check that repeats the start-up rocking until every sensor has seen black; centering on the line before driving; turns that cannot stop early; a search for the line instead of a false dead end just after a turn; ignoring false junctions; a round-two check of every junction with a fallback to exploring; saving the path so it survives power off; a 10-second erase; a stop button; time limits; and a record of every run for debugging. |
| How do you handle loops?              | See "Loops" below.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |

### Say it simply

**How does it find the shortest path for round two? Does it remember time or
turns?** It remembers the turns, not the time. In round one it writes down a
letter at every junction: Left, Straight, Right, or Back (a dead end where it
turned around). When it finishes, it cleans up the list: every dead end is
wasted driving, so it swaps each "went in and came back" for the one turn that
skips it. For example, left, back, left is the same as just going straight. In
round two it simply follows the list, one turn per junction, and skips every
dead end. In our test, round one took 38 junctions and round two needed 23.

**How does it handle loops?** It follows the left wall, so it never goes around
a loop forever. It always takes the leftmost turn, like walking through a maze
with your left hand on the wall, and because START is on the outside edge of
the maze, that wall always leads it on to the finish. If they push further: it
does not compare both sides of a loop, it only knows the route it actually
drove. Picking the shorter side would need a full map of the maze; our first
version drew one but was less reliable, so we chose the method that finishes.

**Where are the letters stored after round one?** In the Arduino's EEPROM, a
small memory inside the Arduino that keeps its data when the power is off.
During round one the letters are in normal working memory; at the finish the
cleaned-up list is copied into the EEPROM and read back to check it. That is
why the robot can be switched off between rounds. The list stays until the
button is held for 10 seconds. Details: the EEPROM holds 1 KB; our 23-move
path takes 27 bytes: one letter per turn, three header bytes, and a check
value that detects corruption.

## Hardware

| Part         | What it is                                                                                                           | What it does                                                                                        |
| ------------ | -------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| Brain        | Arduino Uno (ATmega328P chip, 16 MHz)                                                                                | Runs the program                                                                                    |
| Body         | Pololu Zumo chassis and Zumo Shield, two micro metal gearmotors (75:1), rubber tracks                                | Drives. Each track has its own motor, so the robot steers by running one track slower or backwards. |
| Motor driver | DRV8835 chip on the shield                                                                                           | Turns the Arduino's small signals into motor power                                                  |
| Sensors      | Zumo reflectance sensor array (Pololu #1419): 6 sensors under the front edge, numbered 0 (far left) to 5 (far right) | The robot's "eyes" on the floor                                                                     |
| Controls     | Pushbutton, yellow light, buzzer on the shield                                                                       | Start and stop, status, beeps                                                                       |
| Power        | 4 AA batteries and the shield's power switch                                                                         | Power for motors and Arduino                                                                        |
| Speed knob   | Potentiometer on A1 (required by rule 4.3)                                                                           | Sets the robot's speed without changing the program. See "Speed control" below.                     |

### How the parts connect (judges ask this at inspection, rule 4.2)

The Zumo Shield plugs onto the Arduino, so most connections run through its
header pins.

| Connection           | Arduino pins            | How it works                                                                                                    |
| -------------------- | ----------------------- | --------------------------------------------------------------------------------------------------------------- |
| Left motor           | D8 direction, D10 power | The driver chip reads a direction signal and a fast on/off power signal (PWM). More "on" time means more speed. |
| Right motor          | D7 direction, D9 power  | Same as the left motor                                                                                          |
| 6 floor sensors      | D4, A3, D11, A0, A2, D5 | See "How do the sensors see the line?"                                                                          |
| Sensor infrared LEDs | D2                      | Switches the sensors' light on                                                                                  |
| Button               | D12                     | Pressing connects the pin to ground; the program reads that as "pressed"                                        |
| Yellow light         | D13                     | On when the pin is high                                                                                         |
| Speed knob           | A1, plus 5 V and GND    | An analog input reads a voltage from 0 to 5 volts as a number from 0 to 1023                                    |
| Gyro/compass chip    | A4, A5                  | Two-wire link (I2C); not used by the competition program                                                        |

The motor leads are soldered to the motors and to the shield. The sensor
array plugs into a header soldered to the shield.

### Hardware questions people ask

**How do the sensors see the line?** Each sensor has a tiny infrared LED
pointing down and a light detector next to it. White floor reflects a lot of
the infrared light back; black tape absorbs most of it. The Arduino charges a
tiny capacitor on each sensor and times how fast the reflected light drains
it: fast means white, slow means black. That time becomes a number from 0
(white) to 1000 (black).

**Why does it rock left and right at the start?** That is calibration. Black
and white read differently on each floor and under each light. By sweeping its
sensors over the START line, the robot records the darkest and brightest
reading of each sensor, then scales every later reading between them. It then
checks that every sensor really passed over the black line; if one did not,
it rocks again (up to 3 times). Finally it turns slowly until the line is under
its middle sensors, and only then drives.

**How does it know where the line is?** It combines the six readings into one
number for the line's position: 0 under the far-left sensor, 2500 in the
middle, 5000 under the far right.

**Does it know how far it has driven?** No. This robot has no wheel encoders
(rotation counters). Short distances, such as driving 1.5 inches into a
crossing, are timed: at a known speed, a known number of milliseconds is about
an inch.

**Does it use the gyro?** No. Turns stop when the sensors see the new line,
not after a measured angle.

**Where is the path stored?** In the Arduino's EEPROM, 1 KB of memory that
keeps its contents with the power off (see "Say it simply" above). The Arduino has 32 KB for the program
(ours uses about 50%) and 2 KB of working memory (ours uses about 22%).

## Speed control (rule 4.3, 15 points)

The **speed knob** is a potentiometer: a small part with a turning shaft and
three pins, like a volume knob. Turning it changes the robot's speed without
touching the program. The rules require it, and judges score the circuit, the
scaling and a live demonstration.

> **Status:** fitted on 2026-10-23 (wiper on A1, ends on 5 V and GND) and
> switched on in the program (`SPEED_KNOB_FITTED 1` in
> `src/pololu_maze_firmware.cpp`; setting it to 0 fixes the speed at 200).
> Check that it is securely mounted and its joints insulated before
> inspection.

| Item                 | Our design                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Part                 | A potentiometer, 10 kilo-ohm linear recommended (any value from 1 to 100 kilo-ohm works)                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Circuit              | One outer pin to 5 V, the other outer pin to ground (GND), the middle pin (wiper) to Arduino pin **A1**. The shield's battery-monitor jumper on A1 is confirmed open, so A1 is free.                                                                                                                                                                                                                                                                                                                                                                                   |
| How it works         | The knob is a voltage divider: its middle pin gives 0 V at one end of the turn and 5 V at the other. The Arduino measures that voltage as a number from **0 to 1023**, averaging 16 readings to smooth out noise.                                                                                                                                                                                                                                                                                                                                                      |
| Scaling equation     | **speed = 160 + reading x 80 / 1023**, so the full turn covers motor speed **160 to 240**, and the middle of the turn gives **200**, the speed proven on the maze. Below 160 the robot stalled on turns, so the range stops there.                                                                                                                                                                                                                                                                                                                                     |
| Effect on the motors | The speed number is the motor command, out of 400 for full power. The motor driver turns it into PWM, a fast on/off signal: 200 means the motors are on about half the time. Line following, the drive into each crossing and the turns at junctions all use it, and the drive-in time is scaled to it so the robot still goes about 1.5 inches. Three movements ignore the knob on purpose: the calibration rocking at the start always runs at 200 (it failed at speeds 160 to 171), and the slow centering turn after it and the search for a lost line run at 150. |
| When it is read      | Once at the start of every run, after the start press, so the speed cannot change during a run. The rules allow adjusting it before each round, never during a run.                                                                                                                                                                                                                                                                                                                                                                                                    |
| Proof                | With the USB cable in, the robot shows the knob live while it waits with the light on: every time the knob moves it prints `KNOB 529 of 1023 -> motor speed 202`, so it can be set without pressing reset. It also prints `SPEED knob=... -> motor speed ...` at power on and at every start, and each run's record stores the speed used.                                                                                                                                                                                                                             |

**Demonstration for the judges:** set the knob low, start a run, and point
out how slowly it drives; stop it with the button; turn the knob up, start
again, and it is visibly faster.

**Advice for official runs:**

- **Use the same knob setting for round one and round two.** Round two
  repeats round one's turns; at a different speed the robot stops at slightly
  different spots in crossings and can lose the path.
- **With fresh batteries, set the knob a little below the middle: about 370
  (speed about 188).** Both rounds passed without the cable at that setting
  on 2026-10-23. Fresh batteries make every setting faster; with older
  batteries the middle (about 512, speed 200) matches.
- The top of the knob (240) made the robot miss branches; avoid it for
  official runs.

## How it drives

1. **Follows the line.** If the tape drifts to one side, it slows that side's
   motor and curves back. The further off, the harder it steers.
2. **Ignores the light tape.** The maze has light tape where there is no path.
   Black reads about 800 to 1000 and light tape under 100; the robot only
   counts readings above 300 as black.
3. **Spots a junction.** When the far-left or far-right sensor sees black, a
   side path has started. It drives 1.5 inches further (our tape is 1 inch
   wide) and checks:
   - far-left or far-right sensor saw black → a **left** or **right** exit;
   - middle sensors still see tape → a **straight** exit;
   - all four middle sensors still black after 1.5 inches → **FINISH**, which
     is much longer than tape.

   If it finds only a straight exit, the "junction" was just the robot
   wobbling onto the edge of the tape, so it is ignored and never written
   into the path.

4. **Spots a dead end.** All six sensors see white at the end of a segment.
   All white **right after a turn** is not a dead end: the turn ended a little
   off the line, so the robot turns slowly back and forth to find it again.
5. **Turns (a controlled turn, rule 4.4).** It spins in place, one track
   forward and one backward, and stops when the new line passes under its
   inner sensor, so every turn ends on the tape. If that sensor starts on a
   line, it first turns off it, so a turn can never stop after a few degrees.
   A U-turn spins until it finds the line behind it.

## Round one: explore with the left-hand rule

At each junction: **left if possible, otherwise straight, otherwise right,
otherwise turn around.** Imagine walking the maze with your left hand always
on the wall.

Every move is stored as one letter: **L** left, **S** straight, **R** right,
**B** back (U-turn at a dead end).

## Round two: the shortcut

A **B** means the robot went into a dead end and came back. Every time a B
appears between two other moves, the three moves are replaced by the one move
that has the same effect. Add the turn angles (L = 270, S = 0, R = 90,
B = 180) and keep the remainder after 360:

| Moves around the dead end | Angle sum                  | Replaced by |
| ------------------------- | -------------------------- | ----------- |
| L B L                     | 270 + 180 + 270 = 720 → 0  | S           |
| L B S                     | 270 + 180 + 0 = 450 → 90   | R           |
| S B L                     | 0 + 180 + 270 = 450 → 90   | R           |
| L B R                     | 270 + 180 + 90 = 540 → 180 | B           |

Repeating this removes every dead end, every U-turn and the turns that led
into them. In round two the robot reads the shortened list and makes one move
per junction.

**Safety net in round two:** at every junction the robot checks that the exit
it planned to take is really there. If it is not (for example it miscounted a
junction), it knows it is lost, switches to the left-hand rule from that spot
and still drives to FINISH, just by a longer route. The saved path is kept
for the next attempt.

## Loops (rules 1 and 4.5)

The rules say the maze has loops: paths that lead back to a place the robot
has already been.

- **It does not get trapped.** Following one wall never circles forever when
  START is on the outer edge of the maze, because the outer wall is one
  unbroken boundary. Our START is on the outer edge.
- **Round two skips dead ends.** The shortcut above removes every dead end it
  explored.
- **Honest limit:** the left-hand rule always goes around a loop on the left
  side. If the right side of a loop is shorter, round two still takes the left
  side, because the robot never drove the right side.
- **What we built to go further:** our first version (below) drew a grid map
  of every junction, recognised when it came back to a junction it had seen,
  and used a shortest-path search on that map. That version can choose the
  shorter side of a loop, but it was less reliable on this maze.

## Running the robot

### The controls

- **Power switch** on the shield: ON / OFF.
- **Button**: the small pushbutton on the shield, **not** the Arduino's RESET
  button.
  - **Quick press** (under 2 seconds): start a run.
  - **Press during a run**: stop.
  - **Hold 10 seconds** while the light is on: erase the saved path. The
    light flickers after 2 seconds; keep holding until three slow blinks.
    Letting go earlier erases nothing.
- **Yellow light** on the shield: shows what the robot is doing.

| You see                                   | It means                                                              |
| ----------------------------------------- | --------------------------------------------------------------------- |
| Short chirp at switch-on, then light on   | Ready                                                                 |
| Light blinks for 1 second after the press | Hands off, it is about to move                                        |
| Robot rocks left and right                | Calibrating on the START line (normal)                                |
| Robot rocks a second or third time        | A sensor did not see the black line; it is calibrating again (normal) |
| A short slow turn after the rocking       | Centering on the line before it drives (normal)                       |
| Stopped on FINISH with a beep, light on   | Round finished; path saved                                            |
| Stopped somewhere else, light on, no beep | Run stopped: see Troubleshooting                                      |

### Before the competition

- Fresh AA batteries (low batteries make it slow and can restart it).
- Wipe the sensors under the front with a dry cloth.
- Check the tracks are clean and nothing is caught in them.
- Check solder joints and wires are tight and nothing rubs the tracks.
- Unplug the USB cable for official runs.

### Round one

1. Switch ON. The light comes on.
2. **Erase the old path:** hold the button 10 seconds, until three slow
   blinks. Do this only before round one.
3. Switch OFF. Place the robot at START on the black line, facing into the
   maze, with the line under the middle of the front edge.
4. **Photograph the placement**, so round two can start in exactly the same
   spot.
5. Set the speed knob if needed.
6. Switch ON, wait for the light. On the judge's signal press once quickly and
   take your hands away.
7. It blinks, rocks left and right, then explores. It stops on FINISH and
   beeps. The path is saved.

### Round two

1. Do **not** hold the button (that would erase the path).
2. Place the robot exactly as in the round-one photo. Switch ON.
3. On the judge's signal press once quickly, hands away. It rocks, then drives
   the short route to FINISH and beeps.

## Troubleshooting

| Problem                                                                 | Likely cause                                                                             | What to do                                                                                     |
| ----------------------------------------------------------------------- | ---------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| Nothing happens at switch-on, no chirp, no light                        | Batteries flat or in wrong way, switch not fully on                                      | Check the switch and battery direction, fit fresh batteries                                    |
| Press does nothing                                                      | Press was longer than 2 seconds                                                          | Press again, quickly                                                                           |
| It rocks once, then spins and stops after about 3 seconds               | Not on a black line, so it never found the line                                          | Place the sensors over the START line, press again                                             |
| It rocks three times, then stops (CALIBRATION_FAILED in the record)     | The line was not under the middle of the robot, so some sensors never passed over it     | Re-place with the line exactly under the middle of the front edge, press again                 |
| It rocks, turns slowly for a few seconds, then stops (NO_LINE_AT_START) | After calibrating it could not find the line again                                       | Re-place on the line, press again                                                              |
| Stops in the maze with the short switch-on chirp, light on              | The Arduino restarted, usually from weak batteries                                       | Fresh batteries if the judge allows, then retry                                                |
| Spins in place for 3 seconds, then stops                                | It lost the line during a turn                                                           | Retry from START if the judge allows                                                           |
| Stops on an ordinary crossing and beeps                                 | It mistook the crossing for FINISH                                                       | Retry; tell the code owner where it happened                                                   |
| Goes back and forth on the same stretch                                 | It lost the line on that stretch and keeps turning around                                | Press to stop, check that stretch of tape is flat and clean, retry                             |
| Wobbles a lot or misses junctions                                       | Dirty sensors, low batteries, strong sunlight, or speed knob set high                    | Wipe the sensors, fresh batteries, avoid direct sunlight; turn the knob back to the middle     |
| Much slower or faster than usual                                        | Speed knob moved                                                                         | Set the knob back to its mark (about 370 with fresh batteries)                                 |
| Round two gets lost although round one was fine                         | The knob was moved between the rounds                                                    | Use the same knob setting for both rounds                                                      |
| Round two drives the wrong way                                          | Not placed exactly as in round one                                                       | Re-place from the photo and retry; the path is still saved                                     |
| Round two suddenly starts exploring part way                            | It found a junction that did not match the saved path and switched to the left-hand rule | Let it run: it should still reach FINISH. Afterwards the record says at which move it got lost |
| START or the maze was changed                                           | The saved path belongs to the old START                                                  | Hold 10 s to erase, then run round one again                                                   |
| Round two explores instead (round-one behaviour)                        | No path saved: round one did not finish, or the path was erased                          | Run round one again                                                                            |
| Close to 5 minutes and not finished                                     | Lost or circling                                                                         | Ask the judge before touching it; touching ends the attempt                                    |

A retry after a failed **round one** starts round one again. A retry after a
failed **round two** drives the saved path again.

### Reading what happened (code owner)

The robot records every run. Plug in USB and run, from the project folder:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -MonitorOnly
```

The robot restarts and prints the last run: each junction, each turn, and how
it ended. "No end recorded" means it lost power or restarted mid-run.

To upload the program again (battery OFF, USB in):

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-competition-firmware.ps1 -Port COM4 -Pololu
```

## Safety (rule 3.7)

- Disconnect or switch off the batteries before touching any wiring.
- Never let battery terminals touch each other or metal.
- Keep fingers, hair, sleeves and loose wires away from the tracks.
- Eye protection when soldering, lead-free solder only.

## What we tried first, and what we learned

We first wrote our own version: it turned exactly 90 degrees using the gyro
and drew a map of the maze in memory. It completed both rounds on our practice
board, but on the competition maze small errors at crossings sent it onto the
wrong line and its map got confused. Pololu's approach of turning until the
line is under the sensor proved more forgiving, so we built on it.

Lessons from testing:

- Measure the real course: the competition tape is 1 inch wide, not the
  0.75 inch Pololu's example assumes, and that alone caused false finishes.
- The light non-path tape had to be measured too: it reads under 100, far
  below black, so a threshold of 300 ignores it safely.
- Faster is not always better: above speed 200 the batteries dipped and the
  Arduino restarted.
- Fresh batteries change the speed for the same knob setting, so set the
  knob by testing, and never change it between rounds.
- Check the calibration: a start-up rocking that never put some sensors over
  the black line made light tape look black.
- Pololu's turns could stop after a few degrees if a sensor began on a line;
  that one detail made round two lose its path.
- A record of every run, read over USB afterwards, found problems that
  watching the robot could not.

## Credits (rule 3.3)

- Pololu ZumoShield Arduino library and its MazeSolver example (Pololu
  Corporation), which is based on Pololu's 3pi maze-solving example.
- Pololu Zumo Shield User's Guide for wiring and pins.
- AI assistance: code and documentation were developed with help from
  Anthropic's Claude. Check that the course allows this and that it is
  disclosed on the poster, as rule 3.3 requires.

## Current status (update after testing)

- Pololu's original example finished round one on the competition maze
  (2026-10-22).
- **The competition version completed round one and round two without the
  USB cable on 2026-10-23** (knob 369, speed 188, fresh batteries; 23-move
  learned path). Settings are recorded in
  [known-good-config.md](known-good-config.md) under the tag
  `competition-ready-2026-10-23`.
- Speed knob: fitted to A1 on 2026-10-23 and switched on in the program;
  demonstrate it before the competition.
