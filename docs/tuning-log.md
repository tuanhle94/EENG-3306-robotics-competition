# Tuning log

## 2026-10-20 - first integrated autonomous run

- Stored calibration loaded successfully after reset without another manual
  sweep.
- A four-way feature reported mask `15`; the learner selected left.
- The robot completed the left turn at 88.4 degrees, reacquired the outgoing
  line, and resumed following.
- It then recognized a dead end as mask `8` and completed the commanded U-turn
  at 178.2 degrees.
- The first U-turn trial ended with `LINE_REACQUIRE_TIMEOUT`. The next candidate
  uses all six sensors for steering during reacquisition and performs a bounded
  slow search when the returning line is initially outside the sensor bar.
- The next integrated trial completed another 88.0-degree left turn. During
  reacquisition, sensor 4 clearly measured the outgoing line at 1000, but the
  firmware rejected it because neither center sensor was active. It then
  searched in the completed-turn direction instead of toward the last observed
  line. The corrected logic accepts a stable line anywhere inside the two outer
  sensors and bases fallback search direction on the last observed line side.
- A later trial physically reached an intersection but remained in the follow
  state until `SEGMENT_TIMEOUT`. That log contained no sensor or motor detail.
  The next diagnostic candidate prints bounded follow-state readings, junction
  scan state, motor commands, and the final timeout readings before changing
  intersection behavior.
- Detailed telemetry then showed an outer-only sensor-5 reading being accepted
  as line reacquisition. The reset junction detector immediately interpreted
  the same tape as a new right branch, causing an incorrect turn into the white
  area. Reacquisition now requires one of sensors 1 through 4 plus clear outer
  sensors. The dead-end grace period also now uses slow equal motor commands
  instead of steering from a synthetic lost-line edge, and fallback search
  alternates directions rather than committing to one side.
- The following run correctly rejected outer-only reacquisition, but sensor 5
  stayed near 208 while the robot was commanded `80,0` until timeout. Command
  80 is below the physically verified reliable minimum of 100. Outer-only
  recovery now performs an in-place command-100 pivot toward the observed side;
  all reacquisition and search motion also uses command 100.
- One follow-up run regressed before reaching the intersection: the center-line
  reading fell to 82 and remained unchanged while the temporary gap command was
  `50,50`. The robot's known reliable minimum is 100, so it had stalled and the
  150 ms detector timer falsely reported a dead end. Gap traversal is corrected
  to equal `100,100` commands.

Record every hardware configuration and tuning session. Do not replace old
results; add a dated entry so poster claims remain traceable to evidence.

Create one copy of the template for every test session, even if the run fails.
Record the settings before changing them. This makes it possible to explain
which change improved or harmed the robot.

If a field is unknown, write `unknown` instead of leaving it blank. Include
units such as volts, millimeters, milliseconds, and motor-command values.

## Entry template

- Date/time:
- Tester:
- Controller and shield revision:
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage:
- Surface and line width:
- Potentiometer ADC range observed:
- Sensor calibrated minima/maxima:
- Speed limits tested:
- PD gains:
- Turn settings:
- Runs passed / attempted:
- Failure observations:
- Change selected and rationale:

## Field explanations

- **Battery voltage:** measure or record the battery-pack voltage because motor
  behavior changes as batteries discharge.
- **Surface and line width:** describe the floor material and measure the black
  line; both affect sensor readings.
- **Potentiometer ADC range:** the lowest and highest Arduino readings observed
  while turning the speed knob.
- **Sensor minima/maxima:** the lightest and darkest reading observed by each of
  the six sensors during calibration.
- **Speed limits:** the minimum and maximum motor commands allowed during the
  test. The Pololu library accepts values from -400 through 400.
- **PD gains:** the numbers controlling how strongly and how quickly the robot
  corrects a line-position error.
- **Runs passed / attempted:** write a fraction such as `8 / 10`.
- **Rationale:** the evidence-based reason for keeping a change.

## 2026-10-19 initial hardware bring-up

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; 6 V nominal, unmeasured actual
- Surface and line width: white paper and wide matte-black test area; widths not recorded
- Potentiometer ADC range observed: no potentiometer connected; A1 is floating
- Sensor calibrated minima/maxima: preliminary white `1376,860,604,736,860,1244`; black `2000,2000,2000,2000,2000,2000`
- Speed limits tested: motor command 100 for 400 ms
- PD gains: not tested
- Turn settings: not tested
- Runs passed / attempted: drivetrain 1 / 1
- Failure observations: none reported
- Change selected and rationale: keep default motor polarity because equal
  positive left/right commands moved the robot straight toward the blade and
  the automatic 400 ms stop worked

## 2026-10-19 manual sensor calibration

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: battery switch off; sensors powered through USB
- Surface and line width: flat white surface with matte-black test area; widths not recorded
- Potentiometer ADC range observed: not tested
- Sensor calibrated minima/maxima: minimums `1288,1208,1000,1208,1132,1208`; maximums `2000,2000,2000,2000,2000,2000`; spans `712,792,1000,792,868,792`
- Speed limits tested: motors locked at zero
- PD gains: not tested
- Turn settings: not tested
- Runs passed / attempted: calibration 1 / 1; 3567 samples
- Failure observations: none; smallest span was 712 on sensor 0
- Change selected and rationale: accept a 500-count minimum calibration span
  because every physical sensor exceeded it with at least 212 counts of margin

## 2026-10-19 first straight-line trials

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; actual voltage unmeasured
- Surface and line width: straight matte-black line on white surface; dimensions not recorded
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: run minimums `1376,1292,1088,1208,1248,1376`; maximums all `2000`; spans `624,708,912,792,752,624`
- Speed limits tested: fixed command limit 100
- PD gains: proportional 1/4, derivative 6
- Turn settings: not tested
- Runs passed / attempted: straight-line following 0 / 2
- Failure observations: both attempts stopped with `LINE_LOST`; the first
  reached peak error 1192, and the second held position 2000 while commands
  saturated to `0,100` before reaching peak error 500
- Change selected and rationale: reduce the speed-100 diagnostic gains to
  proportional 1/12 and derivative 1, and initialize derivative history from
  the measured starting position so small offsets produce gradual steering

## 2026-10-19 gentler straight-line trial

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; actual voltage unmeasured
- Surface and line width: straight matte-black line on white surface; dimensions not recorded
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: calibration passed immediately before this run; detailed values not copied for this attempt
- Speed limits tested: fixed command limit 100
- PD gains: proportional 1/12, derivative 1; derivative history initialized from the measured start position
- Turn settings: not tested
- Runs passed / attempted: timed straight-line following 1 / 1
- Failure observations: none; firmware reported `LINE result=COMPLETED` and
  `peak_error=508`; the tester observed that the robot stayed on the black line
  and moved smoothly
- Change selected and rationale: retain these gains as the next-test baseline
  because the robot corrected from start position 2981 toward center 2500,
  followed smoothly for the full three-second window, and stopped automatically

## 2026-10-19 first gentle-curve trial

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; actual voltage unmeasured
- Surface and line width: gentle curved matte-black line on white surface; dimensions not recorded
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: calibration passed before the trial; detailed values not copied
- Speed limits tested: fixed command limit 100
- PD gains: proportional 1/12, derivative 1; reverse correction disabled
- Turn settings: not tested
- Runs passed / attempted: complete gentle-curve following 0 / 1
- Failure observations: the robot stayed on the black line but did not complete
  the curve. Position increased to 5000 and commands saturated at `100,0`.
  The old diagnostic incorrectly reported `COMPLETED` when its three-second
  timer expired, with `peak_error=2500`.
- Change selected and rationale: permit bounded reverse correction at command
  100 so the robot can pivot more tightly, and stop with
  `EDGE_NOT_RECOVERED` if the line remains at an extreme edge for 750 ms
  instead of reporting a misleading completion

## 2026-10-19 weak-line threshold observation

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; actual voltage unmeasured
- Surface and line width: curved matte-black line on white surface; dimensions not recorded
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: minimums `1448,1404,1204,1328,1288,1368`; maximums all `2000`; spans `552,596,796,672,712,632`
- Speed limits tested: fixed command limit 100
- PD gains: proportional 1/12, derivative 1; bounded reverse correction enabled
- Turn settings: not tested
- Runs passed / attempted: curve retry 0 / 1
- Failure observations: after reporting position 3000, normalized readings
  reached `0,0,0,130,0,0`. The estimator's old 200-count detection threshold
  rejected that remaining signal and stopped with `LINE_LOST`.
