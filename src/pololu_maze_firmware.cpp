#if defined(POLOLU_MAZE_FIRMWARE) && !defined(UNIT_TEST)

// Competition firmware built on Pololu's Zumo Shield MazeSolver example
// (examples/MazeSolver in the ZumoShield library). The stock example ran
// round one on the competition maze at speed 200 on 2026-10-22 when our
// gyro-turn firmware did not, so its driving is kept as it is: follow a
// segment until an outer sensor sees a branch, drive one tape width
// further, take the leftmost exit, and turn until sensor 1 or 4 crosses the
// outgoing line.
//
// Added for the competition:
// - the learned path is saved in EEPROM, so round two survives power off;
// - short press starts, a 10 s hold clears the saved path;
// - the calibration sweep runs at every start, not only after power on;
// - a button press during a run stops it, and turns have a time limit;
// - each run is recorded in EEPROM and printed at the next power on.

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <ZumoShield.h>

// SENSOR_THRESHOLD is a value to compare reflectance sensor
// readings to to decide if the sensor is over a black line.
// The competition maze's light tape reads below 100 (2026-10-22).
#define SENSOR_THRESHOLD 300

// ABOVE_LINE is a helper macro that takes returns
// 1 if the sensor is over the line and 0 if otherwise
#define ABOVE_LINE(sensor)((sensor) > SENSOR_THRESHOLD)

// Motor speeds (motor command, 400 = full power). Rule 4.3 requires an
// external speed control, so both are set from the potentiometer on A1 at
// the start of every run: knob reading 0..1023 maps linearly to
// SPEED_MIN..SPEED_MAX, with the middle of the knob at 200. 200 is Pololu's
// default and the only setting verified on the competition maze; faster
// settings stopped the robot dead with no finish beep, which points to the
// board restarting on a battery dip.
// The potentiometer was wired to A1, 5 V and GND on 2026-10-23. Set to 0 to
// ignore it and run at SPEED_FIXED (an unconnected A1 floats and gives
// random speeds).
#define SPEED_KNOB_FITTED 1
#define SPEED_FIXED 200
// The calibration sweep always pivots at 200: every sweep timeout so far
// happened at 160-171, so a low knob setting must not weaken it.
#define SWEEP_SPEED 200
#define SPEED_KNOB_PIN A1
// Every successful run so far was at 164-203. At the old minimum of 120 the
// closing spin crept and the robot spun at START (2026-10-23), and below
// about 160 the timed drive into each crossing falls short, so the knob
// covers 160-240 with its middle still at 200.
#define SPEED_MIN 160
#define SPEED_MAX 240
int SPEED = 200;
int TURN_SPEED = 200;

// Thickness of your line in inches. The competition maze uses 1 inch tape
// (Pololu's example assumed 0.75).
#define LINE_THICKNESS 1.0

// How far past a crossing's first edge the robot looks for a straight exit
// and the finish. A 1 inch crossing has ended by then (0.5 inch margin); the
// finish rectangle (4 x 2 inches) still covers sensors 1-4 even if it is
// entered along its 2 inch side. At the example's 0.75 inch a 4-way still
// covered sensors 1-4 and ended round one on 2026-10-22.
#define CHECK_DISTANCE 1.5

// When the motor speed of the zumo is set by
// motors.setSpeeds(200,200), 200 is in ZUNITs/Second.
// A ZUNIT is a fictitious measurement of distance
// and only helps to approximate how far the Zumo has
// traveled. Experimentally it was observed that for
// every inch, there were approximately 17142 ZUNITs.
#define INCHES_TO_ZUNITS 17142.0

// When the Zumo reaches the end of a segment it needs
// to find out three things: if it has reached the finish line,
// if there is a straight segment ahead of it, and which
// segment to take. OVERSHOOT tells the Zumo how far it needs
// to overshoot the segment to find out any of these things.
#define OVERSHOOT(line_thickness)(((INCHES_TO_ZUNITS * (line_thickness)) / SPEED))

namespace {

// A U-turn at speed 200 takes about one second; three means it never found
// a line and would otherwise spin until the batteries die.
const unsigned long kTurnTimeoutMs = 3000;
const unsigned long kStartPressMaxMs = 2000;
const unsigned long kClearHoldMs = 10000;
const unsigned long kButtonDebounceMs = 50;
const unsigned long kHandsOffMs = 1000;
// Centering after the calibration sweep: the line counts as centred within
// one sensor spacing of the middle (position 2500 of 0..5000), and the
// robot turns at the 150 command verified for slow pivots.
const unsigned int kCenterTolerance = 500;
const int kCenterTurnSpeed = 150;
const unsigned long kCenterSettleMs = 100;
// With no line seen yet, search the first way this long, then the other way.
const unsigned long kCenterReverseMs = 1500;
const unsigned long kCenterTimeoutMs = 4500;

const uint8_t kMaxPath = 100;
// EEPROM layout, clear of the gyro firmware's route (0-36) and calibration
// (64-90). Path: magic, version, length, turns, checksum.
const int kPathAddress = 400;
const uint8_t kPathMagic = 'P';
const uint8_t kPathVersion = 1;
// Run record: three-byte events followed by an end marker.
const int kRecordAddress = 512;
const uint8_t kRecordBytes = 3;
const uint8_t kMaxRecords = (1024 - kRecordAddress) / kRecordBytes;
const uint8_t kRecordEnd = 0xFF;

enum Outcome : uint8_t {
  OUTCOME_ROUND_ONE_FINISH = 1,
  OUTCOME_ROUND_TWO_FINISH = 2,
  OUTCOME_BUTTON_STOP = 3,
  OUTCOME_TURN_TIMEOUT = 4,
  OUTCOME_PATH_TOO_LONG = 5,
  OUTCOME_SAVE_FAILED = 6,
  OUTCOME_NO_LINE_AT_START = 7,
  OUTCOME_ROUND_TWO_FINISH_EXPLORING = 8,
  OUTCOME_CALIBRATION_FAILED = 9
};

}  // namespace

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;

// path[] keeps a log of all the turns made
// since starting the maze
char path[kMaxPath] = "";
unsigned char path_length = 0; // the length of the path
bool havePath = false;

// Set when the run must end; every blocking loop checks it.
bool stopped = false;
uint8_t stopOutcome = 0;
bool buttonWasUp = false;

bool recording = false;
uint8_t recordSlot = 0;
uint8_t junctionCount = 0;
// A junction's record is written once the robot is following again, so the
// 3 ms per EEPROM byte never delays a turn or the drive through a crossing.
bool pendingJunction = false;
uint8_t pendingExits = 0;
char pendingTurn = 0;
// Serial only: calibrated sensors at the junction check and turn duration.
unsigned int checkSensors[6];
unsigned long lastTurnMs = 0;

bool buttonDown() { return digitalRead(ZUMO_BUTTON) == LOW; }

void recordEvent(uint8_t type, uint8_t a, uint8_t b) {
  // Keep the last slot for the end of the run.
  const uint8_t reserve = type == 'E' ? 1 : 2;
  if (!recording || recordSlot + reserve >= kMaxRecords) return;
  const int base = kRecordAddress + recordSlot * kRecordBytes;
  EEPROM.update(base, type);
  EEPROM.update(base + 1, a);
  EEPROM.update(base + 2, b);
  ++recordSlot;
  EEPROM.update(kRecordAddress + recordSlot * kRecordBytes, kRecordEnd);
}

// The Arduino's own 5 V supply in millivolts, measured against the chip's
// internal 1.1 V reference. With USB the laptop supplies it; without USB it
// comes from the 4 AA batteries and can sag when the motors pull hard.
// Untethered runs on 2026-10-23 timed out, circled and restarted while the
// same runs with USB worked, so every run now records this.
uint16_t readSupplyMillivolts() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC)) {}
  const uint16_t reading = ADC;
  return reading == 0 ? 0 : static_cast<uint16_t>(1125300UL / reading);
}

uint16_t lowestSupplyMv = 0xFFFF;

void noteSupply() {
  const uint16_t mv = readSupplyMillivolts();
  if (mv < lowestSupplyMv) lowestSupplyMv = mv;
}

void startRecording(bool replay) {
  recording = true;
  recordSlot = 0;
  junctionCount = 0;
  pendingJunction = false;
  recordEvent('S', replay ? 1 : 0, static_cast<uint8_t>(SPEED / 2));
  const uint16_t mv = readSupplyMillivolts();
  lowestSupplyMv = mv;
  recordEvent('V', static_cast<uint8_t>(min(mv / 20U, 254U)), 0);
  Serial.print(F("SUPPLY at start="));
  Serial.print(mv);
  Serial.println(F(" mV"));
}

// The knob position, 0..1023, averaged as the wiper can be noisy.
int knobReading() {
  long total = 0;
  for (uint8_t i = 0; i < 16; ++i) total += analogRead(SPEED_KNOB_PIN);
  return static_cast<int>(total / 16);
}

int speedForKnob(int reading) {
  return SPEED_MIN + static_cast<int>(
      static_cast<long>(reading) * (SPEED_MAX - SPEED_MIN) / 1023);
}

// Reads the speed knob and sets the driving and turning speed for this run.
void readSpeedKnob() {
#if !SPEED_KNOB_FITTED
  SPEED = SPEED_FIXED;
  TURN_SPEED = SPEED;
  Serial.print(F("SPEED knob not fitted -> fixed motor speed "));
  Serial.print(SPEED);
  Serial.println(F(" of 400"));
  return;
#endif
  const int reading = knobReading();
  SPEED = speedForKnob(reading);
  TURN_SPEED = SPEED;
  Serial.print(F("SPEED knob="));
  Serial.print(reading);
  Serial.print(F(" of 1023 -> motor speed "));
  Serial.print(SPEED);
  Serial.println(F(" of 400"));
}

void printExits(uint8_t exits) {
  Serial.print(exits & 1 ? 'L' : '-');
  Serial.print(exits & 2 ? 'S' : '-');
  Serial.print(exits & 4 ? 'R' : '-');
}

void flushJunction() {
  if (!pendingJunction) return;
  pendingJunction = false;
  ++junctionCount;
  noteSupply();
  recordEvent('J', pendingExits, static_cast<uint8_t>(pendingTurn));
  Serial.print(F("MAZE #"));
  Serial.print(junctionCount);
  Serial.print(F(" exits="));
  printExits(pendingExits);
  Serial.print(F(" turn="));
  Serial.print(pendingTurn);
  Serial.print(F(" turn_ms="));
  Serial.print(lastTurnMs);
  Serial.print(F(" sensors="));
  for (uint8_t i = 0; i < 6; ++i) {
    if (i) Serial.print(',');
    Serial.print(checkSensors[i]);
  }
  Serial.println();
}

void noteJunction(uint8_t exits, char turnTaken) {
  pendingExits = exits;
  pendingTurn = turnTaken;
  pendingJunction = true;
}