- Change selected and rationale: reduce the calibrated line-seen threshold from
  200 to 100 because the measured 130-count signal was still the tracked line;
  retain a separate 50-count contribution threshold and test that readings
  below 100 remain rejected as noise

## 2026-10-19 hard-floor calibration threshold

- Date/time: 2026-10-19; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: battery switch off; sensors powered through USB
- Surface and line width: white printer paper on a hard, flat floor; black reference readings approximately 2000
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: failed sweep minimums `1844,1536,1288,1368,1412,1620`; maximums all `2000`; spans `156,464,712,632,588,380`
- Speed limits tested: motors locked at zero
- PD gains: not applicable
- Turn settings: not applicable
- Runs passed / attempted: calibration 0 / 1 under the former 500-count rule
- Failure observations: all sensors distinguished the surfaces, but the two
  outside sensors had substantially less contrast than the middle sensors;
  Pololu's default pin order was confirmed in the installed library
- Change selected and rationale: use a 150-count minimum for this physical
  array because the weakest measured span was 156 and the organizer did not
  specify 500; retain per-sensor normalization and motion lockout on failure

## 2026-10-20 bounded-reverse curve retry

- Date/time: 2026-10-20; exact time not recorded
- Tester: team member
- Controller and shield revision: Arduino Uno; Zumo Shield v1.3
- Robot model: Pololu item #2510
- Motor gear ratio: 75:1 HP
- Battery type/voltage: four AA alkaline cells; actual voltage unmeasured
- Surface and line width: curved matte-black line on white paper over a hard floor; dimensions not recorded
- Potentiometer ADC range observed: no potentiometer connected
- Sensor calibrated minima/maxima: minimums `1632,1336,1048,1128,1164,1252`; maximums all `2000`; spans `368,664,952,872,836,748`
- Speed limits tested: fixed magnitude limit 100
- PD gains: proportional 1/12, derivative 1; bounded reverse correction enabled
- Turn settings: not tested
- Runs passed / attempted: complete gentle-curve following 0 / 1
- Failure observations: the robot turned only slightly. The estimated position
  settled near 4345 while commands settled near `100,-53`; the three-second
  timer expired with `peak_error=1871`.
- Change selected and rationale: use a full `100,-100` pivot when absolute
  position error reaches 1600, apply the 750 ms unrecovered-edge timer at that
  same boundary, and report `TIME_LIMIT_REACHED` instead of the ambiguous
  `COMPLETED`

### Follow-up from the full-pivot trace

The first full-pivot run still completed only a small part of the curve.
Position repeatedly moved from roughly 4110 to just below the 1600-error entry
boundary and then returned above it. The controller was therefore switching
between full pivot and normal driving. Recovery now remains active until
absolute error falls below 800. The derivative gain is also reduced from 1 to
0 because a position change from 2000 to 2410 caused an unnecessary
`100,-74` command near the center.

## 2026-10-20 practice-board geometry correction

- Evidence: overhead photograph `IMG_6583.jpeg`
- Observation: the board uses sharp 90-degree corners, T-junctions, crosses,
  and dead ends. It does not contain the smooth curve assumed by the previous
  diagnostic attempts.
- Decision: stop treating sharp corners as ordinary line-following errors.
  Add a separate low-speed turn diagnostic that watches an inside sensor cross
  onto and then off the selected branch, matching Pololu's sensor-feedback
  turn method. Route selection and complete junction handling remain separate
  future milestones.

### First physical pivot attempts

- Both LEFT and RIGHT attempts at motor command 100 ended with `TIMEOUT` and
  zero sensor transitions.
- The robot visibly rotated only a little, so command 100 did not overcome the
  track friction needed for an in-place turn.
- The next turn-only diagnostic uses command 200 with a two-second automatic
  stop. The verified line-following command remains 100.
- At command 200, the robot rotated much more effectively but continued to
  about 180 degrees. The original tracker waited for the selected sensor to
  enter and then leave the new line, so its stop condition occurred too late
  for this board geometry.
- The corrected tracker stops when the new line is detected for three
  consecutive readings. This rejects a one-sample spike without intentionally
  driving off the line.
- The physical retest still rotated about 180 degrees. Line crossing therefore
  remains dependent on placement and is not a reliable angle measurement for
  this board.
- Decision: replace line-based stopping in the isolated turn diagnostic with
  the already verified LSM6DS33 gyroscope. The new test calibrates stationary
  gyro bias, integrates the Z-axis rotation, slows after 70 degrees, and stops
  at a measured 88 degrees with a two-second safety timeout.
- First gyro-test start attempt was cancelled at zero degrees because raw
  button bounce looked like a second press during the countdown. Countdown
  cancellation was removed; an emergency button stop remains active after
  motor motion begins. Failed or stopped attempts also retain the same turn
  direction instead of alternating it.
- The corrected LEFT gyro turn reached the 70-degree slowdown point, continued
  at command 100, and stopped automatically with
  `TURN result=ANGLE_REACHED direction=LEFT angle_deg=88.0`. The full attempt
  remained within the two-second safety limit. The tester visually confirmed
  that the robot physically completed approximately a 90-degree quarter-turn.
- The first RIGHT gyro attempt reached 71.6 degrees, slowed to command 100,
  then stopped making physical progress. Gyro readings stayed near the resting
  offset and the two-second safety limit correctly reported `TIMEOUT` at 72.2
  degrees. This shows asymmetric drivetrain stiction during an in-place pivot.
- The slow-turn command is increased from 100 to 150 for both directions. The
  fast command remains 200 and the gyro stop target remains 88 degrees.
- The separate LEFT U-turn diagnostic ran at command 200, slowed to 150 at
  150 degrees, and stopped automatically with
  `TURN result=ANGLE_REACHED direction=LEFT angle_deg=178.1`. The physical
  result was accepted as the dead-end half-turn. Production dead-end reversal
  will consistently turn LEFT because either rotation direction produces the
  same final heading.

## 2026-10-20 first junction approach

- Target: center four-way intersection, approached on its vertical incoming
  line at command 100.
- Result: false `DEAD_END` before reaching the intersection.
- Evidence: normalized signal moved from sensor 3 toward sensor 2 and briefly
  fell below the 100-count line threshold (`0,0,0,50,0,0`, then
  `0,0,28,0,0,0`).
- Cause: during the detector's 150 ms line-loss grace period, the diagnostic
  kept both motors at 100 instead of steering toward the last-known line side.
- Correction: feed the estimator's last-side recovery position into the
  bounded non-reversing controller during a brief sensor gap, and print motor
  commands in telemetry. A physical retry is required.
- Retry evidence: the line stayed exclusively on sensor 3 and the controller
  continuously commanded `100,59` until the robot left the line. The Pololu
  steering convention is correct; the narrow tape quantized the physically
  centered start to position 3000 instead of mathematical center 2500.
- Correction: record the physically centered starting estimate and translate
  it to controller center 2500 for this run. A centered sensor-3 start now
  commands `100,100`, while later movement across the array still produces a
  steering error. A regression test covers the translation and clamping.
- Successful four-way retry: calibration spans were
  `1796,1836,1876,1836,1836,1836`. The physically centered start measured
  3003 and was recentered to 2500. Initial motor commands stayed close to
  `100,100`; the detector latched both side branches, found the straight
  continuation, stopped automatically, and reported
  `JUNCTION result=FEATURE exits=LSR`. This is the first physical four-way
  classification pass.
- Successful T-junction trial: calibration spans were
  `468,544,796,712,752,636`. The centered start measured 3000 and was
  recentered to 2500, producing equal `100,100` approach commands. The detector
  crossed the top T, found left and right branches with no straight
  continuation, stopped automatically, and reported
  `JUNCTION result=FEATURE exits=L-R`.
- Successful dead-end trial: the robot approached the right endpoint of the
  top horizontal segment. It briefly attempted bounded last-side recovery when
  the line disappeared, then correctly required persistent loss, stopped
  automatically, and reported `JUNCTION result=DEAD_END`.
- Successful one-sided right-corner trial: approaching the upper-left corner
  from the right, the detector latched only the robot-relative right branch,
  found no straight continuation, stopped automatically, and reported
  `JUNCTION result=FEATURE exits=--R`.