const __FlashStringHelper* outcomeName(uint8_t outcome) {
  switch (outcome) {
    case OUTCOME_ROUND_ONE_FINISH: return F("ROUND_ONE_FINISH (path saved)");
    case OUTCOME_ROUND_TWO_FINISH: return F("ROUND_TWO_FINISH");
    case OUTCOME_BUTTON_STOP: return F("BUTTON_STOP");
    case OUTCOME_TURN_TIMEOUT: return F("TURN_TIMEOUT (spun 3 s without finding a line)");
    case OUTCOME_PATH_TOO_LONG: return F("PATH_TOO_LONG");
    case OUTCOME_SAVE_FAILED: return F("PATH_SAVE_FAILED");
    case OUTCOME_NO_LINE_AT_START: return F("NO_LINE_AT_START (lost the line after the calibration sweep)");
    case OUTCOME_CALIBRATION_FAILED: return F("CALIBRATION_FAILED (some sensors never saw the black line in 3 sweeps)");
    case OUTCOME_ROUND_TWO_FINISH_EXPLORING: return F("ROUND_TWO_FINISH after losing the saved path (explored the rest)");
    default: return F("UNKNOWN");
  }
}

void recordOutcome(uint8_t outcome) {
  flushJunction();
  noteSupply();
  recordEvent('E', outcome,
              static_cast<uint8_t>(min(lowestSupplyMv / 20U, 254U)));
  recording = false;
  Serial.print(F("MAZE result="));
  Serial.print(outcomeName(outcome));
  Serial.print(F(" lowest_supply="));
  Serial.print(lowestSupplyMv);
  Serial.println(F(" mV"));
}

void printPreviousRun() {
  if (EEPROM.read(kRecordAddress) != 'S') {
    Serial.println(F("LOG previous run: none recorded"));
    return;
  }
  Serial.println(F("LOG ===== previous run, recorded on the robot ====="));
  bool ended = false;
  for (uint8_t slot = 0; slot < kMaxRecords; ++slot) {
    const int base = kRecordAddress + slot * kRecordBytes;
    const uint8_t type = EEPROM.read(base);
    if (type == kRecordEnd) break;
    const uint8_t a = EEPROM.read(base + 1);
    const uint8_t b = EEPROM.read(base + 2);
    if (type == 'S') {
      Serial.print(F("LOG start round="));
      Serial.print(a ? F("TWO (saved path)") : F("ONE (exploring)"));
      Serial.print(F(" speed="));
      Serial.println(b * 2U);
    } else if (type == 'J') {
      Serial.print(F("LOG junction exits="));
      printExits(a);
      Serial.print(F(" turn="));
      Serial.println(static_cast<char>(b));
    } else if (type == 'V') {
      Serial.print(F("LOG supply at start mV="));
      Serial.println(a * 20U);
    } else if (type == 'W') {
      Serial.print(F("LOG timed out in "));
      if (a >= '1' && a <= '4') {
        Serial.print(F("calibration sweep swing "));
        Serial.print(static_cast<char>(a));
        Serial.print(F(" of 4"));
      } else {
        Serial.print(F("turn "));
        Serial.print(static_cast<char>(a));
      }
      Serial.print(F(", line edges seen="));
      Serial.print(b);
      Serial.println(F(" of 2"));
    } else if (type == 'G') {
      Serial.print(F("LOG lost the line just after junction "));
      Serial.print(a);
      Serial.println(b ? F("; found it again") : F("; not found, treated as dead end"));
    } else if (type == 'X') {
      Serial.print(F("LOG lost the saved path at move "));
      Serial.println(a);
    } else if (type == 'Y') {
      Serial.print(F("LOG   planned turn="));
      Serial.print(static_cast<char>(a));
      Serial.print(F(" but exits="));
      printExits(b);
      Serial.println(F("; explored from there"));
    } else if (type == 'K') {
      Serial.print(F("LOG calibration weakest sensor="));
      Serial.print(a);
      Serial.print(F(" span="));
      Serial.println(b * 8U);
    } else if (type == 'C') {
      Serial.print(F("LOG after sweep line_position="));
      if (a == 255) Serial.print(F("none"));
      else Serial.print(a * 20U);
      Serial.print(F(" after centering="));
      if (b == 255) Serial.println(F("none"));
      else Serial.println(b * 20U);
    } else if (type == 'E') {
      Serial.print(F("LOG END "));
      Serial.print(outcomeName(a));
      if (b != 0) {
        Serial.print(F(" lowest_supply_mV="));
        Serial.print(b * 20U);
      }
      Serial.println();
      ended = true;
    }
  }
  if (!ended) {
    Serial.println(F("LOG no end recorded: the robot was switched off or the board restarted during the run (a battery dip resets it)."));
  }
  Serial.println(F("LOG ===== end of previous run ====="));
}

uint8_t pathChecksum(uint8_t length) {
  uint8_t sum = kPathMagic ^ kPathVersion ^ length;
  for (uint8_t i = 0; i < length; ++i) {
    sum = static_cast<uint8_t>((sum << 1) | (sum >> 7));
    sum ^= EEPROM.read(kPathAddress + 3 + i);
  }
  return sum;
}

bool loadPath() {
  if (EEPROM.read(kPathAddress) != kPathMagic ||
      EEPROM.read(kPathAddress + 1) != kPathVersion) {
    return false;
  }
  const uint8_t length = EEPROM.read(kPathAddress + 2);
  if (length > kMaxPath ||
      EEPROM.read(kPathAddress + 3 + length) != pathChecksum(length)) {
    return false;
  }
  for (uint8_t i = 0; i < length; ++i) {
    const char step = static_cast<char>(EEPROM.read(kPathAddress + 3 + i));
    if (step != 'L' && step != 'R' && step != 'S' && step != 'B') return false;
    path[i] = step;
  }
  path_length = length;
  return true;
}

bool savePath() {
  EEPROM.update(kPathAddress, kPathMagic);
  EEPROM.update(kPathAddress + 1, kPathVersion);
  EEPROM.update(kPathAddress + 2, path_length);
  for (uint8_t i = 0; i < path_length; ++i) {
    EEPROM.update(kPathAddress + 3 + i, static_cast<uint8_t>(path[i]));
  }
  EEPROM.update(kPathAddress + 3 + path_length, pathChecksum(path_length));
  const uint8_t savedLength = path_length;
  char saved[kMaxPath];
  for (uint8_t i = 0; i < savedLength; ++i) saved[i] = path[i];
  const bool ok = loadPath() && path_length == savedLength;
  for (uint8_t i = 0; ok && i < savedLength; ++i) {
    if (path[i] != saved[i]) return false;
  }
  return ok;
}

void clearPath() {
  EEPROM.update(kPathAddress, 0);
  path_length = 0;
  havePath = false;
}

void printPath() {
  Serial.print(F("PATH length="));
  Serial.print(path_length);
  Serial.print(F(" turns="));
  for (uint8_t i = 0; i < path_length; ++i) Serial.print(path[i]);
  Serial.println();
}

void stopRun(uint8_t outcome) {
  motors.setSpeeds(0, 0);
  if (!stopped) {
    stopped = true;
    stopOutcome = outcome;
  }
}

// A fresh press during a run stops it. The start press has to be released
// first, so it cannot stop the run it started.
void checkStopButton() {
  if (!buttonDown()) {
    buttonWasUp = true;
  } else if (buttonWasUp) {
    stopRun(OUTCOME_BUTTON_STOP);
  }
}

void showReady() {
  digitalWrite(13, HIGH);
  Serial.println(havePath
      ? F("READY round TWO: short press drives the saved path. Hold 10 s to clear it.")
      : F("READY round ONE: short press explores. Hold 10 s to clear memory."));
}

void blinkSlowly(uint8_t times) {
  for (uint8_t i = 0; i < times; ++i) {
    digitalWrite(13, LOW);
    delay(400);
    digitalWrite(13, HIGH);
    delay(400);
  }
}

// Waits on the READY light. A press shorter than 2 s starts a run when it is
// released; holding 10 s clears the saved path. Anything in between is
// ignored so an accidental long press neither starts nor erases.
void waitForStart() {
#if SPEED_KNOB_FITTED
  int shownKnob = -100;
  unsigned long lastKnobCheckMs = 0;
#endif
  while (true) {
#if SPEED_KNOB_FITTED
    // Show the knob live while waiting, so it can be set without a reset.
    if (millis() - lastKnobCheckMs >= 250) {
      lastKnobCheckMs = millis();
      const int reading = knobReading();
      if (reading > shownKnob + 8 || reading < shownKnob - 8) {
        shownKnob = reading;
        Serial.print(F("KNOB "));
        Serial.print(reading);
        Serial.print(F(" of 1023 -> motor speed "));
        Serial.print(speedForKnob(reading));
        Serial.println(F(" (middle: about 512 -> 200)"));
      }
    }
#endif
    if (!buttonDown()) continue;
    delay(kButtonDebounceMs);
    if (!buttonDown()) continue;
    const unsigned long pressedMs = millis();
    bool cleared = false;
    while (buttonDown()) {
      const unsigned long held = millis() - pressedMs;
      if (held >= kStartPressMaxMs && !cleared) {
        // Flicker: keep holding to clear, or let go to cancel.
        digitalWrite(13, (held / 100U) % 2U);
      }
      if (held >= kClearHoldMs && !cleared) {
        clearPath();
        cleared = true;
        Serial.println(F("CLEARED saved path. Next run explores (round one)."));
        blinkSlowly(3);
        digitalWrite(13, HIGH);
      }
    }
    const unsigned long held = millis() - pressedMs;
    delay(kButtonDebounceMs);
    digitalWrite(13, HIGH);
    if (cleared) {
      showReady();
      continue;
    }
    if (held < kStartPressMaxMs) return;
    Serial.println(F("Long press ignored: press quickly to start."));
  }
}

// Turns according to the parameter dir, which should be
// 'L' (left), 'R' (right), 'S' (straight), or 'B' (back).
// Pololu counts the first edge from an assumed white start. When the
// watched sensor already sits on black (the line being left, or the edge of
// a crossbar), leaving it counted as the first edge and the turn stopped
// after a few degrees: turns of 6-61 ms instead of 300-500 appear in the
// 2026-10-23 logs, and round two reached a different junction after the left
// turn at a T and lost its path. Turn off any line under the sensor first.
bool leaveLineUnder(uint8_t sensor, char dir, unsigned long startedMs) {
  unsigned int sensors[6];
  reflectanceSensors.readLine(sensors);
  while (ABOVE_LINE(sensors[sensor])) {
    checkStopButton();
    if (millis() - startedMs >= kTurnTimeoutMs) {
      recordEvent('W', static_cast<uint8_t>(dir), 0);
      stopRun(OUTCOME_TURN_TIMEOUT);
    }
    if (stopped) return false;
    reflectanceSensors.readLine(sensors);
  }
  return true;
}