- Successful mirrored left-corner trial: approaching the middle-right corner
  from below, the detector latched only the robot-relative left branch, found
  no straight continuation, stopped automatically, and reported
  `JUNCTION result=FEATURE exits=L--`.

## 2026-10-20 integrated weak-start safeguard

- An integrated learning run began with only `224` normalized counts on the
  strongest inner sensor, declined to `82`, and was then classified as a dead
  end. Earlier successful approaches began near 700-1000 counts.
- The firmware had accepted any start above the normal 100-count tracking
  threshold. That was too permissive for placing the robot at START.
- Startup now requires at least 500 counts on one of the four inner sensors.
  A weak or misaligned start leaves the motors stopped and asks the operator to
  reposition the robot and press the button again.
- Diagnostics now print both raw and normalized sensor readings, peak signal,
  continuous line-loss time, and the final readings used for dead-end,
  segment-timeout, and reacquisition-timeout decisions.
- This safeguard addresses the observed bad start. A physical run is still
  required before treating the integrated firmware as competition-ready.
- The next run passed the new check with inner peak 853. While driving, sensor
  3 raw readings decreased from 1872 to 960 as sensor 4 increased from 288 to
  1004. Both normalized readings were briefly below the line threshold, and
  the old 150 ms timer declared a false dead end at 153 ms.
- Dead-end confirmation is now 350 ms. An automated regression test verifies
  that a 200 ms between-sensor gap is ignored, line reacquisition clears the
  timer, and a separate continuous 350 ms loss still reports a dead end.
- The following physical run reached a feature, completed an 88.1-degree left
  turn, and found the outgoing tape. Reacquisition was declared while the tape
  was still on sensor 4 near the edge. Normal following resumed with a hard
  `100,0` correction, swept across the feature, and produced a false second
  feature before the next turn timed out while searching for a line.
- Reacquisition now uses sensors 1 and 4 for steering but requires three stable
  readings on center sensor 2 or 3 before returning to normal following. A
  regression test rejects both an outer-only line and a sensor-4 edge line.
- The next run confirmed centered reacquisition after the first 88.1-degree
  left turn. At a later sharp left corner, however, the tape remained strongly
  on sensor 1 for about one second while the line controller was already
  steering left. The junction detector waited for outermost sensor 0, then
  commanded another complete 88.4-degree turn and ended away from all tape.
- A strong sensor-1 reading with both center sensors clear now starts a left
  corner scan; sensor 4 provides the mirrored right behavior. Center-plus-inner
  readings do not trigger this shortcut, preventing an offset straight line
  from being treated as a corner. Automated tests cover all three cases.
- Physical telemetry then showed that the earlier trigger worked, but the scan
  still drove straight for roughly another half-second while waiting for the
  outer tape to clear. The axle passed the corner before the 88-degree turn and
  no sensor could find the outgoing tape.
- A strong inner-side sensor with both center sensors clear now completes the
  sharp-corner feature immediately. Wide intersections still use the full
  outer-sensor scan. The mirrored corner tests now require immediate completion
  and an offset straight-line test still rejects the shortcut.
- The immediate-turn retry also missed the outgoing tape. It removed the old
  overshoot, but began rotating while the axle was still behind the corner.
- Sharp-corner centering is now sensor-based instead of choosing either timing
  extreme: inner sensor 1 or 4 starts a straight centering pass, and the turn
  begins as soon as outermost sensor 0 or 5 reaches the same tape. A 400 ms
  fallback bounds motion if the outer sensor never arrives. Tests cover left,
  right, timeout fallback, and rejection of an offset straight line.
- Untethered before-and-after photographs ruled out USB-cable drag and showed
  that the first center-intersection turn works. At the later far-left corner,
  the front sensor row reached the outgoing tape while the tracked chassis
  pivot was still several centimeters behind the corner, so turning on outer
  detection alone was too early.
- Course-photo scale indicates roughly 4 cm from the front sensor row to the
  pivot and about 6 cm/s at command 100. Sharp-corner logic now continues
  straight for a bounded 400 ms after outer detection before starting the gyro
  turn. A 1200 ms total fallback bounds the maneuver, and timing-boundary tests
  cover both left and right corners.
- The next physical run completed the 88-degree left turn, then held the line
  steadily on sensor 1 for the entire 2.5-second reacquisition timeout. The
  telemetry repeatedly showed `motors=0,100`, and the nearly unchanged sensor
  readings confirmed that the one-track correction was physically stalled.
- An intermediate candidate replaced the stalled one-track command with a true
  two-track pivot (`-100,100` for a line on the left edge and `100,-100` for a
  line on the right edge). This kept both track commands at the verified
  minimum that overcomes drivetrain stiction.
- The next run proved that both tracks moved, but also showed why an in-place
  pivot was the wrong motion. After the 88.4-degree left turn, the outgoing
  tape moved from sensor 5 to sensor 4 and then stayed under sensor 4 for the
  entire timeout while `motors=100,-100`. The robot was rotating around its
  axle instead of advancing onto the outgoing branch.
- Edge recovery now uses a forward arc. A right-edge line commands `200,100`;
  a left-edge line commands `100,200`. Both tracks remain at or above the
  measured anti-stiction minimum, while their speed difference steers the
  center sensors toward the tape. Regression tests assert these exact motor
  commands.

### Official Pololu MazeSolver comparison

- The Arduino Zumo Shield library's `MazeSolver` example was reviewed directly.
  Its most useful physical-control idea is to center the axle, rotate, and use
  reflectance-sensor feedback to finish alignment with the outgoing line.
- Its complete route algorithm cannot be copied because it explicitly assumes
  a maze with no loops, does not persist its route or calibration, and has no
  bounded turn timeout. The competition firmware keeps its loop-capable graph,
  EEPROM storage, gyro measurement, and safety stops.
- The hybrid controller keeps the physically verified gyro turn, then pivots
  at command 150 toward whichever edge sensor sees the outgoing tape. It stops
  and resumes following only after a center sensor confirms the line for three
  readings. Thus the gyro supplies the approximate angle and the reflectance
  array supplies the final alignment.
- Pololu's documented 0.75-inch-line overshoot is about 64 ms at command 200.
  Scaled to this robot's command-100 approach, the sharp-corner axle advance is
  now 130 ms rather than the earlier photo-derived 400 ms. The overall corner
  scan remains bounded at 700 ms.

## 2026-10-21 forward-track stiction correction

- A fresh stored calibration passed with spans `460,588,796,672,632,504`.
  The second start was correctly accepted with an inner peak of 763.
- During ordinary following, the line moved from sensor 2 to sensor 3. The
  controller then repeatedly commanded `motors=100,17`; raw and normalized
  readings stayed nearly unchanged until `SEGMENT_TIMEOUT`. This proves the
  chassis was physically stalled before reaching any maze feature.
- Command 100 is the measured breakaway minimum, so using 100 as both base
  speed and maximum leaves no usable differential-steering range. Competition
  following now uses base command 150 and a controller-enforced forward floor
  of 100. The failing position-3500 case therefore produces `150,100`, with
  the mirrored case producing `100,150`. A unit test also verifies that this
  floor cannot override a lower external safety limit.

### First full run with the anti-stiction floor

- The robot followed the first segment at command 150, completed the first
  88.3-degree left turn, aligned from outer sensor 5 through sensor 4 to a
  center sensor in about 150 ms, and resumed following. This verifies the
  gyro-plus-reflectance alignment sequence on a real intersection.
- It reached the next sharp left corner, correctly reported exits `L--`, and
  completed another 88.2-degree gyro turn. All six sensors were then on white.
  The bounded recovery alternated left and right as designed, but no tape was
  available anywhere in its sweep, so it timed out safely.
- To distinguish linear overshoot from angular overshoot, turn telemetry now
  includes all normalized sensor values. After 60 degrees on a quarter-turn
  (150 degrees on a U-turn), three stable center-sensor readings can complete
  the turn before the gyro limit. The gyro target and timeout remain the
  fallback safety bounds.
- The sharp-corner axle advance is reduced from 130 ms to 85 ms, matching
  Pololu's 0.75-inch reference after scaling from command 200 to this robot's
  command-150 approach speed.

### Inspect the pivot position before more timing changes