void turn(char dir)
{
  if (stopped) return;

  // count and last_status help
  // keep track of how much further
  // the Zumo needs to turn.
  unsigned short count = 0;
  unsigned short last_status = 0;
  unsigned int sensors[6];
  const unsigned long startedMs = millis();
  lastTurnMs = 0;

  // dir tests for which direction to turn
  switch(dir)
  {

  // Since we're using the sensors to coordinate turns instead of timing them,
  // we can treat a left turn the same as a direction reversal: they differ only
  // in whether the zumo will turn 90 degrees or 180 degrees before seeing the
  // line under the sensor. If 'B' is passed to the turn function when there is a
  // left turn available, then the Zumo will turn onto the left segment.
    case 'L':
  case 'B':
      // Turn left.
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      if (!leaveLineUnder(1, dir, startedMs)) return;

      // This while loop monitors line position
      // until the turn is complete.
      while(count < 2)
      {
        reflectanceSensors.readLine(sensors);

        // Increment count whenever the state of the sensor changes
    // (white->black and black->white) since the sensor should
    // pass over 1 line while the robot is turning, the final
    // count should be 2
        count += ABOVE_LINE(sensors[1]) ^ last_status;
        last_status = ABOVE_LINE(sensors[1]);
        checkStopButton();
        if (millis() - startedMs >= kTurnTimeoutMs) {
          recordEvent('W', static_cast<uint8_t>(dir), static_cast<uint8_t>(count));
          stopRun(OUTCOME_TURN_TIMEOUT);
        }
        if (stopped) return;
      }

    break;

    case 'R':
      // Turn right.
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      if (!leaveLineUnder(4, dir, startedMs)) return;

      // This while loop monitors line position
      // until the turn is complete.
      while(count < 2)
      {
        reflectanceSensors.readLine(sensors);
        count += ABOVE_LINE(sensors[4]) ^ last_status;
        last_status = ABOVE_LINE(sensors[4]);
        checkStopButton();
        if (millis() - startedMs >= kTurnTimeoutMs) {
          recordEvent('W', static_cast<uint8_t>(dir), static_cast<uint8_t>(count));
          stopRun(OUTCOME_TURN_TIMEOUT);
        }
        if (stopped) return;
      }

    break;

    case 'S':
    // Don't do anything!
    break;
  }
  lastTurnMs = millis() - startedMs;
}

// This function decides which way to turn during the learning phase of
// maze solving.  It uses the variables found_left, found_straight, and
// found_right, which indicate whether there is an exit in each of the
// three directions, applying the "left hand on the wall" strategy.
char selectTurn(unsigned char found_left, unsigned char found_straight,
  unsigned char found_right)
{
  // Make a decision about how to turn.  The following code
  // implements a left-hand-on-the-wall strategy, where we always
  // turn as far to the left as possible.
  if(found_left)
    return 'L';
  else if(found_straight)
    return 'S';
  else if(found_right)
    return 'R';
  else
    return 'B';
}

// All white right after a turn is not a dead end: a real dead end is at the
// end of a segment. On 2026-10-23, with fresh batteries, the robot lost the
// line straight after the same left turn three laps running, called it a
// dead end, turned around and circled the maze without reaching FINISH.
const unsigned long kLostAfterTurnMs = 500;
const unsigned long kRefindReverseMs = 700;
const unsigned long kRefindTimeoutMs = 2100;

// Turns slowly toward the side where the line was last seen, then the other
// way, until a middle sensor is on it. Returns false if it never is.
bool findLineAgain() {
  unsigned int sensors[6];
  const unsigned long startedMs = millis();
  const bool firstWayLeft = reflectanceSensors.readLine(sensors) < 2500;
  while (millis() - startedMs < kRefindTimeoutMs) {
    const bool reversed = millis() - startedMs >= kRefindReverseMs;
    if (reversed != firstWayLeft) motors.setSpeeds(-kCenterTurnSpeed, kCenterTurnSpeed);
    else motors.setSpeeds(kCenterTurnSpeed, -kCenterTurnSpeed);
    reflectanceSensors.readLine(sensors);
    if (ABOVE_LINE(sensors[2]) || ABOVE_LINE(sensors[3])) return true;
    checkStopButton();
    if (stopped) return false;
  }
  motors.setSpeeds(0, 0);
  return false;
}

// The maze is broken down into segments. Once the Zumo decides
// which segment to turn on, it will navigate until it finds another
// intersection. followSegment() will then return after the
// intersection is found.
void followSegment()
{
  unsigned int position;
  unsigned int sensors[6];
  int offset_from_center;
  int power_difference;

  if (stopped) return;
  const unsigned long segmentStartedMs = millis();
  while(1)
  {
    // Get the position of the line.
    position = reflectanceSensors.readLine(sensors);

    // The offset_from_center should be 0 when we are on the line.
    offset_from_center = ((int)position) - 2500;

    // Compute the difference between the two motor power settings,
    // m1 - m2.  If this is a positive number the robot will turn
    // to the left.  If it is a negative number, the robot will
    // turn to the right, and the magnitude of the number determines
    // the sharpness of the turn.
    power_difference = offset_from_center / 3;

    // Compute the actual motor settings.  We never set either motor
    // to a negative value.
    if(power_difference > SPEED)
      power_difference = SPEED;
    if(power_difference < -SPEED)
      power_difference = -SPEED;

    if(power_difference < 0)
      motors.setSpeeds(SPEED + power_difference, SPEED);
    else
      motors.setSpeeds(SPEED, SPEED - power_difference);

    // Motors are driving forward again: safe to spend time on the record.
    flushJunction();
    checkStopButton();
    if (stopped) return;

    // We use the inner four sensors (1, 2, 3, and 4) for
    // determining whether there is a line straight ahead, and the
    // sensors 0 and 5 for detecting lines going to the left and
    // right.

    if(!ABOVE_LINE(sensors[0]) && !ABOVE_LINE(sensors[1]) && !ABOVE_LINE(sensors[2]) && !ABOVE_LINE(sensors[3]) && !ABOVE_LINE(sensors[4]) && !ABOVE_LINE(sensors[5]))
    {
      // There is no line visible ahead, and we didn't see any
      // intersection.  Must be a dead end, unless the robot has only just
      // left a turn (see kLostAfterTurnMs).
      if (millis() - segmentStartedMs < kLostAfterTurnMs) {
        Serial.println(F("MAZE lost the line just after a turn; searching"));
        if (findLineAgain()) {
          recordEvent('G', junctionCount, 1);
          continue;
        }
        recordEvent('G', junctionCount, 0);
        if (stopped) return;
      }
      return;
    }
    else if(ABOVE_LINE(sensors[0]) || ABOVE_LINE(sensors[5]))
    {
      // Found an intersection.
      return;
    }

  }
}

// simplifyPath analyzes the path[] array and reduces all the
// turns. For example: Right turn + Right turn = (1) Back turn.
void simplifyPath()
{

  // only simplify the path if the second-to-last turn was a 'B'
  if(path_length < 3 || path[path_length - 2] != 'B')
  return;

  int total_angle = 0;
  int i;

  for(i = 1; i <= 3; i++)
  {
    switch(path[path_length - i])
    {
      case 'R':
        total_angle += 90;
        break;
      case 'L':
        total_angle += 270;
        break;
      case 'B':
        total_angle += 180;
        break;
    }
  }

  // Get the angle as a number between 0 and 360 degrees.
  total_angle = total_angle % 360;

  // Replace all of those turns with a single one.
  switch(total_angle)
  {
    case 0:
      path[path_length - 3] = 'S';
      break;
    case 90:
      path[path_length - 3] = 'R';
      break;
    case 180:
      path[path_length - 3] = 'B';
      break;
    case 270:
      path[path_length - 3] = 'L';
      break;
  }

  // The path is now two steps shorter.
  path_length -= 2;
}

// An outer sensor can touch black when the robot wobbles onto the edge of
// the tape or a corner of light tape. Examined, that "junction" shows only a
// straight exit. Recorded in round one but not triggered in round two (or
// the other way round), it shifted every later turn: on 2026-10-23 round two
// turned right where there was no line at move 29. Neither round counts it.
const uint8_t kStraightOnly = 2;

// Drives CHECK_DISTANCE through a junction, reading the sensors at its
// first edge, halfway and at the end (Pololu's MazeSolver sequence), and
// returns the exits seen: 1 left, 2 straight, 4 right. finish is set when
// all four middle sensors are still black at the end.
uint8_t examineJunction(bool& finish)
{
    // These variables record whether the robot has seen a line to the
    // left, straight ahead, and right, while examining the current
    // intersection.
    unsigned char found_left = 0;
    unsigned char found_straight = 0;
    unsigned char found_right = 0;

    // Now read the sensors and check the intersection type.
    unsigned int sensors[6];
    reflectanceSensors.readLine(sensors);

    // Check for left and right exits.
    if(ABOVE_LINE(sensors[0]))
        found_left = 1;
    if(ABOVE_LINE(sensors[5]))
        found_right = 1;

    // Drive straight a bit more, until we are
    // approximately in the middle of intersection.
    // This should help us better detect if we
    // have left or right segments.
    motors.setSpeeds(SPEED, SPEED);
    delay(OVERSHOOT(CHECK_DISTANCE)/2);

    reflectanceSensors.readLine(sensors);

    // Check for left and right exits.
    if(ABOVE_LINE(sensors[0]))
        found_left = 1;
    if(ABOVE_LINE(sensors[5]))
        found_right = 1;

    // After driving a little further, we
    // should have passed the intersection
    // and can check to see if we've hit the
    // finish line or if there is a straight segment
    // ahead.
    delay(OVERSHOOT(CHECK_DISTANCE)/2);

    // Check for a straight exit.
    reflectanceSensors.readLine(sensors);

    // Check again to see if left or right segment has been found
    if(ABOVE_LINE(sensors[0]))
        found_left = 1;
    if(ABOVE_LINE(sensors[5]))
        found_right = 1;

    if(ABOVE_LINE(sensors[1]) || ABOVE_LINE(sensors[2]) || ABOVE_LINE(sensors[3]) || ABOVE_LINE(sensors[4]))
        found_straight = 1;

    for (uint8_t i = 0; i < 6; ++i) checkSensors[i] = sensors[i];
    const uint8_t exits = (found_left ? 1 : 0) | (found_straight ? 2 : 0) |
                          (found_right ? 4 : 0);

    finish = ABOVE_LINE(sensors[1]) && ABOVE_LINE(sensors[2]) &&
             ABOVE_LINE(sensors[3]) && ABOVE_LINE(sensors[4]);
    return exits;
}

// The solveMaze() function works by applying a "left hand on the wall" strategy:
// the robot follows a segment until it reaches an intersection, where it takes the
// leftmost fork available to it. It records each turn it makes, and as long as the
// maze has no loops, this strategy will eventually lead it to the finish. Afterwards,
// the recorded path is simplified by removing dead ends. More information can be
// found in the 3pi maze solving example.
// Returns true when the finish was reached.
bool solveMaze()
{
    while(1)
    {
        // Navigate current line segment
        followSegment();
        if (stopped) return false;

        bool finish = false;
        const uint8_t exits = examineJunction(finish);
        const unsigned char found_left = exits & 1;
        const unsigned char found_straight = exits & 2;
        const unsigned char found_right = exits & 4;

        // Check for the ending spot (all four middle sensors black).
        if (finish)
        {
          motors.setSpeeds(0,0);
          noteJunction(exits, 'F');
          return true;
        }

        if (exits == kStraightOnly) {
          // Not a decision point; keep it out of the path (see kStraightOnly).
          Serial.println(F("MAZE ignored: only a straight exit"));
          continue;
        }

        // Intersection identification is complete.
        unsigned char dir = selectTurn(found_left, found_straight, found_right);

        // Make the turn indicated by the path.
    turn(dir);
        if (stopped) return false;

        if (path_length >= kMaxPath) {
          stopRun(OUTCOME_PATH_TOO_LONG);
          return false;
        }
        // Store the intersection in the path variable.
        path[path_length] = dir;
        path_length++;
        noteJunction(exits, static_cast<char>(dir));

        // Simplify the learned path.
        simplifyPath();

    }
}

uint8_t exitBitForTurn(char turnTaken) {
  if (turnTaken == 'L') return 1;
  if (turnTaken == 'S') return 2;
  if (turnTaken == 'R') return 4;
  return 0;
}