- The sensor-logged retry completed its first turn at 60.7 degrees. During the
  second turn, center sensors saw tape at 27.3 and 38.9 degrees; the 60-degree
  eligibility rule rejected those readings. By 88.1 degrees all sensors saw
  white. This confirms that tape crossed the sensor bar, but does not establish
  that the early crossing was aligned with the intended outgoing segment.
- A separate `uno_corner_inspection` build now stops before the second turn and
  stays stopped until reset. The operator can photograph the untouched robot
  relative to the corner. See [corner-inspection.md](archive/corner-inspection.md).
- This build uses the existing driving behavior and saved calibration. It
  selects learning without erasing or overwriting a saved route. No further
  turn thresholds or centering timings are changed for this inspection.

### Photo identifies a premature first turn and a false second junction

- The inspection log `c1894e53-3272-49d2-be69-ad5784b637b3` reports first-turn
  completion at 60.8 degrees. The stopped photo shows the robot diagonally
  across the outgoing line near the original intersection, well short of the
  far-left corner. The earlier claim that the robot had reached a real second
  corner was incorrect.
- Center-sensor tape detection during rotation was being treated as proof of
  correct chassis heading. After the incomplete turn the robot drove away from
  the line, then the inner-side corner shortcut classified that same line as
  a new junction. The learner assigned a false second node.
- Early sensor-based turn completion is removed. The gyro again requires its
  physically tested 88/178-degree targets. Before testing departure, a new
  `-InspectFirstTurn` build stops immediately after the first gyro turn, with
  no subsequent search or forward motion. Its photo will reveal any lateral
  error in pivot placement. Corner timing is unchanged in this experiment.

### Full first-turn checkpoint establishes remaining position error

- The first-turn inspection reached 88.2 degrees. The supplied photo shows the
  chassis approximately parallel to the left-going stripe. Final normalized
  readings `0,0,0,0,25,0` contain no line above the detection threshold of 100.
- During rotation, tape progressed across outer sensor 0 at 29.4 degrees,
  sensor 1 at 46.2, center sensor 3 at 63.8, and sensor 4 at 78.3. This supports
  a remaining pivot/line-position error. It does not by itself measure the
  required change in approach travel.
- `-SensorsOnly` now provides a passive check at the unchanged stopped pose.
  It reads saved calibration, prints raw/normalized values twice per second,
  and keeps both motors off. The button cannot start motion and setup cannot
  erase or write EEPROM. The operator measures the nearest sideways move that
  places a center sensor over the stripe. No speed or scan timing is changed.

### Reference-based total junction advance experiment

- Reviewed `Complete Source Code` and the Zumo 32U4 library's MazeSolver,
  GridMovement, TurnSensor, and LineFollower examples. The reusable pattern is
  to advance to the pivot position while collecting both side branches, then
  turn. Hardware APIs, five-sensor indexing, gyro scales, and timing values
  cannot be copied unchanged. See [reference-code-review.md](archive/reference-code-review.md).
- The downloaded shield sketch uses about 257 ms at command 200 for assumed
  0.75-inch tape. Scaling to command 150 gives an experimental 343 ms. This
  estimate has not been physically validated. The 32U4 example's separate
  130 ms setting does not confirm or supplement it.
- Competition builds now apply that total duration from first confirmed outer
  evidence, collecting later branch readings without restarting the clock.
  An inner-only possible corner cannot authorize a turn. An 800 ms deadline
  faults and stops rather than inventing a branch or driving indefinitely.
  The standalone diagnostic keeps its older timing.
- Added six native sequence tests covering branch memory, configurable timing,
  both inner-only corner directions, missing/late outer evidence, reset, and
  timer wraparound. Physical success is still pending.
- The next trial uses `-InspectFirstTurn`: stop after the unchanged 88-degree
  gyro turn, before alignment or departure. Compare the stopped position and
  center-sensor readings with the previous checkpoint. Do not change gyro
  angles or replace motor libraries in the same experiment.

### 343 ms first-turn result and Pololu comparison

- Two first-turn runs ended at 89.0 and 88.2 degrees with all six normalized
  sensors at 0. Tape now reached sensors 2 and 3 at 78.1 and 82.9 degrees
  (previously sensor 3 at 63.8), so the 343 ms advance moved the pivot
  closer to the intersection.
- The saved calibration was invalid: from logged raw/normalized pairs its
  minimums were about 1748, 1532, 1284, 1284, 1616, 1836, while white read
  164-332 raw. Partly covered tape (raw 940-1500) therefore normalized to 0.
  This hid the straight exit (mask 13 instead of 15), blinded steering, and
  made the final all-white reading meaningless. Span 150 accepted it.
- Pololu's `QTRSensors::calibrate()` records extremes from 10-read bursts.
  Calibration now does the same and also requires every minimum <= 600 and
  every maximum >= 1500. A stored calibration failing that check is rejected
  at startup and replaced by a new sweep.
- The search after a turn started in the turn direction and reversed every
  400 ms with equal duration, so it never reached tape the turn had passed
  (the earlier all-white timeout). The turn now records which sensors saw
  tape during its final 45 degrees; if the tape reached the far center
  sensor, the search starts back the other way. Reversals now last 400, 800,
  then 1200 ms, covering both sides within the unchanged 2.5 s timeout.
- At the start of both turns the gyro read 64 and 70 against boot offsets of
  -462 and -442, suggesting a battery-on bias. The offset is now re-measured
  during the last 2 seconds of the countdown; a shift above 2000 counts
  rejects the start as handled. The first measurement disproved the bias:
  countdown -418 versus boot -422. The turn-start reading is braking motion.
- Run 1 started with all six sensors dark and mapped START as a four-way
  node. A start with sensor 0 or 5 on tape is now rejected.
- New `-InspectFirstAlignment` build stops when the turn has found the new
  line. Turn telemetry adds `turn_summary` (first-tape angle per sensor,
  raw peaks, gyro peak) and `turn_end` (raw and normalized). Junction
  advance, turn angles, speeds, and every timeout are unchanged.
- Root cause of the bad calibration: the sweep was done with the battery
  OFF, as the procedure then said. Battery-off sweep: min `1588,1416,1212,
  1248,1540,1720` (FAIL). Immediately after, battery-on sweep: min
  `172,132,128,128,172,212`, max 2000, span 1788-1872 (PASS). The IR
  emitters need battery power; all calibration instructions now say ON.

### First alignment run with valid calibration: PASS

- Start at 2433, followed to the middle four-way crossing, mask 15, LEFT.
- Turn stopped at 88.3 degrees (gyro peak 23620, no clipping). The outgoing
  tape crossed sensor 0 at 45.9, sensor 1 at 54.6, and the center pair near
  70-80 degrees, so the robot turns roughly 10 degrees past the tape center.
  It still ended with the tape under sensors 3 and 4 (raw 944, 772), sweep
  inference said `right`, and the search confirmed the line in 16 ms.
- The earlier all-white result was the invalid calibration, not pivot
  placement. Advance (343 ms) and turn target (88 degrees) stay unchanged.
- The alignment build now follows the new line to the next feature and
  reports `follow_result`, to prove continuation.
- Follow-to-next-feature result: the overhead-angle photo shows the robot
  stopped at the far-left corner after turning left at the middle crossing
  and following the new line. The operator reported the repeated runs as
  working; their serial logs were not captured (monitor started before
  automatic `log2file` saving was added). Left-turn alignment is accepted
  on that basis; the full learning run is the next test.

### First full learning run: six features, then a blocked U-turn

- Log `logs/device-monitor-261021-223319.log`. The robot mapped six features
  without help: middle crossing (15, LEFT), far-left corner (9, LEFT), bottom
  T on the left (13, LEFT), bottom T at START (9, LEFT; loop closed onto
  node 0), middle crossing again (15, STRAIGHT), top T (13, LEFT), top-left
  stub (12, RIGHT), stub end (8, dead end). Every gyro turn found its
  outgoing line within 136 ms; the turn-sweep side inference was correct
  each time, including the first verified RIGHT turn.
- Stop: the dead-end U-turn reached 9 degrees, then the gyro read rest
  values for 3.5 s. The photo shows the robot against the board frame past
  the stub end. `TURN_TIMEOUT` stopped the motors as designed. The stub end
  is too close to the frame for an in-place spin.
- Misclassification: at the left bottom T a false right exit was recorded and
  the straight exit missed; at the START T the straight exit (the only path
  toward FINISH) was missed. At completion the continuation was under sensor
  4 (normalized 400) or 5, outside the 2-3 straight check.