// Drives the saved path to the finish. Pololu's version only counted
// junctions; on 2026-10-23 a miscount put a right turn where there was no
// line at move 29 and the robot spun out. Now each junction is examined as
// in round one: the planned turn is taken only if that exit is there.
// Otherwise the robot is lost, so it explores with the left-hand rule from
// here to still reach FINISH; the saved path in EEPROM is not changed.
// Returns true at FINISH; sets usedFallback when it had to explore.
bool goToFinishLine(bool& usedFallback)
{
  usedFallback = false;
  int i = 0;

  // Turn around if the Zumo is facing the wrong direction.
  if(path[0] == 'B')
  {
    turn('B');
    i++;
  }

  while (i < path_length)
  {
    followSegment();
    if (stopped) return false;

    bool finish = false;
    const uint8_t exits = examineJunction(finish);
    if (finish) {
      motors.setSpeeds(0, 0);
      noteJunction(exits, 'F');
      return true;
    }
    if (exits == kStraightOnly) {
      Serial.println(F("MAZE ignored: only a straight exit"));
      continue;
    }

    const char planned = path[i];
    if (planned == 'B' || (exits & exitBitForTurn(planned)) != 0) {
      turn(planned);
      if (stopped) return false;
      noteJunction(exits, planned);
      ++i;
      continue;
    }

    // Lost: record where, then explore from this junction.
    usedFallback = true;
    flushJunction();
    recordEvent('X', static_cast<uint8_t>(i + 1), 0);
    recordEvent('Y', static_cast<uint8_t>(planned), exits);
    Serial.print(F("MAZE lost the saved path at move "));
    Serial.print(i + 1);
    Serial.print(F(": planned "));
    Serial.print(planned);
    Serial.print(F(" but exits="));
    printExits(exits);
    Serial.println(F(". Exploring to FINISH."));
    const char dir = selectTurn(exits & 1, exits & 2, exits & 4);
    turn(dir);
    if (stopped) return false;
    noteJunction(exits, dir);
    path_length = 0;
    const bool reached = solveMaze();
    loadPath();  // the RAM path was used for exploring; restore the saved one
    return reached;
  }

  // Follow the last segment up to the finish.
  followSegment();
  if (stopped) return false;

  // The finish line has been reached.
  motors.setSpeeds(0,0);
  return true;
}

// Calibration at START: Pololu's setup() sweep, repeated until every sensor
// has seen black, then centering on the line.
bool centerOnLine();
void recordCalibration();

// Raw sensor readings (microseconds, 0..2000) on the competition maze:
// white and light tape about 200-330, black 1400-2000.
const unsigned int kRawBlack = 800;
// A calibration is usable only if every sensor saw real black and a clear
// difference from white. On 2026-10-23 a sweep stopped early with sensors
// 0-3 at max 460-676: light tape and white then read as black and round one
// turned left at every "junction". 1000 also rejected a usable sweep whose
// weakest sensor reached 848, so the bar sits between bad (up to 676) and
// good (848 and up).
const unsigned int kCalMinBlackRaw = 700;
const unsigned int kCalMinSpan = 400;
const uint8_t kCalibrationAttempts = 3;

bool calibrationUsable() {
  for (uint8_t i = 0; i < 6; ++i) {
    const unsigned int low = reflectanceSensors.calibratedMinimumOn[i];
    const unsigned int high = reflectanceSensors.calibratedMaximumOn[i];
    if (high < kCalMinBlackRaw || high < low + kCalMinSpan) return false;
  }
  return true;
}

// One Pololu sweep: rock left and right over the start line twice,
// calibrating as it goes. Pololu decided when each swing had crossed the
// line from the calibration being built, so early on, with a tiny range,
// sensor flicker counted as crossings and ended swings before the line had
// passed the other sensors. Crossings are now judged from raw readings.
void sweepOnce()
{
  unsigned int raw[6];
  unsigned short count = 0;
  unsigned short last_status = 0;
  int turn_direction = 1;

  reflectanceSensors.resetCalibration();
  // Calibrate the Zumo by sweeping it from left to right
  for(int i = 0; i < 4; i ++)
  {
    // Zumo will turn clockwise if turn_direction = 1.
    // If turn_direction = -1 Zumo will turn counter-clockwise.
    turn_direction *= -1;

    // Turn direction.
    motors.setSpeeds(turn_direction * SWEEP_SPEED, -1*turn_direction * SWEEP_SPEED);

    const unsigned long startedMs = millis();
    // This while loop monitors line position
    // until the turn is complete.
    while(count < 2)
    {
      reflectanceSensors.calibrate();
      noteSupply();  // the sweep's pivots draw the most current
      reflectanceSensors.read(raw);
      // Count each change of the watched outer sensor between white and
      // black: the right-most while turning left, the left-most while
      // turning right.
      const uint8_t watched = turn_direction < 0 ? 5 : 0;
      const unsigned short black = raw[watched] > kRawBlack ? 1 : 0;
      count += black ^ last_status;
      last_status = black;
      checkStopButton();
      if (millis() - startedMs >= kTurnTimeoutMs) {
        recordEvent('W', static_cast<uint8_t>('1' + i), static_cast<uint8_t>(count));
        stopRun(OUTCOME_TURN_TIMEOUT);
      }
      if (stopped) return;
    }

    count = 0;
    last_status = 0;
  }
  motors.setSpeeds(0, 0);
}

void calibrateOnStartLine()
{
  for (uint8_t attempt = 1; ; ++attempt) {
    sweepOnce();
    if (stopped) return;
    recordCalibration();
    if (calibrationUsable()) break;
    if (attempt >= kCalibrationAttempts) {
      stopRun(OUTCOME_CALIBRATION_FAILED);
      return;
    }
    Serial.println(F("CAL some sensors never saw black; rocking again"));
  }

  // Pololu ends the sweep with turn('L'), which waits for sensor 1 to cross
  // the line. On 2026-10-23 that spin twice saw no line at all for 3 s
  // (round two, with and without the cable) and the robot spun at START.
  // centerOnLine() does the same job with all six sensors and searches both
  // ways, so it replaces that spin.
  if (!centerOnLine() && !stopped) {
    stopRun(OUTCOME_NO_LINE_AT_START);
  }
}