- Cause: the chassis yaws left at equal commands. During every 150,150
  junction advance the line moved right across the array, and in several
  segments saturated `150,100` merely held the line parallel under sensor
  3.5-4.2. Right gyro turns also peaked lower (23381) than left (25000-26000).
- An earlier attempt (`logs/device-monitor-261021-223157.log`) shows the same
  cause: the left bottom T was read as mask 12 (right and back only), and
  later the line drifted under sensor 4, started an inner-corner probe that
  drives straight, slipped off the array, and stopped with
  `JUNCTION_SCAN_TIMEOUT` with no outer confirmation.
- Changes: straight exits now use sensors 1-4 as in Pololu's MazeSolver
  (timed competition path only). The follower's outer track may rise to 200
  while the inner track keeps the 100 floor, doubling steering margin. Base
  speed, gains, advance, turn angles and every timeout are unchanged.

### Second learning run: seven correct features, then a phantom branch

- Log `logs/device-monitor-261021-224755.log`. START was beside the FINISH
  square, facing east along the bottom line. Correctly mapped: bottom-right
  corner, right-side top corner, center-line junction (STRAIGHT), bottom-left
  T, its dead-end stub (first full U-turn, 178.5 degrees, line found in
  46 ms), the same T again (loop closed), and the left-side top corner
  (RIGHT).
- With the wider steering range, the first segment held the line at
  2500-2650 with commands near 153,147, so the earlier left drift is not a
  fixed motor imbalance. Westbound it needed about 192,108, and northbound on
  the left side the line stayed under sensors 4-5 even at 200,100. The bias
  depends on where the robot is and which way it faces, which points to the
  dragged USB cable or the board sagging on the mattress.
- Failure: 100 ms after the right turn onto the middle bar, the robot was
  still crossing that line at an angle; sensor 0 read it as a left branch.
  The robot turned left into empty space, and the search found tape only at
  2287 ms, so `LINE_REACQUIRE_TIMEOUT` stopped it.
- Change: after every turn, branch entry is ignored until the line is on a
  center sensor with both outer sensors clear for 3 readings, or 400 ms.
  Dead-end detection stays active. Telemetry reports `branch_detection=ON`.

### Third learning run: phantom corner from the inner-corner probe

- Log `logs/device-monitor-261021-225808.log` (1202 lines). Started again
  beside FINISH. The first six features and the U-turn were mapped exactly
  as in the second run. Leaving the bottom-left T northbound (STRAIGHT) with
  the line under sensor 4, the inner-corner probe started at once, drove
  718 ms at 150,150 without steering, and confirmed a "corner" when the same
  line drifted onto sensor 5 halfway up the left side. From then on the map
  and the board diverged; the run ended with `NO_EXPLORATION_ROUTE` after
  12 features.
- Across all four learning logs, every branch reached with reasonable
  centering read at least 3 adjacent sensors at 300 or more when first
  confirmed. The 1-2-wide confirmations were the phantoms, and the probe
  caused the phantom corner here, the false right exit and the
  `JUNCTION_SCAN_TIMEOUT` in earlier runs. No real corner depended on it.
- Change: in timed (competition) mode an inner-only line no longer starts a
  scan; following keeps steering, and a branch needs an outer sensor at 500
  or more. The diagnostic mode keeps its probe. Three tests were rewritten
  to cover the new rule and the unchanged 800 ms scan deadline.

### FINISH marker is a black dot

- The team reports that FINISH is a large black dot at the end of a short
  branch, with START far away. All six sensors black for 400 ms would likely
  miss a ~10 cm dot crossed at about 25 cm/s (outer sensors covered for
  roughly 300 ms).
- In every log so far, sensors 1-4 were dark together for at most two
  consecutive 50 ms telemetry samples at crossings. FINISH is now sensors
  1-4 at 700 or more for 200 ms, following Pololu's sensors 1-4 rule.
- Each feature line now reports `wide_dark_ms`, the longest such stretch in
  that segment, so the crossing-versus-FINISH margin is measured on every run.

### First completed learning round

- Log `logs/device-monitor-261021-231719.log`. Ten features including two
  dead ends with U-turns; both revisited junctions matched their first
  observation, so the map stayed consistent. FINISH was recognized with
  `wide_dark_ms=201`; crossings in the same run measured 46-61 ms.
- Result: `EEPROM route=SAVED length=7`, `MISSION result=ROUND_ONE_COMPLETE`.
- Round two, same log after reset: `EEPROM route=VALID length=7`, mode
  REPLAY. All six replay decisions found the required exit (LEFT, RIGHT,
  STRAIGHT, LEFT, RIGHT, RIGHT); each turn ended at 88.0-88.8 degrees and
  found its line in 15-181 ms. Both dead ends were skipped. FINISH at
  `wide_dark_ms=203` (crossings 57-60 ms): `MISSION result=ROUND_TWO_COMPLETE`.
- This is one complete two-round run on the practice board, tethered by USB.
  Repeatability, an untethered run, and the real competition maze and finish
  dot size remain unverified.

### Larger maze: two dead ends merged on one grid square

- Earlier untethered attempts on the new maze did not start: the robot was
  still in REPLAY mode with the old 7-step route, and the logged starts were
  refused because the front sensors sat on wide black or on white.
- After clearing the route, round one completed (log
  `logs/device-monitor-261021-235349.log`): 21 features, four dead ends,
  five correct loop closures, `EEPROM route=SAVED length=11`.
- Replaying the log through the learner showed a map error. Every segment
  counts as one grid step, so a second, different dead end landed on the
  first dead end's square and was merged with it, linking two junctions
  through a wall. The saved route did not use that link, but another maze
  could.
- Every true revisit in all logs repeated the first visit's exits exactly;
  the false merge did not (east only versus north only). The learner now
  merges only when the exits match. Otherwise it makes a separate node with
  no grid square, and nothing reached from it is merged by position either.
  An arrival whose square already has a different edge on that side is
  also kept separate instead of faulting. Firmware logs `map_relocated`.
- Replaying both logs: the earlier maze gives the same 7-step route with no
  relocation; the new maze gives the same 11-step route with the two dead
  ends kept apart. A regression test replays the new maze's sequence.
- Round two on the larger maze (same log, before the merge fix was
  uploaded): all ten replay decisions found their exits, turns ended at
  88.0-88.7 degrees with the line found within 136 ms, every dead end was
  skipped, and FINISH at `wide_dark_ms=201` gave `ROUND_TWO_COMPLETE`.

### Merge check confirmed; search pivot momentum made a phantom corner

- Log `logs/device-monitor-261022-000837.log`, round one with the merge
  fix: `map_relocated reason=EXITS_DIFFER_FROM_SAME_SQUARE` kept a junction
  separate from an earlier dead end on the same square. Round one completed.
- The saved route was 13 steps instead of 11. At one junction the previous
  run correctly found a short dead end. This run needed a 292 ms
  `align-left` search pivot after the turn, began following while still
  rotating, and steered further left because the line was left of center.
  The line swept from sensor 1 to sensor 5 within about 350 ms and was
  confirmed as a right corner. The post-turn guard passed because the line
  crossed the center pair on the way.
- Change: if the search had to pivot, the robot now stops for 120 ms (the
  existing post-turn settle time) and checks the line again before
  following. A line still on a center sensor resumes following; otherwise
  the search continues inside the same 2.5 s limit. Logs
  `reacquire_brake`. Searches that found the line without moving are
  unchanged.

### Brake confirmed; a sideways offset made the search pick the wrong branch

- Log `logs/device-monitor-261022-001656.log`. The dead end misread in the
  previous run was mapped correctly, with `reacquire_brake` after each
  pivoting search. The run reached FINISH but stopped with
  `MAP_ARRIVAL_FAILED`: the map had used all 32 nodes.
- Cause: after one left turn the new line lay under sensor 0, parallel to
  the robot but beside it. The alignment pivot kept it under sensor 0 for
  over 400 ms, then kept spinning for 1.7 s across other tape and settled on
  a different branch. Later features no longer matched the board, creating
  extra nodes.
- Across all logs, successful searches held the line on an outer sensor for
  at most six 50 ms samples; the failures held it for 34 and 50 samples.
  After 400 ms with an outer sensor on tape the search now arcs forward
  toward the line at 100/200 (the follower's full correction) instead of
  pivoting, and logs `reacquire_arc`. With no line visible it still pivots.
- Next round one on the same maze (`logs/device-monitor-261022-002544.log`)
  observed the same 21 features in the same order as the correct 23:53 run,
  relocated the second dead end, and saved the same 11-step route
  (`N W N E N N W W N N E`, 17 map nodes). Nine pivoting searches braked and
  confirmed within 263 ms; the arc fallback was not needed.

### Separate left quarter-turn target

- Gyro stops were very repeatable (88.0-89.1 degrees), but over 55 left and
  21 right quarter-turns the line was under the center pair at the stop in
  33% of left turns versus 70% of right turns, usually to the left. The
  outgoing tape reached the near-center sensor at a median 71.5 degrees in
  left turns (sensor 1) versus 64.4 in right turns (sensor 4).
- Left quarter-turns now target 95 degrees and slow at 77; right turns and
  U-turns are unchanged. Check the centered-at-stop rate in the next logs.
- An untethered run got stuck; no log exists. Calibration had been made
  with USB connected.

### Automatic calibration at every start

- Manual calibration was hard to repeat, and a USB-powered calibration may
  not suit a battery-only run. Following Pololu's MazeSolver, the robot now
  calibrates itself after every countdown: it pivots 50 degrees left, 50
  right, and back to its starting heading by gyro (about 2 s) while taking
  10-read bursts. It must see real white and black on every sensor
  (min <= 600, max >= 1500); otherwise the start is refused, the previous
  calibration is restored, and the light stays solid. A pass is saved to
  EEPROM. A 5 s limit faults and stops the motors.
- The hand-sweep calibration states were removed. Operators only clear the
  old path before round one; the run card and operator guide were updated.
- Not yet run on the robot. First check: `CAL span=` values near 1800,
  `end_angle_deg` near 0, and start accepted.

### Ten-second hold clears everything

- Holding the button at power-on used to erase the saved route at once,
  which an operator could do by accident. A three-press confirmation was
  tried and judged too fiddly.
- Now, while the robot is ready (solid light): a press shorter than 2 s
  starts the run when released; holding 10 s erases the saved route and
  calibration, confirmed by three slow blinks; letting go between 2 and
  10 s does nothing. The light flickers from 2 s to show the hold. A hold
  begun at power-on also counts toward the 10 s but never starts a run.
- A corrupt stored route is now cleared automatically instead of stopping
  the robot at power-on, and a corrupt calibration image is ignored, since
  every start recalibrates.
- A startup regression from the automatic-calibration change (setup ended
  silently after loading the route) was found on the robot and fixed.

### 95-degree left turns reverted; arc trigger tightened

- Log `logs/device-monitor-261022-011525.log`. The 10 s clear worked.
  Automatic calibration passed (spans 1564-1828, end angle 5.0 degrees).
- Left turns at 95 degrees ended centered 4/14 (29%), versus 33% at 88;
  misses moved from mostly left to both sides (3 left, 5 right, 2 none).
  The miss is not a steady angle bias, so the target is back to 88.
- The run ended with `LINE_REACQUIRE_TIMEOUT`. After a left turn over a
  junction, the search swept the line across the center pair at 201 ms,
  but other tape stayed under sensor 5, so the line was not accepted. The
  arc rule counted that as a sideways offset, arced away, and kept arcing
  toward faint edge readings until the timeout.
- The arc now needs an outer sensor dark and the center pair clear for
  400 ms, and arcs only toward a reading of 500 or more. Against every
  logged search, only the earlier sideways failure (about 450 ms) qualifies;
  successful searches peak at about 300 ms, and this run's failure never
  qualifies.

### Run recorder for untethered runs

- Untethered runs left no evidence. The firmware now records each run in
  EEPROM (bytes 96-1023, up to 223 four-byte events): start (mode,
  smallest calibration span, calibration end angle), every junction (exits,
  FINISH, wide-dark time), decision, turn end (angle and where the line sat
  under the sensors), line search (time, brake, arc), and the final result
  text. Writes happen only while the motors are stopped.
- At every power-up the previous run is printed as `LOG` lines before
  anything else. Plugging in USB restarts the board, so opening the monitor
  (`-MonitorOnly`) after a cable-free run prints and saves its record. A new
  run overwrites it once automatic calibration passes.

### First untethered record: phantom right corner after a straight

- Recorded round one on the 11-step maze stopped with
  `NO_EXPLORATION_ROUTE` at feature #11. Exits and decisions #1-#9 match the
  tethered 00:25 round one exactly. At #10 the tethered run saw a left corner
  (`feature_mask=9`, LEFT); the untethered run saw `--R`, turned right with no
  line under the sensors (`line_at_stop=none`), searched 340 ms and reached a
  dead end. The real left exit was never mapped, so no unexplored exit was
  left and the learner stopped correctly for its map.
- Mechanism: #9 was a STRAIGHT. In the tethered run the robot left that
  crossing with the line at position 4123 (sensor 4 at 903, sensor 5 at 59)
  because of the known rightward drift during the 150,150 advance. Timed
  junction entry needs only sensor 5 at 500 or more, so slightly more drift
  reads as a right branch. Branch entry was already held after turns but not
  after a straight.
- Change: a STRAIGHT action now applies the same hold as a turn: branch entry
  is ignored until three samples have the line centered with both outer
  sensors clear, or 400 ms. In the tethered run the real corner came about
  850 ms after the straight (17 samples at about 50 ms), well after the hold
  ends.
- The recorder now adds a `branch_trigger` line to each junction with the
  outer sensor values and line position at the sample that opened the scan,
  so a drift trigger (line near 4500, sensor 5 just over 500) can be told
  from a real branch (line near center).
- Rejected: requiring the center sensors to be dark at branch entry. Logged
  real right branches entered with the center pair at 4-222, so that rule
  would miss real exits.


## 2026-10-22 competition maze: light non-path tape

The competition maze draws the path in black tape and the rest of the grid
in light tape, which made the robot lose its way. Static readings with the
sensor-only build, battery on, stored practice-board calibration:

| Spot | raw 0-5 | normalized 0-5 |
|---|---|---|
| Plain white board | 272,236,204,204,236,308 | 11,14,20,21,13,27 |
| Black tape centered | 268,200,268,1416,516,304 | 9,0,60,818,170,25 |
| Light tape centered | 272,204,204,308,308,308 | 11,0,20,89,53,27 |

- Light tape reads 86-89, just under the 100 line threshold, and above the
  50 position-signal threshold, so it already pulled the steering estimate
  and any tilt or glare could make it count as line (straight exits, missed
  dead ends, following the wrong tape).
- Black on this board peaks at raw 1416 versus 1724 on the practice board.
  Automatic calibration at each start measures the board actually used.
- Change: line-seen threshold 100 -> 200 in the firmware, junction detector
  and line estimate; position signal threshold 50 -> 150.
- Evidence the real line keeps clear of 200: in 2,543 logged follow samples
  the peak was below 200 only 6 times and 300 or more 99.6% of the time.
  Straight exits at junction completion read 398 or more except two
  ambiguous samples (229, 89) taken beside a full left branch.
- Still to measure: light tape under one outer sensor, and where light tape
  meets black tape. Not yet run on the robot.

## 2026-10-22 competition maze: circling one block (MAP_ARRIVAL_FAILED)

Tethered round one on the competition maze (log 18:20), with the 200
line threshold. The light tape caused no false features: 36 features, all
masks plausible, auto-calibration passed (spans 1476-1748).

- Every real segment took 1.5-2.0 s of following (one grid square).
- Feature 19 was a 4-way. The LEFT turn ended with only sensor 0 on tape,
  so the search pivoted left for 610 ms, swept across the whole crossing
  (all six sensors dark at 459-561 ms), braked and followed. 100 ms later
  it reported a junction (mask 13) on the same crossing. The map took that
  as a new square, facing north, while the robot faced another arm.
- From there every coordinate was wrong. The next arrival collided with
  an existing square with different exits, became a detached node, and
  past a detached node no square can merge, so the robot circled one block
  (LEFT at masks 13, 9, 15, 9 repeatedly) until the 32-node map was full.

Changes:
- The gyro is integrated continuously while moving (counter-clockwise
  positive, starting from the calibration twist's end angle). At each
  feature the heading is rounded to a grid direction and snapped when
  within 30 degrees, so bias drift cannot build up. In learn mode the map
  uses the measured direction when it differs from the intended one
  (`heading_corrected`). Replay only logs it.
- A feature less than 600 ms after a search that pivoted is the same
  junction (`still_at_junction`): the learner keeps its node, takes the
  measured heading and decides again from the known exits.
- A segment over 3.0 s counts as round(ms / 2000) squares, for a black line
  running straight past a crossing that only has light tape.
- Map capacity 32 -> 48 nodes (the maze has about 60 crossings); RAM 1287 of
  2048 bytes. The saved route stays capped at 32 directions.
- The recorder adds a `heading_by_gyro` line per feature and always keeps a
  slot for the final result.
- Not fixed: why the turn ended with the outgoing arm only under sensor 0
  (likely over-advance at that crossing). Not yet run on the robot.

## 2026-10-22 Pololu MazeSolver as the competition base

- The stock ZumoShield MazeSolver example (speed 200) completed round one on
  the competition maze. It advances only one tape width (64 ms) past a
  branch and turns until sensor 1 or 4 crosses the outgoing line, so a turn
  always ends on a line however far the robot drove into the crossing.
- With higher SPEED/TURN_SPEED it stopped dead without the finish beep. The
  example only stops the motors at the finish (which beeps), so the board
  most likely restarted on a battery dip under the higher current.
- New build `uno_pololu_maze` (`src/pololu_maze_firmware.cpp`, script
  option `-Pololu`) keeps the example's driving, path simplification and
  speed 200 unchanged, and adds: the path saved in EEPROM (bytes 400-503) so
  round two survives power off; short press starts, 10 s hold clears; the
  calibration sweep at every start; a button press stops a run; turns and
  the sweep time out after 3 s; a run record (bytes 512-1023) printed at the
  next power on, which reports a run with no end as a power loss or restart.
- The gyro-turn firmware (`uno_competition`) is unchanged and still builds.
- Not yet run on the robot.
- Measured: black tape 1 inch wide (the example assumes 0.75); finish is a
  rectangle about 4 x 2 inches. The single check 0.75 inch past a crossing's
  first edge read a 4-way as the finish. A second look one tape width later
  then missed the real finish (the robot was past the dot, turned to explore
  and timed out on white), so it was removed.
- Change: LINE_THICKNESS 1.0; the straight-exit and finish check is now made
  1.5 inches past the first edge (a 1 inch crossing has ended; the dot still
  covers sensors 1-4 even entered along its 2 inch side). Replay drives the
  same 1.5 inches through each crossing. Serial junction lines now include
  turn duration and the sensors at the check, for the repeated
  dead end -> U-turn -> dead end pattern.

## 2026-10-23 speed knob (rule 4.3)

- The competition guidelines (rule 4.3, 15 of 70 competition points) require
  an external speed control read by the program. `uno_pololu_maze` now reads
  a potentiometer wiper on A1 (16-sample average) at power on and at every
  start: speed = 120 + reading x 160 / 1023, used for driving and turning,
  so the middle of the knob gives the verified 200. The junction drive-in
  time already scales with SPEED; it assumes speed is proportional to the
  command, so settings far from 200 need their own test on the maze.
- With nothing wired, A1 floats near 430-446 (blocker B-12), giving about
  187-190, so the robot still runs.
- Not yet wired or tested on the robot.

## 2026-10-23 first two-round success on the competition maze

- `uno_pololu_maze` (commit `1e90283`, tag `known-good-2026-10-23`), USB
  connected. Round one at knob 298 / speed 166: 49 features, 5 dead ends,
  FINISH read all six sensors at 1000 at the 1.5 inch check, path of 26
  moves saved. Round two at knob 370 / speed 177: 26 moves, FINISH.
- Two earlier starts at speed 160 and 163 timed out during the calibration
  sweep; cause not confirmed.
- Full configuration and log: [known-good-config.md](known-good-config.md),
  [runs/2026-10-23-two-round-success.log](runs/2026-10-23-two-round-success.log).

## 2026-10-23 untethered start turned around after the calibration sweep

- Without the USB cable the robot rocked (calibration sweep), then turned
  around at START. The sweep ends with Pololu's left spin that stops when
  sensor 1 crosses the line; nothing checked that the line was centred.
  Most likely it ended off the line, saw only white, took it for a dead end
  and made a U-turn. With the cable the same start worked, probably because
  the cable's drag slowed the spin. Not confirmed: the run record was not
  read before this change.
- Change: after the sweep the robot turns at 150 toward the line until its
  position is within 500 of the centre (2500), stops for 100 ms and checks
  again. If no line is found within 3 s it stops with NO_LINE_AT_START
  instead of turning around. The run record adds the line position after
  the sweep and after centering (`LOG after sweep ...`).
- Not yet run on the robot; `known-good-2026-10-23` still holds the version
  without this step.
- Next untethered tries (knob 298-331, speed 166-171) ended `TURN_TIMEOUT`
  with no junction and no centering record: the calibration sweep itself
  timed out. All sweep timeouts so far were at speed 160-171 (two of them
  tethered); the successful rounds ran at 166 and 177. Suspected: too little
  pivot power at the low end of the knob or with weak batteries. The run
  record now notes which sweep swing or turn timed out and how many line
  edges it saw (`LOG timed out in ...`).

## 2026-10-23 no knob fitted: speed fixed at 200

- The potentiometer is not fitted yet, so A1 floated: the "knob" readings
  260-415 were noise and set random speeds of 160-184 at each start. This
  explains the varying speeds, and the calibration sweep timeouts all fell
  at the low end (160-171). The two-round success ran at 166 and 177 by
  chance.
- Change: `SPEED_KNOB_FITTED 0` in `src/pololu_maze_firmware.cpp`; speed
  is a fixed 200 (Pololu's default) until the knob is wired to A1. Set it
  to 1 after fitting the potentiometer.
- Not yet run on the robot.

## 2026-10-23 cable works, battery-only does not: recording the supply

- With USB connected, round one again finished (same 26-move path; the new
  centering step reported 2043 -> 2158). Untethered runs instead timed out
  in the sweep, saved a path of 34 left turns (circling, then a false
  finish), and restarted mid-run (record with no end).
- USB powers the Arduino from the laptop; without it the Arduino's 5 V
  rail comes from the 4 AA batteries through the shield and can sag under
  motor load, affecting sensor timing and causing brown-out resets.
- Change: the program measures its own supply against the internal 1.1 V
  reference at the start, during the calibration sweep and at every
  junction, and records the start value and the lowest value
  (`LOG supply at start mV=`, `lowest_supply_mV=`).
- Not yet run on the robot.

## 2026-10-23 proof of concept: round one without the cable

- The centering build (tag `poc-untethered-2026-10-23`, commit `b3a10d3`)
  completed round one without USB: after sweep 2040, after centering 2140,
  49 features, FINISH, the same 26-move path as every successful run. Speed
  164 from the unconnected knob pin. Round two untethered not yet recorded
  for this build.
- The build after it (fixed speed 200, supply voltage and timeout records)
  has not run yet.

## 2026-10-23 speed knob fitted

- Potentiometer wired: wiper to A1, ends to 5 V and GND. `SPEED_KNOB_FITTED`
  set to 1, so speed = 120 + reading x 160 / 1023 again.
- The calibration sweep now always pivots at 200 (`SWEEP_SPEED`), whatever
  the knob says: every sweep timeout so far was at 160-171.
- Not yet run on the robot.

## 2026-10-23 round two: final sweep spin found no line

- Untethered round two "turned around at START": the record shows
  `timed out in turn L, line edges seen=0 of 2` at speed 202, supply 5080 mV.
  The first tethered attempt failed the same way (supply 4829 mV); the next
  tethered attempt completed all 26 moves (centering 2153 -> 2314). So the
  four sweep swings finished, but Pololu's closing turn('L'), which waits
  for sensor 1 to cross the line, saw no line in 3 s and kept spinning.
  Intermittent; supply was healthy.
- Change: the closing turn('L') is removed; centering does that job with
  all six sensors, turning toward the side where the line was last seen,
  and if it sees no line within 1.5 s it searches the other way (4.5 s in
  total, then NO_LINE_AT_START). Each sensor's calibrated range is printed
  and the weakest is recorded (`LOG calibration weakest sensor=`).
- Round two with the cable: knob 533, speed 203, 26 moves, FINISH.
- Not yet run on the robot.

## 2026-10-23 knob range 160-240

- At knob minimum (speed 120) the robot "got confused and turned around"
  at START: the closing sweep spin then ran at the knob speed and crept.
  Successful runs so far: 164-203. Below about 160 the timed drive into
  crossings also falls short.
- Change: SPEED_MIN 160, SPEED_MAX 240 (speed = 160 + reading x 80 / 1023);
  the middle stays at 200, so the knob mark at about 530 still gives 201.
- Result: with this build round two finished at speed 172 (after sweep
  2280, after centering 2440, supply 5080 mV), and the live display tracked
  the knob over its full range, 0 -> 160 to 1023 -> 240. Log:
  [runs/2026-10-23-knob-round-two.log](runs/2026-10-23-knob-round-two.log).

## 2026-10-23 a different START: long path, round two lost at move 29

- Round one from a new START learned a 49-move path with no dead ends to
  remove (`LLSLRRLRLRLRLRLLSLSSLRRRSSSS` then the tail of the usual route):
  the left-hand rule walked the long way and path shortening only removes
  dead ends, not the longer side of a loop.
- Round two followed 28 moves, then at move 29 turned R where there was no
  line (`timed out in turn R, line edges seen=0`). Pololu's replay only
  counts junctions, so one miscount shifts every later turn.
- Change: round two examines each junction exactly as round one does (same
  1.5 inch drive and readings). The planned turn is taken only if that exit
  is there; otherwise the robot records the move number and what it saw,
  explores with the left-hand rule from there to FINISH, and keeps the
  saved path. Reaching FINISH early also ends round two. Outcome
  `ROUND_TWO_FINISH after losing the saved path`.
- Not fixed: choosing the shorter side of a loop needs a map.
- Not yet run on the robot.
- Likely cause of the move-29 shift: false junctions. An outer sensor
  touching the tape edge (wobble) or light tape starts a junction check
  that finds only a straight exit (`-S-` appears in earlier logs). Round
  one records it as S; if round two does not trigger it at the same spot,
  or triggers one round one did not, every later turn shifts.
- Change: a junction with only a straight exit is ignored in both rounds:
  not added to the path in round one, not counted as a step in round two
  (`MAZE ignored: only a straight exit`).

## 2026-10-23 bad calibration: light tape read as black

- Round one from the new START saved 16 left turns. The printed calibration
  was `332,544 288,460 172,500 172,676 252,1424 332,2000` (raw min,max per
  sensor): sensors 0-3 never saw the black line (black is 1400-2000), so
  white and light tape crossed the 300 threshold and every outer-sensor
  touch looked like a left exit. The earlier move-29 run also had
  `weakest sensor=1 span=176`. Log:
  [runs/2026-10-23-bad-calibration-round-one.log](runs/2026-10-23-bad-calibration-round-one.log).
- Cause: Pololu's sweep decides a swing has crossed the line from the
  calibration being built; with a tiny range early on, sensor flicker
  counts as crossings and swings end before the line passes the other
  sensors.
- Change: swing crossings are judged from raw readings (black above 800;
  white and light tape 200-330). After the sweep every sensor must have a
  maximum of at least 1000 and a range of at least 600, otherwise the sweep
  is repeated (3 tries) and then the run stops with CALIBRATION_FAILED.
- Not yet run on the robot.

## 2026-10-23 round two at 240 and 204; calibration bar lowered

- At 240: round two missed a left branch at move 2 (saw `-SR`), the safety
  net explored with the left-hand rule and circled one block about five
  times before reaching FINISH. Supply dipped to 4760 mV. Earlier tries at
  240 failed calibration three times and once could not find the line after
  the sweep. 240 is not reliable; use about 200, the round-one speed.
- At 204: the calibration check rejected a sweep whose weakest sensor
  reached 848, and the retry timed out.
- Change: calibration bar 1000/600 -> 700/400 (bad sweeps reached at most
  676; usable ones 848 and up).
- Known limit, unchanged: round two cannot be shorter than round one's
  left-hand route minus dead ends; loops are not shortened.

## 2026-10-23 fresh batteries: false dead end after a left turn

- Untethered round one from the new START with fresh batteries (knob 202)
  circled the maze about three times and never reached FINISH. Compared
  with the successful round one from the same START (`... L-- L, L-R L,
  -SR S, L-- L, FINISH`), this run reported `--- B` right after the left
  turn at the `L-R` junction, on every lap: the turn ended off the line,
  all six sensors read white, and the robot took it for a dead end. Log:
  [runs/2026-10-23-fresh-batteries-false-dead-end.log](runs/2026-10-23-fresh-batteries-false-dead-end.log).
  The operator confirmed it turned around a few turns before FINISH.
- Change: all white within 500 ms of starting a segment (that is, just
  after a turn or crossing) is not a dead end. The robot turns at 150
  toward where the line was last seen, reversing after 700 ms, for up to
  2.1 s, and resumes following once sensor 2 or 3 is on the line; only if
  it finds nothing is it a dead end. Recorded as
  `LOG lost the line just after junction N`.
- Fresh batteries make the same knob setting faster; set the knob lower
  (about 180) and use the same setting for both rounds.
- Not yet run on the robot.

## 2026-10-23 untethered round one at 188 (fresh batteries)

- Build with the round-two exit check and fallback, straight-only junctions
  ignored, raw-reading sweep with calibration check (700/400), and the
  search after losing the line just after a turn.
- Knob 378, speed 188, fresh batteries, no cable. Supply 5080 mV
  throughout; weakest calibrated sensor range 1664; centering 2380.
- 39 junctions, FINISH at the 39th, path of 24 moves
  `LLLRRLRLSRSLRLLRLLSRRSSL`. The left turn that caused the false dead ends
  (junctions 36-37, `L-R L` then `-SR S`) went through; the line search was
  not needed. Log:
  [runs/2026-10-23-untethered-round-one-speed-188.log](runs/2026-10-23-untethered-round-one-speed-188.log).
- Round two with this build: not yet run.

## 2026-10-23 round two lost at move 19: turns that end at once

- Round two of the 24-move path (knob moved from 378 to 496: speed 188 in
  round one, 198 in round two) matched moves 1-18, then at move 19 planned
  S but found `L-R`, so the fallback explored to FINISH by a longer route.
  Move 18 is the left turn at the T a few junctions before FINISH, the same
  turn that produced false dead ends earlier. Log:
  [runs/2026-10-23-round-two-lost-at-move-19.log](runs/2026-10-23-round-two-lost-at-move-19.log).
- Cause (likely): Pololu's turn counts edges on sensor 1 (or 4) from an
  assumed white start. If that sensor is already on black when the turn
  begins, leaving it counts as the first edge and the turn ends after a few
  degrees. Earlier logs show turns of 6-61 ms against a normal 300-500 ms.
  Whether the sensor starts on black depends on where the robot stops in
  the crossing, which changes with speed and batteries.
- Change: a turn first spins until the watched sensor is off any line, then
  counts the new line's two edges as before.
- Keep the knob at the same setting for both rounds.
- Not yet run on the robot; round one must be relearned with this build.
- Result: round one with the turn fix, no cable, knob 369 / speed 188,
  supply 5080 mV, weakest calibrated sensor range 1328, centering 2240:
  38 junctions, FINISH at the 38th, 23-move path
  `LLRRLRLSRSLRLLRLLSRRSSL`. The left turn at the T (junction 21, `L-R L`)
  led to the `-SR` junction as it should. Log:
  [runs/2026-10-23-round-one-turn-fix-188.log](runs/2026-10-23-round-one-turn-fix-188.log).
  Round two with this path: not yet run.

## 2026-10-23 both rounds without the cable: competition version

- With the turn fix (commit `92c5b7f`), knob 369 / speed 188, fresh
  batteries: round one completed (23-move path, above) and round two drove
  the saved path to FINISH, both without the USB cable, as reported by the
  operator. Tagged `competition-ready-2026-10-23`. The robot's record of
  round two is still to be read over USB and saved.

## 2026-10-24 only the competition program is kept

- Removed the first program (`competition_firmware.cpp`), its modules,
  headers and unit tests, and its inspection builds. `platformio.ini` now
  has one build, `uno_pololu_maze`, and the upload script builds it by
  default (`-Pololu` still accepted). The competition program is unchanged:
  it builds to 16,126 bytes, the same as `competition-ready-2026-10-23`.
- Older versions remain reachable through their tags.