// Prints each sensor's calibrated range and records the weakest one, so a
// sweep that never saw black or white on some sensor shows in the record.
void recordCalibration() {
  uint8_t weakest = 0;
  unsigned int weakestSpan = 0xFFFF;
  Serial.print(F("CAL min,max:"));
  for (uint8_t i = 0; i < 6; ++i) {
    const unsigned int low = reflectanceSensors.calibratedMinimumOn[i];
    const unsigned int high = reflectanceSensors.calibratedMaximumOn[i];
    const unsigned int span = high > low ? high - low : 0;
    if (span < weakestSpan) {
      weakestSpan = span;
      weakest = i;
    }
    Serial.print(' ');
    Serial.print(low);
    Serial.print(',');
    Serial.print(high);
  }
  Serial.println();
  recordEvent('K', weakest, static_cast<uint8_t>(min(weakestSpan / 8U, 255U)));
}

bool anyAboveLine(const unsigned int sensors[6]) {
  for (uint8_t i = 0; i < 6; ++i) {
    if (ABOVE_LINE(sensors[i])) return true;
  }
  return false;
}

uint8_t recordedPosition(unsigned int position, bool lineSeen) {
  return lineSeen ? static_cast<uint8_t>(position / 20) : 255;
}

// Pololu's sweep ends with a left spin that stops as sensor 1 crosses the
// line, and nothing checks that the line is centred. On 2026-10-23 the same
// start worked with the USB cable (its drag slows the spin) but, without it,
// ended off the line: the robot saw only white, took it for a dead end and
// turned around at START. Turn slowly until the line is under the middle
// sensors and stays there after a short stop.
bool centerOnLine() {
  unsigned int sensors[6];
  const unsigned long startedMs = millis();
  unsigned int position = reflectanceSensors.readLine(sensors);
  const uint8_t before = recordedPosition(position, anyAboveLine(sensors));
  Serial.print(F("CENTER after sweep line_position="));
  Serial.println(position);
  const bool firstWayLeft = position < 2500;
  bool everSeen = false;
  while (true) {
    const bool lineSeen = anyAboveLine(sensors);
    if (lineSeen) everSeen = true;
    if (lineSeen && position + kCenterTolerance >= 2500 &&
        position <= 2500 + kCenterTolerance) {
      motors.setSpeeds(0, 0);
      delay(kCenterSettleMs);
      position = reflectanceSensors.readLine(sensors);
      if (anyAboveLine(sensors) && position + kCenterTolerance >= 2500 &&
          position <= 2500 + kCenterTolerance) {
        recordEvent('C', before, recordedPosition(position, true));
        Serial.print(F("CENTER done line_position="));
        Serial.println(position);
        return true;
      }
      continue;
    }
    // Once the line has been seen, readLine reports 0 or 5000 toward the side
    // it was last seen, so this turns back toward a lost line. Before that,
    // search one way, then the other.
    bool turnLeft = position < 2500;
    if (!everSeen) {
      const bool reversed = millis() - startedMs >= kCenterReverseMs;
      turnLeft = reversed ? !firstWayLeft : firstWayLeft;
    }
    if (turnLeft) motors.setSpeeds(-kCenterTurnSpeed, kCenterTurnSpeed);
    else motors.setSpeeds(kCenterTurnSpeed, -kCenterTurnSpeed);
    checkStopButton();
    if (stopped || millis() - startedMs >= kCenterTimeoutMs) {
      motors.setSpeeds(0, 0);
      recordEvent('C', before, 255);
      return false;
    }
    position = reflectanceSensors.readLine(sensors);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  pinMode(ZUMO_BUTTON, INPUT_PULLUP);
  reflectanceSensors.init();
  buzzer.play(">g32>>c32");
  Serial.println(F("ZUMO MAZE FIRMWARE (Pololu MazeSolver based)"));
  printPreviousRun();
  havePath = loadPath();
  if (havePath) printPath();
  else path_length = 0;
  readSpeedKnob();
  showReady();
}

void loop()
{
  waitForStart();
  const bool replay = havePath;
  stopped = false;
  stopOutcome = 0;
  buttonWasUp = false;
  Serial.println(replay ? F("START round TWO") : F("START round ONE"));

  // Hands off, then the sweep. The light blinks until the robot moves.
  const unsigned long handsOffMs = millis();
  while (millis() - handsOffMs < kHandsOffMs) {
    digitalWrite(13, (millis() / 100U) % 2U);
  }
  digitalWrite(13, LOW);
  readSpeedKnob();
  startRecording(replay);
  calibrateOnStartLine();

  if (!stopped) {
    if (replay) {
      bool usedFallback = false;
      if (goToFinishLine(usedFallback) && !stopped) {
        recordOutcome(usedFallback ? OUTCOME_ROUND_TWO_FINISH_EXPLORING
                                   : OUTCOME_ROUND_TWO_FINISH);
        buzzer.play(">>a32");
      }
    } else {
      path_length = 0;
      if (solveMaze()) {
        printPath();
        if (savePath()) {
          havePath = true;
          recordOutcome(OUTCOME_ROUND_ONE_FINISH);
        } else {
          havePath = false;
          recordOutcome(OUTCOME_SAVE_FAILED);
        }
        buzzer.play(">>a32");
      }
    }
  }
  motors.setSpeeds(0, 0);
  if (stopped) recordOutcome(stopOutcome);

  // Wait for the stop or start press to be released before READY.
  while (buttonDown()) {}
  delay(kButtonDebounceMs);
  showReady();
}

#endif
